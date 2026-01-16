# Build Stage
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Runtime Stage
FROM ubuntu:22.04

# Install basic runtime libs
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/hft_engine .
COPY --from=builder /app/configs ./configs

# Latency tuning settings (Note: Needs privileged mode to actual apply)
ENV OMP_PROC_BIND=true
ENV OMP_PLACES=cores

ENTRYPOINT ["./hft_engine"]
