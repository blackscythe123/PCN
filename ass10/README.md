# Assignment 10 — RIP vs OSPF vs AODV in NS-3

## Topology

```
        n0                                   n1
          \                                 /
           r1 ---- r2 ----                r3
          /          \                   /  \
        n2             ---- r4 --------      n3

n0-r1, n2-r1, r1-r2, r1-r4, r2-r3, r4-r3, r3-n1, r3-n3
```

A diamond so there's a real alternate path (r1-r4-r3) when r1-r2 is cut. One
program, `ass10.cc`, reproduces every run: `--protocol=RIP|OSPF|AODV` picks
the routing approach, `--condition=normal|congestion|failure` picks the
scenario.

## Why "OSPF" here isn't real OSPF

NS-3's mainline modules do **not** include an OSPF implementation. The
closest available equivalent is `Ipv4GlobalRoutingHelper`, which is also a
link-state, shortest-path algorithm (the same family OSPF belongs to) — it
is used here as a documented stand-in, not a claim that it's protocol-exact.

## Why AODV runs over CSMA instead of point-to-point

AODV needs a broadcast-capable link for route discovery (RREQ flooding). For
the AODV run only, each link in the *same* graph is built with `CsmaHelper`
(one 2-node segment per link) instead of `PointToPointHelper`, so the
multi-hop structure is identical either way — only the underlying device
type changes.

## How "Routing Table Updates" is measured

Each protocol logs its own control-plane activity via `NS_LOG`; the count is
the number of matching lines in that log, which is meaningful because each
line *is* a route-table-affecting event:
- **RIP** — number of `SendTo:` route-update packets (`Rip` log component).
- **AODV** — number of RREQ/RREP/RERR sends (`AodvRoutingProtocol` component).
- **OSPF stand-in** — number of `Ipv4GlobalRoutingHelper::RecomputeRoutingTables()`
  calls (1 initial + 1 more after a failure), since global routing recomputes
  the whole table in one shot rather than exchanging incremental messages.

## Files

```
ass10/
├── Assign10_PCN.pdf              the assignment spec
├── report.tex / report.pdf       the lab report
├── observation.md                short summary of findings
├── ns-3-work/scratch/ass10.cc    the simulation
├── run_sweep.sh                  runs all 9 protocol x condition combos
├── plot.py                       renders the 3 required graphs
├── output.txt                    captured FlowMonitor stats, all 9 runs
└── screenshots/
    ├── pdr_vs_condition.png
    ├── delay_vs_condition.png
    └── updates_vs_condition.png
```

## How to run

```bash
docker start pcn_ns3
docker cp ass10/ns-3-work/scratch/ass10.cc pcn_ns3:/root/ns-3-dev/scratch/ass10.cc
docker cp ass10/run_sweep.sh pcn_ns3:/root/ns-3-dev/run_sweep.sh
docker exec -it pcn_ns3 bash
cd /root/ns-3-dev
./ns3 build
./run_sweep.sh          # runs all 9 combinations, writes output.txt/updates.txt
python3 plot.py         # needs python3-matplotlib
```
