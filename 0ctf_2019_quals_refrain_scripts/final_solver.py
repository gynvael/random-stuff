#!/usr/bin/python
# refrain solver by Gynvael Coldwind of Dragon Sector
# This script requires a map of character/traces in out/ directory,
# as well as the perf script output in f:/dump2.
import re
import sys

addresses = []

NODES = {
  "af0",  "b07",  "b15",  "b46",  "b20",  "b4f",  "b25",  "b54",
  "b29",  "b57",  "b60",  "b32",  "b65",  "b70",  "b78",  "b69",
  "b77",  "b7a"
}

TRANS = {
  ("af0", "b18"): ["b07", "b15"],
  ("b46", "b4d"): [],
  ('b20', 'b27'): ["b25"],
  ('b57', 'b5e'): [],
  ('b32', 'b4d'): ["b46"],
  ('b32', 'b44'): [],
  ('b20', 'b4d'): ["b25", "b29", "b32", "b46"],
  ('b20', 'b44'): ["b25", "b29", "b32"],
  ('b20', 'b23'): [],
  ('b60', 'b63'): [],
}

CHARMAP = {}

for i in xrange(32, 127):
  trace = ["af0"]
  with open("out/%.2x" % i) as f:
    for ln in f:
      ln = ln.strip()
      if not ln:
        continue
      m = re.match('0x7ffff7ba9([0-9a-f]+)', ln)
      if m is None:
        sys.exit("fail X: %s" % ln)
      addr = m.group(1)
      trace.append(addr)

      if addr not in NODES:
        sys.exit("fail Y: %s" % ln)

  if len(trace) > 1:
    CHARMAP[i] = trace
    #print i, trace
  else:
    #print "skipping %i" % i
    pass

text = ""
for ln in open("f:\\dump2"): # perf script output (requires symbols for freetype)
  ln = ln.strip()
  if 'branches' not in ln:
    continue
  a, b = ln.split('=>')
  a = a.strip()
  b = b.strip()

  for x in [a, b]:
    if 'ps_unicodes_char_index' not in x:
      if addresses:
        found = False
        for ch, trace in CHARMAP.items():
          if addresses == trace:
            found = True
            text += chr(ch)
        if not found:
          #print "not found"
          pass
        addresses = []
      continue

    m = re.match('.*7fd5dd3ec([0-9a-f]+)[ \t].*', x)
    if m is None:
      sys.exit("fail: %s" % x)

    addr = m.group(1)

    if addr in NODES:
      addresses.append(addr)
      continue

    if not addresses:
      continue

    last_addr = addresses[-1]

    k = (last_addr, addr)
    if k not in TRANS:
      print "missing:", k
      addresses.append("_" + addr)
    else:
      addresses.extend(TRANS[k])

print text
