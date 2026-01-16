#include "../core/time.hpp"
#include "../core/types.hpp"
#include "../data/market_data_types.hpp"
#include "../models/stat_arb.hpp"


#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>


using namespace hft;

// -------------------------------------------------------------------------
// Simulation Config
// -------------------------------------------------------------------------
struct SimConfig {
  double initial_price = 100.0;
  double volatility = 0.5;     // Sigma
  double mean_reversion = 0.1; // Theta (speed of reversion)
  double long_term_mean = 100.0;
  int steps = 10000;
  double dt = 1.0; // Time step
};

// -------------------------------------------------------------------------
// Portfolio Tracker
// -------------------------------------------------------------------------
class Portfolio {
public:
  void fill(core::Side side, core::Price price, core::Quantity qty) {
    double px = price.to_float();
    double q = qty.to_float();

    if (side == core::Side::Buy) {
      position_ += q;
      cash_ -= px * q;
      fees_ += px * q * 0.0001; // 1 bps fee
    } else {
      position_ -= q;
      cash_ += px * q;
      fees_ += px * q * 0.0001;
    }
  }

  double get_equity(double current_price) const {
    return cash_ +
           (position_ * current_price); // - fees_; (cash already includes fees
                                        // if we subtract them)
  }

  double position() const { return position_; }
  double cash() const { return cash_; }

private:
  double cash_ = 10000.0; // Start with $10k
  double position_ = 0.0;
  double fees_ = 0.0;
};

// -------------------------------------------------------------------------
// Ornstein-Uhlenbeck Process Generator (Mean Reverting)
// dP = theta * (mu - P) * dt + sigma * dW
// -------------------------------------------------------------------------
class MarketSimulator {
public:
  MarketSimulator(SimConfig config)
      : config_(config), current_price_(config.initial_price) {
    std::random_device rd;
    gen_.seed(rd());
    dist_ = std::normal_distribution<>(0.0, 1.0);
  }

  data::TradeUpdate next_step() {
    // OU Process
    double dw = dist_(gen_) * std::sqrt(config_.dt);
    double dp = config_.mean_reversion *
                    (config_.long_term_mean - current_price_) * config_.dt +
                config_.volatility * dw;

    current_price_ += dp;

    // Ensure positive price
    if (current_price_ < 0.01)
      current_price_ = 0.01;

    data::TradeUpdate trade;
    trade.header.symbol_id = 1;
    trade.header.exchange_ts = core::now_nanos(); // Fake TS
    trade.header.type = data::UpdateType::Trade;
    trade.price = core::Price::from_float(current_price_);
    trade.qty = core::Quantity::from_float(1.0); // Dummy volume

    return trade;
  }

private:
  SimConfig config_;
  double current_price_;
  std::mt19937 gen_;
  std::normal_distribution<> dist_;
};

// -------------------------------------------------------------------------
// Main Backtest Loop
// -------------------------------------------------------------------------
int main(int argc, char **argv) {
  std::cout << "=== HFT Backtester ===\n";
  std::cout << "Strategy: Mean Reversion (StatArb)\n";
  std::cout << "Market: Ornstein-Uhlenbeck Process (Theta=0.1, Vol=0.5)\n\n";

  // 1. Setup
  SimConfig config;
  config.steps = 5000;

  MarketSimulator market(config);
  Portfolio portfolio;

  // Strategy: ID=1, Threshold=0.05 (trade when deviation > 0.05)
  // Using a tighter threshold because our volatility is low-ish
  models::StatArbStrategy strategy(1, 0.5);

  // Output file
  std::ofstream out("equity_curve.csv");
  out << "step,price,inventory,equity\n";

  // 2. Simulation Loop
  int trades_count = 0;
  for (int t = 0; t < config.steps; ++t) {
    // Generate Market Data
    auto trade = market.next_step();
    double px_float = trade.price.to_float();

    // Feed to Strategy
    auto signal = strategy.on_trade(trade);

    // Execution (Instant Fill Assumption)
    if (signal.should_trade) {
      // Check risk/inventory limits (simple clip)
      double current_pos = portfolio.position();
      if (std::abs(current_pos) < 5.0) { // Max position 5 lots
        portfolio.fill(signal.side, signal.price, signal.qty);
        trades_count++;
      }
    }

    // Log
    double equity = portfolio.get_equity(px_float);
    out << t << "," << px_float << "," << portfolio.position() << "," << equity
        << "\n";
  }

  std::cout << "Simulation Complete.\n";
  std::cout << "Trades Executed: " << trades_count << "\n";
  std::cout << "Final Equity: $" << std::fixed << std::setprecision(2)
            << portfolio.get_equity(100.0) << "\n";
  std::cout << "Data exported to equity_curve.csv\n";

  return 0;
}
