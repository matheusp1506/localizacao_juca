import paho.mqtt.client as mqtt
import json
import time
import math

# Configuration
BROKER = "localhost" 
TOPIC = "esp32/odom_raw"

# Start MQTT client
client = mqtt.Client()
client.connect(BROKER, 1883, 60)

print("Starting Fake ESP32... Publishing to MQTT")

angle = 0.0
while True:
    # Simulate a circular path and publish odometry data
    x = math.cos(angle)
    y = math.sin(angle)
    yaw = angle + (math.pi / 2) # Facing the direction of travel

    data = {
        "x": round(x, 3),
        "y": round(y, 3),
        "yaw": round(yaw, 3)
    }

    client.publish(TOPIC, json.dumps(data))
    print(f"Sent: {data}")

    angle += 0.1
    time.sleep(0.1) # 10Hz
