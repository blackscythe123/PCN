# Observation — Assignment 11 (Wi-Fi vs Bluetooth vs Zigbee, mobility)

## Aim
Simulate a heterogeneous wireless network (Wi-Fi, Bluetooth, Zigbee) in
NS-3 and analyze how increasing node mobility (1/5/10 m/s) affects signal
strength, throughput, packet loss, and end-to-end delay for each technology.

## Algorithm
1. Create a fixed gateway/coordinator node and a mobile node, each with an
   ad-hoc `WifiNetDevice` configured per `--tech` (Tx power/rate/packet size
   set to match Wi-Fi, Bluetooth, or Zigbee's real characteristics, since
   NS-3 has no Bluetooth module and full 802.15.4+6LoWPAN Zigbee was out of
   scope).
2. Place the gateway at the origin; give the mobile node a
   `ConstantVelocityMobilityModel` starting 2 m away, moving outward at
   `--speed` m/s.
3. Install the Internet stack and start a UDP CBR flow from the mobile node
   to a `PacketSink` on the gateway.
4. Trace the gateway's `WifiPhy` `MonitorSnifferRx` source to record the
   received signal strength (dBm) of every successfully received frame.
5. Install `FlowMonitor`; run for 30 s; compute throughput, packet loss
   (Tx-Rx directly, so it stays correct even with zero reception), average
   delay, and average signal strength.

## Test Cases and Sample Output
| Test case (tech / speed) | Signal | Throughput | Loss | Delay |
|---|---|---|---|---|
| Wi-Fi / 1 m/s     | -61.96 dBm | 205.47 Kbps | 0.00%   | 0.247 ms |
| Wi-Fi / 10 m/s    | -69.98 dBm | 25.54 Kbps  | 87.57%  | 0.436 ms |
| Bluetooth / 10 m/s| -78.78 dBm | 6.60 Kbps   | 97.10%  | 2.423 ms |
| Zigbee / 5 m/s    | no link    | 0.00 Kbps   | 100.00% | -- |

Sample console output:
```
===== tech=Zigbee speed=1m/s =====
Tx=7249 Rx=995 | Throughput=35.1338Kbps | PacketLoss=86.274% | AvgDelay=1.56718ms | AvgSignal=-77.2507dBm
```

## Learning Outcomes
- All four metrics degrade monotonically with speed for every technology,
  since higher speed means more distance covered from the gateway.
- Range/robustness ranking follows transmit power exactly: Wi-Fi > Bluetooth
  > Zigbee, with Zigbee's link fully dropping out by 5 m/s.
- Delay stays roughly flat with speed — mobility mainly determines whether a
  packet arrives at all, not how long a delivered packet takes.
