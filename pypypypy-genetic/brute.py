import sys
import random
import json
import time

def run(x):
  try:
    r = wrun(x)
    return r
  except:
    return None

def wrun(x):
  stack = [1]
  for i in x:

    # Safety - if top of the stack is too large, get out.
    if stack[-1] > 1000:
      raise Exception()

    if i == "DUP_TOP":
      stack.append(stack[-1])
      continue

    if i == "DUP_TOP_TWO":
      stack.append(stack[-2])
      stack.append(stack[-2])
      continue

    if i == "BINARY_OR":
      a = stack.pop()
      b = stack.pop()
      stack.append(a|b)
      continue

    if i == "BINARY_AND":
      a = stack.pop()
      b = stack.pop()
      stack.append(a&b)
      continue

    if i == "BINARY_XOR":
      a = stack.pop()
      b = stack.pop()
      stack.append(a^b)
      continue

    if i == "BINARY_ADD":
      a = stack.pop()
      b = stack.pop()
      stack.append(a+b)
      continue

    if i == "BINARY_FLOOR_DIVIDE":
      a = stack.pop()
      b = stack.pop()
      stack.append(b//a)
      continue

    if i == "BINARY_MODULO":
      a = stack.pop()
      b = stack.pop()
      stack.append(b%a)
      continue

    if i == "BINARY_SUBTRACT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b-a)
      continue

    if i == "BINARY_MULTIPLY":
      a = stack.pop()
      b = stack.pop()
      stack.append(a*b)
      continue

    if i == "BINARY_POWER":
      a = stack.pop()
      b = stack.pop()

      # Don't do too large powers.
      if b > 10:
        raise Exception("No.")

      stack.append(b**a)
      continue

    if i == "BINARY_LSHIFT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b<<a)
      continue

    if i == "BINARY_RSHIFT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b>>a)
      continue

    if i == "UNARY_NEGATIVE":
      a = stack.pop()
      stack.append(-a)
      continue

    if i == "UNARY_INVERT":
      a = stack.pop()
      stack.append(~a)
      continue

    if i == "ROT_TWO":
      a = stack.pop()
      b = stack.pop()
      stack.append(a)
      stack.append(b)
      continue

    if i == "ROT_THREE":
      a = stack.pop()
      b = stack.pop()
      c = stack.pop()
      stack.append(a)
      stack.append(c)
      stack.append(b)
      continue

    if i == "ROT_FOUR":
      a = stack.pop()
      b = stack.pop()
      c = stack.pop()
      d = stack.pop()
      stack.append(a)
      stack.append(d)
      stack.append(c)
      stack.append(b)
      continue

  if len(stack) != 1:
    return None

  return stack[0]


ops = [
  "DUP_TOP",
  "DUP_TOP_TWO",
  "BINARY_OR",
  "BINARY_AND",
  "BINARY_XOR",
  "BINARY_ADD",
  "BINARY_SUBTRACT",
  "BINARY_MULTIPLY",
  "BINARY_POWER",
  "BINARY_LSHIFT",
  "BINARY_RSHIFT",
  "UNARY_NEGATIVE",
  "UNARY_INVERT",
  "ROT_TWO",
  "ROT_THREE",
  "ROT_FOUR",
  "BINARY_FLOOR_DIVIDE",
  "BINARY_MODULO",
]

shortest = {}



def make(n):
  op = []

  if shortest and random.randint(1, 100) < 10:  # Pre-initialize with a number.
    for i in range(3):
      k = str(random.randint(0, 200))
      v = shortest.get(k)
      if v is None:
        v = []
        continue

      if len(v) + 2 > n:
        v = []
        continue
      break

    op.extend(v)
    n -= len(v)

  #print(len(op), n)

  op.extend(random.choices(ops, k=n))

  return op



def go(n):
  for i in range(100000):
    op = make(n)
    #sys.stdout.write('<')
    #sys.stdout.flush()
    r = run(op)
    #sys.stdout.write('>')
    #sys.stdout.flush()


    if r is None:
      continue
    if type(r) is not int:
      continue
    if r < 0 or r > 200:
      continue

    s = str(r)

    if s not in shortest or len(shortest[s]) > len(op):
      shortest[s] = op
      print(s, op)


with open("best.json", "r") as f:
  shortest = json.load(f)

print(len(shortest))


#run(['DUP_TOP', 'UNARY_INVERT', 'BINARY_SUBTRACT', 'DUP_TOP', 'BINARY_POWER', 'DUP_TOP', 'BINARY_ADD', 'DUP_TOP', 'DUP_TOP', 'ROT_THREE', 'BINARY_LSHIFT', 'BINARY_POWER', 'BINARY_AND', 'DUP_TOP', 'BINARY_POWER', 'DUP_TOP', 'BINARY_MODULO', 'BINARY_LSHIFT'])

#sys.exit(0)


last = time.time()
while True:
  cnt = random.randint(4, 20)
  go(cnt)

  if time.time() - last < 5:
    continue

  sys.stdout.write('.')
  sys.stdout.flush()

  with open("best", "w") as f:
    for i in range(0, 200):
      i = str(i)
      if i not in shortest:
        f.write("--------------------- %s MISSING\n" % i)
        continue
      f.write("--------------------- %s\n" % i)
      for op in shortest[i]:
        f.write("      %s, 0,\n" % op)

  with open("best.json", "w") as f:
    f.write(json.dumps(shortest))

  last = time.time()





