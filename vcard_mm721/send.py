import serial
import sys
import time
import json

def slow_write(s, data):
  for b in data:
    s.write(bytes([b]))
    time.sleep(0.01)

with open("ez") as f:
  data = f.read().splitlines()

s = serial.Serial('/dev/ttyUSB0')

i = 0
try:
  with open("state.json") as f:
    i = json.load(f)
    print("Resuming from", i)
except FileNotFoundError:
  print("No state file, resuming from 0.")

while i < len(data):
  print(f"--------------------------------------- {i}")
  d = data[i]

  while True:
    time.sleep(2)
    print("AT?")
    slow_write(s, b'AT\r\n')
    empty = s.readline()
    resp = s.readline()

    if empty != b'\r\n':
      sys.exit("empty was: " + str(empty))

    if resp != b'OK\r\n':
      print(resp)
      continue

    time.sleep(2)
    print("Sending line: ", d)
    slow_write(s, d.encode() + b'\r\n')

    empty = s.readline()
    resp = s.readline()

    if empty != b'\r\n':
      sys.exit("empty was: " + str(empty))

    if resp != b'OK\r\n':
      print(resp)
      continue

    print("OK!")

    break
  i += 1
  with open("state.json", "w") as f:
    json.dump(i, f)
