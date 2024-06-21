#!python
#
# 2021  Zhengyu Peng
# Website: https://zpeng.me
#
# `                      `
# -:.                  -#:
# -//:.              -###:
# -////:.          -#####:
# -/:.://:.      -###++##:
# ..   `://:-  -###+. :##:
#        `:/+####+.   :##:
# .::::::::/+###.     :##:
# .////-----+##:    `:###:
#  `-//:.   :##:  `:###/.
#    `-//:. :##:`:###/.
#      `-//:+######/.
#        `-/+####/.
#          `+##+.
#           :##:
#           :##:
#           :##:
#           :##:
#           :##:
#            .+:

import numpy as np


def semicircle_generator(radius, steps, reverse=False):
    assert (steps % 4) == 0
    halfsteps = int(steps/2)

    step_angle = np.pi / halfsteps

    result = np.zeros((steps, 3))
    halfsteps_array = np.arange(halfsteps)

    # first half, move backward (only y change)
    result[:halfsteps, 1] = radius - halfsteps_array*radius*2/(halfsteps)

    # second half, move forward in semicircle shape (y, z change)
    angle = np.pi - step_angle*halfsteps_array
    result[halfsteps:, 1] = radius * np.cos(angle)
    result[halfsteps:, 2] = radius * np.sin(angle)

    result = np.roll(result, int(steps/4), axis=0)

    if reverse:
        result = np.flip(result, axis=0)
        result = np.roll(result, 1, axis=0)

    return result


def semicircle2_generator(steps, y_radius, z_radius, x_radius, reverse=False):
    assert (steps % 4) == 0
    halfsteps = int(steps/2)

    step_angle = np.pi / halfsteps

    result = np.zeros((steps, 3))
    halfsteps_array = np.arange(halfsteps)

    # first half, move backward (only y change)
    result[:halfsteps, 1] = y_radius - halfsteps_array*y_radius*2/(halfsteps)

    # second half, move forward in semicircle shape (x, y, z change)
    angle = np.pi - step_angle*halfsteps_array
    result[halfsteps:, 0] = x_radius * np.sin(angle)
    result[halfsteps:, 1] = y_radius * np.cos(angle)
    result[halfsteps:, 2] = z_radius * np.sin(angle)

    result = np.roll(result, int(steps/4), axis=0)

    if reverse:
        result = np.flip(result, axis=0)
        result = np.roll(result, 1, axis=0)

    return result


def get_rotate_x_matrix(angle):
    angle = angle * np.pi / 180
    return np.matrix([
        [1, 0, 0, 0],
        [0, np.cos(angle), -np.sin(angle), 0],
        [0, np.sin(angle), np.cos(angle), 0],
        [0, 0, 0, 1],
    ])


def get_rotate_y_matrix(angle):
    angle = angle * np.pi / 180
    return np.matrix([
        [np.cos(angle), 0, np.sin(angle), 0],
        [0, 1, 0, 0],
        [-np.sin(angle), 0, np.cos(angle), 0],
        [0, 0, 0, 1],
    ])


def get_rotate_z_matrix(angle):
    angle = angle * np.pi / 180
    return np.matrix([
        [np.cos(angle), -np.sin(angle), 0, 0],
        [np.sin(angle), np.cos(angle), 0, 0],
        [0, 0, 1, 0],
        [0, 0, 0, 1],
    ])


def matrix_mul(m, pt):
    ptx = list(pt) + [1]
    return list((m * np.matrix(ptx).T).T.flat)[:-1]


def path_rotate_x(path, angle):
    ptx = np.append(path, np.ones((np.shape(path)[0], 1)), axis=1)
    return ((get_rotate_x_matrix(angle) * np.matrix(ptx).T).T)[:, :-1]


def path_rotate_y(path, angle):
    ptx = np.append(path, np.ones((np.shape(path)[0], 1)), axis=1)
    return ((get_rotate_y_matrix(angle) * np.matrix(ptx).T).T)[:, :-1]


def path_rotate_z(path, angle):
    ptx = np.append(path, np.ones((np.shape(path)[0], 1)), axis=1)
    return ((get_rotate_z_matrix(angle) * np.matrix(ptx).T).T)[:, :-1]


def inverse_kinematics(dest, config):
    mount_x = np.array(config['legMountX'])
    mount_y = np.array(config['legMountY'])
    root_j1 = config['legRootToJoint1']
    j1_j2 = config['legJoint1ToJoint2']
    j2_j3 = config['legJoint2ToJoint3']
    j3_tip = config['legJoint3ToTip']
    mount_angle = np.array(config['legMountAngle'])/180*np.pi
    mount_position = np.zeros((6, 3))
    mount_position[:, 0] = mount_x
    mount_position[:, 1] = mount_y
    leg_scale = np.array(config['legScale'])

    temp_dest = dest-mount_position
    local_dest = np.zeros_like(dest)
    local_dest[:, 0] = temp_dest[:, 0] * \
        np.cos(mount_angle) + \
        temp_dest[:, 1] * np.sin(mount_angle)
    local_dest[:, 1] = temp_dest[:, 0] * \
        np.sin(mount_angle) - \
        temp_dest[:, 1] * np.cos(mount_angle)
    local_dest[:, 2] = temp_dest[:, 2]

    angles = np.zeros((6, 3))
    x = local_dest[:, 0] - root_j1
    y = local_dest[:, 1]

    angles[:, 0] = -(np.arctan2(y, x) * 180 / np.pi)+90

    x = np.sqrt(x*x + y*y) - j1_j2
    y = local_dest[:, 2]
    ar = np.arctan2(y, x)
    lr2 = x*x + y*y
    lr = np.sqrt(lr2)
    a1 = np.arccos((lr2 + j2_j3*j2_j3 -
                    j3_tip*j3_tip)/(2*j2_j3*lr))
    a2 = np.arccos((lr2 - j2_j3*j2_j3 +
                    j3_tip*j3_tip)/(2*j3_tip*lr))

    angles[:, 1] = 90-((ar + a1) * 180 / np.pi)*leg_scale[:,1]
    angles[:, 2] = (90 - ((a1 + a2) * 180 / np.pi))*leg_scale[:,1]+90

    return angles

if __name__ == '__main__':
    pt = [0, 1, 0]
