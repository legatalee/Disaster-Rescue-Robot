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
        <div id="grid"></div>
    </div>
    <script>
    function tempToColor(temp) {
        let r, g, b;

        if (temp < 5) {
            // 매우 차가운 온도는 보라색 계열
            r = 128; g = 0; b = 128; // 보라색
        } else if (temp < 15) {
            // 0도에서 15도까지는 파란색 계열로 그라데이션
            r = 0;
            g = Math.round((temp / 15) * 255); // 0에서 255까지
            b = 255;
        } else if (temp < 25) {
            // 15도에서 23도까지는 초록색 계열로 그라데이션
            r = 0;
            g = 255;
            b = Math.round((1 - (temp - 15) / 8) * 255); // 255에서 0까지
        } else if (temp < 35) {
            // 23도에서 28도까지는 노란색 계열로 그라데이션
            r = Math.round((temp - 23) / 5 * 255); // 0에서 255까지
            g = 255;
            b = 0;
        } else if (temp < 60) {
            // 28도에서 35도까지는 주황색 계열로 그라데이션
            r = 255;
            g = Math.round((1 - (temp - 28) / 7) * 255); // 255에서 0까지
            b = 0;
        } else {
            // 35도 이상은 빨간색
            r = 255; g = 0; b = 0; // 빨간색
        }

        return `rgb(${r}, ${g}, ${b})`;
    }

    function fetchTemperature() {
        fetch('/temperature')
            .then(response => response.json())
            .then(data => {
                const grid = document.getElementById("grid");
                grid.innerHTML = "";  // 기존 그리드 비우기
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
    setInterval(fetchTemperature, 1000); // 1초마다 온도 데이터 요청
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
picam2.configure(picam2.create_video_configuration(main={"size": (640, 480)}, transform=libcamera.Transform(hflip=1, vflip=1)))
output = StreamingOutput()
picam2.start_recording(JpegEncoder(), FileOutput(output))

try:
    address = ('', 8000)
    server = StreamingServer(address, StreamingHandler)
    print("Server started at http://localhost:8000")
    server.serve_forever()
finally:
    picam2.stop_recording()
