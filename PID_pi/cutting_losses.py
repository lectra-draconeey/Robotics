import socket
import time
import serial
import select

# === Serial setup ===
UDP_IP = "0.0.0.0"       # 监听所有网卡
UDP_PORT = 8888

# === Camera setup ===
max_width = 640
max_height = 480
#The camera is set up to send data in the format "offset,width,height"

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False)

print(f"Listening on {UDP_IP}:{UDP_PORT}...")
try:
    ser = serial.Serial('/dev/ttyACM1', 115200, timeout=1)
    print("Serial connection established")
except Exception as e:
    print(f"Serial connection failed: {e}")
    ser = None    

while True:
    inputs = [sock]
    inputs.append(ser)

    rlist, _, _ = select.select(inputs, [], [],  1.0)
    print*(rlist)
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        print(f"Arduino says: {line}")
        if line.find("Clamped") != -1:
            print(f"Arduino says: {line} ")
            print("Clamp is closed")
            cmd = 'X'  # Stop the robot
            ser.write((cmd + '\n').encode())
            print(f"Sent: {cmd}")
            continue

    try:
    
        data, addr = sock.recvfrom(1024)
        message = data.decode().strip()
        offset_str, width_str, height_str = message.split(',')
        
        offset = int(offset_str) # box offset from center already calulated 
        box_width = int(width_str)
        box_height = int(height_str)

        print(f"offset: {offset}, box_width: {box_width}, box_height: {box_height}")

        if offset > 20:
            cmd = 'D'
        elif offset < -20:
            cmd = 'A'
        elif box_width < max_width and box_height < 0.60 * max_height:
            cmd = 'W'
        else:
            cmd = 'A' #turn left till you see the box

        ser.write((cmd + '\n').encode())
        print(f"Sent: {cmd}")
        time.sleep(0.1)                  

    except KeyboardInterrupt:
        print("Stopped")
        ser.close()

    except Exception as e:
        print(f"wrong:{e}")    
