#!Actually works only on Windows ;)
# A simple environment switcher by Gynvael Coldwind (assume MIT license)
# Before first usage:
#   - Make sure to change/fix all the paths.
#   - Probably the code won't work anyway. Happy debugging!
import sys
from winapicon import *
from glob import glob

COMPILERS = []

def usage():
  print "-=- Available compilers/interpreters -=-"
  print "ID               Description"

  global COMPILERS
  COMPILERS = sorted(COMPILERS, key=lambda x: x[0])

  for cid, cname, _ in COMPILERS:
    set_color(RED|BLUE|LIGHT)
    print "%-16s" % cid,
    set_color(GREEN|LIGHT)
    print "%-s" % cname

  set_color(GREEN|LIGHT)
  print "usage: switch <id>"

def write_bat_script(s=""):
  with open("d:\\commands\\switch-worker-output.bat", "w") as f:
    f.write("@echo off\r\n")
    f.write(s)

def populate_python():
  for path in glob("d:\\bin\\python*"):
    # Fixing the one time I used "-" insted of "_"
    tmp = path.replace("-", "_")

    tmp = tmp.replace("python", "")
    tmp = tmp.replace("Python", "")

    ver, bits = tmp.split("\\")[-1].split("_")
    v1, v2, v3 = ver[0], ver[1], ver[2:]
    ver = "%s.%s" % (v1, v2)
    if v3:
      ver += ".%s" % v3

    cid = "py%s-%s" % (ver, bits)
    cname = "Python %s (%s bits)" % (ver, bits)

    COMPILERS.append((cid, cname, """
    set PATH=%s;%%PATH%%
    """ % (path)))    


def populate_clang():
  for path in glob("d:\\bin\\LLVM-*"):
    tmp = path.split("\\")[-1]
    _, ver, arch = tmp.split("-")
    bits = {
        "win32": "32",
        "win64": "64",
        }[arch]

    cid = "clang%s-%s" % (ver, bits)
    cname = "LLVM clang %s (%s bits)" % (ver, bits)

    call_cmd = {
        "win32": "d:\\bin\\msvc14\\VC\\bin\\vcvars32.bat",
        "win64": "d:\\bin\\msvc14\\VC\\bin\\x86_amd64\\vcvarsx86_amd64.bat",
        }[arch]    

    COMPILERS.append((cid, cname, """
    call %s
    set PATH=%s\\bin;%%PATH%%
    set CPATH=d:\\bin\\msvc14\\VC\\include;d:\\bin\\Windows Kits\\10\\Include\\10.0.10240.0\\ucrt;d:\\etc\\llvm-hack\\
    """ % (call_cmd, path)))

def populate_gcc():
  for path in glob("d:\\bin\\gcc\\*"):
    tmp = path.split("\\")[-1]
    arch, ver, threads, excp, runtime, rev = tmp.split("-")

    bits = {
        "i686": "32",
        "x86_64": "64",
        }[arch]

    cid = "gcc%s-%s" % (ver, bits)
    cname = "GCC %s (%s bits, %s threads, %s excp)" % (
        ver, bits, threads, excp
        )

    COMPILERS.append((cid, cname, """
    set PATH=%s\\mingw%s\\bin;%%PATH%%
    """ % (path, bits)))

def populate_msvc():
  COMPILERS.extend([
    ("msvc10-32", "Microsoft C++ 2010 [16.10] (32 bits)",
     "call d:\\bin\\msvc\\VC\\bin\\vcvars32.bat"),
    ("msvc15-32", "Microsoft C++ 2015 [19.00] (32 bits)",
     "call d:\\bin\\msvc14\\VC\\bin\\vcvars32.bat"),
    ("msvc15-64", "Microsoft C++ 2015 [19.00] (64 bits)",
     "call d:\\bin\\msvc14\\VC\\bin\\x86_amd64\\vcvarsx86_amd64.bat"),
  ])

def populate_compilers():
  populate_python()
  populate_clang()
  populate_gcc()
  populate_msvc()

def main():
  populate_compilers()

  if len(sys.argv) == 1:
    usage()
    write_bat_script()
    return 1

  selected_cid = sys.argv[1].strip()
  for cid, cname, cscript in COMPILERS:
    if cid != selected_cid:
      continue

    print "Switching to:",
    set_color(BLUE|RED|LIGHT)
    print cname
    set_color(GREEN|LIGHT)    
    write_bat_script(cscript)
    return 0

  print "Error: %s not found" % selected_cid
  write_bat_script()
  return 1

if __name__ == "__main__":
  sys.exit(main())


