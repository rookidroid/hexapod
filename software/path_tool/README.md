# Path Tool

Generates the motion look-up tables (`motion.h`) that the firmware plays back. One shared set of path generators covers every robot. What differs between robots is data, and it lives in `robots/<name>.json`.

## Files

- `robots/<name>.json`: one robot's leg mount positions and angles, link lengths, servo range, standby/laydown postures and gait parameters
- `generate_motion.py`: builds every gait for a robot and writes its `motion.h`
- `lut_generator.ipynb`: interactive version of `generate_motion.py`, for inspecting postures and paths
- `path_tool.py`: path generators for walking, fast walking, turning, climbing, body rotations and the stand-up sequence
- `path_lib.py`: inverse kinematics and path primitives

## Usage

Requires Python 3 with `numpy`.

```bash
cd software/path_tool
python generate_motion.py mochi      # one robot
python generate_motion.py --all      # every robot in robots/
```

The header is written straight into the firmware, at `../hexapod_esp32/src/robots/<name>/motion.h`. Re-upload the firmware to use it. Pass `-o <file>` to write it somewhere else instead.

## Adding a Robot

1. Copy an existing `robots/<name>.json` to `robots/<new>.json` and fill in the new geometry.
2. Run `python generate_motion.py <new>`.
3. In the firmware, add `src/robots/<new>/robot_config.h` and a `ROBOT_<NEW>` entry in `robot.h` and `motion.h`.
