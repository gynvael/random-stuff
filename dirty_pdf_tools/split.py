#!/usr/bin/python
# Quick and dirty PDF version splitter
# by Gynvael Coldwind
import sys

if len(sys.argv) != 2:
  sys.exit("usage: split.py <fname.pdf>")

fname = sys.argv[1]

with open(fname, "rb") as f:
  d = f.read()

parts = d.split("%%EOF")
print "Found %i potential version" % max(len(parts) - 1, 1)

b = ""
for i, p in enumerate(parts):
  b += p
  with open("%s.%.3i.pdf" % (fname, i), "wb") as f:
    f.write(b)
    f.write("%%EOF\n")


