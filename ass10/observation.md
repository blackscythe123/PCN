# Observation — Assignment 10 (RIP vs OSPF vs AODV)

## Aim
Simulate a multi-router network in NS-3 using RIP, OSPF, and AODV routing,
and compare Packet Delivery Ratio, end-to-end delay, and routing-table
updates under normal, congested, and link-failure conditions.

## Algorithm
1. Build a diamond topology (n0/n2 -> r1 -> {r2, r4} -> r3 -> n1/n3) so a
   failed link has a real alternate path; use point-to-point links for
   RIP/OSPF and CSMA segments for AODV (needs a broadcast-capable link).
2. Install the routing protocol selected by `--protocol`: `RipHelper` on the
   routers (hosts get a static default route); `Ipv4GlobalRoutingHelper`
   (NS-3's OSPF stand-in, since NS-3 has no OSPF module) with
   `PopulateRoutingTables()`; or `AodvHelper` on every node.
3. Start a CBR/UDP flow n0->n1; in the congestion condition, add a second
   simultaneous flow n2->n3.
4. In the failure condition, bring the r1-r2 interface down mid-simulation
   (`Ipv4::SetDown`) and, for the OSPF stand-in only, explicitly trigger
   `RecomputeRoutingTables()`.
5. Install `FlowMonitor`; run for 20 s; compute PDR and average delay.
6. Count each protocol's own control-plane log lines (`Rip` / `AodvRouting
   Protocol` NS_LOG output) as the routing-table-update metric.

## Test Cases and Sample Output
| Test case (protocol / condition) | PDR | Avg delay | Routing updates |
|---|---|---|---|
| RIP / normal      | 94.90% | 25.34 ms  | 36 |
| RIP / failure      | 39.42% | 25.34 ms  | 46 |
| OSPF / failure     | 99.32% | 25.34 ms  | 2  |
| AODV / congestion  | 58.67% | 741.36 ms | 6  |

Sample console output:
```
===== RIP / failure =====
Flow 10.0.0.1 -> port 6000 | Tx=2197 Rx=866 | PDR=39.4174% | AvgDelay=25.344ms
```

## Learning Outcomes
- RIP converges slowly after a failure (PDR stays low for the rest of the
  run) while OSPF-style link-state recomputation recovers almost instantly.
- AODV only communicates when a route is actually needed, so its update
  count stays low unless a new flow forces fresh route discovery.
- A routing protocol's reaction to congestion vs. failure can differ a lot —
  low update counts don't always mean better delivery (AODV under
  congestion).
