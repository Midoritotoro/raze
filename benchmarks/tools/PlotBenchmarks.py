import argparse
import json
import math
import os
import re
import shutil
import subprocess
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, NullLocator

COMPILERS = {
    "msvc":  {"color": "#0078D4", "marker": "o", "label": "MSVC"},
    "gcc":   {"color": "#2CA02C", "marker": "s", "label": "GCC"},
    "clang": {"color": "#FF7F0E", "marker": "^", "label": "Clang"},
    "icpx":  {"color": "#D62728", "marker": "D", "label": "ICPX"},
}

VARIANTS = {
    "raze": {
        "ls": "-", "lw": 2.5, "alpha": 0.96, "zorder": 5,
        "hollow": False, "ms": 6.0, "label": "raze",
    },
    "std": {
        "ls": (0, (6, 3)), "lw": 1.9, "alpha": 0.90, "zorder": 4,
        "hollow": True, "ms": 5.8, "label": "std",
    },
    "raze_scalar": {
        "ls": ":", "lw": 1.3, "alpha": 0.55, "zorder": 2,
        "hollow": False, "force_marker": "x", "ms": 4.8, "label": "raze[fscalar]",
    },
}

TYPE_ORDER = ["raze::i8", "raze::i16", "raze::i32", "raze::i64", "raze::f32", "raze::f64"]

TIME_UNIT_TO_NS = {
    "ns": 1.0,
    "us": 1e3,
    "ms": 1e6,
    "s":  1e9,
}

NAME_RE = re.compile(r"^(BM_[A-Za-z0-9_]+)<([^>]+)>(?:/(\d+(?:/\d+)*))?")


def fmt_size(val: float, _) -> str:
    n = int(round(val))
    if n >= 1024 and n % 1024 == 0:
        return f"{n // 1024}K"
    return str(n)


def fmt_y(val: float, _) -> str:
    if val >= 1000:
        return f"{val / 1000:g}k"
    if val >= 1:
        return f"{val:g}"
    return f"{val:.3g}"


def find_vcvars64() -> Path | None:
    vswhere = (
        Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
        / "Microsoft Visual Studio"
        / "Installer"
        / "vswhere.exe"
    )
    if not vswhere.exists():
        return None
    out = subprocess.check_output(
        [
            str(vswhere),
            "-latest",
            "-prerelease",
            "-products",
            "*",
            "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property",
            "installationPath",
        ],
        text=True,
    ).strip()
    if not out:
        return None
    vcvars = Path(out) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
    return vcvars if vcvars.exists() else None


def load_bat_env(bat_paths: list[Path]) -> dict[str, str]:
    chain = " && ".join(f'call "{p}" >nul' for p in bat_paths)
    proc = subprocess.run(
        f'cmd.exe /d /s /c "{chain} && set"',
        capture_output=True,
        text=True,
        check=True,
    )
    env: dict[str, str] = {}
    for line in proc.stdout.splitlines():
        if "=" in line and not line.startswith("*"):
            k, v = line.split("=", 1)
            env[k] = v
    return env


def env_for_compiler(comp: str) -> dict[str, str]:
    if comp == "gcc":
        env = dict(os.environ)
    elif comp == "clang" and "msys64" in (shutil.which("clang++") or "").lower():
        env = dict(os.environ)
    else:
        bats: list[Path] = []
        if vcvars := find_vcvars64():
            bats.append(vcvars)

        if comp == "icpx":
            for candidate in [
                Path(r"D:\icpx\compiler\2026.2\env\vars.bat"),
                Path(r"D:\icpx\setvars.bat"),
            ]:
                if candidate.exists():
                    bats.append(candidate)
                    break

        env = load_bat_env(bats) if bats else dict(os.environ)
        if comp == "icpx":
            path_key = next((k for k in env if k.upper() == "PATH"), "PATH")
            if not shutil.which("icx", path=env.get(path_key)):
                icx_bin = Path(r"D:\icpx\compiler\2026.2\bin")
                if icx_bin.exists():
                    env[path_key] = f"{icx_bin};{env.get(path_key, '')}"

    for key in list(env):
        if key.upper() in ("CC", "CXX"):
            del env[key]
    return env


def classify_variant(bm_name: str) -> str:
    low = bm_name.lower()
    if "scalar" in low:
        return "raze_scalar"
    if "std" in low:
        return "std"
    return "raze"


def run_compiler_bench(
    root: Path,
    preset: str,
    target: str,
    exe_name: str,
    out_json: Path,
    do_build: bool,
    reconfigure: bool,
    bm_filter: str | None,
) -> None:
    comp = preset.removeprefix("bench-")
    env = env_for_compiler(comp)
    build_dir = (root / "out" / "bench" / comp).resolve()

    if do_build:
        if reconfigure or (
            (build_dir / "CMakeCache.txt").exists() and not (build_dir / "build.ninja").exists()
        ):
            shutil.rmtree(build_dir, ignore_errors=True)

        if not (build_dir / "build.ninja").exists():
            subprocess.run(["cmake", "--preset", preset], cwd=root, env=env, check=True)

        subprocess.run(
            ["cmake", "--build", "--preset", preset, "--target", target],
            cwd=root,
            env=env,
            check=True,
        )

    exe = next(build_dir.rglob(exe_name))
    cmd = [
        str(exe),
        f"--benchmark_out={out_json}",
        "--benchmark_out_format=json",
    ]
    if bm_filter:
        cmd.append(f"--benchmark_filter={bm_filter}")
    subprocess.run(cmd, env=env, check=True)


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

        fn_name, tpl_args, slash_args = m.groups()
        if "Pred" in fn_name:
            continue

        parts = [p.strip() for p in tpl_args.split(",")]
        dtype = parts[0]
        if slash_args is not None:
            nums = [int(x) for x in slash_args.split("/") if x]
            size = nums[0]
            pos = nums[1] if len(nums) > 1 else 0
            cnt = nums[2] if len(nums) > 2 else 0
        else:
            size = int(parts[1])
            pos = int(parts[2]) if len(parts) > 2 else 0
            cnt = int(parts[3]) if len(parts) > 3 else 0

        scenario = f"count={cnt}, pos={'start' if (pos == 0 and cnt < size) else 'end'}"
        scale = TIME_UNIT_TO_NS.get(entry.get("time_unit", "ns"), 1.0)
        time_ns = entry.get("real_time", 0.0) * scale
        rows.append((dtype, scenario, classify_variant(fn_name), size, time_ns))
    return rows


def ordered_types(by_type: dict) -> list[str]:
    return [t for t in TYPE_ORDER if t in by_type] + [t for t in by_type if t not in TYPE_ORDER]


def style_axes(ax, xs: list[int], xlabel: str | None, ylabel: str | None, logy: bool) -> None:
    ax.set_xscale("log", base=2)
    if xs:
        ax.set_xticks(sorted(set(xs)))
    ax.xaxis.set_minor_locator(NullLocator())
    ax.xaxis.set_major_formatter(FuncFormatter(fmt_size))

    if logy:
        ax.set_yscale("log")
        ax.yaxis.set_major_formatter(FuncFormatter(fmt_y))

    ax.margins(y=0.12)
    if xlabel:
        ax.set_xlabel(xlabel)
    if ylabel:
        ax.set_ylabel(ylabel)
    ax.grid(True, which="major", linestyle="--", alpha=0.4)
    ax.grid(True, which="minor", linestyle=":", alpha=0.18)


def metric_value(time_ns: float, size: int, metric: str) -> float:
    return time_ns / size if metric == "ns_per_elem" else time_ns


def metric_label(metric: str) -> str:
    return "Time / element (ns)" if metric == "ns_per_elem" else "Time (ns)"


def active_variants(include_scalar: bool) -> dict:
    if include_scalar:
        return VARIANTS
    return {k: v for k, v in VARIANTS.items() if k != "raze_scalar"}


def plot_line(ax, xs, ys, c_cfg, v_cfg, label: str, scale_ms: float = 1.0) -> None:
    marker = v_cfg.get("force_marker", c_cfg["marker"])
    color = c_cfg["color"]

    if marker == "x":
        mfc, mec, mew = color, color, 1.3
    elif v_cfg["hollow"]:
        mfc, mec, mew = "white", color, 1.8
    else:
        mfc, mec, mew = color, "white", 0.7

    ax.plot(
        xs,
        ys,
        color=color,
        linestyle=v_cfg["ls"],
        marker=marker,
        markersize=v_cfg["ms"] * scale_ms,
        markerfacecolor=mfc,
        markeredgecolor=mec,
        markeredgewidth=mew,
        linewidth=v_cfg["lw"],
        alpha=v_cfg["alpha"],
        zorder=v_cfg["zorder"],
        label=label,
    )


def plot_type_grid(
    target: str, scenario: str, by_type: dict, out_dir: Path, logy: bool, metric: str, include_scalar: bool
) -> None:
    dtypes = ordered_types(by_type)
    cols = min(3, len(dtypes))
    rows = math.ceil(len(dtypes) / cols)
    variants = active_variants(include_scalar)

    fig, axes = plt.subplots(rows, cols, figsize=(5.8 * cols, 4.5 * rows), dpi=150, squeeze=False)
    axes_flat = axes.ravel()

    legend_handles = None
    legend_labels = None

    for idx, dtype in enumerate(dtypes):
        ax = axes_flat[idx]
        series = by_type[dtype]
        all_xs: list[int] = []

        for comp, c_cfg in COMPILERS.items():
            for var, v_cfg in variants.items():
                pts = sorted(series.get((comp, var), []), key=lambda x: x[0])
                if not pts:
                    continue
                xs = [p[0] for p in pts]
                ys = [metric_value(p[1], p[0], metric) for p in pts]
                all_xs.extend(xs)
                plot_line(ax, xs, ys, c_cfg, v_cfg, f"{c_cfg['label']} {v_cfg['label']}", scale_ms=0.9)

        style_axes(ax, all_xs, "Elements (Size)", metric_label(metric), logy)
        ax.set_title(dtype, fontsize=11, fontweight="bold")

        if legend_handles is None:
            legend_handles, legend_labels = ax.get_legend_handles_labels()

    for idx in range(len(dtypes), len(axes_flat)):
        axes_flat[idx].set_visible(False)

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=4, fontsize=8.5,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{target} | {scenario}", fontsize=13, y=0.99)
    fig.tight_layout(rect=[0, 0, 1, 0.865])

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"grid_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_type_compiler_matrix(
    target: str, scenario: str, by_type: dict, out_dir: Path, logy: bool, metric: str, include_scalar: bool
) -> None:
    dtypes = ordered_types(by_type)
    comps = list(COMPILERS.items())
    variants = active_variants(include_scalar)
    variant_colors = {"raze": "#0078D4", "std": "#D62728", "raze_scalar": "#7F7F7F"}

    rows, cols = len(dtypes), len(comps)
    fig, axes = plt.subplots(
        rows, cols, figsize=(3.6 * cols, 2.5 * rows), dpi=150,
        sharex=True, sharey="row", squeeze=False,
    )

    legend_handles = None
    legend_labels = None

    for r, dtype in enumerate(dtypes):
        series = by_type[dtype]
        for c, (comp, c_cfg) in enumerate(comps):
            ax = axes[r, c]
            all_xs: list[int] = []

            for var, v_cfg in variants.items():
                pts = sorted(series.get((comp, var), []), key=lambda x: x[0])
                if not pts:
                    continue
                xs = [p[0] for p in pts]
                ys = [metric_value(p[1], p[0], metric) for p in pts]
                all_xs.extend(xs)
                local_c = {"color": variant_colors[var], "marker": c_cfg["marker"]}
                plot_line(ax, xs, ys, local_c, v_cfg, v_cfg["label"], scale_ms=0.85)

            style_axes(
                ax,
                all_xs,
                "Size" if r == rows - 1 else None,
                f"{dtype}\n{metric_label(metric)}" if c == 0 else None,
                logy,
            )
            if r == 0:
                ax.set_title(c_cfg["label"], fontsize=11, fontweight="bold")

            if legend_handles is None and all_xs:
                legend_handles, legend_labels = ax.get_legend_handles_labels()

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=len(variants), fontsize=9.5,
            bbox_to_anchor=(0.5, 0.968), frameon=True,
        )

    fig.suptitle(f"{target} | Matrix (Type x Compiler) | {scenario}", fontsize=13, y=0.992)
    fig.tight_layout(rect=[0, 0, 1, 0.94])

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"matrix_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup_grid(target: str, scenario: str, by_type: dict, out_dir: Path) -> None:
    dtypes = ordered_types(by_type)
    cols = min(3, len(dtypes))
    rows = math.ceil(len(dtypes) / cols)

    fig, axes = plt.subplots(rows, cols, figsize=(5.6 * cols, 4.2 * rows), dpi=150, squeeze=False)
    axes_flat = axes.ravel()

    legend_handles = None
    legend_labels = None

    for idx, dtype in enumerate(dtypes):
        ax = axes_flat[idx]
        series = by_type[dtype]
        all_xs: list[int] = []

        for comp, c_cfg in COMPILERS.items():
            r_pts = dict(series.get((comp, "raze"), []))
            s_pts = dict(series.get((comp, "std"), []))
            xs = sorted(set(r_pts) & set(s_pts))
            ys = [s_pts[x] / r_pts[x] for x in xs if r_pts[x] > 0]
            xs = [x for x in xs if r_pts[x] > 0]
            if not xs:
                continue
            all_xs.extend(xs)
            ax.plot(
                xs, ys,
                color=c_cfg["color"], marker=c_cfg["marker"],
                markersize=5.6, markeredgecolor="white", markeredgewidth=0.6,
                linewidth=2.2, label=f"{c_cfg['label']} (std / raze)",
            )

        ax.axhline(1.0, color="#444444", linestyle="--", linewidth=1.1, label="1.0x (parity)")
        ax.axhspan(0.0, 1.0, color="#d62728", alpha=0.05)
        style_axes(ax, all_xs, "Elements (Size)", "Speedup (std / raze)", logy=False)
        ax.set_ylim(bottom=0.7)
        ax.set_title(dtype, fontsize=11, fontweight="bold")

        if legend_handles is None and all_xs:
            legend_handles, legend_labels = ax.get_legend_handles_labels()

    for idx in range(len(dtypes), len(axes_flat)):
        axes_flat[idx].set_visible(False)

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=5, fontsize=8.8,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{target} | Speedup raze vs std | {scenario}", fontsize=13, y=0.99)
    fig.tight_layout(rect=[0, 0, 1, 0.87])

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"grid_speedup_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def compute_geomean_series(by_type: dict, metric: str):
    agg = defaultdict(lambda: defaultdict(list))
    for series in by_type.values():
        for key, pts in series.items():
            for size, time_ns in pts:
                agg[key][size].append(metric_value(time_ns, size, metric))

    result = {}
    for key, by_size in agg.items():
        xs = sorted(by_size)
        ys = [
            math.exp(sum(math.log(max(v, 1e-9)) for v in by_size[s]) / len(by_size[s]))
            for s in xs
        ]
        result[key] = (xs, ys)
    return result


def plot_geomean_summary(
    target: str, scenario: str, by_type: dict, out_dir: Path, logy: bool, metric: str, include_scalar: bool
) -> None:
    geo = compute_geomean_series(by_type, metric)
    variants = active_variants(include_scalar)
    fig, ax = plt.subplots(figsize=(11.5, 6.8), dpi=150)
    all_xs: list[int] = []

    for comp, c_cfg in COMPILERS.items():
        for var, v_cfg in variants.items():
            if (comp, var) not in geo:
                continue
            xs, ys = geo[(comp, var)]
            all_xs.extend(xs)
            plot_line(ax, xs, ys, c_cfg, v_cfg, f"{c_cfg['label']} {v_cfg['label']}", scale_ms=1.05)

    style_axes(ax, all_xs, "Elements (Size)", f"Geomean {metric_label(metric)}", logy)
    handles, labels = ax.get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=4, fontsize=8.8, bbox_to_anchor=(0.5, 0.94), frameon=True)
    fig.suptitle(f"{target} | Geomean across all types | {scenario}", fontsize=13, y=0.985)
    fig.tight_layout(rect=[0, 0, 1, 0.84])

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"geomean_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_by_compiler(
    target: str, scenario: str, by_type: dict, out_dir: Path, logy: bool, metric: str, include_scalar: bool
) -> None:
    geo = compute_geomean_series(by_type, metric)
    variants = active_variants(include_scalar)
    fig, axes = plt.subplots(2, 2, figsize=(11.5, 8.2), dpi=150, sharex=True, sharey=True)
    axes_flat = axes.ravel()

    variant_colors = {
        "raze": "#0078D4",
        "std": "#D62728",
        "raze_scalar": "#7F7F7F",
    }

    for idx, (comp, c_cfg) in enumerate(COMPILERS.items()):
        ax = axes_flat[idx]
        all_xs: list[int] = []
        for var, v_cfg in variants.items():
            if (comp, var) not in geo:
                continue
            xs, ys = geo[(comp, var)]
            all_xs.extend(xs)
            local_c = {"color": variant_colors[var], "marker": c_cfg["marker"]}
            plot_line(ax, xs, ys, local_c, v_cfg, v_cfg["label"], scale_ms=1.0)

        style_axes(ax, all_xs, "Elements (Size)", f"Geomean {metric_label(metric)}", logy)
        ax.set_title(c_cfg["label"], fontsize=11.5, fontweight="bold")
        ax.legend(loc="best", fontsize=9)

    fig.suptitle(f"{target} | By Compiler (Geomean) | {scenario}", fontsize=13, y=0.98)
    fig.tight_layout(rect=[0, 0, 1, 0.95])

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"by_compiler_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup(target: str, scenario: str, by_type: dict, out_dir: Path) -> None:
    geo = compute_geomean_series(by_type, "ns")
    fig, ax = plt.subplots(figsize=(10.5, 5.8), dpi=150)
    all_xs: list[int] = []

    for comp, c_cfg in COMPILERS.items():
        if (comp, "raze") not in geo or (comp, "std") not in geo:
            continue
        r_xs, r_ys = geo[(comp, "raze")]
        s_xs, s_ys = geo[(comp, "std")]
        s_map = dict(zip(s_xs, s_ys))

        xs, speedups = [], []
        for x, r_t in zip(r_xs, r_ys):
            if x in s_map and r_t > 0:
                xs.append(x)
                speedups.append(s_map[x] / r_t)

        if not xs:
            continue
        all_xs.extend(xs)
        ax.plot(
            xs,
            speedups,
            color=c_cfg["color"],
            marker=c_cfg["marker"],
            markersize=6.2,
            markeredgecolor="white",
            markeredgewidth=0.7,
            linewidth=2.4,
            label=f"{c_cfg['label']} (std / raze)",
        )

    ax.axhline(1.0, color="#444444", linestyle="--", linewidth=1.2, label="1.0x (parity)")
    ax.axhspan(0.0, 1.0, color="#d62728", alpha=0.05)
    style_axes(ax, all_xs, "Elements (Size)", "Speedup vs std (higher is better)", logy=False)
    ax.set_ylim(bottom=0.8)
    ax.set_title(f"{target} | Speedup raze vs std (Geomean across all types) | {scenario}")
    ax.legend(loc="best", fontsize=9.5, frameon=True)
    fig.tight_layout()

    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", f"speedup_{scenario}")
    chart_path = out_dir / f"{safe}.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path("."))
    ap.add_argument("--target", default="SearchNBenchmark.native")
    ap.add_argument("--exe", default="SearchNBenchmark.native.exe")
    ap.add_argument("--build", action="store_true")
    ap.add_argument("--reconfigure", action="store_true")
    ap.add_argument("--plot-only", action="store_true", help="Reuse existing JSONs in --out-dir")
    ap.add_argument("--no-scalar", action="store_true", help="Hide raze[fscalar] to zoom into raze vs std")
    ap.add_argument("--filter", default=None)
    ap.add_argument("--linear-y", action="store_true", help="Use linear Y axis instead of log scale")
    ap.add_argument("--metric", choices=["ns", "ns_per_elem"], default="ns")
    ap.add_argument("--out-dir", type=Path, default=Path("out/charts"))
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    grouped = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))

    for comp in COMPILERS:
        json_file = args.out_dir / f"{comp}.json"
        if not args.plot_only:
            run_compiler_bench(
                args.root,
                f"bench-{comp}",
                args.target,
                args.exe,
                json_file,
                args.build,
                args.reconfigure,
                args.filter,
            )
        if not json_file.exists():
            continue
        for dtype, scenario, variant, size, time_ns in load_results(json_file):
            grouped[scenario][dtype][(comp, variant)].append((size, time_ns))

    logy = not args.linear_y
    include_scalar = not args.no_scalar
    for scenario, by_type in grouped.items():
        plot_type_grid(args.target, scenario, by_type, args.out_dir, logy, args.metric, include_scalar)
        plot_type_compiler_matrix(args.target, scenario, by_type, args.out_dir, logy, args.metric, include_scalar)
        plot_speedup_grid(args.target, scenario, by_type, args.out_dir)
        plot_geomean_summary(args.target, scenario, by_type, args.out_dir, logy, args.metric, include_scalar)
        plot_by_compiler(args.target, scenario, by_type, args.out_dir, logy, args.metric, include_scalar)
        plot_speedup(args.target, scenario, by_type, args.out_dir)


if __name__ == "__main__":
    main()