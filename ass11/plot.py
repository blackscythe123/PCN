import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

speeds = [1, 5, 10]
data = {
    "WiFi":      {"signal": [-61.96, -68.40, -69.98], "throughput": [205.47, 58.91, 25.54], "loss": [0.0, 71.33, 87.57], "delay": [0.247, 0.561, 0.436]},
    "Bluetooth": {"signal": [-73.26, -76.21, -78.78], "throughput": [137.18, 21.13, 6.60],  "loss": [39.82, 90.73, 97.10], "delay": [2.357, 2.376, 2.423]},
    "Zigbee":    {"signal": [-77.25, None, None],     "throughput": [35.13, 0.0, 0.0],      "loss": [86.27, 100.0, 100.0], "delay": [1.567, None, None]},
}
colors = {"WiFi": "tab:blue", "Bluetooth": "tab:orange", "Zigbee": "tab:green"}

fig, axs = plt.subplots(2, 2, figsize=(10, 7.5))

def plot_metric(ax, key, ylabel, title):
    for tech, vals in data.items():
        y = vals[key]
        x = [s for s, v in zip(speeds, y) if v is not None]
        yy = [v for v in y if v is not None]
        ax.plot(x, yy, marker="o", label=tech, color=colors[tech])
    ax.set_xlabel("Speed (m/s)")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.set_xticks(speeds)
    ax.grid(alpha=0.3)
    ax.legend()

plot_metric(axs[0, 0], "signal", "Avg Received Signal (dBm)", "Signal Strength vs Speed")
plot_metric(axs[0, 1], "throughput", "Throughput (Kbps)", "Throughput vs Speed")
plot_metric(axs[1, 0], "loss", "Packet Loss (%)", "Packet Loss vs Speed")
plot_metric(axs[1, 1], "delay", "Avg End-to-End Delay (ms)", "Delay vs Speed")

plt.tight_layout()
plt.savefig("screenshots/mobility_comparison.png", dpi=150)
print("done")
