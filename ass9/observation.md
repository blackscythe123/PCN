# Observation — Assignment 9 (TCP Congestion Control: Reno / Cubic / BBR)

## Aim
Simulate 5 vehicles sending TCP traffic through a roadside router to a
traffic server in NS-3, and compare TCP Reno, Cubic, and BBR on throughput,
delay, loss, fairness, and congestion window behaviour.

## Algorithm
1. Create 5 vehicle nodes, 1 roadside router, 1 server; connect each vehicle
   to the router (access link) and the router to the server (bottleneck
   link).
2. Install the Internet stack; assign one subnet per link; populate global
   routing tables.
3. Set `ns3::TcpL4Protocol::SocketType` to `TcpNewReno` / `TcpCubic` /
   `TcpBbr` based on the `--tcpVariant` argument.
4. Start one saturating `BulkSend` TCP flow per vehicle to a distinct port
   on the server, staggered slightly in start time.
5. Trace each flow's congestion window over time via a callback on
   `TcpL4Protocol/SocketList/0/CongestionWindow`.
6. Run for 20 s; read per-flow throughput/delay/loss from `FlowMonitor`;
   compute Jain's Fairness Index across the 5 flows.

## Test Cases and Sample Output
| Test case (`--tcpVariant`) | Avg throughput | Avg delay | Avg loss | Fairness |
|---|---|---|---|---|
| Reno  | 1.570 Mbps | 124.6 ms | 0.38% | 0.9994 |
| Cubic | 1.581 Mbps | 139.8 ms | 0.91% | 0.9986 |
| BBR   | 1.547 Mbps | 125.8 ms | 1.49% | 0.9995 |

Sample console output (Reno, one flow):
```
Flow 10.1.0.1 -> port 5000 | Throughput=1.60159Mbps | AvgDelay=131.76ms | Loss=0.696379%
Jain's Fairness Index (5 flows) = 0.999417
```

## Learning Outcomes
- Reno's window follows a linear-growth/halve-on-loss sawtooth; Cubic grows
  faster along a concave curve; BBR cycles based on estimated bandwidth
  instead of reacting to loss.
- All three achieve similar throughput here because the shared bottleneck,
  not the algorithm, is the limiting factor.
- Low delay and reliable delivery matter more than raw throughput for ITS,
  since stale vehicle/traffic data is often useless by the time it arrives.
