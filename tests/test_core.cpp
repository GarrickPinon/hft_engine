
#include "../core/ring_buffer.hpp"
#include "../core/types.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("RingBuffer basic operations", "[core]") {
  hft::core::SPSCRingBuffer<int, 4> rb;
  int val = 0;

  REQUIRE(rb.push(1));
  REQUIRE(rb.push(2));
  REQUIRE(rb.push(3));

  REQUIRE(rb.pop(val));
  REQUIRE(val == 1);

  REQUIRE(rb.pop(val));
  REQUIRE(val == 2);
}

TEST_CASE("Price Fixed Point Math", "[core]") {
  using hft::core::Price;

  Price p1 = Price::from_float(100.0);
  Price p2 = Price::from_float(50.0);

  REQUIRE((p1 + p2).to_float() == 150.0);
  REQUIRE((p1 - p2).to_float() == 50.0);

  // Bitcoin precision check
  Price sat = Price::from_float(0.00000001);
  REQUIRE(sat.ticks == 1);
}
