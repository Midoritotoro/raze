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
import numpy as np
from matplotlib.colors import LinearSegmentedColormap, TwoSlopeNorm
from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter, NullLocator

COMPILERS = {
    "msvc":     {"color": "#0078D4", "marker": "o", "label": "MSVC"},
    "gcc":      {"color": "#2CA02C", "marker": "s", "label": "GCC"},
    "clang":    {"color": "#9467BD", "marker": "P", "label": "Clang (libc++)",      "short_label": "Clang\n(libc++)"},
    "clang-cl": {"color": "#FF7F0E", "marker": "^", "label": "Clang-cl (MSVC STL)", "short_label": "Clang-cl\n(MSVC STL)"},
    "icpx":     {"color": "#D62728", "marker": "D", "label": "ICPX (MSVC STL)",     "short_label": "ICPX\n(MSVC STL)"},
}

VARIANTS = {
    "raze": {
        "ls": "-", "lw": 2.5, "alpha": 0.96, "zorder": 5,
        "hollow": False, "ms": 6.0, "label": "raze",
    },
    "std": {
        "ls": (0, (5, 3)), "lw": 2.0, "alpha": 0.92, "zorder": 4,
        "hollow": True, "ms": 5.8, "label": "std",
    },
    "raze_scalar": {
        "ls": ":", "lw": 1.25, "alpha": 0.45, "zorder": 2,
        "hollow": False, "force_marker": "x", "ms": 4.5, "label": "raze[fscalar]",
    },
}

TYPE_ORDER = ["raze::i8", "raze::i16", "raze::i32", "raze::i64", "raze::f32", "raze::f64"]

TIME_UNIT_TO_NS = {
    "ns": 1.0,
    "us": 1e3,
    "ms": 1e6,
    "s":  1e9,
}

SPEEDUP_CMAP = LinearSegmentedColormap.from_list(
    "raze_speedup",
    [
        (0.0, "#c0392b"),  
        (0.5, "#f4f6f7"),   
        (0.75, "#7dcea0"), 
        (1.0, "#196f3d"),  
    ],
)

NAME_RE = re.compile(r"^(BM_[A-Za-z0-9_]+)(?:<([^>]+)>)?(?:/(\d+(?:/\d+)*))?")


def fmt_size(val: float, _) -> str:
    n = int(round(val))
    if n >= 1024 and n % 1024 == 0:
        return f"{n // 1024}K"
    return str(n)


def fmt_y(val: float, _) -> str:
    if val <= 0:
        return "0"
    if val >= 1000:
        return f"{val / 1000:g}k"
    if val >= 1:
        return f"{val:g}"
    return f"{val:.3g}"


def fmt_speedup(val: float, _) -> str:
    return f"{val:g}×"


def geomean(vals: list[float]) -> float:
    pos = [v for v in vals if v > 0]
    if not pos:
        return float("nan")
    return math.exp(sum(math.log(v) for v in pos) / len(pos))


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


def prepend_existing_path(env: dict[str, str], candidate: Path) -> None:
    if not candidate.exists():
        return
    path_key = next((k for k in env if k.upper() == "PATH"), "PATH")
    cur = env.get(path_key, "")
    if str(candidate).lower() not in cur.lower():
        env[path_key] = f"{candidate};{cur}" if cur else str(candidate)


def env_for_compiler(comp: str) -> dict[str, str]:
    if comp == "gcc":
        env = dict(os.environ)
        prepend_existing_path(env, Path(r"D:\Libraries\win64\msys64\ucrt64\bin"))
    elif comp == "clang":
        env = dict(os.environ)
        prepend_existing_path(env, Path(r"D:\Libraries\win64\msys64\clang64\bin"))
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
        path_key = next((k for k in env if k.upper() == "PATH"), "PATH")

        if comp == "clang-cl" and not shutil.which("clang-cl", path=env.get(path_key)):
            vs_dir = env.get("VSINSTALLDIR") or env.get("VCToolsInstallDir")
            if vs_dir:
                vs_root = Path(vs_dir)
                for candidate in [
                    vs_root / "VC" / "Tools" / "Llvm" / "x64" / "bin",
                    vs_root.parent.parent.parent / "Llvm" / "x64" / "bin",
                ]:
                    if candidate.exists():
                        prepend_existing_path(env, candidate)
                        break

        if comp == "icpx" and not shutil.which("icx", path=env.get(path_key)):
            prepend_existing_path(env, Path(r"D:\icpx\compiler\2026.2\bin"))

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


def discover_benchmarks(root: Path) -> list[str]:
    bench_dir = root / "benchmarks"
    if not bench_dir.exists():
        return []
    return sorted(p.stem for p in bench_dir.glob("*.cpp") if p.stem != "main")


def ensure_configured(root: Path, comp: str, env: dict[str, str], reconfigure: bool) -> Path | None:
    preset = f"bench-{comp}"
    build_dir = (root / "out" / "bench" / comp).resolve()

    if reconfigure or (
        (build_dir / "CMakeCache.txt").exists() and not (build_dir / "build.ninja").exists()
    ):
        shutil.rmtree(build_dir, ignore_errors=True)

    if not (build_dir / "build.ninja").exists():
        res = subprocess.run(["cmake", "--preset", preset], cwd=root, env=env)
        if res.returncode != 0:
            print(f"[SKIP] Configure failed for {preset}")
            return None

    return build_dir


def build_and_run_target(
    root: Path,
    comp: str,
    env: dict[str, str],
    build_dir: Path,
    bench_name: str,
    suffix: str,
    out_json: Path,
    do_build: bool,
    bm_filter: str | None,
) -> bool:
    preset = f"bench-{comp}"
    target = f"{bench_name}.{suffix}"
    exe_name = f"{target}.exe" if os.name == "nt" else target

    if do_build:
        res = subprocess.run(
            ["cmake", "--build", "--preset", preset, "--target", target],
            cwd=root,
            env=env,
        )
        if res.returncode != 0:
            print(f"[SKIP] Build failed: {target} ({comp})")
            return False

    exe = next(build_dir.rglob(exe_name), None)
    if exe is None:
        print(f"[SKIP] Executable not found: {exe_name} in {build_dir}")
        return False

    out_json.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(exe),
        f"--benchmark_out={out_json}",
        "--benchmark_out_format=json",
    ]
    if bm_filter:
        cmd.append(f"--benchmark_filter={bm_filter}")

    res = subprocess.run(cmd, env=env)
    if res.returncode != 0:
        print(f"[SKIP] Benchmark execution failed: {exe_name} ({comp})")
        return False

    return True


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
        parts = [p.strip() for p in tpl_args.split(",")] if tpl_args else []
        dtype = parts[0] if parts else "bytes"

        scenario_parts: list[str] = []
        size = 0

        if slash_args is not None:
            nums = [int(x) for x in slash_args.split("/") if x]
            if not nums:
                continue
            size = nums[0]
            if len(nums) >= 3:
                pos, cnt = nums[1], nums[2]
                scenario_parts.append(f"count={cnt}")
                scenario_parts.append(f"pos={'start' if (pos == 0 and cnt < size) else 'end'}")
            elif len(nums) == 2:
                pos = nums[1]
                scenario_parts.append(f"pos={'start' if pos == 0 else 'end'}")
        elif len(parts) >= 2:
            try:
                size = int(parts[1])
            except ValueError:
                continue
            if len(parts) >= 4:
                pos, cnt = int(parts[2]), int(parts[3])
                scenario_parts.append(f"count={cnt}")
                scenario_parts.append(f"pos={'start' if (pos == 0 and cnt < size) else 'end'}")
            elif len(parts) == 3:
                pos = int(parts[2])
                scenario_parts.append(f"pos={'start' if pos == 0 else 'end'}")
        else:
            continue

        if "pred" in fn_name.lower():
            scenario_parts.append("pred")
        if "binary" in fn_name.lower():
            scenario_parts.append("binary")

        scenario = ", ".join(scenario_parts) if scenario_parts else "default"
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
    ax.margins(x=0.03, y=0.07)

    if logy:
        ax.set_yscale("log")
        ax.autoscale_view()
        ymin, ymax = ax.get_ylim()
        span = ymax / max(ymin, 1e-12)
        if span <= 25:
            subs = (1.0, 1.5, 2.0, 3.0, 5.0, 7.0)
        elif span <= 250:
            subs = (1.0, 2.0, 5.0)
        else:
            subs = (1.0,)
        ax.yaxis.set_major_locator(LogLocator(base=10.0, subs=subs, numticks=12))
        ax.yaxis.set_major_formatter(FuncFormatter(fmt_y))
        ax.yaxis.set_minor_locator(LogLocator(base=10.0, subs="auto", numticks=12))
        ax.yaxis.set_minor_formatter(NullFormatter())

    if xlabel:
        ax.set_xlabel(xlabel)
    if ylabel:
        ax.set_ylabel(ylabel)
    ax.grid(True, which="major", linestyle="--", alpha=0.42)
    ax.grid(True, which="minor", linestyle=":", alpha=0.18)


def metric_value(time_ns: float, size: int, metric: str) -> float:
    return time_ns / max(size, 1) if metric == "ns_per_elem" else time_ns


def metric_label(metric: str) -> str:
    return "Time / element (ns)" if metric == "ns_per_elem" else "Time (ns)"


def active_variants(include_scalar: bool) -> dict:
    if include_scalar:
        return VARIANTS
    return {k: v for k, v in VARIANTS.items() if k != "raze_scalar"}


def safe_filename(prefix: str, bench_name: str, scenario: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", f"{prefix}_{bench_name}_{scenario}") + ".png"


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


def shade_and_annotate_gap(
    ax,
    r_xs: list[int],
    r_ys: list[float],
    s_xs: list[int],
    s_ys: list[float],
    logy: bool,
) -> None:
    r_map = dict(zip(r_xs, r_ys))
    s_map = dict(zip(s_xs, s_ys))
    pts = [
        (x, r_map[x], s_map[x])
        for x in sorted(set(r_map) & set(s_map))
        if r_map[x] > 0 and s_map[x] > 0
    ]
    if not pts:
        return

    cxs = [p[0] for p in pts]
    rys = [p[1] for p in pts]
    sys_ = [p[2] for p in pts]

    ax.fill_between(
        cxs, rys, sys_,
        where=[s >= r for r, s in zip(rys, sys_)],
        interpolate=True, color="#2CA02C", alpha=0.12, zorder=1,
    )
    ax.fill_between(
        cxs, rys, sys_,
        where=[s < r for r, s in zip(rys, sys_)],
        interpolate=True, color="#D62728", alpha=0.14, zorder=1,
    )

    best_idx = max(range(len(pts)), key=lambda i: max(sys_[i] / rys[i], rys[i] / sys_[i]))
    bx, br, bs = pts[best_idx]
    if max(bs / br, br / bs) < 1.12:
        return

    ax.annotate(
        "",
        xy=(bx, br),
        xytext=(bx, bs),
        arrowprops=dict(arrowstyle="<->", color="#222222", lw=1.1, alpha=0.7, shrinkA=3, shrinkB=3),
        zorder=6,
    )

    y_mid = math.sqrt(br * bs) if logy else 0.5 * (br + bs)
    near_right = best_idx >= len(pts) - 2
    txt = f"{bs / br:.1f}×" if bs >= br else f"{bs / br:.2f}×"
    edge_col = "#2CA02C" if bs >= br else "#D62728"

    ax.annotate(
        txt,
        xy=(bx, y_mid),
        xytext=(-6 if near_right else 6, 0),
        textcoords="offset points",
        ha="right" if near_right else "left",
        va="center",
        fontsize=8.5,
        fontweight="bold",
        color="#111111",
        bbox=dict(boxstyle="round,pad=0.18", facecolor="white", edgecolor=edge_col, lw=0.9, alpha=0.9),
        zorder=7,
    )


def compute_geomean_series(by_type: dict, metric: str):
    agg = defaultdict(lambda: defaultdict(list))
    for series in by_type.values():
        for key, pts in series.items():
            for size, time_ns in pts:
                agg[key][size].append(metric_value(time_ns, size, metric))

    result = {}
    for key, by_size in agg.items():
        xs = sorted(by_size)
        ys = [geomean(by_size[s]) for s in xs]
        result[key] = (xs, ys)
    return result


def plot_article_summary(
    bench_name: str,
    title_prefix: str,
    grouped: dict,
    out_dir: Path,
    logy: bool,
    metric: str,
    include_scalar: bool,
    scenario_filter: str | None = None,
) -> None:
    scenarios = list(grouped.items())
    if scenario_filter:
        filtered = [(s, d) for s, d in scenarios if scenario_filter.lower() in s.lower()]
        if filtered:
            scenarios = filtered

    if not scenarios:
        return

    all_ratios: list[float] = []
    for _, by_type in scenarios:
        for series in by_type.values():
            for comp in COMPILERS:
                r_pts = dict(series.get((comp, "raze"), []))
                s_pts = dict(series.get((comp, "std"), []))
                for x in set(r_pts) & set(s_pts):
                    if r_pts[x] > 0 and s_pts[x] > 0:
                        all_ratios.append(s_pts[x] / r_pts[x])

    overall_geo = geomean(all_ratios)
    overall_peak = max(all_ratios) if all_ratios else float("nan")

    n_rows = len(scenarios)
    fig, axes = plt.subplots(
        n_rows, 3,
        figsize=(17.6, 4.7 * n_rows + 0.9),
        dpi=160,
        gridspec_kw={"width_ratios": [1.12, 1.12, 1.58]},
        squeeze=False,
        layout="constrained",
    )

    variant_styles = {
        "raze":        {"color": "#0078D4", "ls": "-",          "lw": 2.6, "marker": "o", "hollow": False, "label": "raze"},
        "std":         {"color": "#D62728", "ls": (0, (5, 3)),  "lw": 2.1, "marker": "o", "hollow": True,  "label": "std"},
        "raze_scalar": {"color": "#7F7F7F", "ls": ":",          "lw": 1.4, "marker": "x", "hollow": False, "label": "raze[fscalar]"},
    }
    variants_to_plot = ["raze", "std", "raze_scalar"] if include_scalar else ["raze", "std"]

    for row_idx, (scenario, by_type) in enumerate(scenarios):
        ax_abs, ax_spd, ax_hm = axes[row_idx]
        geo = compute_geomean_series(by_type, metric)

        abs_xs: list[int] = []
        center_curves: dict[str, tuple[list[int], list[float]]] = {}

        for var in variants_to_plot:
            by_size: dict[int, list[float]] = defaultdict(list)
            for comp in COMPILERS:
                if (comp, var) in geo:
                    c_xs, c_ys = geo[(comp, var)]
                    for x, y in zip(c_xs, c_ys):
                        if y > 0:
                            by_size[x].append(y)
            if not by_size:
                continue

            xs = sorted(by_size)
            y_geo = [geomean(by_size[x]) for x in xs]
            y_min = [min(by_size[x]) for x in xs]
            y_max = [max(by_size[x]) for x in xs]
            abs_xs.extend(xs)
            center_curves[var] = (xs, y_geo)

            st = variant_styles[var]
            ax_abs.fill_between(xs, y_min, y_max, color=st["color"], alpha=0.14 if var != "raze_scalar" else 0.08, zorder=1)
            plot_line(
                ax_abs, xs, y_geo,
                {"color": st["color"], "marker": st["marker"]},
                {"ls": st["ls"], "lw": st["lw"], "ms": 5.5, "hollow": st["hollow"],
                 "alpha": 0.96 if var != "raze_scalar" else 0.55, "zorder": 4 if var == "raze" else 3,
                 **({"force_marker": "x"} if var == "raze_scalar" else {})},
                st["label"],
            )

        if "raze" in center_curves and "std" in center_curves:
            shade_and_annotate_gap(ax_abs, *center_curves["raze"], *center_curves["std"], logy)

        y_prefix = f"[{scenario}]\n" if n_rows > 1 else ""
        style_axes(
            ax_abs, abs_xs,
            "Elements (Size)" if row_idx == n_rows - 1 else None,
            f"{y_prefix}{metric_label(metric)}",
            logy,
        )
        if row_idx == 0:
            ax_abs.set_title("Performance (band = min–max across compilers)", fontsize=11, fontweight="bold")
        ax_abs.legend(loc="best", fontsize=8.5, frameon=True)

        spd_xs: list[int] = []
        min_spd, max_spd = 1.0, 1.2

        for comp, c_cfg in COMPILERS.items():
            if (comp, "raze") not in geo or (comp, "std") not in geo:
                continue
            r_xs, r_ys = geo[(comp, "raze")]
            s_xs, s_ys = geo[(comp, "std")]
            s_map = dict(zip(s_xs, s_ys))

            xs, spds = [], []
            for x, r_t in zip(r_xs, r_ys):
                if x in s_map and r_t > 0 and s_map[x] > 0:
                    xs.append(x)
                    spds.append(s_map[x] / r_t)
            if not xs:
                continue

            spd_xs.extend(xs)
            min_spd = min(min_spd, min(spds))
            max_spd = max(max_spd, max(spds))
            ax_spd.plot(
                xs, spds,
                color=c_cfg["color"], marker=c_cfg["marker"],
                markersize=5.6, markeredgecolor="white", markeredgewidth=0.6,
                linewidth=2.2,
                label=f"{c_cfg['label']}: {geomean(spds):.1f}× (max {max(spds):.1f}×)",
            )

        ax_spd.axhline(1.0, color="#444444", linestyle="--", linewidth=1.1, label="1.0× (parity)")
        use_log2_spd = max_spd > 12.0
        if use_log2_spd:
            ax_spd.set_yscale("log", base=2)
            bottom = min(0.5, max(0.125, min_spd * 0.85))
            top = max_spd * 1.22
        else:
            bottom = min(0.75, max(0.0, min_spd * 0.9))
            top = max_spd * 1.14

        ax_spd.set_ylim(bottom=bottom, top=top)
        ax_spd.axhspan(bottom, 1.0, color="#d62728", alpha=0.06, zorder=0)
        ax_spd.axhspan(1.0, top, color="#2ca02c", alpha=0.04, zorder=0)
        style_axes(
            ax_spd, spd_xs,
            "Elements (Size)" if row_idx == n_rows - 1 else None,
            "Speedup (std / raze)",
            logy=False,
        )
        ax_spd.yaxis.set_major_formatter(FuncFormatter(fmt_speedup))
        if row_idx == 0:
            ax_spd.set_title("Speedup vs std by array size", fontsize=11, fontweight="bold")
        ax_spd.legend(loc="best", fontsize=7.8, frameon=True)

        dtypes = ordered_types(by_type)
        active_comps = [
            c for c in COMPILERS
            if any((c, "raze") in by_type[dt] and (c, "std") in by_type[dt] for dt in dtypes)
        ]

        if not dtypes or not active_comps:
            ax_hm.text(0.5, 0.5, "No paired raze/std data", ha="center", va="center")
            ax_hm.set_axis_off()
            continue

        mat = np.full((len(dtypes), len(active_comps)), np.nan, dtype=float)
        for r, dt in enumerate(dtypes):
            series = by_type[dt]
            for c, comp in enumerate(active_comps):
                r_pts = dict(series.get((comp, "raze"), []))
                s_pts = dict(series.get((comp, "std"), []))
                ratios = [
                    s_pts[x] / r_pts[x]
                    for x in set(r_pts) & set(s_pts)
                    if r_pts[x] > 0 and s_pts[x] > 0
                ]
                if ratios:
                    mat[r, c] = geomean(ratios)

        row_labels = [dt.replace("raze::", "") for dt in dtypes]
        col_labels = [COMPILERS[c].get("short_label", COMPILERS[c]["label"]) for c in active_comps]

        if len(active_comps) > 1:
            col_geo = np.array([
                geomean([v for v in mat[r, :] if not np.isnan(v)])
                for r in range(mat.shape[0])
            ])[:, None]
            mat = np.hstack([mat, col_geo])
            col_labels.append("Geomean")

        if len(dtypes) > 1:
            row_geo = np.array([
                geomean([v for v in mat[:, c] if not np.isnan(v)])
                for c in range(mat.shape[1])
            ])[None, :]
            mat = np.vstack([mat, row_geo])
            row_labels.append("Geomean")

        valid_vals = mat[~np.isnan(mat) & (mat > 0)]
        log_mat = np.where(np.isnan(mat) | (mat <= 0), np.nan, np.log2(np.maximum(mat, 1e-9)))
        vmin = min(-0.6, float(np.nanmin(log_mat)) * 1.05) if len(valid_vals) else -1.0
        vmax = max(1.0, float(np.nanmax(log_mat)) * 1.05) if len(valid_vals) else 2.0
        norm = TwoSlopeNorm(vmin=vmin, vcenter=0.0, vmax=vmax)

        cmap = SPEEDUP_CMAP.copy()
        cmap.set_bad(color="#eeeeee")
        ax_hm.imshow(np.ma.masked_invalid(log_mat), cmap=cmap, norm=norm, aspect="auto")

        ax_hm.set_xticks(range(len(col_labels)))
        ax_hm.set_xticklabels(col_labels, fontsize=8.4, fontweight="bold")
        ax_hm.set_yticks(range(len(row_labels)))
        ax_hm.set_yticklabels(row_labels, fontsize=9.5, fontweight="bold")

        ax_hm.set_xticks(np.arange(-0.5, len(col_labels), 1), minor=True)
        ax_hm.set_yticks(np.arange(-0.5, len(row_labels), 1), minor=True)
        ax_hm.grid(which="minor", color="white", linestyle="-", linewidth=1.6)
        ax_hm.tick_params(which="minor", bottom=False, left=False)

        if len(active_comps) > 1:
            ax_hm.axvline(len(active_comps) - 0.5, color="#222222", linewidth=1.8)
        if len(dtypes) > 1:
            ax_hm.axhline(len(dtypes) - 0.5, color="#222222", linewidth=1.8)

        for r in range(mat.shape[0]):
            for c in range(mat.shape[1]):
                val = mat[r, c]
                if np.isnan(val):
                    ax_hm.text(c, r, "—", ha="center", va="center", color="#777777", fontsize=9)
                    continue
                n_val = norm(log_mat[r, c])
                t_col = "white" if (n_val < 0.22 or n_val > 0.82) else "#111111"
                label_txt = f"{val:.1f}×" if val >= 2.0 else f"{val:.2f}×"
                is_summary = (len(active_comps) > 1 and c == mat.shape[1] - 1) or (
                    len(dtypes) > 1 and r == mat.shape[0] - 1
                )
                ax_hm.text(
                    c, r, label_txt,
                    ha="center", va="center",
                    color=t_col,
                    fontsize=9.3 if is_summary else 8.8,
                    fontweight="bold" if (is_summary or val >= 1.5) else "normal",
                )

        if row_idx == 0:
            ax_hm.set_title("Geomean speedup (Type × Compiler)", fontsize=11, fontweight="bold")

    headline = f" | Geomean: {overall_geo:.1f}× vs std (peak {overall_peak:.1f}×)" if not math.isnan(overall_geo) else ""
    fig.suptitle(f"{title_prefix}{headline}", fontsize=14, fontweight="bold")

    chart_path = out_dir / f"{bench_name}_article.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_type_grid(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    dtypes = ordered_types(by_type)
    cols = min(3, len(dtypes))
    rows = math.ceil(len(dtypes) / cols)
    variants = active_variants(include_scalar)

    fig, axes = plt.subplots(rows, cols, figsize=(6.2 * cols, 4.9 * rows), dpi=150, squeeze=False)
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

        if legend_handles is None and all_xs:
            legend_handles, legend_labels = ax.get_legend_handles_labels()

    for idx in range(len(dtypes), len(axes_flat)):
        axes_flat[idx].set_visible(False)

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=min(5, max(2, cols * 2)), fontsize=8.0,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{title_prefix} | {scenario}", fontsize=13, y=0.99)
    fig.tight_layout(rect=[0, 0, 1, 0.855])

    chart_path = out_dir / safe_filename("grid", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_type_compiler_matrix(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    dtypes = ordered_types(by_type)
    comps = list(COMPILERS.items())
    variants = active_variants(include_scalar)
    variant_colors = {"raze": "#0078D4", "std": "#D62728", "raze_scalar": "#7F7F7F"}

    rows, cols = len(dtypes), len(comps)
    fig, axes = plt.subplots(
        rows, cols, figsize=(4.0 * cols, 3.3 * rows), dpi=150,
        sharex=True, sharey="row", squeeze=False,
    )

    legend_handles = None
    legend_labels = None
    cell_xs: dict[tuple[int, int], list[int]] = {}

    for r, dtype in enumerate(dtypes):
        series = by_type[dtype]
        for c, (comp, c_cfg) in enumerate(comps):
            ax = axes[r, c]
            all_xs: list[int] = []
            plotted: dict[str, tuple[list[int], list[float]]] = {}

            for var, v_cfg in variants.items():
                pts = sorted(series.get((comp, var), []), key=lambda x: x[0])
                if not pts:
                    continue
                xs = [p[0] for p in pts]
                ys = [metric_value(p[1], p[0], metric) for p in pts]
                all_xs.extend(xs)
                plotted[var] = (xs, ys)
                local_c = {"color": variant_colors[var], "marker": c_cfg["marker"]}
                plot_line(ax, xs, ys, local_c, v_cfg, v_cfg["label"], scale_ms=0.88)

            if "raze" in plotted and "std" in plotted:
                shade_and_annotate_gap(ax, *plotted["raze"], *plotted["std"], logy)

            cell_xs[(r, c)] = all_xs
            if r == 0:
                ax.set_title(c_cfg["label"], fontsize=11, fontweight="bold")
            if legend_handles is None and all_xs:
                legend_handles, legend_labels = ax.get_legend_handles_labels()

    for r, dtype in enumerate(dtypes):
        for c in range(cols):
            style_axes(
                axes[r, c],
                cell_xs[(r, c)],
                "Size" if r == rows - 1 else None,
                f"{dtype}\n{metric_label(metric)}" if c == 0 else None,
                logy,
            )

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=len(variants), fontsize=9.5,
            bbox_to_anchor=(0.5, 0.968), frameon=True,
        )

    fig.suptitle(f"{title_prefix} | Matrix (Type x Compiler) | {scenario}", fontsize=13, y=0.992)
    fig.tight_layout(rect=[0, 0, 1, 0.94])

    chart_path = out_dir / safe_filename("matrix", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup_grid(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict, out_dir: Path
) -> None:
    dtypes = ordered_types(by_type)
    cols = min(3, len(dtypes))
    rows = math.ceil(len(dtypes) / cols)

    fig, axes = plt.subplots(rows, cols, figsize=(5.8 * cols, 4.4 * rows), dpi=150, squeeze=False)
    axes_flat = axes.ravel()

    legend_handles = None
    legend_labels = None

    for idx, dtype in enumerate(dtypes):
        ax = axes_flat[idx]
        series = by_type[dtype]
        all_xs: list[int] = []
        min_y, max_y = 1.0, 1.2

        for comp, c_cfg in COMPILERS.items():
            r_pts = dict(series.get((comp, "raze"), []))
            s_pts = dict(series.get((comp, "std"), []))
            xs = sorted(set(r_pts) & set(s_pts))
            ys = [s_pts[x] / r_pts[x] for x in xs if r_pts[x] > 0]
            xs = [x for x in xs if r_pts[x] > 0]
            if not xs:
                continue
            all_xs.extend(xs)
            min_y = min(min_y, min(ys))
            max_y = max(max_y, max(ys))
            ax.plot(
                xs, ys,
                color=c_cfg["color"], marker=c_cfg["marker"],
                markersize=5.6, markeredgecolor="white", markeredgewidth=0.6,
                linewidth=2.2, label=c_cfg["label"],
            )

        ax.axhline(1.0, color="#444444", linestyle="--", linewidth=1.1, label="1.0× (parity)")
        bottom = min(0.7, max(0.0, min_y * 0.9))
        top = max_y * 1.12
        ax.set_ylim(bottom=bottom, top=top)
        ax.axhspan(bottom, 1.0, color="#d62728", alpha=0.06, zorder=0)
        ax.axhspan(1.0, top, color="#2ca02c", alpha=0.04, zorder=0)
        style_axes(ax, all_xs, "Elements (Size)", "Speedup (std / raze)", logy=False)
        ax.yaxis.set_major_formatter(FuncFormatter(fmt_speedup))
        ax.set_title(dtype, fontsize=11, fontweight="bold")

        if legend_handles is None and all_xs:
            legend_handles, legend_labels = ax.get_legend_handles_labels()

    for idx in range(len(dtypes), len(axes_flat)):
        axes_flat[idx].set_visible(False)

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=min(6, max(3, cols * 2)), fontsize=8.5,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{title_prefix} | Speedup raze vs std | {scenario}", fontsize=13, y=0.99)
    fig.tight_layout(rect=[0, 0, 1, 0.87])

    chart_path = out_dir / safe_filename("grid_speedup", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_geomean_summary(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    geo = compute_geomean_series(by_type, metric)
    variants = active_variants(include_scalar)
    fig, ax = plt.subplots(figsize=(12.4, 7.2), dpi=150)
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
    fig.legend(handles, labels, loc="upper center", ncol=min(5, len(COMPILERS)), fontsize=8.0, bbox_to_anchor=(0.5, 0.94), frameon=True)
    fig.suptitle(f"{title_prefix} | Geomean across all types | {scenario}", fontsize=13, y=0.985)
    fig.tight_layout(rect=[0, 0, 1, 0.84])

    chart_path = out_dir / safe_filename("geomean", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_by_compiler(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    geo = compute_geomean_series(by_type, metric)
    variants = active_variants(include_scalar)
    n_comps = len(COMPILERS)
    cols = 3 if n_comps > 4 else 2
    rows = math.ceil(n_comps / cols)

    fig, axes = plt.subplots(
        rows, cols, figsize=(5.8 * cols, 4.4 * rows), dpi=150,
        sharex=True, sharey=True, squeeze=False,
    )
    axes_flat = axes.ravel()

    variant_colors = {
        "raze": "#0078D4",
        "std": "#D62728",
        "raze_scalar": "#7F7F7F",
    }

    panel_xs: list[list[int]] = []

    for idx, (comp, c_cfg) in enumerate(COMPILERS.items()):
        ax = axes_flat[idx]
        all_xs: list[int] = []
        plotted: dict[str, tuple[list[int], list[float]]] = {}

        for var, v_cfg in variants.items():
            if (comp, var) not in geo:
                continue
            xs, ys = geo[(comp, var)]
            all_xs.extend(xs)
            plotted[var] = (xs, ys)
            local_c = {"color": variant_colors[var], "marker": c_cfg["marker"]}
            plot_line(ax, xs, ys, local_c, v_cfg, v_cfg["label"], scale_ms=1.0)

        if "raze" in plotted and "std" in plotted:
            shade_and_annotate_gap(ax, *plotted["raze"], *plotted["std"], logy)

        panel_xs.append(all_xs)
        ax.set_title(c_cfg["label"], fontsize=11.5, fontweight="bold")
        ax.legend(loc="best", fontsize=9)

    for idx in range(n_comps, len(axes_flat)):
        axes_flat[idx].set_visible(False)

    for idx in range(n_comps):
        is_bottom = (idx + cols) >= n_comps
        style_axes(
            axes_flat[idx],
            panel_xs[idx],
            "Elements (Size)" if is_bottom else None,
            f"Geomean {metric_label(metric)}" if idx % cols == 0 else None,
            logy,
        )
        if is_bottom:
            axes_flat[idx].tick_params(labelbottom=True)

    fig.suptitle(f"{title_prefix} | By Compiler (Geomean) | {scenario}", fontsize=13, y=0.98)
    fig.tight_layout(rect=[0, 0, 1, 0.95])

    chart_path = out_dir / safe_filename("by_compiler", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup(bench_name: str, title_prefix: str, scenario: str, by_type: dict, out_dir: Path) -> None:
    geo = compute_geomean_series(by_type, "ns")
    fig, ax = plt.subplots(figsize=(11.0, 6.2), dpi=150)
    all_xs: list[int] = []
    min_y, max_y = 1.0, 1.2

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
        min_y = min(min_y, min(speedups))
        max_y = max(max_y, max(speedups))
        ax.plot(
            xs,
            speedups,
            color=c_cfg["color"],
            marker=c_cfg["marker"],
            markersize=6.2,
            markeredgecolor="white",
            markeredgewidth=0.7,
            linewidth=2.4,
            label=f"{c_cfg['label']}: max {max(speedups):.1f}×, end {speedups[-1]:.1f}×",
        )

    ax.axhline(1.0, color="#444444", linestyle="--", linewidth=1.2, label="1.0× (parity)")
    bottom = min(0.8, max(0.0, min_y * 0.9))
    top = max_y * 1.12
    ax.set_ylim(bottom=bottom, top=top)
    ax.axhspan(bottom, 1.0, color="#d62728", alpha=0.06, zorder=0)
    ax.axhspan(1.0, top, color="#2ca02c", alpha=0.04, zorder=0)
    style_axes(ax, all_xs, "Elements (Size)", "Speedup vs std (higher is better)", logy=False)
    ax.yaxis.set_major_formatter(FuncFormatter(fmt_speedup))
    ax.set_title(f"{title_prefix} | Speedup raze vs std (Geomean across all types) | {scenario}")
    ax.legend(loc="best", fontsize=9.2, frameon=True)
    fig.tight_layout()

    chart_path = out_dir / safe_filename("speedup", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path("."))
    ap.add_argument(
        "--targets", nargs="+", default=None,
        help="Benchmark names (e.g. FindBenchmark SearchNBenchmark); defaults to all benchmarks/*.cpp",
    )
    ap.add_argument("--suffix", choices=["native", "portable"], default="native")
    ap.add_argument("--build", action="store_true")
    ap.add_argument("--reconfigure", action="store_true")
    ap.add_argument("--plot-only", action="store_true", help="Reuse existing JSONs in --out-dir")
    ap.add_argument("--no-scalar", action="store_true", help="Hide raze[fscalar] to zoom into raze vs std")
    ap.add_argument("--article-only", action="store_true", help="Only render one unified summary figure per benchmark")
    ap.add_argument("--article-scenario", default=None, help="Filter scenario substring for article summary figure")
    ap.add_argument("--filter", default=None)
    ap.add_argument("--linear-y", action="store_true", help="Use linear Y axis instead of log scale")
    ap.add_argument(
        "--metric", choices=["ns_per_elem", "ns"], default="ns_per_elem",
        help="ns_per_elem (default) normalizes by size so speedups stand out clearly; ns plots total time",
    )
    ap.add_argument("--out-dir", type=Path, default=Path("out/charts"))
    args = ap.parse_args()

    root = args.root.resolve()
    benchmarks = (
        [t.removesuffix(f".{args.suffix}") for t in args.targets]
        if args.targets
        else discover_benchmarks(root)
    )
    if not benchmarks:
        raise SystemExit(f"No benchmark .cpp files found in {root / 'benchmarks'}")

    print(f"Discovered benchmarks: {', '.join(benchmarks)}")
    args.out_dir.mkdir(parents=True, exist_ok=True)

    compiler_envs: dict[str, tuple[dict[str, str], Path]] = {}
    skipped: list[tuple[str, str]] = []

    if not args.plot_only:
        for comp in COMPILERS:
            env = env_for_compiler(comp)
            if args.build:
                build_dir = ensure_configured(root, comp, env, args.reconfigure)
            else:
                build_dir = (root / "out" / "bench" / comp).resolve()
            if build_dir is not None and build_dir.exists():
                compiler_envs[comp] = (env, build_dir)
            else:
                skipped.append(("<configure>", comp))

    logy = not args.linear_y
    include_scalar = not args.no_scalar

    for bench_name in benchmarks:
        bench_out = args.out_dir / bench_name
        bench_out.mkdir(parents=True, exist_ok=True)
        title_prefix = f"{bench_name}.{args.suffix}"

        grouped = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))

        for comp in COMPILERS:
            json_file = bench_out / f"{comp}.json"
            if not args.plot_only:
                if comp not in compiler_envs:
                    continue
                env, build_dir = compiler_envs[comp]
                ok = build_and_run_target(
                    root, comp, env, build_dir, bench_name, args.suffix,
                    json_file, args.build, args.filter,
                )
                if not ok:
                    skipped.append((bench_name, comp))
                    continue

            if not json_file.exists():
                continue

            try:
                rows = load_results(json_file)
            except Exception as exc:
                print(f"[SKIP] Failed to parse {json_file}: {exc}")
                skipped.append((bench_name, comp))
                continue

            for dtype, scenario, variant, size, time_ns in rows:
                grouped[scenario][dtype][(comp, variant)].append((size, time_ns))

        if not grouped:
            print(f"[WARN] No data to plot for {bench_name}")
            continue

        plot_article_summary(
            bench_name, title_prefix, grouped, args.out_dir,
            logy, args.metric, include_scalar, args.article_scenario,
        )

        if args.article_only:
            continue

        for scenario, by_type in grouped.items():
            plot_type_grid(bench_name, title_prefix, scenario, by_type, bench_out, logy, args.metric, include_scalar)
            plot_type_compiler_matrix(bench_name, title_prefix, scenario, by_type, bench_out, logy, args.metric, include_scalar)
            plot_speedup_grid(bench_name, title_prefix, scenario, by_type, bench_out)
            plot_geomean_summary(bench_name, title_prefix, scenario, by_type, bench_out, logy, args.metric, include_scalar)
            plot_by_compiler(bench_name, title_prefix, scenario, by_type, bench_out, logy, args.metric, include_scalar)
            plot_speedup(bench_name, title_prefix, scenario, by_type, bench_out)

    if skipped:
        print("\n=== Skipped (configure/build/run failed) ===")
        for item, comp in skipped:
            print(f"  - {item} [{comp}]")


if __name__ == "__main__":
    main()