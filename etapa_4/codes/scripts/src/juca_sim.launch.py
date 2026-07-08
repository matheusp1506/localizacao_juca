import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    # --- 1. Setup Paths ---
    # Replace this with the actual path to your juca.urdf
    urdf_file = os.path.expanduser('~/robot_sim_ws/juca.urdf')
    
    with open(urdf_file, 'r') as infp:
        robot_desc = infp.read()

    # --- 2. Gazebo Sim Launch ---
    # This opens the empty Gazebo world
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
        launch_arguments={'gz_args': '-r empty.sdf'}.items(), # -r runs the sim immediately
    )

    # --- 3. Robot State Publisher ---
    # Converts URDF to a format ROS 2 and Gazebo understand
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='both',
        parameters=[{'robot_description': robot_desc}]
    )

    # --- 4. Spawn the Robot ---
    # This uses the 'create' executable specifically for the new Gazebo
    gz_spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-topic', 'robot_description',
            '-name', 'juca_twin',
            '-allow_renaming', 'true'
        ],
        output='screen',
    )

    return LaunchDescription([
        gazebo,
        rsp_node,
        gz_spawn_entity
    ])
