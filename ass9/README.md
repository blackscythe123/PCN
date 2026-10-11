# Assignment 9 — TCP Congestion Control (Reno / Cubic / BBR) in NS-3

## Scenario

```
Vehicle0 --|
Vehicle1 --|
Vehicle2 --|---- Roadside Router ====Internet(bottleneck)==== Traffic Control Server
Vehicle3 --|
Vehicle4 --|
```

5 vehicles each open one saturating TCP (BulkSend) flow to the server through
a shared roadside router and an 8 Mbps/20 ms bottleneck ("the Internet"). One
program, `ass9.cc`, reproduces all three required runs — the TCP congestion
control algorithm used by every flow is picked with `--tcpVariant` (`Reno`,
`Cubic`, or `Bbr`), so nothing is recompiled between runs.

## What is measured, and how

- **Throughput / delay / packet loss** per flow — from NS-3's `FlowMonitor`.
- **Congestion window over time** — a trace sink on
  `/NodeList/i/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow`, connected
  shortly after each flow starts, dumped to `cwnd-<variant>.dat`.
- **Fairness** — Jain's Fairness Index computed from the 5 flows' throughputs:
  `(Σx)² / (n · Σx²)`. A value of 1 means all flows got an equal share.

## Files

```
ass9/
├── Assign9_PCN.pdf              the assignment spec
├── report.tex / report.pdf      the lab report
├── observation.md               short summary of findings
├── ns-3-work/scratch/ass9.cc    the simulation
├── plot.py                      renders the two result graphs
├── output.txt                   captured console output, all 3 runs
└── screenshots/
    ├── cwnd_comparison.png      congestion window vs time, all 3 variants
    └── metrics_comparison.png   throughput/delay/loss/fairness bar charts
```

## How to run

Same persistent container as Assignments 7-8 (`pcn_ns3`, Ubuntu 24.04 + NS-3
3.42), now also built with the `wifi`, `aodv`, `mobility`, `lr-wpan`,
`internet-apps`, `csma` modules for Assignments 9-11:

```bash
docker start pcn_ns3
docker cp ass9/ns-3-work/scratch/ass9.cc pcn_ns3:/root/ns-3-dev/scratch/ass9.cc
docker exec -it pcn_ns3 bash
cd /root/ns-3-dev
./ns3 build
./ns3 run "scratch/ass9 --tcpVariant=Reno"
./ns3 run "scratch/ass9 --tcpVariant=Cubic"
./ns3 run "scratch/ass9 --tcpVariant=Bbr"
python3 plot.py   # needs python3-matplotlib installed in the container
```
