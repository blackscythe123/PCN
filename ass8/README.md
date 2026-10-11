# Assignment 8 — TCP/UDP Performance over a Bottleneck (NS-3)

## What this simulates

```
n0 ---10Mbps/2ms--- r1 ===bottleneck=== r2 ---10Mbps/2ms--- n3
n1 ---10Mbps/2ms--- r1                  r2 ---10Mbps/2ms--- n2

TCP flow: n0 -> n3   (BulkSend, saturating source, 1024-byte segments)
UDP flow: n1 -> n2   (OnOff, 512-byte packets, configurable rate)
```

Both flows must cross the single **bottleneck link** between the two
"routers" `r1` and `r2` — a classic dumbbell topology used to study how a
loss-driven, congestion-controlled protocol (TCP) behaves when it shares a
narrow pipe with a rate-based, non-adaptive protocol (UDP).

## Why one flow is TCP and the other UDP — and why that matters

* **UDP** just blasts packets at its configured rate regardless of whether
  the network can carry them. If the bottleneck queue fills up, excess UDP
  packets are simply dropped — UDP never finds out and never slows down.
* **TCP** treats packet loss as a congestion signal: its congestion window
  grows until a loss (or a full queue) is detected, then it backs off. This
  is why, in the results below, an oversized UDP flow can push queuing delay
  and TCP's own loss rate up while UDP's *own* loss climbs even faster (see
  Experiment 2) — TCP politely gives way, UDP does not.

## Metrics computed (via NS-3's `FlowMonitor`)

Unlike Assignment 7's Experiment 1 (which was required to avoid
`FlowMonitor`), this assignment does not forbid it, so we use it directly —
it classifies every IP flow and reports, per flow: transmitted/received
packet counts, lost packets, and cumulative one-way delay. From these we
derive:
* **Packet loss %** = `lostPackets / txPackets × 100`
* **Throughput** = `rxBytes × 8 / activeDuration` (Mbps)
* **Average end-to-end delay** = `delaySum / rxPackets`
* **TCP retransmission rate (approx.)** = `(txPackets − rxPackets) / txPackets × 100`
  for the TCP flow specifically — a segment is only ever retransmitted after
  being lost, so the gap between what was sent and what arrived at the IP
  layer for that flow is exactly the retransmission overhead.

`FlowMonitor` also reports the TCP handshake's return-path ACK flow
(`n3 -> n0`) as a third entry in the output — that is expected and shown for
completeness, it is not a bug.

## Files

```
ass8/
├── Assgn8_PCN.pdf                 the assignment spec
├── report.tex / report.pdf        the lab report
├── ns-3-work/scratch/ass8.cc      the simulation (single parameterised program)
└── output.txt                     captured console output, every experiment run
```

## How to run

Same persistent container as Assignment 7 (`pcn_ns3`, Ubuntu 24.04 + NS-3
3.42, now also with the `flow-monitor` module built in):

```bash
docker start pcn_ns3        # if stopped
docker cp ass8/ns-3-work/scratch/ass8.cc pcn_ns3:/root/ns-3-dev/scratch/ass8.cc
docker exec -it pcn_ns3 bash
cd /root/ns-3-dev
./ns3 build
./ns3 run scratch/ass8                                   # baseline (spec defaults)
./ns3 run "scratch/ass8 --bottleneckBw=5Mbps"             # Experiment 1
./ns3 run "scratch/ass8 --udpRate=2Mbps"                  # Experiment 2 (optional)
./ns3 run "scratch/ass8 --queueSize=100"                  # Experiment 3 (optional)
./ns3 run "scratch/ass8 --bottleneckDelay=20ms"           # Experiment 4 (optional)
```

Every knob the spec's four experiments vary — bottleneck bandwidth, UDP
rate, queue size, propagation delay — is a `CommandLine` argument on the
same binary, so no recompiling is needed between runs; only relinking if you
edit the source. Pcap files (`ass8-trace-*.pcap`, one per point-to-point
interface) are written to `/root/ns-3-dev/` and can be pulled out with
`docker cp` and opened in Wireshark to inspect the TCP handshake, 1024-byte
segments, and 512-byte UDP datagrams at the packet level.
