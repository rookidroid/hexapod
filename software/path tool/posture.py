import numpy as np

def gen_posture(j2_angle, j3_angle, config):
    mount_x = np.array(config['legMountX'])
    mount_y = np.array(config['legMountY'])
    root_j1 = config['legRootToJoint1']
    j1_j2 = config['legJoint1ToJoint2']
    j2_j3 = config['legJoint2ToJoint3']
    j3_tip = config['legJoint3ToTip']
    mount_angle = np.array(config['legMountAngle'])/180*np.pi

    j2_rad = j2_angle/180*np.pi
    j3_rad = j3_angle/180*np.pi
    posture = np.zeros((6, 3))

    posture[:, 0] = mount_x+(root_j1+j1_j2+(
        j2_j3*np.sin(j2_rad))+j3_tip*np.cos(j3_rad)) *\
        np.cos(mount_angle)
    posture[:, 1] = mount_y + (root_j1+j1_j2+(
        j2_j3*np.sin(j2_rad))+j3_tip*np.cos(j3_rad)) *\
        np.sin(mount_angle)
    posture[:, 2] = j2_j3 * \
        np.cos(j2_rad) - j3_tip * \
        np.sin(j3_rad)
    return posture
