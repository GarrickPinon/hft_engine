#!/bin/bash
# HFT Environment Setup Script meant for bare-metal Linux

echo "Setting up Low Latency Environment..."

# 1. CPU Scaling Governor -> Performance
if [ -d "/sys/devices/system/cpu/cpu0/cpufreq" ]; then
    for governor in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
        echo performance > $governor
    done
fi

# 2. Disable IRQ Balancing (Mock command, requires irqbalance package)
# service irqbalance stop

# 3. Network Tuning
sysctl -w net.core.rmem_max=16777216
sysctl -w net.core.wmem_max=16777216
sysctl -w net.ipv4.tcp_low_latency=1

echo "Done. Please restart your application."
