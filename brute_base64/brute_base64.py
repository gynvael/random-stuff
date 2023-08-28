#!/usr/bin/env python
# A brute-forcing base64 extractor.
# by Gynvael Coldwind (http://gynvael.coldwind.pl)
#
# This tool basically goes through the whole file and finds every base64-looking
# string and decodes it. It produces A LOT of output, especially that a lot of
# normal ASCII strings are valid base64 strings.
#
import base64
import sys
import math

MIN_DECODED_DATA_LEN = 10  # You might want to decrease is.
MIN_ENCODED_DATA_LEN = math.ceil((MIN_DECODED_DATA_LEN * 4) / 3)

ALL_BASE64_CHARS = set(
    b'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
    b'-_,'  # RFC 3501 and RFC 4648 chars.
    b'\r\n \t'  # Whitechars.
)

def get_longest_base64_substring(data, i):
  b64 = []

  while data[i] in ALL_BASE64_CHARS:
    b64.append(data[i])
    i += 1

  if len(b64) < MIN_ENCODED_DATA_LEN:
    return None, i + 1

  # Eat up any additional =.
  while data[i] == '=':
    i += 1

  b64 = bytes(b64)

  # This part is a bit slow.
  b64 = b64.replace(b' ', b'')
  b64 = b64.replace(b'\t', b'')
  b64 = b64.replace(b'\r', b'')
  b64 = b64.replace(b'\n', b'')
  b64 = b64.replace(b'-', b'+')  # RFC 4648 to standard base64.
  b64 = b64.replace(b'_', b'/')  # RFC 4648 to standard base64.
  b64 = b64.replace(b',', b'/')  # RFC 3501 to standard base64.

  # Note: This doesn't handle padding nor length.
  return b64, i + 1

def wiggle_base64_candidate(b64, offset):
  # If we have a base64 candidate string like:
  #   abcdefgh
  # Then we don't really know if it's:
  #   abcdefgh
  #   bcdefgh=
  #   cdefgh==
  #   defg
  # So we have to try all four.
  # Note that base64 self-synchronizes every 4 bytes.
  # Furthermore, the following lengths are valid:
  #    len % 4 == 0
  #    len % 4 == 2  with padding ==
  #    len % 4 == 3  with padding =
  # The len % 4 == 1 is invalid, as it has too few characters.
  for i in range(4):
    candidate = b64[i:]

    if len(candidate) % 4 == 0:
      yield candidate, offset + i
    elif len(candidate) % 4 == 1:
      yield candidate[:-1], offset + i  # Truncate last character.
    elif len(candidate) % 4 == 2:
      yield candidate + b'==', offset + i  # Add padding.
    elif len(candidate) % 4 == 3:
      yield candidate + b'=', offset + i  # Add padding.

if len(sys.argv) != 2:
  sys.exit("usage: brute_base64.py <fname>")

with open(sys.argv[1], "rb") as f:
  d = f.read()

i = 0
while i < len(d) - 1:
  offset = i

  b64, i = get_longest_base64_substring(d, i)
  if b64 is None:
    continue

  for variant, offset in wiggle_base64_candidate(b64, offset):
    fname = f'base64_{offset:08x}.bin'
    decoded = base64.b64decode(variant)
    with open(fname, "wb") as f:
      f.write(decoded)

    followup = f"... ({len(decoded)} bytes total)" if len(decoded) > 40 else ''
    print(f"Some potential data at {offset:08x}: {decoded[:40]}{followup}")

