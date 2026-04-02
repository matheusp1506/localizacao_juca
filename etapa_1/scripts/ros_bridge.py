import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Quaternion
import paho.mqtt.client as mqtt
import json
import math

# This node bridges MQTT messages from the ESP32 to ROS 2 Odometry messages.
# It subscribes to the "esp32/odom_raw" topic, parses the incoming JSON data, and publishes it as Odometry on the "odom" topic.
# In this example it calculates the quaternion by itself, showing the possibility of doing processing on the computer side, if needed.
class MqttToOdomBridge(Node):
    def __init__(self):
        super().__init__('mqtt_odom_bridge')
        self.publisher_ = self.create_publisher(Odometry, 'odom', 10)
        
        # MQTT Setup
        self.client = mqtt.Client()
        self.client.on_message = self.on_message
        self.client.connect("localhost", 1883, 60)
        self.client.subscribe("esp32/odom_raw")
        self.client.loop_start()
        self.get_logger().info("Bridge Node Started. Listening to MQTT...")

    def euler_to_quaternion(self, yaw):
        """Helper to convert Yaw to ROS 2 Quaternion"""
        q = Quaternion()
        q.x = 0.0
        q.y = 0.0
        q.z = math.sin(yaw / 2.0)
        q.w = math.cos(yaw / 2.0)
        return q

    def on_message(self, client, userdata, msg):
        try:
            data = json.loads(msg.payload.decode())
            
            odom = Odometry()
            odom.header.stamp = self.get_clock().now().to_msg()
            odom.header.frame_id = "odom"
            odom.child_frame_id = "base_link"

            # Fill Position
            odom.pose.pose.position.x = data['x']
            odom.pose.pose.position.y = data['y']
            
            # Fill Orientation (Convert Yaw to Quaternion)
            odom.pose.pose.orientation = self.euler_to_quaternion(data['yaw'])

            self.publisher_.publish(odom)
        except Exception as e:
            self.get_logger().error(f"Failed to parse MQTT: {e}")

def main():
    rclpy.init()
    node = MqttToOdomBridge()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()
