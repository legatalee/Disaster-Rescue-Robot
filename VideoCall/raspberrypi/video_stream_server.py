import time
import sys
import io
import logging
import socketserver
from http import server
from threading import Condition, Thread
import pyaudio
import numpy as np
from picamera2 import Picamera2
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput
import libcamera
import amg8833_i2c
import json
import subprocess
import socket
import asyncio

PAGE = """\
<html>
<head>
    <title>Disaster Robot</title>
    <meta charset="utf-8">
    <style>
        body {
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .container {
            display: flex;
            justify-content: center;
            align-items: flex-start;
            max-width: 1200px; /* 최대 너비 설정 */
            width: 100%;
            margin: 20px; /* 주변 여백 설정 */
        }
        img {
            display: block;
            width: 800px; /* 고정 너비 */
            height: auto;
            margin-right: 20px; /* 이미지와 그리드 간의 간격 */
        }
        #grid {
            display: grid;
            grid-template-columns: repeat(8, 50px);
            grid-gap: 0; /* 간격 없애기 */
        }
        .cell {
            width: 50px;
            height: 50px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 14px;
            color: white;
            margin: 0; /* 간격 없애기 */
            border: none; /* 테두리 없애기 */
        }
        @media (max-width: 800px) {
            .container {
                flex-direction: column; /* 세로 방향으로 변경 */
                align-items: center;
            }
            img {
                width: 100%; /* 화면 폭에 맞게 이미지 크기 조정 */
                max-width: 800px; /* 최대 너비 설정 */
                margin-right: 0; /* 이미지 오른쪽 여백 제거 */
            }
        }
    </style>
</head>
<body>
    <h1>재난 구조 현장 화면</h1>
    <div class="container">
        <img src="stream.mjpg" />
        <canvas id="thermalCanvas" width="480" height="480"></canvas>
    </div>

    <script>
    function tempToColor(temp, minTemp = 12, maxTemp = 43) {
        const clampedTemp = Math.max(minTemp, Math.min(temp, maxTemp)); // Clamp temp to range
        const ratio = (clampedTemp - minTemp) / (maxTemp - minTemp);
        const r = Math.round(255 * (1 - ratio));
        const b = Math.round(255 * ratio);
        return `rgb(${r}, 0, ${b})`; // Gradient from blue to red
    }

    function interpolateGrid(data, scale) {
        const height = data.length;
        const width = data[0].length;
        const newHeight = height * scale;
        const newWidth = width * scale;
        const result = Array.from({ length: newHeight }, () =>
            Array(newWidth).fill(0)
        );

        for (let y = 0; y < newHeight; y++) {
            const y0 = Math.floor(y / scale);
            const yT = (y / scale) - y0;
            const yPoints = [
                data[Math.max(0, y0 - 1)] || data[y0],
                data[y0],
                data[Math.min(height - 1, y0 + 1)] || data[y0],
                data[Math.min(height - 1, y0 + 2)] || data[y0]
            ];

            for (let x = 0; x < newWidth; x++) {
                const x0 = Math.floor(x / scale);
                const xT = (x / scale) - x0;
                const p = yPoints.map(row =>
                    cubicInterpolate(
                        row[Math.max(0, x0 - 1)] || row[x0],
                        row[x0],
                        row[Math.min(width - 1, x0 + 1)] || row[x0],
                        row[Math.min(width - 1, x0 + 2)] || row[x0],
                        xT
                    )
                );
                result[y][x] = cubicInterpolate(p[0], p[1], p[2], p[3], yT);
            }
        }
        return result;
    }

    function cubicInterpolate(p0, p1, p2, p3, t) {
        return (
            0.5 *
            ((-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t +
                (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
                (-p0 + p2) * t +
                2 * p1)
        );
    }

    function fetchTemperature() {
        fetch('/temperature')
            .then(response => response.json())
            .then(data => {
                const grid = document.getElementById("grid");
                grid.innerHTML = "";  // 기존 그리드 비우기
                console.log(data);
                data.forEach((temp, index) => {
                    const cell = document.createElement("div");
                    cell.className = "cell";
                    
                    // 온도에 따른 색상 조정
                    cell.style.backgroundColor = tempToColor(temp);

                    cell.innerText = temp.toFixed(2);  // 소수점 2자리로 표시
                    grid.appendChild(cell);
                });
            })
            .catch(error => console.error('온도 데이터 가져오기 오류:', error));
    }

    function drawGrid(canvas, grid, minTemp = 12, maxTemp = 43) {
        const ctx = canvas.getContext("2d");
        const cellWidth = canvas.width / grid[0].length;
        const cellHeight = canvas.height / grid.length;

        // Clear the canvas before drawing
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        for (let y = 0; y < grid.length; y++) {
            for (let x = 0; x < grid[0].length; x++) {
                const temp = grid[y][x];
                ctx.fillStyle = tempToColor(temp);
                ctx.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
            }
        }
    }

    // Main loop
    function main() {
        const canvas = document.getElementById("thermalCanvas");
        const scale = 6; // Interpolation scale factor

        setInterval(() => {
            fetch('/temperature')
            .then(response => response.json())
            .then(data => {
                data.forEach((temp, index) => {
                    const gridData = [];
                    for (let i = 0; i < 8; i++) {
                        gridData.push(data.slice(i * 8, (i + 1) * 8)); // 8개의 값을 한 줄로 묶어 8x8 그리드 생성
                    }
                    const interpolatedData = interpolateGrid(gridData, scale); // Interpolate
                    drawGrid(canvas, interpolatedData); // Draw interpolated grid
                });
            })
            .catch(error => console.error('온도 데이터 가져오기 오류:', error));
        }, 300);
    }

    main();
</script>
</body>
</html>
"""

class StreamingOutput(io.BufferedIOBase):
    def __init__(self):
        self.frame = None
        self.condition = Condition()

    def write(self, buf):
        with self.condition:
            self.frame = buf
            self.condition.notify_all()

class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            content = PAGE.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning('Removed streaming client %s: %s', self.client_address, str(e))
        elif self.path == '/temperature':
            # 온도 데이터를 JSON으로 제공
            status, pixels = sensor.read_temp(64)
            if status:
                self.send_response(500)
                self.end_headers()
                return
            thermistor_temp = sensor.read_thermistor()
            # 숫자 배열 형식으로 데이터 생성
            data = pixels  # 64개의 픽셀 데이터, 8x8 크기의 배열로 사용
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(str(data).encode('utf-8'))
        else:
            self.send_error(404)
            self.end_headers()

class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

# AMG8833 초기화
sensor = amg8833_i2c.AMG8833(addr=0x69)

# Start video streaming
picam2 = Picamera2(camera_num=0)
picam2.configure(picam2.create_video_configuration(main={"size": (720, 480)}, transform=libcamera.Transform(hflip=1, vflip=1)))
output = StreamingOutput()
picam2.start_recording(JpegEncoder(), FileOutput(output))

try:
    address = ('', 8000)
    server = StreamingServer(address, StreamingHandler)
    print("Server started at http://localhost:8000")
    server.serve_forever()
finally:
    picam2.stop_recording()