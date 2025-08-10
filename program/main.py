import serial
import csv
import time
import matplotlib.pyplot as plt
from collections import deque

# === Konfigurasi ===
PORT = 'COM7'
BAUD_RATE = 115200
CSV_FILE = 'data_ultrasonic.csv'
BUFFER_SIZE = 100
INTERVAL = 0.5  # dalam detik (500ms)
ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
time.sleep(2)

csv_file = open(CSV_FILE, mode='w', newline='')
csv_writer = csv.writer(csv_file)
csv_writer.writerow(['Waktu', 'Distance (cm)', 'Duration (us)'])

# === Setup grafik real-time ===
plt.ion()
fig, ax = plt.subplots()
distance_data = deque([0]*BUFFER_SIZE, maxlen=BUFFER_SIZE)
duration_data = deque([0]*BUFFER_SIZE, maxlen=BUFFER_SIZE)
x_data = deque(range(BUFFER_SIZE), maxlen=BUFFER_SIZE)

line1, = ax.plot(x_data, distance_data, label='Distance (cm)')
line2, = ax.plot(x_data, duration_data, label='Duration (us)', color='orange')
ax.set_ylim(0, 1000)
ax.set_title("Grafik Real-Time Sensor Ultrasonik")
ax.set_xlabel("Data Ke-")
ax.set_ylabel("Nilai")
ax.legend()

# === Loop utama ===
last_time = time.time()

try:
    while True:
        now = time.time()
        if now - last_time >= INTERVAL:
            line = ser.readline().decode('utf-8').strip()
            if line:
                try:
                    parts = line.split(',')
                    distance = float(parts[0].split(':')[1])
                    duration = float(parts[1].split(':')[1])
                    timestamp = time.strftime('%Y-%m-%d %H:%M:%S')

                    # Simpan ke CSV
                    csv_writer.writerow([timestamp, distance, duration])

                    # Update grafik
                    distance_data.append(distance)
                    duration_data.append(duration)
                    line1.set_ydata(distance_data)
                    line2.set_ydata(duration_data)
                    ax.relim()
                    ax.autoscale_view()
                    fig.canvas.draw()
                    fig.canvas.flush_events()

                    print(f"[{timestamp}] Volume: {distance} cm, Durasi: {duration} us")
                except (IndexError, ValueError):
                    print("Format data tidak valid:", line)
            last_time = now
except KeyboardInterrupt:
    print("\nPerekaman dihentikan oleh pengguna.")
finally:
    ser.close()
    csv_file.close()
