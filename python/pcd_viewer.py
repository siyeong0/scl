import os
import numpy as np
import cv2 as cv
import open3d as o3d

if __name__ == "__main__":
    lines = []
    file = open("../pcd.txt")
    while True:
        line = file.readline()
        if not line: break
        lines.append(line.split(','))

    n = len(lines)
    print(f"Number of Points : {n}")
    points = np.zeros((n, 3), dtype=np.float32)
    normals = np.zeros((n, 3), dtype=np.float32)
    colors = np.zeros((n, 3), dtype=np.float32)
    for i, data in enumerate(lines):
        points[i] = np.array([float(data[0]), float(data[1]), float(data[2])])
        normals[i] = np.array([float(data[3]), float(data[4]), float(data[5])])
        colors[i] = np.array([float(data[6]), float(data[7]), float(data[8])])

    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(points)
    pcd.colors = o3d.utility.Vector3dVector(colors / 255)

    vis = [pcd]
    mesh_frame = o3d.geometry.TriangleMesh.create_coordinate_frame(
    size=0.6, origin=[0, 0, 0])
    vis.append(mesh_frame)
    o3d.visualization.draw_geometries(vis)
