import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu
import paho.mqtt.client as mqtt
import json
import os
from datetime import datetime
import math

class MqttToGzBridge(Node):
    def __init__(self):
        super().__init__('mqtt_bridge')
        
        # 1. Initialize ROS 2 Publishers for the EKF Node
        self.odom_pub = self.create_publisher(Odometry, 'odom', 10)
        self.imu_pub = self.create_publisher(Imu, 'imu/data', 10)
        
        self.get_logger().info("ROS 2 Node started. Publishing raw data to /juca/odom_raw and /juca/imu_raw")
        
        # Logging state
        self.log_file_path = None
        self.logs_dir = "logs"
        
        # 2. MQTT Client Setup
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        
        self.get_logger().info("Connecting to MQTT Broker...")
        self.mqtt_client.connect("172.16.139.214", 1883)
        self.mqtt_client.subscribe("juca/imu/quaternion")
        self.mqtt_client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.get_logger().info("Connected to MQTT successfully!")
        else:
            self.get_logger().error(f"Failed to connect to MQTT, return code {rc}")

    def log_to_file(self, raw_data):
        if not os.path.exists(self.logs_dir):
            os.makedirs(self.logs_dir)
            self.get_logger().info(f"Created directory: {self.logs_dir}")

        if self.log_file_path is None:
            timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            self.log_file_path = os.path.join(self.logs_dir, f"log_{timestamp}.txt")
            self.get_logger().info(f"Logging started: {self.log_file_path}")

        with open(self.log_file_path, "a") as f:
            f.write(f"[{datetime.now().strftime('%H:%M:%S.%f')}] {raw_data}\n")

    def on_message(self, client, userdata, msg):
        payload_str = msg.payload.decode()
        self.get_logger().info(f"Received MQTT data: {payload_str}")
        
        self.log_to_file(payload_str)

        try:
            data = json.loads(payload_str)
            current_time = self.get_clock().now().to_msg()

            # --- 3. BUILD & PUBLISH WHEEL ODOMETRY ---
            odom_msg = Odometry()
            odom_msg.header.stamp = current_time
            odom_msg.header.frame_id = "odom"       # Parent tracking frame
            odom_msg.child_frame_id = "base_link"   # Robot body frame
            
            # Positions calculated from encoders
            odom_msg.pose.pose.position.x = float(data['x'])
            odom_msg.pose.pose.position.y = -float(data['y'])
            odom_msg.pose.pose.position.z = 0.15     # Keeping it on the 2D plane
            
            # Convert the encoder yaw value directly to a 2D quaternion
            yaw = float(data['yaw_enc'])
            odom_msg.pose.pose.orientation.x = 0.0
            odom_msg.pose.pose.orientation.y = 0.0
            odom_msg.pose.pose.orientation.z = math.sin(yaw / 2.0)
            odom_msg.pose.pose.orientation.w = math.cos(yaw / 2.0)
            odom_msg.twist.twist.linear.x = float(data.get('v_lin', 0.0))
            
            p_cov = [0.0] * 36
            p_cov[0] = 0.01 # x
            p_cov[7] = 0.01 # y
            p_cov[35] = 0.02418 # Yaw_enc
            odom_msg.pose.covariance = p_cov

            t_cov = [0.0] * 36
            t_cov[0] = 0.1 # velocidade linear
            odom_msg.twist.covariance = t_cov
            
            self.odom_pub.publish(odom_msg)

            # --- 4. BUILD & PUBLISH MPU IMU ---
            imu_msg = Imu()
            imu_msg.header.stamp = current_time
            imu_msg.header.frame_id = "base_link"
            
            # Absolute orientation from the MPU DMP / filter processing
            imu_msg.orientation.x = 0.0
            imu_msg.orientation.y = 0.0
            imu_msg.orientation.z = float(data['quat_z'])
            imu_msg.orientation.w = float(data['quat_w'])
            
            imu_msg.angular_velocity.z = float(data.get('v_ang', 0.0))

            imu_cov = [0.0] * 9
            imu_cov[0] = 99999.0    # roll
            imu_cov[4] = 99999.0    # pitch
            imu_cov[8] = 0.0001998  # yaw

            imu_msg.orientation_covariance = imu_cov
            imu_msg.angular_velocity_covariance = imu_cov
            
            self.imu_pub.publish(imu_msg)

            self.get_logger().info("Dispatched raw sensor telemetry to EKF topics.")
            
        except Exception as e:
            self.get_logger().error(f"Error parsing or publishing sensor streams: {e}")

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
