"""
Package the ESP32 firmware into one ready-to-open Arduino sketch per robot.

The sketch in ./hexapod_esp32/ drives every robot in the family; robot.h picks
which one. Each package is a copy of that sketch with:

- the main tab renamed to hexapod_<name>.ino, inside a hexapod_<name>/ folder
  (Arduino requires the folder and the main tab to share a name)
- robot.h preset to the robot, so nothing needs editing before upload
- only the robot's own src/robots/<name>/ tables
- FIRMWARE_BUILD in version.h set to the package version

Packages are written to ../../dist/ as hexapod_<name>/ and
hexapod_<name>_<version>.zip.

Usage:
    python package_esp32.py
    python package_esp32.py nougat mochi
    python package_esp32.py --all --version v1.2.0
    python package_esp32.py macaroon --no-zip

- Copyright (C) 2024 - PRESENT  rookidroid.com
- E-mail: info@rookidroid.com
- Website: https://rookidroid.com/
"""

import argparse
import re
import shutil
import subprocess
import zipfile
from pathlib import Path

TOOL_DIR = Path(__file__).resolve().parent
REPO_DIR = TOOL_DIR.parent
SKETCH_DIR = TOOL_DIR / "hexapod_esp32"
ROBOTS_DIR = SKETCH_DIR / "src" / "robots"
MAIN_TAB = SKETCH_DIR.name + ".ino"
DEFAULT_OUT_DIR = REPO_DIR / "dist"


def list_robots():
    """Names of all robots that have a folder in the sketch's src/robots/."""
    return sorted(p.name for p in ROBOTS_DIR.iterdir() if p.is_dir())


def git_version():
    """Version string from git, or "dev" outside a git checkout."""
    try:
        result = subprocess.run(
            ["git", "describe", "--tags", "--always", "--dirty"],
            cwd=REPO_DIR,
            capture_output=True,
            text=True,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return "dev"
    return result.stdout.strip() or "dev"


def select_robot(robot_h, robot):
    """Return robot.h with only ROBOT_<ROBOT> left uncommented."""
    define = "ROBOT_" + robot.upper()
    names = "|".join("ROBOT_" + r.upper() for r in list_robots())
    selection = re.compile(
        rf"^([ \t]*)(?://[ \t]*)?#define ({names})[ \t]*$", re.M
    )

    found = False

    def toggle(match):
        nonlocal found
        indent, name = match.group(1), match.group(2)
        if name == define:
            found = True
            return f"{indent}#define {name}"
        return f"{indent}// #define {name}"

    text = selection.sub(toggle, robot_h)
    if not found:
        raise SystemExit(
            f"robot.h has no '#define {define}' line; add the robot there first"
        )
    return text


def stamp_build(version_h, version):
    """Return version.h with the FIRMWARE_BUILD default replaced by `version`."""
    if re.search(r'["\\\s]', version):
        raise SystemExit(f"version {version!r} cannot go in a C string literal")
    build = re.compile(r'^([ \t]*#define FIRMWARE_BUILD )"[^"]*"', re.M)
    text, count = build.subn(lambda m: m.group(1) + f'"{version}"', version_h)
    if count != 1:
        raise SystemExit("version.h has no '#define FIRMWARE_BUILD \"...\"' line")
    return text


def read_ssid(robot):
    """The access point name from the robot's robot_config.h."""
    config = (ROBOTS_DIR / robot / "robot_config.h").read_text(encoding="utf-8")
    match = re.search(r'#define APSSID "([^"]*)"', config)
    return match.group(1) if match else "?"


def readme_note(robot):
    """Banner prepended to the packaged README.md."""
    return (
        f"> **This package is pre-configured for {robot.capitalize()}** "
        f"(WiFi SSID `{read_ssid(robot)}`). Open `hexapod_{robot}.ino` and upload: "
        "skip *Select Your Robot*, `robot.h` is already set.\n\n"
    )


def package_robot(robot, out_dir, version, make_zip=True):
    """Write the sketch folder (and zip) for one robot. Returns the paths written."""
    name = "hexapod_" + robot
    pkg_dir = out_dir / name
    if pkg_dir.exists():
        shutil.rmtree(pkg_dir)
    pkg_dir.mkdir(parents=True)

    # Top-level sketch tabs, headers and the README
    for src in sorted(SKETCH_DIR.iterdir()):
        if not src.is_file() or src.name.startswith("."):
            continue
        dst_name = name + ".ino" if src.name == MAIN_TAB else src.name
        shutil.copy2(src, pkg_dir / dst_name)

    # Only this robot's tables. The other robots' includes sit in #elif
    # branches that are never taken, so their missing files are harmless.
    shutil.copytree(ROBOTS_DIR / robot, pkg_dir / "src" / "robots" / robot)

    robot_h = pkg_dir / "robot.h"
    robot_h.write_text(
        select_robot(robot_h.read_text(encoding="utf-8"), robot),
        encoding="utf-8",
        newline="\n",
    )

    version_h = pkg_dir / "version.h"
    version_h.write_text(
        stamp_build(version_h.read_text(encoding="utf-8"), version),
        encoding="utf-8",
        newline="\n",
    )

    readme = pkg_dir / "README.md"
    if readme.exists():
        readme.write_text(
            readme_note(robot) + readme.read_text(encoding="utf-8"),
            encoding="utf-8",
            newline="\n",
        )

    license_file = REPO_DIR / "LICENSE"
    if license_file.exists():
        shutil.copy2(license_file, pkg_dir / "LICENSE")

    written = [pkg_dir]
    if make_zip:
        zip_path = out_dir / f"{name}_{version}.zip"
        with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as archive:
            for path in sorted(pkg_dir.rglob("*")):
                if path.is_file():
                    archive.write(path, path.relative_to(out_dir).as_posix())
        written.append(zip_path)
    return written


def main():
    robots = list_robots()

    parser = argparse.ArgumentParser(
        description="Package the ESP32 firmware into one sketch per robot."
    )
    parser.add_argument(
        "robots",
        nargs="*",
        metavar="robot",
        help="robot(s) to package: " + ", ".join(robots) + " (default: all)",
    )
    parser.add_argument("--all", action="store_true", help="package every robot")
    parser.add_argument(
        "-o",
        "--out-dir",
        type=Path,
        default=DEFAULT_OUT_DIR,
        help="output directory (default: <repo>/dist)",
    )
    parser.add_argument(
        "--version",
        help="version in the zip names (default: git describe, or 'dev')",
    )
    parser.add_argument(
        "--no-zip", action="store_true", help="write the sketch folders only"
    )
    args = parser.parse_args()

    unknown = [r for r in args.robots if r not in robots]
    if unknown:
        parser.error("unknown robot(s): " + ", ".join(unknown))

    selected = robots if args.all or not args.robots else args.robots
    version = args.version or git_version()
    out_dir = args.out_dir.resolve()

    for robot in selected:
        for path in package_robot(robot, out_dir, version, not args.no_zip):
            print("Wrote", path)


if __name__ == "__main__":
    main()
