import socket
import time

# === Connection ===
Pi_UDP_IP = "172.20.10.3"  # your Pi IP
OFFSET_PORT = 8888
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# === Parameters ===
offset = 0           # keep target centered for speed test
box_width = 40       # fixed for now

min_height = 100     # simulate far
max_height = 400     # simulate near
ramp_steps = 100     # steps up and down
ramp_delay = 0.05    # seconds per step

# === Ramp up box height ===
for i in range(ramp_steps):
    box_height = int(min_height + (max_height - min_height) * (i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

# === Ramp down box height ===
for i in range(ramp_steps):
    box_height = int(min_height + (max_height - min_height) * (1 - i / ramp_steps))
    msg = f"{offset},{box_width},{box_height}"
    print(f"SEND: {msg}")
    sock.sendto(msg.encode(), (Pi_UDP_IP, OFFSET_PORT))
    time.sleep(ramp_delay)

print("=== SPEED RAMP TEST COMPLETE ===")
