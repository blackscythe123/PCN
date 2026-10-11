import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

conditions = ["normal", "congestion", "failure"]
data = {
    "RIP":  {"pdr": [94.90, 89.62, 39.42], "delay": [25.34, 375.90, 25.34], "updates": [36, 36, 46]},
    "OSPF": {"pdr": [99.86, 94.26, 99.32], "delay": [25.34, 381.49, 25.34], "updates": [1, 1, 2]},
    "AODV": {"pdr": [98.22, 58.67, 98.22], "delay": [26.52, 741.36, 26.52], "updates": [3, 6, 3]},
}
colors = {"RIP": "tab:blue", "OSPF": "tab:orange", "AODV": "tab:green"}

x = np.arange(len(conditions))
width = 0.25

def bar_plot(metric, ylabel, title, fname, log=False):
    plt.figure(figsize=(7, 4.5))
    for i, (proto, vals) in enumerate(data.items()):
        plt.bar(x + (i - 1) * width, vals[metric], width, label=proto, color=colors[proto])
    plt.xticks(x, conditions)
    plt.ylabel(ylabel)
    plt.title(title)
    if log:
        plt.yscale("log")
    plt.legend()
    plt.grid(alpha=0.3, axis="y")
    plt.tight_layout()
    plt.savefig(f"screenshots/{fname}", dpi=150)
    plt.close()

bar_plot("pdr", "Packet Delivery Ratio (%)", "PDR vs Network Condition", "pdr_vs_condition.png")
bar_plot("delay", "End-to-End Delay (ms)", "End-to-End Delay vs Network Condition", "delay_vs_condition.png", log=True)
bar_plot("updates", "Routing Table Updates (count)", "Routing Table Updates vs Network Condition", "updates_vs_condition.png", log=True)
print("done")
