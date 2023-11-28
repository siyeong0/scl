#import rospy
import cv2
import numpy as np
from datetime import datetime
import atexit
import os
import socket
import struct
import select

class ImageSubscriber(object):
    def __init__(self):
        # self.image_sub = rospy.Subscriber("/compressed_image", CompressedImage, self.callback)
        # self.pose_sub = rospy.Subscriber("drone")
        # '192.168.1.59'
        UDP_IP = '192.168.1.182'
        UDP_PORT1 = 9990
        UDP_PORT2 = 9991
        UDP_PORT3 = 9992
        self.current_time = datetime.now().strftime("%Y-%m-%d-%H-%M-%S")
        self.image_buffer = []
        self.depth_buffer = []
        self.camera_buffer = []
        self.sock1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock1.bind((UDP_IP, UDP_PORT1))
        self.sock2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock2.bind((UDP_IP, UDP_PORT2))
        self.sock3 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock3.bind((UDP_IP, UDP_PORT3))
        atexit.register(self.save_buffer_on_exit)
    def receive_image(self):
        try:
            self.sock1.setblocking(0)
            ready = select.select([self.sock1], [], [], 0.0001)
            if ready[0]:
                data, addr = self.sock1.recvfrom(65536)
                data_number = struct.unpack('<H', data[-2:])[0]
                image_data = bytearray(data[:-2])
                np_data = np.frombuffer(image_data, dtype=np.uint8)
                image = cv2.imdecode(np_data, cv2.IMREAD_COLOR)
                cv2.imshow("Received Image", image)
                cv2.waitKey(1)
                self.image_buffer.append(image)
                self.image_buffer.append(data_number)
        except Exception as e:
            pass
            #rospy.logerr("Error in receiving or processing image data: %s", str(e))
    def receive_depth(self):
        try:
            self.sock2.setblocking(0)
            ready = select.select([self.sock2], [], [], 0.0001)
            if ready[0]:
                depth_data = bytearray()
                for _ in range(2):
                    data, addr = self.sock2.recvfrom(65536)
                    depth_data.extend(data)
                data_number = struct.unpack('<H', data[-2:])[0]
                depth_data = depth_data[:-2]
                np_data = np.frombuffer(depth_data, dtype=np.uint16)
                # np_data = (np_data / 256).astype(np.uint8)
                depth = np_data.reshape((144, 256))
                # depth = cv2.resize(depth, (1280, 720))
                cv2.imshow("Received depth", depth)
                cv2.waitKey(1)
                self.depth_buffer.append(depth)
                self.depth_buffer.append(data_number)
        except Exception as e:
            pass
            #rospy.logerr("Error in receiving or processing depth data: %s", str(e))
    def receive_camera(self):
        try:
            self.sock3.setblocking(0)
            ready = select.select([self.sock3], [], [], 0.0001)
            if ready[0]:
                data, addr = self.sock3.recvfrom(65536)
                data_number = struct.unpack('<H', data[-2:])[0]
                camera_data = bytearray(data[:-2])
                self.camera_buffer.append(camera_data)
                self.camera_buffer.append(data_number)
        except Exception as e:
            pass
            #rospy.logerr("Error in receiving or processing depth data: %s", str(e))
            
    def save_buffer_on_exit(self):
        folder_name = "./{}".format(self.current_time)
        image_folder_name = folder_name + "/image"
        depth_folder_name = folder_name + "/depth"
        camera_folder_name = folder_name + "/camera"
        os.mkdir(folder_name)
        os.mkdir(image_folder_name)
        os.mkdir(depth_folder_name)
        os.mkdir(camera_folder_name)
        if len(self.image_buffer) > 0:
            for i, image in enumerate(self.image_buffer):
                if i % 2 == 0:
                    filename = "{}/{}.jpg".format(image_folder_name, self.image_buffer[i+1])
                    cv2.imwrite(filename, image)
        if len(self.depth_buffer) > 0:
            for i, depth in enumerate(self.depth_buffer):
                if i % 2 == 0:
                    filename = "{}/{}.png".format(depth_folder_name, self.depth_buffer[i+1])
                    cv2.imwrite(filename, depth)
        if len(self.camera_buffer) > 0:
            for i, camera in enumerate(self.camera_buffer):
                if i % 2 == 0:
                    filename = "{}/{}.bin".format(camera_folder_name, self.camera_buffer[i+1])
                    with open(filename, 'wb') as file:
                        file.write(camera)
    def callback(self, msg):
        try:
            # Decompress image
            np_arr = np.frombuffer(msg.data, np.uint8)
            image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
            # Display image
            cv2.imshow("Image", image)
            cv2.waitKey(1)
        except Exception as e:
            print(e)
          
import time  
from threading import Thread , Timer
import multiprocessing
if __name__ == '__main__':
    
    n_works = 5
    
    image_subscriber = ImageSubscriber()
 
    while(True):
        image_subscriber.receive_image()
        image_subscriber.receive_depth()
        image_subscriber.receive_camera()
    #th1 = Timer(0.0001, image_subscriber.receive_image).start()
    #th2 = Timer(0.0001, image_subscriber.receive_depth).start()
    #th3 = Timer(0.0001, image_subscriber.receive_camera).start()