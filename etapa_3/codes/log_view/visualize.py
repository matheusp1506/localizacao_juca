import json
import re
import math
import webbrowser
import os

def parse_juca_log(file_path):
    log_pattern = re.compile(r'(?:\[?(\d{2}:\d{2}:\d{2}\.\d+)\]?)?\s*(\{.*\})')
    waypoints = []
    
    with open(file_path, 'r') as f:
        for line in f:
            match = log_pattern.search(line.strip())
            if match:
                _, json_str = match.groups()
                try:
                    data = json.loads(json_str)
                    # Simple Heading extraction from Quaternions for the map info
                    if all(k in data for k in ['quat_z', 'quat_w', 'quat_x', 'quat_y']):
                        siny_cosp = 2.0 * (data['quat_w'] * data['quat_z'] + data['quat_x'] * data['quat_y'])
                        cosy_cosp = 1.0 - 2.0 * (data['quat_y'] * data['quat_y'] + data['quat_z'] * data['quat_z'])
                        data['heading'] = round(math.degrees(math.atan2(siny_cosp, cosy_cosp)), 1)
                    else:
                        data['heading'] = round(math.degrees(data.get('yaw_enc', 0)), 1)
                    waypoints.append(data)
                except:
                    continue
    return waypoints

def generate_html_dashboard(data, output_html="juca_map.html"):
    # Convert data into clear lists for ChartJS mapping scripts
    x_coords = [d['x'] for d in data]
    y_coords = [d['y'] for d in data]
    lin_vel = [d['linear_velocity'] for d in data]
    ang_vel = [d['angular_velocity'] for d in data]
    headings = [d['heading'] for d in data]
    
    html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>Juca Odometry Tracking Map</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #111; color: #eee; margin: 30px; }}
        .grid {{ display: grid; grid-template-columns: 2fr 1fr; gap: 20px; }}
        .card {{ background: #222; padding: 20px; border-radius: 8px; border: 1px solid #333; }}
        h2 {{ margin-top: 0; color: #4CAF50; }}
        .metrics {{ display: flex; gap: 20px; margin-bottom: 20px; }}
        .metric-box {{ background: #2a2a2a; padding: 15px; border-radius: 6px; flex: 1; text-align: center; border-left: 4px solid #4CAF50; }}
        .metric-val {{ font-size: 24px; font-weight: bold; color: #fff; margin-top: 5px; }}
    </style>
</head>
<body>
    <h1>🤖 Juca Robot Telemetry Dashboard</h1>
    
    <div class="metrics">
        <div class="metric-box">Points Tracked<div class="metric-val">{len(data)}</div></div>
        <div class="metric-box">Final X Target<div class="metric-val">{x_coords[-1] if x_coords else 0:.3f} m</div></div>
        <div class="metric-box">Final Y Target<div class="metric-val">{y_coords[-1] if y_coords else 0:.3f} m</div></div>
    </div>

    <div class="grid">
        <div class="card">
            <h2>🗺️ 2D XY Coordinates Map Path</h2>
            <canvas id="xyChart" style="max-height: 550px;"></canvas>
        </div>
        <div>
            <div class="card" style="margin-bottom: 20px;">
                <h2>📈 Linear Velocity</h2>
                <canvas id="velChart" style="max-height: 220px;"></canvas>
            </div>
            <div class="card">
                <h2>🧭 Heading Angle (Degrees)</h2>
                <canvas id="headingChart" style="max-height: 220px;"></canvas>
            </div>
        </div>
    </div>

    <script>
        const xyData = {json.dumps([{"x": x, "y": y} for x, y in zip(x_coords, y_coords)])};
        
        // Render 2D Position Mapping Space
        new Chart(document.getElementById('xyChart'), {{
            type: 'scatter',
            data: {{
                datasets: [{{
                    label: 'Robot Route Trajectory',
                    data: xyData,
                    showLine: true,
                    borderColor: '#4682B4',
                    backgroundColor: 'rgba(70, 130, 180, 0.5)',
                    borderWidth: 3,
                    pointRadius: 2
                }}]
            }},
            options: {{
                responsive: true,
                scales: {{
                    x: {{ title: {{ display: true, text: 'X Position (meters)', color: '#aaa' }}, grid: {{ color: '#333' }} }},
                    y: {{ title: {{ display: true, text: 'Y Position (meters)', color: '#aaa' }}, grid: {{ color: '#333' }}, aspectRatio: 1 }}
                }}
            }}
        }});

        // Render Velocity Profiles
        new Chart(document.getElementById('velChart'), {{
            type: 'line',
            data: {{
                labels: Array.from({{length: {len(data)}}}, (_, i) => i),
                datasets: [{{ label: 'Linear Vel (m/s)', data: {json.dumps(lin_vel)}, borderColor: '#FF4B4B', pointRadius: 0 }}]
            }},
            options: {{ responsive: true, plugins: {{ legend: {{ display: false }} }}, scales: {{ x: {{ display: false }}, y: {{ grid: {{ color: '#333' }} }} }} }}
        }});

        // Render Orientation Profiles
        new Chart(document.getElementById('headingChart'), {{
            type: 'line',
            data: {{
                labels: Array.from({{length: {len(data)}}}, (_, i) => i),
                datasets: [{{ label: 'Heading (°)', data: {json.dumps(headings)}, borderColor: '#29B5E8', pointRadius: 0 }}]
            }},
            options: {{ responsive: true, plugins: {{ legend: {{ display: false }} }}, scales: {{ x: {{ display: false }}, y: {{ grid: {{ color: '#333' }} }} }} }}
        }});
    </script>
</body>
</html>
"""
    with open(output_html, "w") as f:
        f.write(html_content)
    
    # Fire up the user's default browser securely
    full_path = os.path.abspath(output_html)
    # webbrowser.open(f"file://{full_path}")
    print(f"[✔] Visualizer page generated successfully: file://{full_path}")

if __name__ == "__main__":
    # Point directly to your log file name path
    log_file_name = "logs/log_2026-05-21_19-54-47.txt" 
    
    if os.path.exists(log_file_name):
        log_data = parse_juca_log(log_file_name)
        generate_html_dashboard(log_data)
    else:
        print(f"[x] Error: Could not find '{log_file_name}' file in the current directory.")
