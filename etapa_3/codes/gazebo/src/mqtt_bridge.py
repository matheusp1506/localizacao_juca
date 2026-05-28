import rclpy
from rclpy.node import Node
from rclpy.service import ServiceIntrospectionState
from rclpy.qos import qos_profile_system_default
from ros_gz_interfaces.srv import SetEntityPose
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu
from geometry_msgs.msg import Pose
import paho.mqtt.client as mqtt
import json
import os
from datetime import datetime

class MqttToGzBridge(Node):
    def __init__(self):
        super().__init__('mqtt_bridge')
        self.srv_name = '/world/empty/set_pose'
        self.client = self.create_client(SetEntityPose, self.srv_name)

        self.client.configure_introspection(self.get_clock(),qos_profile_system_default,ServiceIntrospectionState.CONTENTS)
        
        while not self.client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Waiting for service')

        self.get_logger().info(f"ROS 2 Node started. Publishing to: {self.srv_name}")
        
        # Logging state
        self.log_file_path = None
        self.logs_dir = "logs"
        
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        
        self.get_logger().info("Connecting to MQTT Broker...")
        self.mqtt_client.connect("localhost", 1883)
        self.mqtt_client.subscribe("juca/imu/quaternion")
        self.mqtt_client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.get_logger().info("Connected to MQTT successfully!")
        else:
            self.get_logger().error(f"Failed to connect to MQTT, return code {rc}")

    def log_to_file(self, raw_data):
        # Create directory if it doesn't exist
        if not os.path.exists(self.logs_dir):
            os.makedirs(self.logs_dir)
            self.get_logger().info(f"Created directory: {self.logs_dir}")

        # Initialize filename on the very first message received
        if self.log_file_path is None:
            timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            self.log_file_path = os.path.join(self.logs_dir, f"log_{timestamp}.txt")
            self.get_logger().info(f"Logging started: {self.log_file_path}")

        # Append data to the file
        with open(self.log_file_path, "a") as f:
            f.write(f"[{datetime.now().strftime('%H:%M:%S.%f')}] {raw_data}\n")

    def on_message(self, client, userdata, msg):
        payload_str = msg.payload.decode()
        self.get_logger().info(f"Received MQTT data: {payload_str}")
        
        # Call the logging function
#        self.log_to_file(payload_str)

        try:
            data = json.loads(payload_str)

            req = SetEntityPose.Request()

            req.entity.name = "juca_twin"
            req.entity.type = 2

            pose = Pose()
            pose.position.x = float(data['x'])
            pose.position.y = float(data['y'])
            pose.position.z = 0.1
            pose.orientation.x = 0.0
            pose.orientation.y = 0.0
            pose.orientation.z = float(data['quat_z'])
            pose.orientation.w = float(data['quat_w'])
            
            req.pose = pose

            future = self.client.call_async(req)

            future.add_done_callback(self.service_response_callback)

            self.get_logger().info("Published Pose to ROS 2")
        except Exception as e:
            self.get_logger().error(f"Error parsing or publishing: {e}")

    def service_response_callback(self, future):
        try:
            response = future.result()
            if response.success:
                self.get_logger().info("Juca twin model pose updated successfully in Gazebo.")
            else:
                self.get_logger().warn("Gazebo received request but failed to update pose.")
        except Exception as e:
            self.get_logger().error(f"Service call failed: {e}")

def main():
    rclpy.init()
    node = MqttToGzBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
