import sys
import os

# Add the current working directory to sys.path
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

import serial
import logging

from proto_gen_classes import floatarray_pb2, int32_pb2
from msg_utils import *

import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32MultiArray
from sensor_msgs.msg import NavSatFix, Imu
from nav_msgs.msg import Odometry
from std_msgs.msg import String

from logging_utils import setup_logger, get_logger
from PBUtils import *
import time
from google.protobuf import message

# Your existing script here...

class ArduinoCommunicationNode(Node):
    def __init__(self):
        super().__init__('pb2ros')

        # Replace '/dev/ttyACM0' with the correct serial port for your Arduino
        serial_port = '/dev/ttyACM0'
        self.ser = serial.Serial(port=serial_port, baudrate=115200, timeout=0.05)

        # Ensure the serial port is open
        if not self.ser.is_open:
            try:
                self.ser.open()
            except serial.SerialException as e:
                self.get_logger().error(f"Failed to open serial port {serial_port}: {e}")
                sys.exit(1)

        # Define your protobuf message object

        self.ard_pub = self.create_publisher(Float32MultiArray, '/ard_pub', 10)
        self.subscription = self.create_subscription(String, 'chatter', self.chatter_callback, 10)

        self.msg_obj = {
            7: floatarray_pb2.FloatArray(),
            13: floatarray_pb2.FloatArray(),
            # Add more message objects if needed, replace '1' with the corresponding message ID
        }

        # Create a PBSerialHandler instance
        self.pb_serial_handler = PBSerialHandler(self.ser, self.callback_function, self.msg_obj)

        # Create a timer to periodically send messages
        self.timer = self.create_timer(2.0, self.send_pb_message)

    def callback_function(self, response):
        if response is not None:
            self.get_logger().info(f"Received: {response}")
            deserialized_msgs = self.pb_serial_handler._serialization_handler.deserialize(response)
            for msg_id, msg_obj in deserialized_msgs:
                if msg_id == 7:  # Replace '1' with the corresponding message ID
                    self.get_logger().info(f"Received message ID {msg_id}: {msg_obj}")
                    # Convert FloatArray to Float32MultiArray
                    ros2_msg = Float32MultiArray()
                    ros2_msg.data = msg_obj.data  # Assuming 'data' is the field name in FloatArray

                    # Publish the ROS 2 message
                    self.ard_pub.publish(ros2_msg)

    def chatter_callback(self, msg):
            # Assuming the message received is a string containing a number
            
            numeric_part = ''.join(filter(str.isdigit, msg.data))

            try:
                number = int(numeric_part)
            except ValueError:
                self.get_logger().error(f"Invalid number received: {msg.data}")
                return

            # Send the number to the Arduino
            float_array_msg = floatarray_pb2.FloatArray()
            float_array_msg.data.append(number)

            # Replace '7' with the corresponding message ID
            self.pb_serial_handler.write_pb_msg(13, float_array_msg)

    def send_pb_message(self):
        # Example: Write a protobuf message
        msg_to_send = floatarray_pb2.FloatArray()
        # Populate your protobuf message fields here

        # Replace '1' with the corresponding message ID
        self.pb_serial_handler.write_pb_msg(1, msg_to_send)

    def shutdown_node(self):
        # Close the serial connection when the node is shut down
        if self.ser.is_open:
            self.ser.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = ArduinoCommunicationNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.shutdown_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
