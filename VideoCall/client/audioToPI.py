import pyaudio
import socket

# UDP 소켓 설정
UDP_IP = "192.168.8.114"  # 수신하는 장치의 IP 주소
UDP_PORT = 5006       # 수신하는 장치의 포트 번호
BUFFER_SIZE = 1024    # 전송할 데이터 크기 (일반적으로 1024 또는 2048)

# 오디오 스트림 설정
FORMAT = pyaudio.paInt16  # 오디오 포맷 (16비트)
CHANNELS = 1              # 모노 채널
RATE = 44100              # 샘플링 레이트 (Hz)

# UDP 소켓 생성
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# PyAudio 객체 생성
audio = pyaudio.PyAudio()
stream = audio.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, frames_per_buffer=BUFFER_SIZE)

print("Streaming audio...")

try:
    while True:
        # 오디오 데이터를 읽고 UDP 소켓으로 전송
        data = stream.read(BUFFER_SIZE)
        sock.sendto(data, (UDP_IP, UDP_PORT))
except KeyboardInterrupt:
    print("Streaming stopped.")
finally:
    # 종료 처리
    stream.stop_stream()
    stream.close()
    audio.terminate()
    sock.close()
