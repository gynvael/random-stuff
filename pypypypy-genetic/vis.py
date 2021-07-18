#!/home/gynvael/foss/Python-3.8.11/bin/bin/python3.8
import sys
from types import CodeType
import random
import json
import time
from copy import deepcopy
from opcodes import opcodes

UNARY = "unary"
BINARY = "binary"

PRECEDENCE = {
  (BINARY, '**'): 1,
  (UNARY, '-'): 2,
  (UNARY, '~'): 2,
  (BINARY, '*'): 3,
  (BINARY, '//'): 3,
  (BINARY, '%'): 3,
  (BINARY, '+'): 4,
  (BINARY, '-'): 4,
  (BINARY, '<<'): 5,
  (BINARY, '>>'): 5,
  (BINARY, '&'): 6,
  (BINARY, '^'): 7,
  (BINARY, '|'): 8
}

def escape_html(s):
  return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

def get_precedence(o):
  t = type(o)
  if t is op_dup:
    return -1 if o.v >= 0 else 1.5

  if t in { int, bool, str }:
    try:
      v = int(o)
    except ValueError:
      sys.exit("uh str? " + o)
    return -1 if v >= 0 else 1.5

  if t is op_bin:
    return PRECEDENCE[(BINARY, o.op)]
  if t is op_un:
    return PRECEDENCE[(UNARY, o.op)]

  sys.exit("uh what? " + str(t) + " " + str(o))

class op_bin:
  def __init__(self, res, op, a, b):
    self.res = res
    self.op = op
    self.a = a
    self.b = b
    self.precedence = get_precedence(self)

  def __repr__(self):
    a = repr(self.a)
    if get_precedence(self.a) >= self.precedence:
      a = "(" + a + ")"

    b = repr(self.b)
    if get_precedence(self.b) >= self.precedence:
      b = "(" + b + ")"

    op = escape_html(self.op)

    return a + op + b

  def __str__(self):
    a = str(self.a)
    if get_precedence(self.a) >= self.precedence:
      a = "(" + a + ")"

    b = str(self.b)
    if get_precedence(self.b) >= self.precedence:
      b = "(" + b + ")"

    return a + self.op + b

class op_un:
  def __init__(self, res, op, a):
    self.res = res
    self.op = op
    self.a = a
    self.precedence = get_precedence(self)

  def __repr__(self):
    a = repr(self.a)
    if get_precedence(self.a) >= self.precedence:
      a = "(" + a + ")"

    return self.op + a

  def __str__(self):
    a = str(self.a)
    if get_precedence(self.a) >= self.precedence:
      a = "(" + a + ")"

    return self.op + a

class op_dup:
  def __init__(self, v):
    self.v = v

  def __repr__(self):
    return '<span class="dup">' + str(self.v) + '</span>'

  def __str__(self):
    return str(self.v)

def ast_to_arr(ast_arr):
  arr = []
  for ast in ast_arr:
    arr.append(repr(ast))
  return arr

def stack_to_arr(stack_arr):
  arr = []
  for el in stack_arr:
    arr.append(str(el))
  return arr

def add_to_state(state, op, ast, stack):
  state.append({
    "op": op,
    "ast": ast_to_arr(ast),
    "stack": stack_to_arr(stack)
  })

def run(x):
  stack = [1]
  before_stacks = []
  ast = [1]

  state = []

  for i in x:
    before_stacks.append(deepcopy(stack))
    #print(i, [str(y) for y in ast])
    #print(i, ast_to_arr(ast), stack)

    add_to_state(state, i, ast, stack)

    # Safety - if top of the stack is too large, get out.
    if stack[-1] > 1000:
      raise Exception()

    if i == "DUP_TOP":
      res = stack[-1]
      stack.append(res)
      ast.append(op_dup(res))
      continue

    if i == "DUP_TOP_TWO":
      r1 = stack[-2]
      r2 = stack[-1]
      stack.append(r1)
      stack.append(r2)
      ast.append(op_dup(r1))
      ast.append(op_dup(r2))
      continue

    if i == "BINARY_OR":
      a = stack.pop()
      b = stack.pop()
      stack.append(a|b)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "|", b, a))
      continue

    if i == "BINARY_AND":
      a = stack.pop()
      b = stack.pop()
      stack.append(a&b)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "&", b, a))
      continue

    if i == "BINARY_XOR":
      a = stack.pop()
      b = stack.pop()
      stack.append(a^b)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "^", b, a))
      continue

    if i == "BINARY_ADD":
      a = stack.pop()
      b = stack.pop()
      stack.append(a+b)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "+", b, a))
      continue

    if i == "BINARY_FLOOR_DIVIDE":
      a = stack.pop()
      b = stack.pop()
      stack.append(b//a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "//", b, a))
      continue

    if i == "BINARY_MODULO":
      a = stack.pop()
      b = stack.pop()
      stack.append(b%a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "%", b, a))
      continue

    if i == "BINARY_SUBTRACT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b-a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "-", b, a))
      continue

    if i == "BINARY_MULTIPLY":
      a = stack.pop()
      b = stack.pop()
      stack.append(a*b)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "*", b, a))
      continue

    if i == "BINARY_POWER":
      a = stack.pop()
      b = stack.pop()

      # Don't do too large powers.
      if b > 10:
        raise Exception("No.")

      stack.append(b**a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "**", b, a))
      continue

    if i == "BINARY_LSHIFT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b<<a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], "<<", b, a))
      continue

    if i == "BINARY_RSHIFT":
      a = stack.pop()
      b = stack.pop()
      stack.append(b>>a)

      a = ast.pop()
      b = ast.pop()
      ast.append(op_bin(stack[-1], ">>", b, a))
      continue

    if i == "UNARY_NEGATIVE":
      a = stack.pop()
      stack.append(-a)

      a = ast.pop()
      ast.append(op_un(stack[-1], "-", a))
      continue

    if i == "UNARY_INVERT":
      a = stack.pop()
      stack.append(~a)

      a = ast.pop()
      ast.append(op_un(stack[-1], "~", a))
      continue

    if i == "ROT_TWO":
      a = stack.pop()
      b = stack.pop()
      stack.append(a)
      stack.append(b)

      a = ast.pop()
      b = ast.pop()
      ast.append(a)
      ast.append(b)
      continue

    if i == "ROT_THREE":
      a = stack.pop()
      b = stack.pop()
      c = stack.pop()
      stack.append(a)
      stack.append(c)
      stack.append(b)

      a = ast.pop()
      b = ast.pop()
      c = ast.pop()
      ast.append(a)
      ast.append(c)
      ast.append(b)
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

      a = ast.pop()
      b = ast.pop()
      c = ast.pop()
      d = ast.pop()
      ast.append(a)
      ast.append(d)
      ast.append(c)
      ast.append(b)
      continue

  before_stacks.append(deepcopy(stack))

  if len(stack) != 1:
    return None

  add_to_state(state, "", ast, stack)

  return stack[0], before_stacks, ast[0], state

def pyrun(code):
  bytecode = [
    opcodes["BUILD_STRING"], 0,
    opcodes["BUILD_STRING"], 0,
    opcodes["COMPARE_OP"], 2,
  ]

  for op in code:
    bytecode.append(opcodes[op])
    bytecode.append(0)

  bytecode.extend([
    opcodes["RETURN_VALUE"], 0
  ])

  codestring = bytes(bytearray(bytecode))
  code = CodeType(0, 0, 0, 0, 0, 0, codestring, (), (), (), '', '', 0, b'')
  return eval(code, {'__builtins__': None}, {})

shortest = {}

with open("best.json", "r") as f:
  shortest = json.load(f)

#print(len(shortest))

stuff = []
nums = []

for i in range(0, 201):
  i = str(i)
  if i not in shortest:
    print("------------------- missing %i" % i)
    continue

  code = shortest[i]

  res, stacks, ast, state = run(code)

  pyres = pyrun(code)
  evalres = eval(str(ast))

  chk = (
    int(i) == res and
    int(i) == pyres and
    int(i) == evalres
  )

  print(chk, i, res, pyres, evalres, str(ast))

  stuff.append(state)
  nums.append((i, repr(ast)))

with open("python-bytecode-numbers.js", "w") as f:
  f.write("let pyBytecodeNumber = ")
  f.write(json.dumps(stuff))
  f.write(";\n")

with open("best.html", "w") as f:
  for e in nums:
    f.write("<tr><td class=\"pytnum\">%s</td><td class=\"pytast\">%s</td></tr>\n" % e)





