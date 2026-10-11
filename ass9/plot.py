import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# --- Congestion window over time (flow 0's socket, one line per TCP variant) ---
plt.figure(figsize=(8, 4.5))
for variant, color in [("Reno", "tab:blue"), ("Cubic", "tab:orange"), ("Bbr", "tab:green")]:
    t, cwnd = [], []
    with open(f"cwnd-{variant}.dat") as f:
        for line in f:
            a, b = line.split()
            t.append(float(a))
            cwnd.append(int(b) / 1024.0)  # segments (1024B each)
    plt.plot(t, cwnd, label=variant, color=color, linewidth=1)
plt.xlabel("Time (s)")
plt.ylabel("Congestion Window (segments)")
plt.title("TCP Congestion Window vs Time — Vehicle 0 flow")
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
plt.savefig("screenshots/cwnd_comparison.png", dpi=150)
plt.close()

# --- Throughput / Delay / Loss / Fairness bar comparison (numbers from the run) ---
variants = ["Reno", "Cubic", "Bbr"]
avg_throughput = [1.5702, 1.5805, 1.5466]
avg_delay = [124.60, 139.81, 125.77]
avg_loss = [0.3816, 0.9067, 1.4861]
fairness = [0.999417, 0.998618, 0.999476]

fig, axs = plt.subplots(2, 2, figsize=(9, 7))
axs[0, 0].bar(variants, avg_throughput, color=["tab:blue", "tab:orange", "tab:green"])
axs[0, 0].set_title("Avg Throughput per flow (Mbps)")
axs[0, 1].bar(variants, avg_delay, color=["tab:blue", "tab:orange", "tab:green"])
axs[0, 1].set_title("Avg End-to-End Delay (ms)")
axs[1, 0].bar(variants, avg_loss, color=["tab:blue", "tab:orange", "tab:green"])
axs[1, 0].set_title("Avg Packet Loss (%)")
axs[1, 1].bar(variants, fairness, color=["tab:blue", "tab:orange", "tab:green"])
axs[1, 1].set_title("Jain's Fairness Index")
axs[1, 1].set_ylim(0.9, 1.01)
for ax in axs.flat:
    ax.grid(alpha=0.3, axis="y")
plt.tight_layout()
plt.savefig("screenshots/metrics_comparison.png", dpi=150)
print("done")
