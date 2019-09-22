#!/usr/bin/python3
# This script generates the values which are to be placed in "load_magics"
# function.

FLAG = "DrgnS{uMFlagPwningUnit}"
#FLAG = "DrgnS{0123456789abcdef}"
mb = bytearray(bytes(FLAG[6:-1], 'UTF-8'))
ma = [0] * 16

for j in range(4):
  for i in range(4):
    src_idx = i + j * 4
    dst_idx = j + i * 4
    ma[dst_idx] = mb[src_idx]

for i in range(16):
  print("%.8x" % ma[i])

for i in range(16):
  ma[i] += 3

r2 = 1103515245
r3 = 0xa5a5a5a5
r4 = 12345
r5 = 0x7fffffff

for i in range(16):
  r3 = (((r3 * r2) & 0xffffffff) + r4) & r5

  v = ma[i] | (ma[i] << 8) | (ma[i] << 16) | (ma[i] << 24)
  print("%.8X %.8X" % (r3, v))
  ma[i] = v ^ r3

print(', '.join(["0x%.8x" % i for i in ma]))









