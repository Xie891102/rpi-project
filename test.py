import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)  # Set BCM GPIO mode

TRIG = 6  # GPIO pin for TRIG
ECHO = 7  # GPIO pin for ECHO

GPIO.setup(TRIG, GPIO.OUT)
GPIO.setup(ECHO, GPIO.IN)

def measure_distance():
    """Measure distance using HC-SR04"""
    GPIO.output(TRIG, False)
    time.sleep(0.5)

    # Send 10us pulse to TRIG
    GPIO.output(TRIG, True)
    time.sleep(0.00001)
    GPIO.output(TRIG, False)

    # Wait for ECHO start
    while GPIO.input(ECHO) == 0:
        pulse_start = time.time()
    
    # Wait for ECHO end
    while GPIO.input(ECHO) == 1:
        pulse_end = time.time()

    pulse_duration = pulse_end - pulse_start

    # Calculate distance in cm
    distance = pulse_duration * 17150
    distance = round(distance, 2)

    return distance

try:
    while True:
        dist = measure_distance()
        print(f"Distance: {dist} cm")
        time.sleep(1)
except KeyboardInterrupt:
    print("Test stopped")
    GPIO.cleanup()
