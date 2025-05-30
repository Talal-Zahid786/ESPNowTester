import serial
import struct

# Serial port setup
ser = serial.Serial('COM7', baudrate=115200, timeout=1)

# Struct format definition
STRUCT_FORMAT = '<' + 'f'*16 + 'f'*16 + 'f'*4 + 'B' + 'i' + ( 'f'*28 + 'B'*2 + 'f'*3 + 'f'*4 + 'f'*4 + 'B' + 'i') * 4

DATA_SIZE = struct.calcsize(STRUCT_FORMAT)
print(DATA_SIZE)
def find_frame_start():
    while True:
        byte = ser.read(1)
        if byte == b'\xAA' and ser.read(1) == b'\x15':
            return

while True:
    find_frame_start()
    data = ser.read(DATA_SIZE)
    if len(data) != DATA_SIZE:
        print("Incomplete data received")
        continue

    unpacked = struct.unpack(STRUCT_FORMAT, data)

    idx = 0

    # TemperatureDevice
    print("\n================ Temperature Device Data ================")
    temp_TD = unpacked[idx:idx+16]; idx += 16
    maxTemp_TD = unpacked[idx:idx+16]; idx += 16
    cvx_TD, hvx_TD, lvx_TD, svx_TD = unpacked[idx:idx+4]; idx += 4
    unitC_TD = bool(unpacked[idx]); idx += 1
    noOfSensors_TD = unpacked[idx]; idx += 1

    print(f"Sensor Count: {noOfSensors_TD} | Unit: {'°C' if unitC_TD else '°F'}")
    print(f"Temp     : {', '.join(f'{t:.2f}' for t in temp_TD)}")
    print(f"MaxTemp  : {', '.join(f'{t:.2f}' for t in maxTemp_TD)}")
    print(f"Voltage  : CVX={cvx_TD:.2f}, HVX={hvx_TD:.2f}, LVX={lvx_TD:.2f}, SVX={svx_TD:.2f}")

    # Wattage Devices
    for dev in range(1, 5):
        print(f"\n================ Wattage Device {dev} Data ================")
        volt = unpacked[idx:idx+2]; idx += 2
        curr = unpacked[idx:idx+2]; idx += 2
        real_p = unpacked[idx:idx+2]; idx += 2
        app_p = unpacked[idx:idx+2]; idx += 2
        max_volt = unpacked[idx:idx+2]; idx += 2
        max_curr = unpacked[idx:idx+2]; idx += 2
        max_real = unpacked[idx:idx+2]; idx += 2
        max_app = unpacked[idx:idx+2]; idx += 2
        pf = unpacked[idx:idx+2]; idx += 2
        imp = unpacked[idx:idx+2]; idx += 2
        min_imp = unpacked[idx:idx+2]; idx += 2
        max_imp = unpacked[idx:idx+2]; idx += 2
        freq = unpacked[idx:idx+2]; idx += 2
        svx = unpacked[idx:idx+2]; idx += 2
        clipping = [bool(unpacked[idx]), bool(unpacked[idx+1])]; idx += 2
        cvx = unpacked[idx]; idx += 1
        hvx = unpacked[idx]; idx += 1
        lvx = unpacked[idx]; idx += 1
        temp = unpacked[idx:idx+4]; idx += 4
        maxTemp = unpacked[idx:idx+4]; idx += 4
        unitC = bool(unpacked[idx]); idx += 1
        noOfSensors = unpacked[idx]; idx += 1

        print(f"Sensor Count: {noOfSensors} | Unit: {'°C' if unitC else '°F'}")
        print(f"Volt     : {', '.join(f'{v:.2f}' for v in volt)}")
        print(f"Curr     : {', '.join(f'{c:.2f}' for c in curr)}")
        print(f"RealP    : {', '.join(f'{p:.2f}' for p in real_p)}")
        print(f"AppP     : {', '.join(f'{p:.2f}' for p in app_p)}")
        print(f"MaxVolt  : {', '.join(f'{v:.2f}' for v in max_volt)}")
        print(f"MaxCurr  : {', '.join(f'{c:.2f}' for c in max_curr)}")
        print(f"MaxRealP : {', '.join(f'{p:.2f}' for p in max_real)}")
        print(f"MaxAppP  : {', '.join(f'{p:.2f}' for p in max_app)}")
        print(f"PF       : {', '.join(f'{p:.2f}' for p in pf)}")
        print(f"Impedance: {', '.join(f'{z:.2f}' for z in imp)}")
        print(f"MinImp   : {', '.join(f'{z:.2f}' for z in min_imp)}")
        print(f"MaxImp   : {', '.join(f'{z:.2f}' for z in max_imp)}")
        print(f"Freq     : {', '.join(f'{f:.2f}' for f in freq)}")
        print(f"SVX      : {', '.join(f'{s:.2f}' for s in svx)}")
        print(f"Voltage  : CVX={cvx:.2f}, HVX={hvx:.2f}, LVX={lvx:.2f}")
        print(f"Temp     : {', '.join(f'{t:.2f}' for t in temp)}")
        print(f"MaxTemp  : {', '.join(f'{t:.2f}' for t in maxTemp)}")
        print(f"Clipping : {clipping[0]}, {clipping[1]}")
