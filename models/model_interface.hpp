#pragma once

#include "../core/types.hpp"

namespace hft {
namespace models {

struct Signal {
  bool should_trade = false;
  core::SymbolId symbol_id = 0;
  core::Side side = core::Side::None;
  core::Price price = core::Price(0);
  core::Quantity qty = core::Quantity(0);
  core::Price ref_price = core::Price(0);
};

// Concept/Interface for strategies
// We strictly use Templates in the Engine to avoid virtual dispatch,
// but this serves as the "Concept" documentation.
/*
class StrategyConcept {
public:
     Signal on_trade(const data::TradeUpdate& trade);
     Signal on_book_update(const data::LevelUpdate& update);
};
*/

} // namespace models
} // namespace hft
