import paho.mqtt.client as mqtt
import time
import json
import math

# MQTT Setup
BROKER = "localhost" # Or your computer's IP
TOPIC = "juca/imu/quaternion"

client = mqtt.Client()
client.connect(BROKER, 1883, 60)

# Initial State
x, y, theta = 0.0, 0.0, 0.0

print("Starting Mock Robot data stream...")

while True:
    # Simulate a circular movement
    theta += 0.05
    x = 2.0 * math.cos(theta)
    y = 2.0 * math.sin(theta)
    
    # Simple 2D Quaternion (Z/W) calculation
    quat_z = math.sin(theta / 2.0)
    quat_w = math.cos(theta / 2.0)

    # Match your odom_final_data_t struct
    data = {
        "x": x,
        "y": y,
        "quat_z": quat_z,
        "quat_w": quat_w,
        "linear_velocity": 0.5,
        "angular_velocity": 0.05
    }

    client.publish(TOPIC, json.dumps(data))
    time.sleep(0.05) # 20Hz update rate
