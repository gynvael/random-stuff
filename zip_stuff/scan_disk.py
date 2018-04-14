#!/usr/bin/python
# Scans disk in search of ZIP files that don't have a weird extension.
import os
import sys

IGNORE_LIST = {
   ".zip", ".docx", ".odt", ".epub", ".jar", ".xlsx", ".pyz",
   ".pptx", ".odp",
}

def process_file(fname):
  try:
    with open(fname, "rb") as f:
      d = f.read(4)
  except WindowsError:
    return False  # No access probably, don't care.
  except IOError:
    return False  # No access probably, don't care.
  if d.startswith("PK\3\4"):
    return True
  return False


def scan_dir(path):
  try:
    entries = os.listdir(path)
  except WindowsError:
    return  # No access probably, don't care.
  for fname in entries:
    name, ext = os.path.splitext(fname)
    if ext.lower() in IGNORE_LIST:
      continue

    if ext == '':
      ext = '_'

    full_path = path + "/" + fname

    if os.path.isfile(full_path):
      ret = process_file(full_path)

      if not ret:
        continue

      print "%s: %s" % (ext, full_path)

      with open("scan_res/%s" % ext, "a") as f:
        f.write("%s\n" % full_path)
      continue

    if os.path.isdir(full_path):
      scan_dir(full_path)
      continue


if len(sys.argv) != 2:
  sys.exit("usage: scan_disk.py <start_dir>")

scan_dir(sys.argv[1])

