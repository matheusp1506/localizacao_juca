import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from ros_gz_interfaces.srv import SetEntityPose
from geometry_msgs.msg import Pose
import os
from datetime import datetime

class EkfToGazeboTeleporter(Node):
    def __init__(self):
        super().__init__('ekf_to_gazebo_teleporter')
        
        # 1. Initialize the Service Client for Gazebo
        self.srv_name = '/world/empty/set_pose'
        self.client = self.create_client(SetEntityPose, self.srv_name)
        
        while not self.client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Waiting for Gazebo set_pose service...')

        self.log_file_path = None
        self.logs_dir = "logsEkf"

        # 2. Subscribe to the EKF Filtered Output
        self.subscription = self.create_subscription(
            Odometry,
            '/odometry/filtered',
            self.ekf_callback,
            10
        )
        self.get_logger().info('Teleporter node initialized. Fusing EKF output to Gazebo.')

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

    def ekf_callback(self, msg: Odometry):
        # Create the service request
        req = SetEntityPose.Request()
        req.entity.name = "juca_twin"
        req.entity.type = 2  # Entity Type: MODEL
        
        self.log_to_file(msg)
        
        # Extract the pose directly from the EKF odometry message
        req.pose = msg.pose.pose
        req.pose.position.z = 0.09
        
        # Call Gazebo asynchronously
        self.client.call_async(req)

def main(args=None):
    rclpy.init(args=args)
    node = EkfToGazeboTeleporter()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
