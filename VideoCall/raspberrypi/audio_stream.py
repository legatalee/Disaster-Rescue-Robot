import socket
import pyaudio
import time

# Audio parameters
CHUNK = 1024
FORMAT = pyaudio.paInt16  # Audio format (16-bit PCM)
CHANNELS = 1  # Mono audio
RATE = 44100  # Sample rate

# Receiver IP address
UDP_IP = "192.168.138.103"  # Receiver IP
UDP_PORT = 5002  # Receiver port

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Initialize PyAudio
p = pyaudio.PyAudio()
stream = p.open(format=FORMAT, channels=CHANNELS, rate=RATE,
                input=True, frames_per_buffer=CHUNK)

print(f"Streaming audio to {UDP_IP}:{UDP_PORT}...")

try:
    while True:
        # Read audio data from the microphone
        data = stream.read(CHUNK, exception_on_overflow=False)

        # Send raw audio data over UDP
        sock.sendto(data, (UDP_IP, UDP_PORT))

        # Log the sent data
        print(f"Sent {len(data)} bytes of audio data")

        # Optional: Sleep to mimic buffer size behavior (adjust as needed)
        time.sleep(0.01)  # Adjust the sleep time as needed to match your desired delay

except KeyboardInterrupt:
    print("Streaming stopped by user.")

finally:
    # Clean up
    stream.stop_stream()
    stream.close()
    p.terminate()
    sock.close()
