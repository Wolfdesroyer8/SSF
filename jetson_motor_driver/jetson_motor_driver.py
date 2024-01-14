import time
import serial

Nano = serial.Serial(port="/dev/ttyUSB0", baudrate=9600, timeout=1)


def serial_write(text):
    Nano.write((text + "\n").encode("utf-8"))


def serial_read():
    return Nano.readline().decode().strip()


while True:
    print("Attempting connect to Nano")
    serial_write("p")
    if serial_read() == "Pong!":
        break
    time.sleep(1)
print("Connected to Nano")
for i in range(30, 101, 1):
    serial_write(f"s {i/100}")
    print(serial_read())
    serial_write("r")
    print(serial_read())
    time.sleep(0.08)
time.sleep(4)

serial_write("s 0")
print(serial_read())
