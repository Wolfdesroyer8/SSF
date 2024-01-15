import time
import sys
import serial

Nano = serial.Serial(port="/dev/ttyUSB0", baudrate=9600, timeout=1)


def serial_write(text):
    Nano.write((text + "\n").encode("utf-8"))


def serial_read():
    return Nano.readline().decode().strip()


def run_motors():
    # ramp up speed while printing speed
    for i in range(30, 101, 1):
        serial_write(f"s {i/100}")
        speed = serial_read()
        time.sleep(0.5)
        serial_write("r")
        rpm = serial_read()
        print(speed, rpm, end="\r")
    print("")
    for i in range(0, 40, 1):
        serial_write("r")
        print(serial_read(), end="\r")
        time.sleep(0.1)
    print("")

    serial_write("s 0")
    print(serial_read())


def conf_motor_dirs():
    input("Press [enter] to start the motor")
    serial_write("s 0.5")
    serial_read()
    while True:
        print("\t[1] Change motor direction")
        print("\t[2] exit")
        choice = input()
        match choice:
            case "1":
                serial_write("s 0")
                serial_read()
                serial_write("t")
                while serial_read() == "ERROR: Stop motor before changing direction":
                    serial_write("t")
                    time.sleep(0.6)
                serial_write("s 0.5")
                serial_read()
                print("Direction Changed")
            case "2":
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
        print("\t[2] Run motors")
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
