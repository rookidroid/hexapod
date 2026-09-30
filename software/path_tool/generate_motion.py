"""
Generate the motion LUT header (motion.h) for one or more hexapod robots.

Each robot is described by a JSON file in ./robots/ (geometry, servo range,
postures and gait parameters). Two headers are written into the firmware
sketch, under ../hexapod_esp32/src/robots/<name>/:

- motion.h, the motion path LUTs
- robot_geometry.h, the JSON config itself, which the firmware serves at
  GET /robot_config so clients can model the robot without a copy of it

Usage:
    python generate_motion.py macaroon
    python generate_motion.py nougat mochi
    python generate_motion.py --all
    python generate_motion.py --all --geometry-only

- Copyright (C) 2024 - PRESENT  rookidroid.com
- E-mail: info@rookidroid.com
- Website: https://rookidroid.com/
"""

import argparse
import json
from pathlib import Path

import numpy as np

from path_lib import inverse_kinematics
from path_tool import gen_posture
from path_tool import gen_walk_path, gen_fastwalk_path, gen_turn_path, gen_climb_path
from path_tool import (
    gen_rotatex_path,
    gen_rotatey_path,
    gen_rotatez_path,
    gen_twist_path,
)
from path_tool import gen_standup_path

TOOL_DIR = Path(__file__).resolve().parent
ROBOTS_DIR = TOOL_DIR / "robots"
FIRMWARE_ROBOTS_DIR = TOOL_DIR.parent / "hexapod_esp32" / "src" / "robots"


def list_robots():
    """Names of all robots that have a config in ./robots/."""
    return sorted(p.stem for p in ROBOTS_DIR.glob("*.json"))


def load_config(robot):
    """Load the JSON config of a robot by name."""
    with open(ROBOTS_DIR / (robot + ".json"), "r", encoding="utf-8") as read_file:
        return json.load(read_file)


def gen_luts(config):
    """Generate all motion paths for a robot.

    Returns:
        list of (name, path) tuples, in the order they are written to motion.h.
        Each path is an array of foot coordinates, shaped (steps, 6, 3).
    """
    gait = config["gait"]
    standby = gen_posture(*config["standbyPosture"], config)
    laydown = gen_posture(*config["laydownPosture"], config)

    walk_radius = gait["walk_radius"]
    fastwalk = {
        "y_radius": gait["fastwalk_y_radius"],
        "z_radius": gait["fastwalk_z_radius"],
        "x_radius": gait["fastwalk_x_radius"],
        "g_steps": 28,
    }

    return [
        ("lut_standby", standby[np.newaxis, :, :]),
        ("lut_walk_0", gen_walk_path(standby, g_radius=walk_radius, direction=0)),
        ("lut_walk_180", gen_walk_path(standby, g_radius=walk_radius, direction=180)),
        ("lut_walk_r45", gen_walk_path(standby, g_radius=walk_radius, direction=315)),
        ("lut_walk_r90", gen_walk_path(standby, g_radius=walk_radius, direction=270)),
        ("lut_walk_r135", gen_walk_path(standby, g_radius=walk_radius, direction=225)),
        ("lut_walk_l45", gen_walk_path(standby, g_radius=walk_radius, direction=45)),
        ("lut_walk_l90", gen_walk_path(standby, g_radius=walk_radius, direction=90)),
        ("lut_walk_l135", gen_walk_path(standby, g_radius=walk_radius, direction=135)),
        ("lut_fast_forward", gen_fastwalk_path(standby, **fastwalk)),
        ("lut_fast_backward", gen_fastwalk_path(standby, **fastwalk, reverse=True)),
        (
            "lut_turn_left",
            gen_turn_path(standby, g_radius=gait["turn_radius"], direction="left"),
        ),
        (
            "lut_turn_right",
            gen_turn_path(standby, g_radius=gait["turn_radius"], direction="right"),
        ),
        ("lut_climb_forward", gen_climb_path(standby, reverse=False)),
        ("lut_climb_backward", gen_climb_path(standby, reverse=True)),
        (
            "lut_rotate_x",
            gen_rotatex_path(standby, g_steps=28, swing_angle=10, y_radius=10),
        ),
        (
            "lut_rotate_y",
            gen_rotatey_path(standby, g_steps=28, swing_angle=10, x_radius=10),
        ),
        ("lut_rotate_z", gen_rotatez_path(standby, g_steps=28, z_lift=7)),
        ("lut_twist", gen_twist_path(standby, g_steps=28)),
        ("lut_standup", gen_standup_path(standby, laydown, steps=28)),
    ]


def to_ticks(coordinate, config):
    """Convert one frame of foot coordinates (6, 3) into servo ticks."""
    servo_min = config["servoMin"]
    servo_range = config["servoMax"] - servo_min
    angles = inverse_kinematics(coordinate, config)
    return np.round(angles / 180 * servo_range + servo_min).astype(int)


def format_motion_header(luts, config):
    """Render the LUTs as the C header consumed by the firmware."""
    lines = [
        "/**\n",
        " * This is an automatically generated header, which includes motion path LUTs\n",
        " * \n",
        " * - Copyright (C) 2024 - PRESENT  rookidroid.com\n",
        " * - E-mail: info@rookidroid.com\n",
        " * - Website: https://rookidroid.com/\n",
        " */\n\n",
        "#ifndef MOTION_H\n",
        "#define MOTION_H\n\n",
    ]

    for var_name, var in luts:
        steps = np.shape(var)[0]
        lines.append("static const int " + var_name + "_length = " + str(steps) + ";\n")

        for idx in range(0, steps):
            pwm = to_ticks(var[idx, :, :], config)

            if idx == 0:
                lines.append("static const int16_t " + var_name + "[" + str(steps) + "][6][3] = {{")
            else:
                lines.append("                                  {")

            for r in range(0, 6):
                if r > 0:
                    lines.append("                                   ")
                lines.append(
                    "{" + str(pwm[r, 0]) + ", " + str(pwm[r, 1]) + ", " + str(pwm[r, 2]) + "}"
                )
                lines.append(",\n" if r < 5 else "}")

            lines.append("};\n\n" if idx == steps - 1 else ",\n")

    lines.append("#endif // MOTION_H\n")
    return "".join(lines)


def format_geometry_header(config):
    """Render the robot's JSON config as a C string for the firmware to serve."""
    geometry = json.dumps(config, separators=(",", ":"))
    return "".join(
        [
            "/**\n",
            " * This is an automatically generated header, which includes the robot's\n",
            " * config from software/path_tool/robots/<name>.json. The firmware serves it\n",
            " * at GET /robot_config.\n",
            " * \n",
            " * - Copyright (C) 2024 - PRESENT  rookidroid.com\n",
            " * - E-mail: info@rookidroid.com\n",
            " * - Website: https://rookidroid.com/\n",
            " */\n\n",
            "#ifndef ROBOT_GEOMETRY_H\n",
            "#define ROBOT_GEOMETRY_H\n\n",
            'static const char ROBOT_GEOMETRY_JSON[] = R"json(' + geometry + ')json";\n\n',
            "#endif // ROBOT_GEOMETRY_H\n",
        ]
    )


def _write(path, text):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fp:
        fp.write(text)
    return path


def generate_geometry(robot):
    """Generate robot_geometry.h for a robot and return the path it was written to."""
    return _write(
        FIRMWARE_ROBOTS_DIR / robot / "robot_geometry.h",
        format_geometry_header(load_config(robot)),
    )


def generate(robot, output=None):
    """Generate motion.h for a robot and return the path it was written to.

    robot_geometry.h is refreshed alongside it, unless `output` sends the LUTs
    somewhere other than the firmware sketch.
    """
    config = load_config(robot)
    if output is None:
        output = FIRMWARE_ROBOTS_DIR / robot / "motion.h"
        generate_geometry(robot)
    return _write(output, format_motion_header(gen_luts(config), config))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("robots", nargs="*", help="robot names: " + ", ".join(list_robots()))
    parser.add_argument("--all", action="store_true", help="generate for every robot")
    parser.add_argument(
        "-o", "--output", help="output file (only with a single robot); default is the firmware sketch"
    )
    parser.add_argument(
        "--geometry-only",
        action="store_true",
        help="only write robot_geometry.h, leaving the motion LUTs untouched",
    )
    args = parser.parse_args()

    robots = list_robots() if args.all else args.robots
    if not robots:
        parser.error("name at least one robot, or pass --all")
    unknown = [r for r in robots if r not in list_robots()]
    if unknown:
        parser.error("unknown robot(s): " + ", ".join(unknown))
    if args.output and len(robots) > 1:
        parser.error("--output can only be used with a single robot")
    if args.output and args.geometry_only:
        parser.error("--output cannot be combined with --geometry-only")

    for robot in robots:
        if args.geometry_only:
            print(robot + ": " + str(generate_geometry(robot)))
        else:
            print(robot + ": " + str(generate(robot, args.output)))


if __name__ == "__main__":
    main()
