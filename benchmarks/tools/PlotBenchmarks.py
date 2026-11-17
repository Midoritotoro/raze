import argparse
import json
import re
import subprocess
from collections import defaultdict
from pathlib import Path
import matplotlib.pyplot as plt

COMPILERS = {
    "msvc": {"color": "#0078D4", "label": "MSVC"},
    "gcc":  {"color": "#2CA02C", "label": "GCC"},
    "clang":{"color": "#FF7F0E", "label": "Clang"},
    "icpx": {"color": "#D62728", "label": "ICPX"},
}

VARIANTS = {
    "raze":        {"ls": "-",  "marker": "o", "lw": 2.2, "label": "raze"},
    "std":         {"ls": "--", "marker": "s", "lw": 1.8, "label": "std"},
    "raze_scalar": {"ls": ":",  "marker": "^", "lw": 1.8, "label": "raze[fscalar]"},
}

NAME_RE = re.compile(r"^(BM_[A-Za-z0-9_]+)<([^>]+)>(?:/(\d+))?")

def classify_variant(bm_name: str) -> str:
    low = bm_name.lower()
    if "scalar" in low:
        return "raze_scalar"
    if "std" in low:
        return "std"
    return "raze"

def run_compiler_bench(root: Path, preset: str, target: str, exe_name: str,
                       out_json: Path, do_build: bool, bm_filter: str | None):
    build_dir = root / "out" / "bench" / preset.removeprefix("bench-")
    if do_build:
        subprocess.run(["cmake", "--preset", preset], cwd=root, check=True)
        subprocess.run(["cmake", "--build", "--preset", preset, "--target", target], cwd=root, check=True)

    exe = next(build_dir.rglob(exe_name))
    cmd = [
        str(exe),
        f"--benchmark_out={out_json}",
        "--benchmark_out_format=json",
    ]
    if bm_filter:
        cmd.append(f"--benchmark_filter={bm_filter}")
    subprocess.run(cmd, check=True)

def load_results(json_path: Path):
    data = json.loads(json_path.read_text(encoding="utf-8"))
    rows = []
    for entry in data.get("benchmarks", []):
        if entry.get("run_type") == "aggregate" and entry.get("aggregate_name") != "median":
            continue

        raw_name = entry.get("run_name", entry["name"])
        m = NAME_RE.match(raw_name)
        if not m:
            continue

        fn_name, tpl_args, slash_size = m.groups()
        if "Pred" in fn_name:
            continue   

        parts = [p.strip() for p in tpl_args.split(",")]
        dtype = parts[0]
        if slash_size is not None:
            size = int(slash_size)
            scenario = ",".join(parts[1:]) if len(parts) > 1 else "default"
        else:
            size = int(parts[1])
            pos = int(parts[2]) if len(parts) > 2 else 0
            cnt = int(parts[3]) if len(parts) > 3 else 0
            scenario = f"count={cnt}, pos={'start' if pos == 0 else 'end'}"

        gbps = entry.get("bytes_per_second", 0.0) / 1e9
        rows.append((dtype, scenario, classify_variant(fn_name), size, gbps))
    return rows

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path("."))
    ap.add_argument("--target", default="SearchNBenchmark.native")
    ap.add_argument("--exe", default="SearchNBenchmark.native.exe")
    ap.add_argument("--build", action="store_true")
    ap.add_argument("--filter", default=None)
    ap.add_argument("--out-dir", type=Path, default=Path("out/charts"))
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    grouped = defaultdict(lambda: defaultdict(list))

    for comp in COMPILERS:
        json_file = args.out_dir / f"{comp}.json"
        run_compiler_bench(args.root, f"bench-{comp}", args.target, args.exe,
                           json_file, args.build, args.filter)
        for dtype, scenario, variant, size, gbps in load_results(json_file):
            grouped[(dtype, scenario)][(comp, variant)].append((size, gbps))

    for (dtype, scenario), series in grouped.items():
        fig, ax = plt.subplots(figsize=(11, 6.5), dpi=140)
        for comp, c_cfg in COMPILERS.items():
            for var, v_cfg in VARIANTS.items():
                pts = sorted(series.get((comp, var), []), key=lambda x: x[0])
                if not pts:
                    continue
                xs, ys = zip(*pts)
                ax.plot(
                    xs, ys,
                    color=c_cfg["color"],
                    linestyle=v_cfg["ls"],
                    marker=v_cfg["marker"],
                    linewidth=v_cfg["lw"],
                    label=f"{c_cfg['label']} {v_cfg['label']}",
                )

        ax.set_xscale("log", base=2)
        ax.set_title(f"{args.target} | {dtype} | {scenario}")
        ax.set_xlabel("Elements (Size)")
        ax.set_ylabel("Throughput (GB/s)")
        ax.grid(True, which="both", linestyle="--", alpha=0.35)
        ax.legend(ncol=3, fontsize=8.5, loc="best")
        fig.tight_layout()

        safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"{dtype}_{scenario}")
        fig.savefig(args.out_dir / f"{safe}.png")
        plt.close(fig)

if __name__ == "__main__":
    main()