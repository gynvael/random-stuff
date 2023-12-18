#!/usr/bin/env
# Uh sorry, I was codegolfing a bit when writing this code as I thought I could
# squeeze it into Paged Out! together with the file and article. But I think
# it's a bit too long.
# Anyway, Issue #3 of Paged Out! will have an explanation what this abomination
# is and what purpose does it serve.
# -- gynvael
import struct
import sys
import zlib
if len(sys.argv) != 2:
  sys.exit("usage: pngzip.py <fname.png>")
with open(sys.argv[1], "rb") as f:
  d = f.read()
out = [ d[:8] ]  # Copy header.
i = 8
idat_already_appeared = False
while i < len(d):
  j = i
  sz, type = struct.unpack(">I4s", d[i:i+8])
  print(type)
  i += 8
  data = d[i:i+sz]
  i += sz
  chksum = struct.unpack(">I", d[i:i+4])
  i += 4

  if type == b'IDAT':
    if idat_already_appeared:
      sys.exit("sorry, this works only on really small PNGs")
      # Well technically I could merge IDAT sections together, since various
      # tools tend to make them pretty small, but meh.
    idat_already_appeared = True

    # Inject a ZIP file header chunk.
    # crc32(4)+len(4)+type(4)+zlibheader(2)
    extrafield_len = 4+4+4+2
    extrafield_header = struct.pack("<HH", extrafield_len, 0x4347)

    dcmp = zlib.decompress(data)
    cmplen = sz-2
    dcmplen = len(dcmp)
    checksum = zlib.crc32(dcmp) & 0xffffffff
    zipfilerecord = struct.pack("<4sHHHHHIIIHH8s",
      b"PK\3\4",20,2048,8,0,0, checksum,
      cmplen,dcmplen,8,extrafield_len + 4,
      b'IDAT.bin'
    )

    chunklen = len(zipfilerecord) + len(extrafield_header)
    out.append(struct.pack(">I", chunklen))
    chunk = b'fzIP' + zipfilerecord + extrafield_header
    fileheader_offset = len(b''.join(out)) + 4
    out.append(chunk)
    out.append(struct.pack(">I", zlib.crc32(chunk) & 0xffffffff))
    print("Injected fzIP")

  if type == b'IEND':
    # Inject a ZIP directory entry and end of directory header.
    # Update: Ange Albertini suggested to use \0 at the beginning, which makes
    # me wonder why didn't I try that.
    comment = "\0Sorry, ignore this comment 🤷\n".encode()

    zipheaders = struct.pack("<4sHHHHHHIIIHHHHHII8s4sHHHHIIH",
      b"PK\1\2",20,20,2048,8,0,0,checksum,cmplen,dcmplen,8,0,0,0,0,0xb481,
      fileheader_offset,b"IDAT.bin",
      b"PK\5\6",0,0,1,1,54,len(b''.join(out))+8,4+12+len(comment)) + comment
    out.append(struct.pack(">I", len(zipheaders)))
    chunk = b'ezIP' + zipheaders
    out.append(chunk)
    out.append(struct.pack(">I", zlib.crc32(chunk) & 0xffffffff))
    print("Injected ezIP")

  out.append(d[j:j+12+sz])

with open("out.zip.png", "wb") as f:
  f.write(b''.join(out))
with open("out.png.zip", "wb") as f:
  f.write(b''.join(out))

