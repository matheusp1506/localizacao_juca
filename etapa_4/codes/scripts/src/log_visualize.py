import json
import re
import math
import os
import webbrowser
import tkinter as tk
from tkinter import filedialog

def select_log_file():
    """Opens a file dialog window for the user to select a log file."""
    root = tk.Tk()
    root.withdraw()
    
    file_path = filedialog.askopenfilename(
        title="Select ROS 2 Odometry Log File",
        filetypes=[("Text Files", "*.txt"), ("Log Files", "*.log"), ("All Files", "*.*")]
    )
    return file_path

def parse_ros2_odom_log(file_path):
    """
    Parses the ROS2 nav_msgs.msg.Odometry dump files into dictionary waypoints.
    """
    waypoints = []
    
    pos_pattern = re.compile(r"position=geometry_msgs\.msg\.Point\(\s*x\s*=\s*([-+]?\d*\.\d+|\d+),\s*y\s*=\s*([-+]?\d*\.\d+|\d+)")
    quat_pattern = re.compile(
        r"orientation=geometry_msgs\.msg\.Quaternion\(\s*x\s*=\s*([-+]?\d*\.\d+|\d+),\s*y\s*=\s*([-+]?\d*\.\d+|\d+),\s*z\s*=\s*([-+]?\d*\.\d+|\d+),\s*w\s*=\s*([-+]?\d*\.\d+|\d+)\)"
    )

    print(f"Reading and parsing log file: {os.path.basename(file_path)}...")
    
    # Thresholds to detect when the robot resets to origin (0, 0) and 0 degrees heading
    POS_TOLERANCE = 0.05    # 5 centimeters
    ANGLE_TOLERANCE = 2.0   # 2 degrees

    with open(file_path, 'r', encoding='utf-8') as f:
        for index, line in enumerate(f):
            pos_match = pos_pattern.search(line)
            if pos_match:
                try:
                    x = float(pos_match.group(1))
                    y = float(pos_match.group(2))
                    
                    quat_match = quat_pattern.search(line)
                    heading = 0.0
                    if quat_match:
                        qx = float(quat_match.group(1))
                        qy = float(quat_match.group(2))
                        qz = float(quat_match.group(3))
                        qw = float(quat_match.group(4))
                        
                        siny_cosp = 2.0 * (qw * qz + qx * qy)
                        cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
                        heading = round(math.degrees(math.atan2(siny_cosp, cosy_cosp)), 1)
                    
                    # Detect potential restart points
                    is_restart = False
                    if abs(x) <= POS_TOLERANCE and abs(y) <= POS_TOLERANCE and abs(heading) <= ANGLE_TOLERANCE:
                        # Ensure it's not just the first point of the file unless intentional
                        is_restart = True

                    waypoints.append({
                        "index": len(waypoints),
                        "x": x,
                        "y": y,
                        "heading": heading,
                        "is_restart": is_restart
                    })
                except Exception:
                    continue
                    
    return waypoints

def generate_html_dashboard(data, log_file_name, output_html="robot_dashboard.html"):
    if not data:
        print("[x] Error: No valid position points found to generate a dashboard.")
        return

    # Separate standard route points from suspected restart markers for ChartJS plotting layers
    restarts = [{"x": d["x"], "y": d["y"], "idx": d["index"]} for d in data if d["is_restart"]]
    
    html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>Robot Trajectory Workspace</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #111; color: #eee; margin: 30px; }}
        .grid {{ display: grid; grid-template-columns: 2fr 1fr; gap: 20px; }}
        .card {{ background: #222; padding: 20px; border-radius: 8px; border: 1px solid #333; margin-bottom: 20px; }}
        h2 {{ margin-top: 0; color: #4CAF50; }}
        .slider-container {{ background: #1a1a1a; padding: 15px; border-radius: 6px; border: 1px solid #444; margin-bottom: 20px; }}
        .slider-group {{ display: flex; align-items: center; gap: 15px; margin: 10px 0; }}
        .slider-group label {{ width: 100px; font-weight: bold; color: #4CAF50; }}
        .slider-group input {{ flex: 1; }}
        .slider-val {{ font-family: monospace; min-width: 50px; text-align: right; }}
    </style>
</head>
<body>
    <h1>🤖 Robot Telemetry Dashboard</h1>
    <p style="color: #888; margin-top: -10px; margin-bottom: 20px;">Active Source Profile: {log_file_name}</p>
    
    <div class="slider-container">
        <h2>⏱️ Filter Path History Window</h2>
        <div class="slider-group">
            <label>Start Point:</label>
            <input type="range" id="startSlider" min="0" max="{len(data)-1}" value="0" oninput="updateFilter()">
            <span id="startVal" class="slider-val">0</span>
        </div>
        <div class="slider-group">
            <label>End Point:</label>
            <input type="range" id="endSlider" min="0" max="{len(data)-1}" value="{len(data)-1}" oninput="updateFilter()">
            <span id="endVal" class="slider-val">{len(data)-1}</span>
        </div>
    </div>

    <div class="grid">
        <div class="card">
            <h2>🗺️ 2D XY Coordinates Map Path</h2>
            <canvas id="xyChart" style="max-height: 550px;"></canvas>
        </div>
        <div>
            <div class="card">
                <h2>🧭 Heading Angle (Degrees)</h2>
                <canvas id="headingChart" style="max-height: 400px;"></canvas>
            </div>
        </div>
    </div>

    <script>
        // Raw dataset compiled from python logs
        const rawDataset = {json.dumps(data)};
        const restartPoints = {json.dumps(restarts)};

        let xyChart, headingChart;

        function renderCharts() {{
            const ctxXy = document.getElementById('xyChart').getContext('2d');
            xyChart = new Chart(ctxXy, {{
                type: 'scatter',
                data: {{
                    datasets: [
                        {{
                            label: 'Robot Route Trajectory',
                            data: rawDataset.map(d => ({{x: d.x, y: d.y}})),
                            showLine: true,
                            borderColor: '#4682B4',
                            backgroundColor: 'rgba(70, 130, 180, 0.3)',
                            borderWidth: 3,
                            pointRadius: 1
                        }},
                        {{
                            label: 'Possible Restart Location (0,0)',
                            data: restartPoints,
                            borderColor: '#FF3333',
                            backgroundColor: '#FF3333',
                            pointRadius: 8,
                            pointHoverRadius: 10,
                            pointStyle: 'triangle',
                            showLine: false
                        }}
                    ]
                }},
                options: {{
                    responsive: true,
                    scales: {{
                        x: {{ title: {{ display: true, text: 'X Position (meters)', color: '#aaa' }}, grid: {{ color: '#333' }} }},
                        y: {{ title: {{ display: true, text: 'Y Position (meters)', color: '#aaa' }}, grid: {{ color: '#333' }} }}
                    }}
                }}
            }});

            const ctxHeading = document.getElementById('headingChart').getContext('2d');
            headingChart = new Chart(ctxHeading, {{
                type: 'line',
                data: {{
                    labels: rawDataset.map(d => d.index),
                    datasets: [{{ 
                        label: 'Heading (°)', 
                        data: rawDataset.map(d => d.heading), 
                        borderColor: '#29B5E8', 
                        pointRadius: 0,
                        borderWidth: 2
                    }}]
                }},
                options: {{ 
                    responsive: true, 
                    scales: {{ 
                        x: {{ title: {{ display: true, text: 'Log Timeline Progress (Ticks)', color: '#aaa' }}, grid: {{ color: '#333' }} }}, 
                        y: {{ grid: {{ color: '#333' }} }} 
                    }} 
                }}
            }});
        }}

        function updateFilter() {{
            let startIdx = parseInt(document.getElementById('startSlider').value);
            let endIdx = parseInt(document.getElementById('endSlider').value);

            // Keep index bounding clean
            if (startIdx > endIdx) {{
                document.getElementById('startSlider').value = endIdx;
                startIdx = endIdx;
            }}

            document.getElementById('startVal').innerText = startIdx;
            document.getElementById('endVal').innerText = endIdx;

            // Filter active data arrays
            const filteredData = rawDataset.slice(startIdx, endIdx + 1);
            const filteredRestarts = restartPoints.filter(pt => pt.idx >= startIdx && pt.idx <= endIdx);

            // Update Map Charts
            xyChart.data.datasets[0].data = filteredData.map(d => ({{x: d.x, y: d.y}}));
            xyChart.data.datasets[1].data = filteredRestarts;
            xyChart.update('none'); // Update smoothly without layout jump animations

            // Update Orientation Timeline Charts
            headingChart.data.labels = filteredData.map(d => d.index);
            headingChart.data.datasets[0].data = filteredData.map(d => d.heading);
            headingChart.update('none');
        }}

        // Initial setup execution
        renderCharts();
    </script>
</body>
</html>
"""
    with open(output_html, "w", encoding="utf-8") as f:
        f.write(html_content)
    
    full_path = os.path.abspath(output_html)
    webbrowser.open(f"file://{full_path}")
    print(f"[✔] Visualizer dashboard updated and launched successfully: file://{full_path}")

if __name__ == "__main__":
    chosen_file = select_log_file()
    if chosen_file:
        log_data = parse_ros2_odom_log(chosen_file)
        file_base_name = os.path.basename(chosen_file)
        generate_html_dashboard(log_data, file_base_name)
    else:
        print("[x] Action cancelled: No file was selected.")