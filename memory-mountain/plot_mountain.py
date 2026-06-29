import matplotlib.pyplot as plt
import numpy as np

sizes = []
rows = []

with open("mountain.txt") as f:
    lines = [line.strip() for line in f if line.strip()]

# line 0: Clock frequency...
# line 1: Memory mountain (MB/sec)
# line 2: header row with s1, s2, ...
header = lines[2].split()
stride_vals = [int(x[1:]) for x in header]

for line in lines[3:]:
    parts = line.split()
    size_str = parts[0]

    if size_str.endswith("m"):
        size_kb = int(size_str[:-1]) * 1024
    elif size_str.endswith("k"):
        size_kb = int(size_str[:-1])
    else:
        continue

    bw = list(map(float, parts[1:]))

    sizes.append(size_kb)
    rows.append(bw)

sizes = np.array(sizes)
Z = np.array(rows)

X, Y = np.meshgrid(stride_vals, sizes)

fig = plt.figure(figsize=(10, 7))
ax = fig.add_subplot(111, projection="3d")

surf = ax.plot_surface(
    X,
    Y,
    Z,
    cmap="turbo",
    edgecolor="k",
    linewidth=0.35,
    antialiased=True,
)

ax.set_xlabel("Stride")
ax.set_ylabel("Size KB")
ax.set_zlabel("Read throughput MB/s")
ax.set_title("Memory Mountain")
ax.view_init(elev=30, azim=-60)

cbar = fig.colorbar(surf, shrink=0.65, aspect=18, pad=0.08)
cbar.set_label("Read throughput MB/s")

plt.tight_layout()
plt.tight_layout()
plt.savefig("memory_mountain.png", dpi=300, bbox_inches="tight")
print("Saved plot as memory_mountain.png")
