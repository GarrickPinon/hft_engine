#include "../core/benchmark.hpp"
#include "../core/time.hpp"
#include "../core/types.hpp"
#include "../data/market_data_types.hpp"
#include "../models/stat_arb.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


using namespace hft;

// Simulate the hot path with realistic operations
void simulate_hot_path(core::LatencyTracker<100000> &tracker, int iterations) {
  // Setup
  core::SymbolId sym_id = 1;
  models::StatArbStrategy strategy(sym_id, 1.5);
  double price = 50000.0;

  for (int i = 0; i < iterations; ++i) {
    // Start measurement
    auto start = core::now_nanos();

    // Simulate market data update
    data::TradeUpdate trade;
    trade.header.symbol_id = sym_id;
    trade.header.exchange_ts = core::now_nanos();
    trade.header.local_ts = core::now_nanos();
    trade.header.type = data::UpdateType::Trade;

    // Random walk price
    price += (rand() % 100 - 50) * 0.01;
    trade.price = core::Price::from_float(price);
    trade.qty = core::Quantity::from_float(0.1);
    trade.side = (rand() % 2 == 0) ? core::Side::Buy : core::Side::Sell;

    // Process through strategy (the hot path we're measuring)
    auto signal = strategy.on_trade(trade);

    // End measurement
    auto end = core::now_nanos();
    tracker.record(end - start);
  }
}

void print_usage(const char *prog) {
  std::cout << "Usage: " << prog << " [options]\n"
            << "Options:\n"
            << "  --iterations N    Number of iterations (default: 100000)\n"
            << "  --output FILE     Output JSON file (default: latency.json)\n"
            << "  --warmup N        Warmup iterations (default: 1000)\n"
            << "  --help           Show this help\n";
}

int main(int argc, char **argv) {
  int iterations = 100000;
  int warmup = 1000;
  std::string output = "latency.json";

  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--iterations" && i + 1 < argc) {
      iterations = std::atoi(argv[++i]);
    } else if (arg == "--output" && i + 1 < argc) {
      output = argv[++i];
    } else if (arg == "--warmup" && i + 1 < argc) {
      warmup = std::atoi(argv[++i]);
    } else if (arg == "--help") {
      print_usage(argv[0]);
      return 0;
    }
  }

  std::cout << "=== HFT Engine Latency Benchmark ===\n";
  std::cout << "Warmup iterations: " << warmup << "\n";
  std::cout << "Benchmark iterations: " << iterations << "\n\n";

  // Warmup phase (Heap allocated to prevent stack overflow)
  auto warmup_tracker = std::make_unique<core::LatencyTracker<100000>>();
  std::cout << "Running warmup..." << std::flush;
  simulate_hot_path(*warmup_tracker, warmup);
  std::cout << " done.\n";

  // Benchmark phase (Heap allocated)
  auto tracker = std::make_unique<core::LatencyTracker<100000>>();
  std::cout << "Running benchmark..." << std::flush;

  auto bench_start = std::chrono::high_resolution_clock::now();
  simulate_hot_path(*tracker, iterations);
  auto bench_end = std::chrono::high_resolution_clock::now();

  std::cout << " done.\n\n";

  // Calculate wall time
  auto wall_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          bench_end - bench_start)
                          .count();

  // Print results
  const auto &hist = tracker->histogram();

  std::cout << "=== Results ===\n";
  std::cout << "Total samples:   " << hist.count() << "\n";
  std::cout << "Wall clock time: " << wall_time_ms << " ms\n";
  std::cout << "Throughput:      " << (iterations * 1000.0 / wall_time_ms)
            << " ops/sec\n\n";

  std::cout << "Latency Statistics:\n";
  std::cout << "  Min:    " << hist.min_latency() << " ns\n";
  std::cout << "  Max:    " << hist.max_latency() << " ns\n";
  std::cout << "  Mean:   " << hist.mean() << " ns\n";
  std::cout << "  P50:    " << tracker->p50() << " ns\n";
  std::cout << "  P95:    " << tracker->p95() << " ns\n";
  std::cout << "  P99:    " << tracker->p99() << " ns\n";
  std::cout << "  P99.9:  " << tracker->p999() << " ns\n\n";

  std::cout << "Histogram:\n";
  for (std::size_t i = 0; i < core::NUM_BUCKETS; ++i) {
    std::int64_t count = hist.bucket_count(i);
    double pct = 100.0 * count / hist.count();
    std::cout << "  " << core::BUCKET_NAMES[i] << ": " << count << " (" << pct
              << "%)\n";
  }

  // Export JSON
  tracker->export_json(output);
  std::cout << "\nResults exported to: " << output << "\n";

  return 0;
}
