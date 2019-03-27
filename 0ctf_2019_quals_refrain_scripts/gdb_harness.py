#!/usr/bin/python
#!/usr/bin/python
# refrain GDB harness by Gynvael Coldwind of Dragon Sector
import os

for i in xrange(32, 127):
  with open('letter', 'w') as f:
    f.write(chr(i))
  os.system("gdb --batch -x script.gdb ./a.out 2>/dev/null | grep '=>' | awk '{print $2}' > out/%.2x" % i)

