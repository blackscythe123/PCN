# Assignment 7 — NS-3 Network Simulation

## What is NS-3?

NS-3 ("Network Simulator 3") is a **discrete-event network simulator**, written in
C++, used to model and study computer networks without needing real hardware.
Instead of physically wiring routers/hosts, you describe a network in C++ code —
nodes, links, IP addressing, applications generating traffic — and NS-3 runs a
virtual clock forward, firing events (packet sent, packet queued, packet
received, timer expired...) in time order. At the end you get exact,
reproducible statistics: bytes transferred, delay, loss, throughput, etc.

Key building blocks used in both experiments here:

| Concept | What it means in NS-3 |
|---|---|
| **Node** | A virtual computer/router (`NodeContainer`). Has no capability by itself until you install things on it. |
| **NetDevice** | A virtual network interface card, created by a channel helper (e.g. `PointToPointHelper`). |
| **Channel** | The virtual "wire" connecting NetDevices; carries a data rate and propagation delay. |
| **InternetStackHelper** | Installs the IPv4/IPv6 + UDP/TCP stack on a node, like installing an OS network stack. |
| **Ipv4AddressHelper** | Assigns IP addresses to interfaces, one subnet per link. |
| **Ipv4GlobalRoutingHelper** | Auto-computes routing tables for all nodes (like running a routing protocol, but instant). |
| **Application** | Code that runs *on* a node and sends/receives data, e.g. `OnOffHelper` (traffic generator), `PacketSinkHelper`/`UdpServerHelper` (receiver that counts bytes/packets). |
| **Simulator::Run()** | Starts the virtual clock; every `Application`/`Socket` event fires in simulated time until `Simulator::Stop()`. |
| **Trace source** | A named hook (e.g. `"Tx"`, `"Rx"`) that fires a C++ callback whenever something happens (a packet is sent/received) — lets you measure things (like latency) without a heavyweight tool. |

We do **not** use `ns3::FlowMonitor` here (Experiment 1 explicitly forbids it for
the latency figure) — instead we connect directly to each Application's
`"Tx"`/`"Rx"` trace sources and time-stamp packets ourselves. This is exactly how
NS-3 users measured delay before FlowMonitor existed, and it is much lighter
weight code.

## Files

```
ass7/
├── report.tex / report.pdf      the lab report (both experiments)
├── ns-3-work/
│   └── scratch/
│       ├── exp1.cc              Experiment 1 — 4-node network, two UDP flows
│       └── exp2.cc              Experiment 2 — 2-node port-based UDP delivery
└── output_exp1.txt, output_exp2.txt   captured run output
```

`ns-3-work/` is bind-mounted into the Docker container at `/root/ns3work`, so
editing files on the Mac and re-running inside the container "just works" — no
copying needed.

## Experiment 1 — Simple 4-node network (`exp1.cc`)

```
n0 ───── n1 ───── n2
          │
          n3
```
Two UDP flows share this network: `n0 → n2` at 1 Mbps and `n3 → n0` at
500 Kbps, over 5 Mbps/2 ms links, for 10 simulated seconds. The program prints,
per flow: total bytes received, throughput, and average packet latency
(measured with `TraceConnectWithoutContext("Tx"/"Rx", ...)` — no FlowMonitor).

## Experiment 2 — UDP port-based delivery (`exp2.cc`)

```
Node1 ───────────── Node2
        P2P link
```
Two independent `UdpClient`/`UdpServer` application pairs run between the same
two nodes, distinguished only by destination port (5000 and 6000). The
propagation delay and whether the second flow is active are given as command
line arguments, so the same binary reproduces every row of both result tables
in the spec. `EnablePcapAll()` writes `.pcap` files viewable in Wireshark to
confirm packets for port 5000 land on Server 1 and port 6000 on Server 2.

## How to run the demo

Everything lives inside a persistent Docker container named **`pcn_ns3`**
(Ubuntu 24.04 + NS-3 3.42, prebuilt). Start it if it isn't running:

```bash
docker start pcn_ns3        # if it was stopped
docker exec -it pcn_ns3 bash
cd /root/ns-3-dev
```

**Experiment 1:**
```bash
./ns3 run scratch/exp1
```

**Experiment 2** (single flow, default 2 ms delay):
```bash
./ns3 run "scratch/exp2 --delay=2ms --twoFlows=false"
```

Sweep the latency table required by the spec:
```bash
for d in 2ms 5ms 10ms 20ms; do
  ./ns3 run "scratch/exp2 --delay=$d --twoFlows=false"
done
for d in 2ms 5ms 10ms 20ms; do
  ./ns3 run "scratch/exp2 --delay=$d --twoFlows=true"
done
```

PCAP files (`udp-port-based-*.pcap`) are written to
`/root/ns-3-dev/` (i.e. visible on the Mac under
`ass7/ns-3-work/` only if you `cd` there first — see note in the script
output). Open them with Wireshark, filter `udp.port==5000` /
`udp.port==6000`, and confirm each lands on the correct server.

To edit code: just edit `ass7/ns-3-work/scratch/exp1.cc` (or `exp2.cc`) on the
Mac in your normal editor — it's the same file inside the container at
`/root/ns3work/scratch/`; symlinked/copied into `ns-3-dev/scratch/` before each
build (see the run script).
