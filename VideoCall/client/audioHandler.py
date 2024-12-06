import socket
import pyaudio
import numpy as np
import librosa
import speech_recognition as sr
from threading import Thread, Lock
from io import BytesIO
import wave
import os
from pydub import AudioSegment
import time
import asyncio
from queue import Queue
import websockets
import json

# WebSocket 클라이언트 관리를 위한 전역 변수
connected_clients = set()

async def websocket_handler(websocket, path):
    """WebSocket 연결 처리"""
    print(f"새로운 WebSocket 클라이언트 연결됨")
    connected_clients.add(websocket)
    try:
        async for message in websocket:
            pass  # 클라이언트로부터의 메시지는 현재 무시
    finally:
        connected_clients.remove(websocket)
        print(f"WebSocket 클라이언트 연결 해제됨")

async def broadcast_stt_result(text):
    """STT 결과를 모든 연결된 클라이언트에게 전송"""
    message = json.dumps({
        "type": "stt_result",
        "text": text,
        "timestamp": time.time()
    })
    if connected_clients:
        await asyncio.gather(
            *[client.send(message) for client in connected_clients]
        )

class AudioProcessor:
    def __init__(self):
        # UDP 설정
        self.UDP_IP = "0.0.0.0"
        self.UDP_PORT = 5005
        self.BUFFER_SIZE = 1024

        # PyAudio 설정
        self.FORMAT = pyaudio.paInt16
        self.CHANNELS = 1
        self.RATE = 44100

        # 잡음 제거 설정
        self.noise_estimation_frames = 20
        self.noise_spectrum = None
        self.frame_count = 0
        self.snr_threshold = 2.0
        self.amplification_factor = 1.5

        # STT 설정
        self.recognizer = sr.Recognizer()
        self.buffer = BytesIO()
        self.last_audio_time = time.time()
        self.silence_threshold = 150  # 임계값 증가
        self.silence_duration = 1
        self.file_counter = 0

        # 소켓 설정 업데이트
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((self.UDP_IP, self.UDP_PORT))
        self.sock.listen(5)  # 대기 큐 크기 증가
        
        # 연결 타임아웃 설정
        self.sock.settimeout(60)  # 60초 타임아웃
        
        try:
            print("클라이언트 연결 대기 중...")
            self.conn, addr = self.sock.accept()
            self.conn.settimeout(5)  # 데이터 수신 타임아웃
            print(f"클라이언트가 연결됨: {addr}")
        except socket.timeout:
            raise Exception("연결 타임아웃")

        # PyAudio 초기화
        self.audio = pyaudio.PyAudio()
        self.stream = self.audio.open(
            format=self.FORMAT,
            channels=self.CHANNELS,
            rate=self.RATE,
            output=True
        )

        # 추가: 동시 처리 방지를 위한 락
        self.processing_lock = Lock()

        # 추가: STT 처리를 위한 큐
        self.audio_queue = Queue()
        self.stt_thread = Thread(target=self.process_stt_queue, daemon=True)
        self.stt_thread.start()

        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)

    def extend_audio(self, file_path, min_duration=1.0):
        """오디오 파일을 최소 지정된 길이로 확장"""
        audio = AudioSegment.from_wav(file_path)
        current_duration = len(audio) / 1000.0

        if current_duration >= min_duration:
            return file_path

        silence_duration = int((min_duration - current_duration) * 1000)
        silence = AudioSegment.silent(duration=silence_duration // 2)
        extended_audio = silence + audio + silence
        
        output_path = "extended_" + os.path.basename(file_path)
        extended_audio.export(output_path, format="wav")
        
        return output_path

    def process_stt_queue(self):
        """별도 스레드에서 STT 처리"""
        while True:
            audio_data = self.audio_queue.get()
            if audio_data is None:
                break
            
            try:
                audio_buffer = BytesIO(audio_data)
                with wave.open(audio_buffer, 'wb') as wav_file:
                    wav_file.setnchannels(1)
                    wav_file.setsampwidth(2)
                    wav_file.setframerate(self.RATE)
                    wav_file.writeframes(audio_data)
                
                audio_buffer.seek(0)
                with sr.AudioFile(audio_buffer) as source:
                    # self.recognizer.adjust_for_ambient_noise(source, duration=1)
                    audio = self.recognizer.record(source)
                    text = self.recognizer.recognize_google(audio, language='ko-KR')
                    print(f'상대방: {text}')
                    
                    # WebSocket으로 결과 전송
                    asyncio.run_coroutine_threadsafe(
                        broadcast_stt_result(text),
                        self.loop
                    )

            except Exception as e:
                print(f"STT 처리 중 오류: {e}")
            finally:
                audio_buffer.close()

    def process_audio(self):
        """메인 오디오 처리 루프"""
        print("오디오 스트림 처리 시작...")
        buffer = b''
        try:
            while True:
                try:
                    data = self.conn.recv(self.BUFFER_SIZE)
                    if not data:  # 연결이 끊어진 경우
                        print("클라이언트 연결이 끊어짐")
                        self.reconnect()
                        continue
                    
                    buffer += data
                    
                    # 완전한 프레임 처리
                    while len(buffer) >= self.BUFFER_SIZE:
                        frame = buffer[:self.BUFFER_SIZE]
                        buffer = buffer[self.BUFFER_SIZE:]
                        
                        audio_data = np.frombuffer(frame, dtype=np.int16).astype(np.float32)

                        # RMS 값 계산
                        rms = np.sqrt(np.mean(audio_data**2))

                        # STFT 변환
                        # S_full = librosa.stft(audio_data, n_fft=self.BUFFER_SIZE, hop_length=self.BUFFER_SIZE//2)
                        # S_magnitude, S_phase = np.abs(S_full), np.angle(S_full)

                        # 잡음 처리
                        # if self.frame_count < self.noise_estimation_frames:
                        #     self.noise_spectrum = S_magnitude if self.noise_spectrum is None else self.noise_spectrum + S_magnitude
                        #     self.frame_count += 1
                        #     if self.frame_count == self.noise_estimation_frames:
                        #         self.noise_spectrum /= self.noise_estimation_frames
                        # else:
                            # # 위너 필터 적용
                            # SNR_estimate = S_magnitude**2 / (self.noise_spectrum**2 + 1e-10)
                            # Wiener_filter = SNR_estimate / (SNR_estimate + 1)
                            # S_denoised = S_magnitude * Wiener_filter
                            
                            # # 선택적 증폭
                            # amplification_mask = (SNR_estimate > self.snr_threshold).astype(float)
                            # S_denoised = S_denoised * (1 + amplification_mask * (self.amplification_factor - 1))
                            
                            # # 신호 재구성
                            # S_reconstructed = S_denoised * np.exp(1j * S_phase)
                            # y_clean = librosa.istft(S_reconstructed, hop_length=self.BUFFER_SIZE//2)
                            
                            # 스피커 출력 (중단 없이 즉시 처리)
                            # output_data = (y_clean * 1.6).astype(np.int16)
                            
                            # self.stream.write(output_data.tobytes())
                        self.stream.write(data)
                        # STT 처리를 위한 버퍼링
                        if rms > self.silence_threshold:
                            # self.buffer.write(output_data.tobytes())
                            self.buffer.write(data)
                            self.last_audio_time = time.time()
                        else:
                            current_time = time.time()
                            if (current_time - self.last_audio_time > self.silence_duration 
                                and self.buffer.tell() > 0):
                                # 버퍼의 데이터를 STT 큐로 직접 전달
                                self.audio_queue.put(self.buffer.getvalue())
                                self.buffer.close()
                                self.buffer = BytesIO()
                
                except socket.timeout:
                    continue
                except ConnectionResetError:
                    print("연결이 재설정됨")
                    self.reconnect()
                    continue
                    
        except KeyboardInterrupt:
            print("프로그램 종료")
        finally:
            self.audio_queue.put(None)
            self.cleanup()

    def reconnect(self):
        """클라이언트 재연결 처리"""
        self.conn.close()
        try:
            print("클라이언트 재연결 대기 중...")
            self.conn, addr = self.sock.accept()
            self.conn.settimeout(5)
            print(f"클라이언트가 재연결됨: {addr}")
        except socket.timeout:
            print("재연결 타임아웃")
            raise Exception("재연결 실패")

    def cleanup(self):
        """리소스 정리"""
        self.stream.stop_stream()
        self.stream.close()
        self.audio.terminate()
        self.sock.close()

if __name__ == "__main__":
    processor = AudioProcessor()
    
    # WebSocket 서버 설정
    async def main():
        websocket_server = await websockets.serve(
            websocket_handler, 
            "localhost",  # 또는 "0.0.0.0"으로 모든 IP 허용
            8765  # WebSocket 포트
        )
        print("WebSocket 서버 시작됨 - ws://localhost:8765")
        await websocket_server.wait_closed()

    # WebSocket 서버 시작 
    processor.loop.create_task(main())
    
    # AudioProcessor를 별도 스레드에서 실행
    processing_thread = Thread(target=processor.process_audio, daemon=True)
    processing_thread.start()
    
    # 이벤트 루프 실행
    try:
        processor.loop.run_forever()
    except KeyboardInterrupt:
        print("\n프로그램 종료")
    finally:
        processor.cleanup()