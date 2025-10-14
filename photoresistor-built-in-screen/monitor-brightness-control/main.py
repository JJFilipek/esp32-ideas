import serial
import json
import screen_brightness_control as sbc

PORT = 'COM5'
BAUD = 115200

def map_light_to_brightness(value):
    return max(10, min(100, int(100 - (value / 4095) * 100)))

try:
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        print("Connected with ESP32. Start auto dimming...")
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue
            try:
                data = json.loads(line)
                raw = data.get("rawLight")
                if raw is not None:
                    brightness = map_light_to_brightness(raw)
                    print(f"Light raw: {raw}, Brightness: {brightness}%")
                    sbc.set_brightness(brightness)
            except json.JSONDecodeError:
                pass
except Exception as e:
    print(f"ERROR: {e}")
