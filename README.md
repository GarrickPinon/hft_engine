# HFT Engine (TXSE / BTC / Stablecoin Edition)

A production-grade, ultra-low-latency High-Frequency Trading (HFT) system designed for:
- **Equities**: TXSE (Texas Stock Exchange), NASDAQ, NYSE.
- **Crypto**: Bitcoin (BTC) Native Execution, Stablecoins (USDC/USDT).
- **Latency Target**: < 4 microseconds wire-to-wire.

## Architecture Architecture

### Philosophy
- **Backend Sovereignty**: No hidden state. Explicit contracts.
- **Agentic Determinism**: Config-driven behavior.
- **Quant-Native Rigor**: Optimization-aware design.
- **Low-Latency**: C++20 Hot Path, Python 3.11 Orchestration.

### Directory Structure
- `/core`: Shared high-performance utilities (RingBuffers, Logger, Allocators).
- `/execution`: The Hot Path (Order Management, Risk, Gateway).
- `/data`: Market Data Ingestion & Normalization.
- `/features`: Microstructure feature generation.
- `/models`: Signal generation (Stat-Arb, ML).
- `/risk`: Pre-trade risk & Kill-switches.
- `/infra`: Docker, Scripts, CI/CD.

## Setup

### Prerequisites
- C++20 Compiler (GCC 11+ or Clang 14+)
- CMake 3.20+
- Python 3.11
- Linux Kernel 5.15+ (Low-latency tuned)

### Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Running
```bash
# Run the trading engine
./bin/hft_engine --config ../configs/prod.yaml
```

## Latency Tuning
- ISOLCPUS: Pin core 2-6 (isolcpus=2-6).
- HugePages: Enable 1GB hugepages.
- Network: Solarflare Onload or DPDK recommended for prod.

## Disclaimer
This system is provided as-is. Verify all risk controls before live deployment.
