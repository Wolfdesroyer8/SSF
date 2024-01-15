import time
import sys
from getkey import getkey, keys
import serial

Nano = serial.Serial(port="/dev/ttyUSB0", baudrate=9600, timeout=1)


def serial_write(text):
    Nano.write((text + "\n").encode("utf-8"))


def serial_read():
    return Nano.readline().decode().strip()


def run_motors():
    input("Press [enter] to start, then press [Ctrl C] to exit program")
    serial_write("s 1")
    serial_read()
    serial_write("m")
    serial_read()
    serial_write("s 1")
    serial_read()
    while True:
        serial_write("r")
        m1_rpm = serial_read()[13 : 13 + 5]
        serial_write("m")
        serial_read()
        serial_write("r")
        m2_rpm = serial_read()[13 : 13 + 5]
        serial_write("m")
        serial_read()
        print(f"Motor 1 Speed: {m1_rpm} | Motor 2 Speed: {m2_rpm}", end="\r")
        time.sleep(0.1)
    serial_write("s 0")
    serial_read()
    serial_write("m")
    serial_read()
    serial_write("s 0")
    serial_read()


def conf_motor_dirs():
    input("Press [enter] to start currently selected motor")
    serial_write("s 0.5")
    serial_read()
    serial_read()
    serial_read()
    while True:
        print("\t[1] Change motor direction")
        print("\t[2] Swap currently selected motor")
        print("\t[3] exit")
        choice = input()
        match choice:
            case "1":
                serial_write("s 0")
                serial_read()
                serial_write("t")
                while serial_read() == "ERROR: Stop motor before changing direction":
                    serial_write("t")
                    time.sleep(0.1)
                serial_read()
                serial_write("s 0.5")
                serial_read()
                print("Direction Changed")
            case "2":
                serial_write("s 0")
                serial_read()
                serial_write("m")
                print(serial_read())
                serial_write("s 0.5")
                serial_read()
            case "3":
                serial_write("s 0")
                serial_read()
                return
            case _:
                print("ERROR: Invalid input")


def main():
    while True:
        print("Attempting connect to Nano")
        serial_write("p")
        if serial_read() == "Pong!":
            break
        time.sleep(1)
    print("Connected to Nano")

    close = False
    while not close:
        print("What would you like to do?")
        print("\t[1] Configure motor directions")
        print("\t[2] Run both motors")
        print("\t[3] Exit")
        choice = input()

        match choice:
            case "1":
                conf_motor_dirs()
            case "2":
                run_motors()
            case "3":
                sys.exit()
            case _:
                print("ERROR: Invalid input")


main()
