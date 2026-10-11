# Assignment 11 — Heterogeneous Wireless Network (Wi-Fi / Bluetooth / Zigbee) in NS-3

## Scenario

One mobile node sends UDP CBR traffic to a fixed gateway/coordinator node
while moving directly away from it at a constant speed (`--speed`, tested at
1, 5, 10 m/s). One program, `ass11.cc`, reproduces every technology and
speed combination via `--tech=WiFi|Bluetooth|Zigbee` and `--speed`.

## Why Wi-Fi/Bluetooth/Zigbee are all built on ns-3's WifiNetDevice

NS-3 has no Bluetooth module at all, and modelling Zigbee "properly" needs
its 802.15.4 PHY/MAC (`lr-wpan` module) plus 6LoWPAN just to get IP on top of
it — a second, incompatible code path just to compare one technology against
the others. Instead, all three are modelled as ad-hoc `WifiNetDevice`s, each
configured with the transmit power, PHY rate, and packet size that actually
characterise the real technology:

| Technology | Tx Power | PHY rate    | Packet size | Rationale |
|---|---|---|---|---|
| Wi-Fi      | 20 dBm  | 54 Mbps (802.11g) | 1024 B | typical AP-class Wi-Fi |
| Bluetooth  | 4 dBm   | 1 Mbps (802.11b DSSS) | 200 B  | ~Bluetooth Class 2 power/range |
| Zigbee     | -10 dBm | 1 Mbps (802.11b DSSS) | 100 B  | 802.15.4's very low Tx power & small MTU |

This is a documented simplification, not a claim of protocol-exact
Bluetooth/Zigbee behaviour — but it keeps one uniform, simple simulation
pipeline (WifiHelper + mobility + FlowMonitor) while still producing
believable, technology-appropriate range/throughput differences.

## Mobility model

A constant-velocity, straight-line departure (`ConstantVelocityMobilityModel`)
rather than `RandomWaypoint` was used deliberately: RandomWaypoint's outcome
for a *single* run at a *given* speed depends heavily on where randomness
happens to place the node, which produced inconsistent, non-monotonic results
in testing. Moving directly away from the gateway at a fixed speed makes
"higher speed -> farther away, sooner" deterministic, which is what the
assignment is actually asking to observe.

## Files

```
ass11/
├── Assign11_PCN.pdf              the assignment spec
├── report.tex / report.pdf       the lab report
├── observation.md                short summary of findings
├── ns-3-work/scratch/ass11.cc    the simulation
├── plot.py                       renders the 4-panel comparison graph
├── output.txt                    captured console output, all 9 runs
└── screenshots/
    └── mobility_comparison.png   signal/throughput/loss/delay vs speed
```

## How to run

```bash
docker start pcn_ns3
docker cp ass11/ns-3-work/scratch/ass11.cc pcn_ns3:/root/ns-3-dev/scratch/ass11.cc
docker exec -it pcn_ns3 bash
cd /root/ns-3-dev
./ns3 build
for t in WiFi Bluetooth Zigbee; do
  for s in 1 5 10; do
    ./ns3 run "scratch/ass11 --tech=$t --speed=$s"
  done
done
python3 plot.py   # needs python3-matplotlib
```
