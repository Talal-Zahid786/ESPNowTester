import serial
import struct

# Match the exact C struct layout
STRUCT_FORMAT = '<' + 'f'*32 + 'i' + 'f'*4 + 'B' + 'i'  # Little endian
DATA_SIZE = struct.calcsize(STRUCT_FORMAT)

# Adjust to your port
ser = serial.Serial('COM7', baudrate=115200, timeout=1)

def find_frame_start():
    while True:
        byte = ser.read(1)
        if byte == b'\xAA':
            if ser.read(1) == b'\x15':
                return

while True:
    find_frame_start()
    data = ser.read(DATA_SIZE)
    if len(data) == DATA_SIZE:
        unpacked = struct.unpack(STRUCT_FORMAT, data)
        noOfSensors = unpacked[32]
        if noOfSensors > 0:
            temperature = unpacked[0:16]
            maxTemp = unpacked[16:32]
            cvx, hvx, lvx, svx = unpacked[33:37]
            unitC = bool(unpacked[37])
            hash_val = unpacked[38]

            print(f"[{noOfSensors} sensors] Temp[0]={temperature[0]}, Unit={'C' if unitC else 'F'}, Hash={hash_val}")
