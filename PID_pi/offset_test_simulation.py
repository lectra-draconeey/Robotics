import socket
import time

# === Connection ===
Pi_UDP_IP = "172.20.10.3"  # your Pi IP
OFFSET_PORT = 8888
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# === Parameters ===
max_offset = 300   # peak offset left/right
box_width = 40     # fixed for now
box_height = 200   # below clamp threshold for testing

ramp_steps = 100   # number of steps in ramp
ramp_delay = 0.05  # seconds per step

# === Ramp up offset ===
for i in range(ramp_steps):
    # ramp from 0 to max_offset
    offset = int(max_offset * (i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

# === Ramp down offset ===
for i in range(ramp_steps):
    offset = int(max_offset * (1 - i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

# === Ramp to other side ===
for i in range(ramp_steps):
    offset = int(-max_offset * (i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

# === Ramp back to zero ===
for i in range(ramp_steps):
    offset = int(-max_offset * (1 - i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

print("=== RAMP TEST COMPLETE ===")
