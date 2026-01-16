#include <chrono>
#include <iostream>
#include <thread>
#include <vector>


#include "../core/config.hpp"
#include "../core/logger.hpp"
#include "../data/feeder_interface.hpp"
#include "../execution/execution_engine.hpp"
#include "../models/stat_arb.hpp"


using namespace hft;

// Mock Gateway that just logs
class MockGateway : public execution::OrderGatewayInterface {
public:
  void send_order(const execution::OrderCommand &cmd) override {
    // In real world: Write to Ringbuffer -> Network Thread
    // Here we just pretend
  }
  void cancel_order(core::OrderId oid, core::SymbolId sid) override {}
};

// Mock Feeder that generates random trades
class MockFeeder : public data::FeederInterface {
public:
  void key_start() override {
    worker_ = std::thread([this]() {
      core::SymbolId sym_id = 1;
      double price = 50000.0; // BTC-ish
      while (running_) {
        data::TradeUpdate t;
        t.header.symbol_id = sym_id;
        t.header.exchange_ts = core::now_nanos();
        t.header.local_ts = core::now_nanos();
        t.header.type = data::UpdateType::Trade;

        // Random walk
        price += (rand() % 100 - 50) * 0.01;

        t.price = core::Price::from_float(price);
        t.qty = core::Quantity::from_float(0.1);
        t.side = (rand() % 2 == 0) ? core::Side::Buy : core::Side::Sell;

        if (on_trade_)
          on_trade_(t);

        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    });
  }

  void stop() override {
    running_ = false;
    if (worker_.joinable())
      worker_.join();
  }

private:
  std::thread worker_;
  bool running_ = true;
};

int main(int argc, char **argv) {
  // 1. Init Logger
  core::Logger::instance().init("hft_engine.log");
  LOG_INFO("Starting HFT Engine (TXSE/BTC Edition)...");

  // 2. Setup Config
  execution::RiskConfig risk_cfg;
  risk_cfg.max_order_qty = core::Quantity::from_float(1.0);
  risk_cfg.max_price_deviation = core::Price::from_float(1000.0);

  // 3. Components
  core::SymbolId trade_sym = 1;
  MockGateway gateway;
  models::StatArbStrategy strategy(trade_sym, 1.5);

  // 4. Engine
  execution::ExecutionEngine<models::StatArbStrategy, MockGateway> engine(
      strategy, gateway, risk_cfg);

  // 5. Data Feed
  MockFeeder feeder;
  feeder.set_on_trade([&](const data::TradeUpdate &t) { engine.on_trade(t); });

  LOG_INFO("Engine Initialized. Starting Feed...");
  feeder.key_start();

  // 6. Keep alive
  std::cout << "Press Enter to stop..." << std::endl;
  std::cin.ignore();

  LOG_INFO("Stopping...");
  feeder.stop();
  core::Logger::instance().stop();

  return 0;
}
