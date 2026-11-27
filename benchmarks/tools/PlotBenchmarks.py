import argparse
import json
import math
import os
import re
import shutil
import subprocess
import tempfile
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import LinearSegmentedColormap, TwoSlopeNorm
from matplotlib.patches import Patch
from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter, NullLocator

COMPILER_STYLES = {
    "gcc":      {"color": "#2CA02C", "marker": "s", "display": "GCC"},
    "clang":    {"color": "#8E44AD", "marker": "P", "display": "Clang"},
    "clang-cl": {"color": "#E67E22", "marker": "^", "display": "Clang-cl"},
    "msvc":     {"color": "#0078D4", "marker": "o", "display": "MSVC"},
    "icpx":     {"color": "#D62728", "marker": "D", "display": "ICPX"},
}

STL_ACCENT = {
    "libstdc++": "#2E7D32",
    "libc++":    "#6A1B9A",
    "MSVC STL":  "#1565C0",
    "unknown":   "#546E7A",
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
        "ls": ":", "lw": 1.3, "alpha": 0.52, "zorder": 2,
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
        (0.0,  "#c0392b"),
        (0.5,  "#f4f6f7"),
        (0.75, "#7dcea0"),
        (1.0,  "#196f3d"),
    ],
)

NAME_RE = re.compile(r"^(BM_[A-Za-z0-9_]+)(?:<([^>]+)>)?(?:/(\d+(?:/\d+)*))?")

STL_PROBE_SOURCE = r"""
#include <version>
#if defined(_MSVC_STL_VERSION)
RAZE_PROBE_STL=MSVC STL
#  if defined(_MSVC_STL_UPDATE)
RAZE_PROBE_STL_VER=_MSVC_STL_VERSION _MSVC_STL_UPDATE
#  else
RAZE_PROBE_STL_VER=_MSVC_STL_VERSION
#  endif
#elif defined(_LIBCPP_VERSION)
RAZE_PROBE_STL=libc++
RAZE_PROBE_STL_VER=_LIBCPP_VERSION
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
RAZE_PROBE_STL=libstdc++
#  if defined(_GLIBCXX_RELEASE)
RAZE_PROBE_STL_VER=_GLIBCXX_RELEASE __GLIBCXX__
#  else
RAZE_PROBE_STL_VER=__GLIBCXX__
#  endif
#else
RAZE_PROBE_STL=unknown
RAZE_PROBE_STL_VER=unknown
#endif

#if defined(__INTEL_LLVM_COMPILER)
RAZE_PROBE_COMP=ICPX
RAZE_PROBE_COMP_VER=__INTEL_LLVM_COMPILER
#elif defined(__clang__) && defined(_MSC_VER)
RAZE_PROBE_COMP=Clang-cl
RAZE_PROBE_COMP_VER=__clang_major__ __clang_minor__ __clang_patchlevel__
#elif defined(__clang__)
RAZE_PROBE_COMP=Clang
RAZE_PROBE_COMP_VER=__clang_major__ __clang_minor__ __clang_patchlevel__
#elif defined(__GNUC__)
RAZE_PROBE_COMP=GCC
RAZE_PROBE_COMP_VER=__GNUC__ __GNUC_MINOR__ __GNUC_PATCHLEVEL__
#elif defined(_MSC_FULL_VER)
RAZE_PROBE_COMP=MSVC
RAZE_PROBE_COMP_VER=_MSC_FULL_VER
#else
RAZE_PROBE_COMP=Unknown
RAZE_PROBE_COMP_VER=unknown
#endif
"""


@dataclass
class CompilerInfo:
    key: str
    display: str
    color: str
    marker: str
    compiler_version: str = ""
    stl: str = ""
    stl_version: str = ""
    isa: str = ""

    @property
    def label(self) -> str:
        if self.stl and self.stl != "unknown":
            return f"{self.display} ({self.stl})"
        return self.display

    @property
    def short_label(self) -> str:
        if self.stl and self.stl != "unknown":
            return f"{self.display}\n({self.stl})"
        return self.display

    @property
    def detailed_tag(self) -> str:
        comp_part = f"{self.display} {self.compiler_version}".strip()
        if self.stl and self.stl != "unknown":
            stl_part = f"{self.stl} {self.stl_version}".strip() if self.stl_version else self.stl
            return f"{comp_part} [{stl_part}]"
        return comp_part


def make_default_compiler_info(comp: str) -> CompilerInfo:
    base = COMPILER_STYLES.get(comp, {"color": "#607D8B", "marker": "v", "display": comp})
    return CompilerInfo(
        key=comp,
        display=base["display"],
        color=base["color"],
        marker=base["marker"],
    )


def apply_matplotlib_style() -> None:
    plt.rcParams.update({
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.edgecolor": "#CCCCCC",
        "axes.linewidth": 0.9,
        "axes.facecolor": "#FCFCFD",
        "figure.facecolor": "white",
        "font.size": 9.5,
        "legend.framealpha": 0.92,
        "legend.edgecolor": "#DDDDDD",
    })


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
    pos = [v for v in vals if v > 0 and not math.isnan(v)]
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
    if vswhere.exists():
        try:
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
                stderr=subprocess.DEVNULL,
            ).strip()
            if out:
                vcvars = Path(out) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
                if vcvars.exists():
                    return vcvars
        except Exception:
            pass

    candidates: list[Path] = []
    for root_env in ("ProgramFiles", "ProgramFiles(x86)"):
        base = Path(os.environ.get(root_env, "")) / "Microsoft Visual Studio"
        if base.exists():
            candidates.extend(base.glob("*/*/VC/Auxiliary/Build/vcvars64.bat"))
    if candidates:
        candidates.sort(key=lambda p: str(p), reverse=True)
        return candidates[0]
    return None


def load_bat_env(bat_paths: list[Path]) -> dict[str, str]:
    chain = " & ".join(f'call "{p}" >nul 2>&1' for p in bat_paths)
    proc = subprocess.run(
        f'cmd.exe /d /s /c "{chain} & set"',
        capture_output=True,
        text=True,
    )
    env: dict[str, str] = {}
    for line in proc.stdout.splitlines():
        if "=" in line and not line.startswith("*"):
            k, v = line.split("=", 1)
            env[k] = v
    return env if env else dict(os.environ)


def prepend_existing_path(env: dict[str, str], candidate: Path) -> None:
    if not candidate.exists():
        return
    path_key = next((k for k in env if k.upper() == "PATH"), "PATH")
    cur = env.get(path_key, "")
    if str(candidate).lower() not in cur.lower():
        env[path_key] = f"{candidate};{cur}" if cur else str(candidate)


def env_for_compiler(comp: str) -> dict[str, str]:
    if os.name != "nt":
        env = dict(os.environ)
        for key in list(env):
            if key.upper() in ("CC", "CXX"):
                del env[key]
        return env

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


def _format_probe_stl_version(stl: str, raw: str) -> str:
    tokens = [t for t in raw.split() if t.isdigit()]
    if not tokens:
        return ""
    if stl == "MSVC STL":
        return f"{tokens[0]} ({tokens[1]})" if len(tokens) >= 2 else tokens[0]
    if stl == "libc++":
        v = int(tokens[0])
        return f"{v // 10000}.{(v // 100) % 100}.{v % 100}"
    if stl == "libstdc++":
        return f"{tokens[0]} ({tokens[1]})" if len(tokens) >= 2 else tokens[0]
    return raw


def _format_probe_comp_version(comp_name: str, raw: str) -> str:
    tokens = [t for t in raw.split() if t.isdigit()]
    if not tokens:
        return ""
    if comp_name == "ICPX":
        v = int(tokens[0])
        return f"{v // 10000}.{(v % 10000) // 100}"
    if comp_name == "MSVC":
        v = int(tokens[0])
        return f"{v // 10000000}.{(v // 100000) % 100}.{v % 100000}"
    if len(tokens) >= 3:
        return f"{tokens[0]}.{tokens[1]}.{tokens[2]}"
    return ".".join(tokens)


def _compiler_from_cmake_cache(build_dir: Path) -> str | None:
    cache = build_dir / "CMakeCache.txt"
    if not cache.exists():
        return None
    for line in cache.read_text(encoding="utf-8", errors="ignore").splitlines():
        if line.startswith("CMAKE_CXX_COMPILER:"):
            _, val = line.split("=", 1)
            val = val.strip()
            if val:
                return val
    return None


def probe_compiler_metadata(
    comp: str,
    env: dict[str, str] | None = None,
    build_dir: Path | None = None,
) -> CompilerInfo:
    info = make_default_compiler_info(comp)
    if env is None:
        try:
            env = env_for_compiler(comp)
        except Exception:
            return info

    path_key = next((k for k in env if k.upper() == "PATH"), "PATH")
    path_val = env.get(path_key)

    cxx = _compiler_from_cmake_cache(build_dir) if build_dir else None
    if not cxx:
        candidates = {
            "gcc":      ["g++"],
            "clang":    ["clang++"],
            "clang-cl": ["clang-cl"],
            "msvc":     ["cl"],
            "icpx":     ["icx", "icpx"] if os.name == "nt" else ["icpx", "icx"],
        }.get(comp, [comp])
        for cand in candidates:
            found = shutil.which(cand, path=path_val)
            if found:
                cxx = found
                break

    if not cxx:
        return info

    cxx_low = Path(cxx).name.lower()
    is_msvc_driver = cxx_low in ("cl", "cl.exe", "clang-cl", "clang-cl.exe") or (
        os.name == "nt" and cxx_low in ("icx", "icx.exe")
    )

    tmp_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False, encoding="utf-8") as tmp:
            tmp.write(STL_PROBE_SOURCE)
            tmp_path = Path(tmp.name)

        if is_msvc_driver:
            cmd = [cxx, "/nologo", "/std:c++latest", "/EP", "/TP", str(tmp_path)]
        else:
            cmd = [cxx, "-std=c++23", "-E", "-P", "-x", "c++", str(tmp_path)]

        proc = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=15)
        if proc.returncode != 0:
            return info

        parsed: dict[str, str] = {}
        for line in proc.stdout.splitlines():
            s = line.strip()
            if s.startswith("RAZE_PROBE_") and "=" in s:
                k, v = s.split("=", 1)
                parsed[k.strip()] = v.strip()

        stl = parsed.get("RAZE_PROBE_STL", "")
        if stl and stl != "unknown":
            info.stl = stl
            info.stl_version = _format_probe_stl_version(stl, parsed.get("RAZE_PROBE_STL_VER", ""))

        comp_name = parsed.get("RAZE_PROBE_COMP", "")
        if comp_name and comp_name != "Unknown":
            info.compiler_version = _format_probe_comp_version(
                comp_name, parsed.get("RAZE_PROBE_COMP_VER", "")
            )
    except Exception:
        pass
    finally:
        if tmp_path is not None:
            tmp_path.unlink(missing_ok=True)

    return info


def merge_context_metadata(info: CompilerInfo, context: dict) -> CompilerInfo:
    stl = context.get("raze_stl", "")
    if stl and stl != "unknown":
        info.stl = stl
    stl_ver = context.get("raze_stl_version", "")
    if stl_ver and stl_ver != "unknown":
        info.stl_version = stl_ver
    comp_ver = context.get("raze_compiler_version", "")
    if comp_ver and comp_ver != "unknown":
        info.compiler_version = comp_ver
    isa = context.get("raze_isa", "")
    if isa:
        info.isa = isa
    return info


def classify_variant(bm_name: str) -> str:
    low = bm_name.lower()
    if "scalar" in low:
        return "raze_scalar"
    if "std" in low:
        return "std"
    return "raze"


def discover_benchmarks(root: Path) -> list[str]:
    algo_dir = root / "benchmarks" / "algorithm"
    if algo_dir.exists():
        return sorted(p.stem for p in algo_dir.rglob("*.cpp") if p.stem != "main")
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
    min_time: str | None = None,
    repetitions: int | None = None,
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
    if min_time:
        cmd.append(f"--benchmark_min_time={min_time}")
    if repetitions is not None:
        cmd.append(f"--benchmark_repetitions={repetitions}")

    res = subprocess.run(cmd, env=env)
    if res.returncode != 0:
        print(f"[SKIP] Benchmark execution failed: {exe_name} ({comp})")
        return False

    return True


def load_results(json_path: Path) -> tuple[list[tuple[str, str, str, int, float]], dict]:
    data = json.loads(json_path.read_text(encoding="utf-8"))
    context = data.get("context", {})
    benchmarks = data.get("benchmarks", [])

    has_aggregates = any(e.get("run_type") == "aggregate" for e in benchmarks)

    rows = []
    for entry in benchmarks:
        run_type = entry.get("run_type", "iteration")
        if has_aggregates:
            if run_type != "aggregate" or entry.get("aggregate_name") != "median":
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

        low = fn_name.lower()
        if "ifnot" in low or "prednot" in low:
            scenario_parts.append("if_not")
        elif "pred" in low:
            scenario_parts.append("pred")
        if "comp" in low:
            scenario_parts.append("comp")
        if "binary" in low:
            scenario_parts.append("binary")
        if "overlap" in low:
            scenario_parts.append("overlap")

        scenario = ", ".join(scenario_parts) if scenario_parts else "default"
        scale = TIME_UNIT_TO_NS.get(entry.get("time_unit", "ns"), 1.0)
        time_ns = entry.get("real_time", 0.0) * scale
        rows.append((dtype, scenario, classify_variant(fn_name), size, time_ns))

    return rows, context


def ordered_types(by_type: dict) -> list[str]:
    return [t for t in TYPE_ORDER if t in by_type] + [t for t in by_type if t not in TYPE_ORDER]


def ordered_active_compilers(
    by_type: dict,
    comp_infos: dict[str, CompilerInfo],
    require_both: bool = False,
) -> list[str]:
    active = []
    for comp in comp_infos:
        has_raze = any((comp, "raze") in series for series in by_type.values())
        has_std = any((comp, "std") in series for series in by_type.values())
        if require_both:
            if has_raze and has_std:
                active.append(comp)
        elif has_raze or has_std:
            active.append(comp)
    return active


def style_axes(ax, xs: list[int], xlabel: str | None, ylabel: str | None, logy: bool) -> None:
    ax.set_xscale("log", base=2)
    if xs:
        ax.set_xticks(sorted(set(xs)))
    ax.xaxis.set_minor_locator(NullLocator())
    ax.xaxis.set_major_formatter(FuncFormatter(fmt_size))
    ax.margins(x=0.04, y=0.08)

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
    ax.grid(True, which="major", linestyle="--", alpha=0.40)
    ax.grid(True, which="minor", linestyle=":", alpha=0.16)


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


def format_environment_footer(active_comps: list[str], comp_infos: dict[str, CompilerInfo]) -> str:
    parts = [comp_infos[c].detailed_tag for c in active_comps if c in comp_infos]
    isas = sorted({comp_infos[c].isa for c in active_comps if c in comp_infos and comp_infos[c].isa})
    footer = "Toolchains: " + "  |  ".join(parts) if parts else ""
    if isas:
        footer += f"   •   ISA: {', '.join(isas)}"
    return footer


def plot_line(ax, xs, ys, color: str, marker: str, v_cfg: dict, label: str, scale_ms: float = 1.0) -> None:
    mk = v_cfg.get("force_marker", marker)
    if mk == "x":
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
        marker=mk,
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
        interpolate=True, color="#2CA02C", alpha=0.11, zorder=1,
    )
    ax.fill_between(
        cxs, rys, sys_,
        where=[s < r for r, s in zip(rys, sys_)],
        interpolate=True, color="#D62728", alpha=0.13, zorder=1,
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
        bbox=dict(boxstyle="round,pad=0.18", facecolor="white", edgecolor=edge_col, lw=0.9, alpha=0.92),
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


def compute_best_vs_best_series(geo: dict, comps: list[str]) -> tuple[list[int], list[float]]:
    r_by_size: dict[int, list[float]] = defaultdict(list)
    s_by_size: dict[int, list[float]] = defaultdict(list)
    for comp in comps:
        if (comp, "raze") in geo:
            for x, y in zip(*geo[(comp, "raze")]):
                if y > 0:
                    r_by_size[x].append(y)
        if (comp, "std") in geo:
            for x, y in zip(*geo[(comp, "std")]):
                if y > 0:
                    s_by_size[x].append(y)

    xs = sorted(set(r_by_size) & set(s_by_size))
    spds = [min(s_by_size[x]) / min(r_by_size[x]) for x in xs]
    return xs, spds


def _draw_compiler_head_to_head_bars(
    ax,
    geo: dict,
    active_comps: list[str],
    comp_infos: dict[str, CompilerInfo],
    metric: str,
    logy: bool,
    include_scalar: bool,
    y_prefix: str = "",
) -> None:
    if not active_comps:
        ax.text(0.5, 0.5, "No benchmark data", ha="center", va="center")
        ax.set_axis_off()
        return

    raze_vals = []
    std_vals = []
    scalar_vals = []
    for c in active_comps:
        raze_vals.append(geomean(geo[(c, "raze")][1]) if (c, "raze") in geo else float("nan"))
        std_vals.append(geomean(geo[(c, "std")][1]) if (c, "std") in geo else float("nan"))
        scalar_vals.append(geomean(geo[(c, "raze_scalar")][1]) if (c, "raze_scalar") in geo else float("nan"))

    has_scalar = include_scalar and any(not math.isnan(v) for v in scalar_vals)
    x = np.arange(len(active_comps), dtype=float)
    width = 0.26 if has_scalar else 0.34

    if has_scalar:
        r_pos = x - width
        s_pos = x
        sc_pos = x + width
    else:
        r_pos = x - width / 2.0
        s_pos = x + width / 2.0
        sc_pos = None

    colors = [comp_infos[c].color for c in active_comps]

    ax.bar(
        r_pos,
        [0.0 if math.isnan(v) else v for v in raze_vals],
        width=width,
        color=colors,
        edgecolor="#1A1A1A",
        linewidth=0.75,
        alpha=0.94,
        zorder=3,
    )
    ax.bar(
        s_pos,
        [0.0 if math.isnan(v) else v for v in std_vals],
        width=width,
        color="white",
        edgecolor=colors,
        hatch="///",
        linewidth=1.4,
        alpha=0.95,
        zorder=3,
    )
    if has_scalar and sc_pos is not None:
        ax.bar(
            sc_pos,
            [0.0 if math.isnan(v) else v for v in scalar_vals],
            width=width * 0.88,
            color="#B0BEC5",
            edgecolor="#607D8B",
            linewidth=0.7,
            alpha=0.75,
            zorder=3,
        )

    valid_std = [v for v in std_vals if not math.isnan(v) and v > 0]
    valid_all = [v for v in (raze_vals + std_vals + (scalar_vals if has_scalar else [])) if not math.isnan(v) and v > 0]

    if valid_std:
        best_std = min(valid_std)
        ax.axhline(
            best_std,
            color="#37474F",
            linestyle=":",
            linewidth=1.25,
            zorder=4,
        )
        ax.text(
            len(active_comps) - 0.52,
            best_std * (1.06 if logy else 1.02),
            f"fastest std ({best_std:.2g} ns)",
            ha="right",
            va="bottom",
            fontsize=7.6,
            fontweight="bold",
            color="#37474F",
            bbox=dict(boxstyle="round,pad=0.14", facecolor="white", edgecolor="#B0BEC5", lw=0.7, alpha=0.88),
            zorder=6,
        )

    if logy and valid_all:
        ax.set_yscale("log")
        ymin = min(valid_all) * 0.55
        ymax = max(valid_all) * 2.15
        ax.set_ylim(ymin, ymax)
        ax.yaxis.set_major_formatter(FuncFormatter(fmt_y))
        ax.yaxis.set_minor_formatter(NullFormatter())
    elif valid_all:
        ax.set_ylim(0.0, max(valid_all) * 1.28)

    for idx, (rv, sv) in enumerate(zip(raze_vals, std_vals)):
        if math.isnan(rv) or math.isnan(sv) or rv <= 0 or sv <= 0:
            continue
        spd = sv / rv
        top_v = max(rv, sv, scalar_vals[idx] if (has_scalar and not math.isnan(scalar_vals[idx])) else 0.0)
        y_badge = top_v * (1.16 if logy else 1.05)
        txt = f"{spd:.1f}×" if spd >= 2.0 else f"{spd:.2f}×"
        ec = "#1B7F3A" if spd >= 1.0 else "#C0392B"
        fc = "#EBF9EE" if spd >= 1.0 else "#FDEDEC"
        ax.text(
            x[idx],
            y_badge,
            txt,
            ha="center",
            va="bottom",
            fontsize=8.3,
            fontweight="bold",
            color=ec,
            bbox=dict(boxstyle="round,pad=0.18", facecolor=fc, edgecolor=ec, lw=0.85),
            zorder=7,
        )

    stls = [comp_infos[c].stl for c in active_comps]
    for i in range(1, len(active_comps)):
        if stls[i] and stls[i - 1] and stls[i] != stls[i - 1]:
            ax.axvline(i - 0.5, color="#BDBDBD", linestyle="--", linewidth=1.0, zorder=2)

    ax.set_xticks(x)
    ax.set_xticklabels([comp_infos[c].short_label for c in active_comps], fontsize=8.3, fontweight="bold")
    ax.set_ylabel(f"{y_prefix}Geomean {metric_label(metric)} (lower is better)")
    ax.grid(True, axis="y", which="major", linestyle="--", alpha=0.38, zorder=0)

    legend_handles = [
        Patch(facecolor="#455A64", edgecolor="#1A1A1A", label="raze (solid)"),
        Patch(facecolor="white", edgecolor="#455A64", hatch="///", linewidth=1.2, label="std (hatched)"),
    ]
    if has_scalar:
        legend_handles.append(
            Patch(facecolor="#B0BEC5", edgecolor="#607D8B", label="raze[fscalar]")
        )
    ax.legend(handles=legend_handles, loc="upper left", fontsize=7.6, frameon=True)


def plot_article_summary(
    bench_name: str,
    title_prefix: str,
    grouped: dict,
    comp_infos: dict[str, CompilerInfo],
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
    best_vs_best_ratios: list[float] = []
    used_comps: list[str] = []

    for _, by_type in scenarios:
        for c in ordered_active_compilers(by_type, comp_infos):
            if c not in used_comps:
                used_comps.append(c)
        for series in by_type.values():
            r_by_size: dict[int, list[float]] = defaultdict(list)
            s_by_size: dict[int, list[float]] = defaultdict(list)
            for comp in comp_infos:
                r_pts = dict(series.get((comp, "raze"), []))
                s_pts = dict(series.get((comp, "std"), []))
                for x, v in r_pts.items():
                    if v > 0:
                        r_by_size[x].append(v)
                for x, v in s_pts.items():
                    if v > 0:
                        s_by_size[x].append(v)
                for x in set(r_pts) & set(s_pts):
                    if r_pts[x] > 0 and s_pts[x] > 0:
                        all_ratios.append(s_pts[x] / r_pts[x])
            for x in set(r_by_size) & set(s_by_size):
                best_vs_best_ratios.append(min(s_by_size[x]) / min(r_by_size[x]))

    overall_geo = geomean(all_ratios)
    overall_peak = max(all_ratios) if all_ratios else float("nan")
    best_geo = geomean(best_vs_best_ratios)

    n_rows = len(scenarios)
    fig, axes = plt.subplots(
        n_rows, 3,
        figsize=(19.2, 4.8 * n_rows + 1.2),
        dpi=160,
        gridspec_kw={"width_ratios": [1.18, 1.15, 1.65]},
        squeeze=False,
        layout="constrained",
    )

    for row_idx, (scenario, by_type) in enumerate(scenarios):
        ax_bar, ax_spd, ax_hm = axes[row_idx]
        geo = compute_geomean_series(by_type, metric)
        active_comps = ordered_active_compilers(by_type, comp_infos)
        paired_comps = ordered_active_compilers(by_type, comp_infos, require_both=True)

        y_prefix = f"[{scenario}]\n" if n_rows > 1 else ""

        _draw_compiler_head_to_head_bars(
            ax_bar, geo, active_comps, comp_infos, metric, logy, include_scalar, y_prefix=y_prefix
        )
        if row_idx == 0:
            ax_bar.set_title("Compiler & STL Head-to-Head (raze vs std)", fontsize=11, fontweight="bold")

        spd_xs: list[int] = []
        min_spd, max_spd = 1.0, 1.2

        for comp in paired_comps:
            c_info = comp_infos[comp]
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
                color=c_info.color, marker=c_info.marker,
                markersize=5.6, markeredgecolor="white", markeredgewidth=0.6,
                linewidth=2.2,
                label=f"{c_info.label}: {geomean(spds):.1f}× (max {max(spds):.1f}×)",
            )

        if len(paired_comps) >= 2:
            b_xs, b_spds = compute_best_vs_best_series(geo, paired_comps)
            if b_xs:
                spd_xs.extend(b_xs)
                min_spd = min(min_spd, min(b_spds))
                max_spd = max(max_spd, max(b_spds))
                ax_spd.plot(
                    b_xs, b_spds,
                    color="#111111", linestyle="--", marker="D",
                    markersize=4.8, markerfacecolor="white", markeredgecolor="#111111",
                    linewidth=1.9, zorder=6,
                    label=f"Best raze vs Best std: {geomean(b_spds):.1f}×",
                )

        ax_spd.axhline(1.0, color="#444444", linestyle=":", linewidth=1.15)
        use_log2_spd = max_spd > 10.0
        if use_log2_spd:
            ax_spd.set_yscale("log", base=2)
            bottom = min(0.5, max(0.125, min_spd * 0.85))
            top = max_spd * 1.25
        else:
            bottom = min(0.75, max(0.0, min_spd * 0.88))
            top = max_spd * 1.16

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
            ax_spd.set_title("Speedup vs std by Array Size", fontsize=11, fontweight="bold")
        ax_spd.legend(loc="best", fontsize=7.6, frameon=True)

        dtypes = ordered_types(by_type)
        if not dtypes or not paired_comps:
            ax_hm.text(0.5, 0.5, "No paired raze/std data", ha="center", va="center")
            ax_hm.set_axis_off()
            continue

        add_best_col = len(paired_comps) > 1
        n_data_cols = len(paired_comps)
        mat = np.full((len(dtypes), n_data_cols), np.nan, dtype=float)
        best_col = np.full((len(dtypes), 1), np.nan, dtype=float)

        for r, dt in enumerate(dtypes):
            series = by_type[dt]
            r_all_by_size: dict[int, list[float]] = defaultdict(list)
            s_all_by_size: dict[int, list[float]] = defaultdict(list)

            for c_idx, comp in enumerate(paired_comps):
                r_pts = dict(series.get((comp, "raze"), []))
                s_pts = dict(series.get((comp, "std"), []))
                for x, v in r_pts.items():
                    if v > 0:
                        r_all_by_size[x].append(v)
                for x, v in s_pts.items():
                    if v > 0:
                        s_all_by_size[x].append(v)
                ratios = [
                    s_pts[x] / r_pts[x]
                    for x in set(r_pts) & set(s_pts)
                    if r_pts[x] > 0 and s_pts[x] > 0
                ]
                if ratios:
                    mat[r, c_idx] = geomean(ratios)

            if add_best_col:
                b_ratios = [
                    min(s_all_by_size[x]) / min(r_all_by_size[x])
                    for x in set(r_all_by_size) & set(s_all_by_size)
                ]
                if b_ratios:
                    best_col[r, 0] = geomean(b_ratios)

        row_labels = [dt.replace("raze::", "") for dt in dtypes]
        col_labels = [comp_infos[c].short_label for c in paired_comps]

        if add_best_col:
            col_geo = np.array([
                geomean([v for v in mat[r, :] if not np.isnan(v)])
                for r in range(mat.shape[0])
            ])[:, None]
            mat = np.hstack([mat, best_col, col_geo])
            col_labels.extend(["Best vs\nBest std", "Geomean"])

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
        ax_hm.set_xticklabels(col_labels, fontsize=8.0, fontweight="bold")
        ax_hm.set_yticks(range(len(row_labels)))
        ax_hm.set_yticklabels(row_labels, fontsize=9.3, fontweight="bold")

        ax_hm.set_xticks(np.arange(-0.5, len(col_labels), 1), minor=True)
        ax_hm.set_yticks(np.arange(-0.5, len(row_labels), 1), minor=True)
        ax_hm.grid(which="minor", color="white", linestyle="-", linewidth=1.6)
        ax_hm.tick_params(which="minor", bottom=False, left=False)

        if add_best_col:
            ax_hm.axvline(n_data_cols - 0.5, color="#222222", linewidth=1.8)
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
                is_summary = (add_best_col and c >= n_data_cols) or (
                    len(dtypes) > 1 and r == mat.shape[0] - 1
                )
                ax_hm.text(
                    c, r, label_txt,
                    ha="center", va="center",
                    color=t_col,
                    fontsize=9.0 if is_summary else 8.5,
                    fontweight="bold" if (is_summary or val >= 1.5) else "normal",
                )

        if row_idx == 0:
            ax_hm.set_title("Geomean Speedup (Type × Compiler & STL)", fontsize=11, fontweight="bold")

    headline_parts = [title_prefix]
    if not math.isnan(overall_geo):
        headline_parts.append(f"Geomean: {overall_geo:.1f}× vs std (peak {overall_peak:.1f}×)")
    if len(used_comps) >= 2 and not math.isnan(best_geo):
        headline_parts.append(f"Best raze vs Best std: {best_geo:.1f}×")

    fig.suptitle("  |  ".join(headline_parts), fontsize=13.5, fontweight="bold")
    footer = format_environment_footer(used_comps, comp_infos)
    if footer:
        fig.supxlabel(footer, fontsize=8.3, color="#455A64")

    chart_path = out_dir / f"{bench_name}_article.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_stl_comparison(
    bench_name: str,
    title_prefix: str,
    scenario: str,
    by_type: dict,
    comp_infos: dict[str, CompilerInfo],
    out_dir: Path,
    logy: bool,
    metric: str,
    include_scalar: bool,
) -> None:
    geo = compute_geomean_series(by_type, metric)
    active_comps = ordered_active_compilers(by_type, comp_infos)
    paired_comps = ordered_active_compilers(by_type, comp_infos, require_both=True)
    if not active_comps:
        return

    fig, (ax_left, ax_right) = plt.subplots(
        1, 2,
        figsize=(16.5, 6.2),
        dpi=160,
        gridspec_kw={"width_ratios": [1.35, 1.0]},
        layout="constrained",
    )

    all_xs: list[int] = []
    variants = active_variants(include_scalar)
    for comp in active_comps:
        c_info = comp_infos[comp]
        for var, v_cfg in variants.items():
            if (comp, var) not in geo:
                continue
            xs, ys = geo[(comp, var)]
            all_xs.extend(xs)
            plot_line(
                ax_left, xs, ys, c_info.color, c_info.marker, v_cfg,
                f"{c_info.label} — {v_cfg['label']}", scale_ms=1.0,
            )

    style_axes(ax_left, all_xs, "Elements (Size)", f"Geomean {metric_label(metric)} (lower is better)", logy)
    ax_left.set_title("raze (solid) vs std (dashed) across Compilers & STLs", fontsize=11, fontweight="bold")
    ax_left.legend(loc="best", fontsize=7.8, ncol=2 if len(active_comps) >= 3 else 1, frameon=True)

    if paired_comps:
        n = len(paired_comps)
        cross = np.full((n, n), np.nan, dtype=float)
        for r, rc in enumerate(paired_comps):
            r_map = dict(zip(*geo[(rc, "raze")]))
            for c, sc in enumerate(paired_comps):
                s_map = dict(zip(*geo[(sc, "std")]))
                ratios = [
                    s_map[x] / r_map[x]
                    for x in set(r_map) & set(s_map)
                    if r_map[x] > 0 and s_map[x] > 0
                ]
                if ratios:
                    cross[r, c] = geomean(ratios)

        valid = cross[~np.isnan(cross) & (cross > 0)]
        log_cross = np.where(np.isnan(cross) | (cross <= 0), np.nan, np.log2(np.maximum(cross, 1e-9)))
        vmin = min(-0.6, float(np.nanmin(log_cross)) * 1.05) if len(valid) else -1.0
        vmax = max(1.0, float(np.nanmax(log_cross)) * 1.05) if len(valid) else 2.0
        norm = TwoSlopeNorm(vmin=vmin, vcenter=0.0, vmax=vmax)

        cmap = SPEEDUP_CMAP.copy()
        cmap.set_bad(color="#eeeeee")
        ax_right.imshow(np.ma.masked_invalid(log_cross), cmap=cmap, norm=norm, aspect="auto")

        row_lbls = [f"raze\n{comp_infos[c].display}" for c in paired_comps]
        col_lbls = [f"std\n{comp_infos[c].short_label}" for c in paired_comps]
        ax_right.set_xticks(range(n))
        ax_right.set_xticklabels(col_lbls, fontsize=8.0, fontweight="bold")
        ax_right.set_yticks(range(n))
        ax_right.set_yticklabels(row_lbls, fontsize=8.5, fontweight="bold")

        ax_right.set_xticks(np.arange(-0.5, n, 1), minor=True)
        ax_right.set_yticks(np.arange(-0.5, n, 1), minor=True)
        ax_right.grid(which="minor", color="white", linestyle="-", linewidth=1.8)
        ax_right.tick_params(which="minor", bottom=False, left=False)

        for r in range(n):
            for c in range(n):
                val = cross[r, c]
                if np.isnan(val):
                    continue
                nv = norm(log_cross[r, c])
                t_col = "white" if (nv < 0.22 or nv > 0.82) else "#111111"
                txt = f"{val:.1f}×" if val >= 2.0 else f"{val:.2f}×"
                ax_right.text(
                    c, r, txt,
                    ha="center", va="center",
                    color=t_col,
                    fontsize=9.3 if r == c else 8.6,
                    fontweight="bold" if (r == c or val >= 1.5) else "normal",
                )
        ax_right.set_title("Cross-Compiler / Cross-STL Speedup (std[col] / raze[row])", fontsize=10.5, fontweight="bold")
    else:
        ax_right.text(0.5, 0.5, "Need paired raze/std runs", ha="center", va="center")
        ax_right.set_axis_off()

    fig.suptitle(f"{title_prefix}  |  Compiler & STL Comparison  |  {scenario}", fontsize=13, fontweight="bold")
    footer = format_environment_footer(active_comps, comp_infos)
    if footer:
        fig.supxlabel(footer, fontsize=8.2, color="#455A64")

    chart_path = out_dir / safe_filename("stl_compare", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_type_grid(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    comp_infos: dict[str, CompilerInfo],
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    dtypes = ordered_types(by_type)
    active_comps = ordered_active_compilers(by_type, comp_infos)
    if not dtypes or not active_comps:
        return

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

        for comp in active_comps:
            c_info = comp_infos[comp]
            for var, v_cfg in variants.items():
                pts = sorted(series.get((comp, var), []), key=lambda x: x[0])
                if not pts:
                    continue
                xs = [p[0] for p in pts]
                ys = [metric_value(p[1], p[0], metric) for p in pts]
                all_xs.extend(xs)
                plot_line(
                    ax, xs, ys, c_info.color, c_info.marker, v_cfg,
                    f"{c_info.label} {v_cfg['label']}", scale_ms=0.9,
                )

        style_axes(ax, all_xs, "Elements (Size)", metric_label(metric), logy)
        ax.set_title(dtype, fontsize=11, fontweight="bold")

        if legend_handles is None and all_xs:
            legend_handles, legend_labels = ax.get_legend_handles_labels()

    for idx in range(len(dtypes), len(axes_flat)):
        axes_flat[idx].set_visible(False)

    if legend_handles:
        fig.legend(
            legend_handles, legend_labels,
            loc="upper center", ncol=min(5, max(2, cols * 2)), fontsize=7.8,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{title_prefix}  |  {scenario}", fontsize=13, fontweight="bold", y=0.99)
    fig.tight_layout(rect=[0, 0.02, 1, 0.855])
    footer = format_environment_footer(active_comps, comp_infos)
    if footer:
        fig.text(0.5, 0.005, footer, ha="center", va="bottom", fontsize=7.8, color="#455A64")

    chart_path = out_dir / safe_filename("grid", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_type_compiler_matrix(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    comp_infos: dict[str, CompilerInfo],
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    dtypes = ordered_types(by_type)
    active_comps = ordered_active_compilers(by_type, comp_infos)
    if not dtypes or not active_comps:
        return

    variants = active_variants(include_scalar)
    variant_colors = {"raze": "#0078D4", "std": "#D62728", "raze_scalar": "#7F7F7F"}

    rows, cols = len(dtypes), len(active_comps)
    fig, axes = plt.subplots(
        rows, cols, figsize=(4.1 * cols, 3.3 * rows), dpi=150,
        sharex=True, sharey="row", squeeze=False,
    )

    legend_handles = None
    legend_labels = None
    cell_xs: dict[tuple[int, int], list[int]] = {}

    for r, dtype in enumerate(dtypes):
        series = by_type[dtype]
        for c, comp in enumerate(active_comps):
            c_info = comp_infos[comp]
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
                plot_line(ax, xs, ys, variant_colors[var], c_info.marker, v_cfg, v_cfg["label"], scale_ms=0.88)

            if "raze" in plotted and "std" in plotted:
                shade_and_annotate_gap(ax, *plotted["raze"], *plotted["std"], logy)

            cell_xs[(r, c)] = all_xs
            if r == 0:
                ax.set_title(c_info.label, fontsize=10.5, fontweight="bold")
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

    fig.suptitle(f"{title_prefix}  |  Matrix (Type × Compiler & STL)  |  {scenario}", fontsize=13, fontweight="bold", y=0.992)
    fig.tight_layout(rect=[0, 0, 1, 0.94])

    chart_path = out_dir / safe_filename("matrix", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup_grid(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    comp_infos: dict[str, CompilerInfo], out_dir: Path,
) -> None:
    dtypes = ordered_types(by_type)
    paired_comps = ordered_active_compilers(by_type, comp_infos, require_both=True)
    if not dtypes or not paired_comps:
        return

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

        for comp in paired_comps:
            c_info = comp_infos[comp]
            r_pts = dict(series.get((comp, "raze"), []))
            s_pts = dict(series.get((comp, "std"), []))
            xs = sorted(x for x in (set(r_pts) & set(s_pts)) if r_pts[x] > 0)
            ys = [s_pts[x] / r_pts[x] for x in xs]
            if not xs:
                continue
            all_xs.extend(xs)
            min_y = min(min_y, min(ys))
            max_y = max(max_y, max(ys))
            ax.plot(
                xs, ys,
                color=c_info.color, marker=c_info.marker,
                markersize=5.6, markeredgecolor="white", markeredgewidth=0.6,
                linewidth=2.2, label=c_info.label,
            )

        ax.axhline(1.0, color="#444444", linestyle="--", linewidth=1.1, label="1.0× (parity)")
        bottom = min(0.7, max(0.0, min_y * 0.9))
        top = max_y * 1.14
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
            loc="upper center", ncol=min(6, max(3, cols * 2)), fontsize=8.3,
            bbox_to_anchor=(0.5, 0.955), frameon=True,
        )

    fig.suptitle(f"{title_prefix}  |  Speedup raze vs std  |  {scenario}", fontsize=13, fontweight="bold", y=0.99)
    fig.tight_layout(rect=[0, 0, 1, 0.87])

    chart_path = out_dir / safe_filename("grid_speedup", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_by_compiler(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    comp_infos: dict[str, CompilerInfo],
    out_dir: Path, logy: bool, metric: str, include_scalar: bool,
) -> None:
    geo = compute_geomean_series(by_type, metric)
    active_comps = ordered_active_compilers(by_type, comp_infos)
    if not active_comps:
        return

    variants = active_variants(include_scalar)
    n_comps = len(active_comps)
    cols = min(3, n_comps)
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

    for idx, comp in enumerate(active_comps):
        c_info = comp_infos[comp]
        ax = axes_flat[idx]
        all_xs: list[int] = []
        plotted: dict[str, tuple[list[int], list[float]]] = {}

        for var, v_cfg in variants.items():
            if (comp, var) not in geo:
                continue
            xs, ys = geo[(comp, var)]
            all_xs.extend(xs)
            plotted[var] = (xs, ys)
            plot_line(ax, xs, ys, variant_colors[var], c_info.marker, v_cfg, v_cfg["label"], scale_ms=1.0)

        if "raze" in plotted and "std" in plotted:
            shade_and_annotate_gap(ax, *plotted["raze"], *plotted["std"], logy)

        panel_xs.append(all_xs)
        ax.set_title(c_info.label, fontsize=11, fontweight="bold")
        ax.legend(loc="best", fontsize=8.8)

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

    fig.suptitle(f"{title_prefix}  |  By Compiler & STL (Geomean)  |  {scenario}", fontsize=13, fontweight="bold", y=0.985)
    fig.tight_layout(rect=[0, 0, 1, 0.95])

    chart_path = out_dir / safe_filename("by_compiler", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_speedup(
    bench_name: str, title_prefix: str, scenario: str, by_type: dict,
    comp_infos: dict[str, CompilerInfo], out_dir: Path,
) -> None:
    geo = compute_geomean_series(by_type, "ns")
    paired_comps = ordered_active_compilers(by_type, comp_infos, require_both=True)
    if not paired_comps:
        return

    fig, ax = plt.subplots(figsize=(11.4, 6.2), dpi=150)
    all_xs: list[int] = []
    min_y, max_y = 1.0, 1.2

    for comp in paired_comps:
        c_info = comp_infos[comp]
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
            xs, speedups,
            color=c_info.color, marker=c_info.marker,
            markersize=6.2, markeredgecolor="white", markeredgewidth=0.7,
            linewidth=2.4,
            label=f"{c_info.label}: geo {geomean(speedups):.1f}×, max {max(speedups):.1f}×",
        )

    if len(paired_comps) >= 2:
        b_xs, b_spds = compute_best_vs_best_series(geo, paired_comps)
        if b_xs:
            all_xs.extend(b_xs)
            min_y = min(min_y, min(b_spds))
            max_y = max(max_y, max(b_spds))
            ax.plot(
                b_xs, b_spds,
                color="#111111", linestyle="--", marker="D",
                markersize=5.2, markerfacecolor="white", markeredgecolor="#111111",
                linewidth=2.0,
                label=f"Best raze vs Best std: geo {geomean(b_spds):.1f}×",
            )

    ax.axhline(1.0, color="#444444", linestyle=":", linewidth=1.2, label="1.0× (parity)")
    if max_y > 10.0:
        ax.set_yscale("log", base=2)
        bottom = min(0.5, max(0.125, min_y * 0.85))
        top = max_y * 1.22
    else:
        bottom = min(0.8, max(0.0, min_y * 0.9))
        top = max_y * 1.14
    ax.set_ylim(bottom=bottom, top=top)
    ax.axhspan(bottom, 1.0, color="#d62728", alpha=0.06, zorder=0)
    ax.axhspan(1.0, top, color="#2ca02c", alpha=0.04, zorder=0)
    style_axes(ax, all_xs, "Elements (Size)", "Speedup vs std (higher is better)", logy=False)
    ax.yaxis.set_major_formatter(FuncFormatter(fmt_speedup))
    ax.set_title(f"{title_prefix}  |  Speedup raze vs std (Geomean)  |  {scenario}", fontweight="bold")
    ax.legend(loc="best", fontsize=8.8, frameon=True)
    footer = format_environment_footer(paired_comps, comp_infos)
    if footer:
        fig.text(0.5, 0.006, footer, ha="center", va="bottom", fontsize=7.8, color="#455A64")
    fig.tight_layout(rect=[0, 0.03, 1, 1])

    chart_path = out_dir / safe_filename("speedup", bench_name, scenario)
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def plot_overall_summary(
    bench_summaries: list[tuple[str, dict[str, float], float]],
    comp_infos: dict[str, CompilerInfo],
    suffix: str,
    out_dir: Path,
) -> None:
    if len(bench_summaries) < 2:
        return

    active_comps = [
        c for c in comp_infos
        if any(c in per_comp and not math.isnan(per_comp[c]) for _, per_comp, _ in bench_summaries)
    ]
    if not active_comps:
        return

    add_best = len(active_comps) > 1
    n_rows = len(bench_summaries)
    n_cols = len(active_comps)

    mat = np.full((n_rows, n_cols), np.nan, dtype=float)
    best_col = np.full((n_rows, 1), np.nan, dtype=float)
    row_labels = []

    for r, (bench_name, per_comp, best_spd) in enumerate(bench_summaries):
        row_labels.append(bench_name.removesuffix("Benchmark"))
        for c, comp in enumerate(active_comps):
            if comp in per_comp:
                mat[r, c] = per_comp[comp]
        best_col[r, 0] = best_spd

    col_labels = [comp_infos[c].short_label for c in active_comps]
    if add_best:
        col_geo = np.array([
            geomean([v for v in mat[r, :] if not np.isnan(v)])
            for r in range(n_rows)
        ])[:, None]
        mat = np.hstack([mat, best_col, col_geo])
        col_labels.extend(["Best vs\nBest std", "Geomean"])

    row_geo = np.array([
        geomean([v for v in mat[:, c] if not np.isnan(v)])
        for c in range(mat.shape[1])
    ])[None, :]
    mat = np.vstack([mat, row_geo])
    row_labels.append("Overall Geomean")

    valid_vals = mat[~np.isnan(mat) & (mat > 0)]
    log_mat = np.where(np.isnan(mat) | (mat <= 0), np.nan, np.log2(np.maximum(mat, 1e-9)))
    vmin = min(-0.6, float(np.nanmin(log_mat)) * 1.05) if len(valid_vals) else -1.0
    vmax = max(1.0, float(np.nanmax(log_mat)) * 1.05) if len(valid_vals) else 2.0
    norm = TwoSlopeNorm(vmin=vmin, vcenter=0.0, vmax=vmax)

    fig_h = max(4.8, 0.44 * len(row_labels) + 2.1)
    fig_w = max(8.5, 1.55 * len(col_labels) + 3.0)
    fig, ax = plt.subplots(figsize=(fig_w, fig_h), dpi=160, layout="constrained")

    cmap = SPEEDUP_CMAP.copy()
    cmap.set_bad(color="#eeeeee")
    ax.imshow(np.ma.masked_invalid(log_mat), cmap=cmap, norm=norm, aspect="auto")

    ax.set_xticks(range(len(col_labels)))
    ax.set_xticklabels(col_labels, fontsize=8.8, fontweight="bold")
    ax.xaxis.tick_top()
    ax.set_yticks(range(len(row_labels)))
    ax.set_yticklabels(row_labels, fontsize=9.0, fontweight="bold")

    ax.set_xticks(np.arange(-0.5, len(col_labels), 1), minor=True)
    ax.set_yticks(np.arange(-0.5, len(row_labels), 1), minor=True)
    ax.grid(which="minor", color="white", linestyle="-", linewidth=1.6)
    ax.tick_params(which="minor", top=False, bottom=False, left=False)

    if add_best:
        ax.axvline(n_cols - 0.5, color="#222222", linewidth=1.9)
    ax.axhline(n_rows - 0.5, color="#222222", linewidth=1.9)

    for r in range(mat.shape[0]):
        for c in range(mat.shape[1]):
            val = mat[r, c]
            if np.isnan(val):
                ax.text(c, r, "—", ha="center", va="center", color="#777777", fontsize=8.8)
                continue
            nv = norm(log_mat[r, c])
            t_col = "white" if (nv < 0.22 or nv > 0.82) else "#111111"
            txt = f"{val:.1f}×" if val >= 2.0 else f"{val:.2f}×"
            is_sum = (r == mat.shape[0] - 1) or (add_best and c >= n_cols)
            ax.text(
                c, r, txt,
                ha="center", va="center",
                color=t_col,
                fontsize=9.2 if is_sum else 8.6,
                fontweight="bold" if (is_sum or val >= 1.5) else "normal",
            )

    fig.suptitle(f"raze vs std — All Algorithms Overview ({suffix})", fontsize=13.5, fontweight="bold")
    footer = format_environment_footer(active_comps, comp_infos)
    if footer:
        fig.supxlabel(footer, fontsize=8.2, color="#455A64")

    chart_path = out_dir / "overall_summary.png"
    fig.savefig(chart_path)
    plt.close(fig)
    print(f"Saved {chart_path}")


def main() -> None:
    apply_matplotlib_style()

    ap = argparse.ArgumentParser(
        description="Build, run, and visualize raze vs std benchmarks across multiple compilers & STLs."
    )
    ap.add_argument("--root", type=Path, default=Path("."))
    ap.add_argument(
        "--targets", nargs="+", default=None,
        help="Benchmark names (e.g. FindBenchmark SearchNBenchmark); defaults to all benchmarks/algorithm/**/*.cpp",
    )
    ap.add_argument(
        "--compilers", nargs="+", default=list(COMPILER_STYLES.keys()),
        choices=list(COMPILER_STYLES.keys()),
        help="Compilers to run/plot (default: gcc clang clang-cl msvc icpx)",
    )
    ap.add_argument("--suffix", choices=["native", "portable"], default="native")
    ap.add_argument("--build", action="store_true")
    ap.add_argument("--reconfigure", action="store_true")
    ap.add_argument("--plot-only", action="store_true", help="Reuse existing JSONs in --out-dir")
    ap.add_argument("--no-probe", action="store_true", help="Skip fallback live compiler STL probing")
    ap.add_argument("--no-scalar", action="store_true", help="Hide raze[fscalar] to zoom into raze vs std")
    ap.add_argument("--article-only", action="store_true", help="Only render unified article summary figures")
    ap.add_argument("--article-scenario", default=None, help="Filter scenario substring for article summary figure")
    ap.add_argument("--filter", default=None, help="Google Benchmark --benchmark_filter regex")
    ap.add_argument("--min-time", default=None, help="Override --benchmark_min_time (e.g. 0.05s)")
    ap.add_argument("--repetitions", type=int, default=None, help="Override --benchmark_repetitions")
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

    print(f"Discovered benchmarks ({len(benchmarks)}): {', '.join(benchmarks)}")
    args.out_dir.mkdir(parents=True, exist_ok=True)

    comp_infos: dict[str, CompilerInfo] = {c: make_default_compiler_info(c) for c in args.compilers}
    compiler_envs: dict[str, tuple[dict[str, str], Path]] = {}
    skipped: list[tuple[str, str]] = []

    if not args.plot_only:
        for comp in args.compilers:
            env = env_for_compiler(comp)
            if args.build:
                build_dir = ensure_configured(root, comp, env, args.reconfigure)
            else:
                build_dir = (root / "out" / "bench" / comp).resolve()
            if build_dir is not None and build_dir.exists():
                compiler_envs[comp] = (env, build_dir)
                if not args.no_probe:
                    comp_infos[comp] = probe_compiler_metadata(comp, env=env, build_dir=build_dir)
            else:
                skipped.append(("<configure>", comp))

    logy = not args.linear_y
    include_scalar = not args.no_scalar
    probed_fallback: set[str] = set()
    overall_rows: list[tuple[str, dict[str, float], float]] = []

    for bench_name in benchmarks:
        bench_out = args.out_dir / bench_name
        bench_out.mkdir(parents=True, exist_ok=True)
        title_prefix = f"{bench_name}.{args.suffix}"

        grouped = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))

        for comp in args.compilers:
            json_file = bench_out / f"{comp}.json"
            if not args.plot_only:
                if comp not in compiler_envs:
                    continue
                env, build_dir = compiler_envs[comp]
                ok = build_and_run_target(
                    root, comp, env, build_dir, bench_name, args.suffix,
                    json_file, args.build, args.filter,
                    min_time=args.min_time, repetitions=args.repetitions,
                )
                if not ok:
                    skipped.append((bench_name, comp))
                    continue

            if not json_file.exists():
                continue

            try:
                rows, context = load_results(json_file)
            except Exception as exc:
                print(f"[SKIP] Failed to parse {json_file}: {exc}")
                skipped.append((bench_name, comp))
                continue

            merge_context_metadata(comp_infos[comp], context)

            if not comp_infos[comp].stl and not args.no_probe and comp not in probed_fallback:
                probed_fallback.add(comp)
                build_dir = (root / "out" / "bench" / comp).resolve()
                probed = probe_compiler_metadata(comp, build_dir=build_dir if build_dir.exists() else None)
                if probed.stl:
                    comp_infos[comp].stl = probed.stl
                if probed.stl_version and not comp_infos[comp].stl_version:
                    comp_infos[comp].stl_version = probed.stl_version
                if probed.compiler_version and not comp_infos[comp].compiler_version:
                    comp_infos[comp].compiler_version = probed.compiler_version

            for dtype, scenario, variant, size, time_ns in rows:
                grouped[scenario][dtype][(comp, variant)].append((size, time_ns))

        if not grouped:
            print(f"[WARN] No data to plot for {bench_name}")
            continue

        per_comp_ratios: dict[str, list[float]] = defaultdict(list)
        best_ratios: list[float] = []
        for by_type in grouped.values():
            for series in by_type.values():
                r_by_sz: dict[int, list[float]] = defaultdict(list)
                s_by_sz: dict[int, list[float]] = defaultdict(list)
                for comp in args.compilers:
                    r_pts = dict(series.get((comp, "raze"), []))
                    s_pts = dict(series.get((comp, "std"), []))
                    for x, v in r_pts.items():
                        if v > 0:
                            r_by_sz[x].append(v)
                    for x, v in s_pts.items():
                        if v > 0:
                            s_by_sz[x].append(v)
                    for x in set(r_pts) & set(s_pts):
                        if r_pts[x] > 0 and s_pts[x] > 0:
                            per_comp_ratios[comp].append(s_pts[x] / r_pts[x])
                for x in set(r_by_sz) & set(s_by_sz):
                    best_ratios.append(min(s_by_sz[x]) / min(r_by_sz[x]))

        overall_rows.append((
            bench_name,
            {c: geomean(rs) for c, rs in per_comp_ratios.items() if rs},
            geomean(best_ratios),
        ))

        plot_article_summary(
            bench_name, title_prefix, grouped, comp_infos, args.out_dir,
            logy, args.metric, include_scalar, args.article_scenario,
        )

        if args.article_only:
            continue

        for scenario, by_type in grouped.items():
            plot_stl_comparison(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out, logy, args.metric, include_scalar)
            plot_type_grid(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out, logy, args.metric, include_scalar)
            plot_type_compiler_matrix(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out, logy, args.metric, include_scalar)
            plot_speedup_grid(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out)
            plot_by_compiler(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out, logy, args.metric, include_scalar)
            plot_speedup(bench_name, title_prefix, scenario, by_type, comp_infos, bench_out)

    if overall_rows:
        print("\n=== Detected Compilers & STLs ===")
        for comp in args.compilers:
            info = comp_infos[comp]
            if info.stl or info.compiler_version:
                print(f"  - {comp:9s}: {info.detailed_tag}" + (f" [ISA: {info.isa}]" if info.isa else ""))

        plot_overall_summary(overall_rows, comp_infos, args.suffix, args.out_dir)

    if skipped:
        print("\n=== Skipped (configure/build/run failed) ===")
        for item, comp in skipped:
            print(f"  - {item} [{comp}]")


if __name__ == "__main__":
    main()
