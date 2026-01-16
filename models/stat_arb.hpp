#pragma once

#include "../core/logger.hpp"
#include "../data/market_data_types.hpp"
#include "../features/feature_calculator.hpp"
#include "model_interface.hpp"

namespace hft::models {

class StatArbStrategy {
public:
  StatArbStrategy(core::SymbolId target_id, double entry_threshold)
      : target_id_(target_id), threshold_(entry_threshold), price_ewma_(0.1) {}

  Signal on_trade(const data::TradeUpdate &trade) {
    Signal s;

    // Only care about our symbol
    if (trade.header.symbol_id != target_id_)
      return s;

    double px = trade.price.to_float();
    price_ewma_.update(px);

    double fairness = price_ewma_.value();
    double deviation = px - fairness;

    // Simple Mean Reversion logic
    if (deviation > threshold_) {
      // Price is too high -> Sell
      s.should_trade = true;
      s.symbol_id = target_id_;
      s.side = core::Side::Sell;
      s.price = trade.price;                    // Aggressive
      s.qty = core::Quantity::from_float(0.01); // Small clip
      s.ref_price = core::Price::from_float(fairness);
    } else if (deviation < -threshold_) {
      // Price is too low -> Buy
      s.should_trade = true;
      s.symbol_id = target_id_;
      s.side = core::Side::Buy;
      s.price = trade.price;
      s.qty = core::Quantity::from_float(0.01);
      s.ref_price = core::Price::from_float(fairness);
    }

    return s;
  }

private:
  core::SymbolId target_id_;
  double threshold_;
  features::EWMA price_ewma_;
};

} // namespace hft::models
