#!/usr/bin/python
# Quick and dirty multi-version PDF finder
# by Gynvael Coldwind
import glob
import sys
import os

if len(sys.argv) != 2:
  sys.exit("usage: findpdf.py <path>")

path = sys.argv[1]

def process_pdf(fname):
  with open(fname, "rb") as f:
    d = f.read()
  parts = d.split("%%EOF")

  if len(parts) > 4:
    # sorry about the spaces at the end, windows console doesn't handle \1xb[2K at all...
    print len(parts), fname, "                               "
    return True
  return False

def scan_path(path):
  print path, "                                           \r",
  sys.stdout.flush()
  for fname in glob.glob("%s/*" % path):
    if os.path.isdir(fname):
      scan_path(fname)
      continue

    name, ext = os.path.splitext(fname)
    if ext.lower() != '.pdf':
      continue

    if process_pdf(fname):
      print path, "\r",


scan_path(path)

