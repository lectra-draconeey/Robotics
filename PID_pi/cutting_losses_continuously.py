import socket
import serial

UDP_IP = "0.0.0.0"
UDP_PORT = 8888

max_width = 640
max_height = 480

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print(f"Listening on {UDP_IP}:{UDP_PORT}...")

try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=0.001)
    print("Serial connection established")
except Exception as e:
    print(f"Serial connection failed: {e}")
    ser = None 

try:
    while True:
        data, addr = sock.recvfrom(1024)
        message = data.decode().strip()
        offset_str, width_str, height_str = message.split(',')

        offset = int(offset_str)
        box_width = int(width_str)
        box_height = int(height_str)

        # Scale offset to -64..64
        steer = int((offset / (max_width / 2)) * 64)
        steer = max(-64, min(64, steer))

        steer_cmd = f"STEER:{steer}\n"
        ser.write(steer_cmd.encode())
        print(f"Sent: {steer_cmd.strip()}")

        if box_width < max_width and box_height < 0.85 * max_height:
            ser.write(b'W\n')
            print("Forward")
        else:
            ser.write(b'X\n')
            print("Stop")

except KeyboardInterrupt:
    print("Stopped")
    ser.close()
