#!/usr/bin/python
# A brute-forcing zlib decompressor
# by Gynvael Coldwind (http://gynvael.coldwind.pl)
# Note: This isn't really a tool - it's a code snippet that works by accident.
#       No guarantee is given that it does its job.
#
# It's somewhat useful to extract data from corrupted ZIP archives and other
# binary blobs which might contains DEFLATE streams.
# Creates a lot of garbage files - that's normal.
# Might not work good with larger streams (i.e. it will only dump the first
# several hundred bytes of the stream) - you can manually change this in the
# code though.
import zlib
import sys

def DecompressStream(d):
  """Decompresses the stream (if possible) and returns the unused data as well.
  """
  try:
    obj = zlib.decompressobj()
    data = obj.decompress(d)
    return (data, obj.unused_data)
  except zlib.error:
    # This might be a ZIP file, let it pass.
    pass

  try:
    obj = zlib.decompressobj(-15)
    data = obj.decompress(d)
    return (data, obj.unused_data)
  except zlib.error as e:
    # No idea.
    return (None, e)

if len(sys.argv) != 2:
  print("usage: brute_zlib.py <fname>")
  sys.exit(1)

with open(sys.argv[1], "rb") as f:
  d = f.read()

threshold = 10

for i in range(len(d)):
  if i % 1234 == 0:
    print("  0x%x / 0x%x\r" % (i, len(d)), end="")
    sys.stdout.flush()

  data, unused = DecompressStream(d[i:i+128])
  if type(data) is bytes and len(data) > 0:

    #if len(unused) == 0:
    #  # Re-try on full.
    #  print("RETRY:", i, data, unused)
    # data, unused = DecompressStream(d[i:])
    #  print("RESULT:", data, unused)

    if len(data) > threshold:
      with open("%.8x.bin" % i, "wb") as f:
        f.write(data)
      print(f"Some data at {i:08x}: {data[:40]}{'...' if len(data)>40 else ''}")
