#!/usr/bin/python3
from struct import pack, unpack
from functools import partial

_REG = "REG"
_IMM8 = "IMM8"
_FLOAT = "FLOAT"
_ASCIIZ = "ASCIIZ"
_IMM16BE = "IMM16BE"
_IMM32BE = "IMM32BE"
_MULTI_IMM32BE = "MULTI_IMM32BE"
_MULTI_IMM16BE = "MULTI_IMM16BE"
_MULTI_IMM8 = "MULTI_IMM8"
_MULTI_FLOAT = "MULTI_FLOAT"
_REL8_LABEL = "REL8_LABEL"
_ABS16_LABEL = "ABS16_LABEL"
_CC = "CC"
_EFUNC = "EFUNC"
_ALIGN = "ALIGN"  # Pseudo-opcode (alignment in bytes).
_ENCSTART = "ENCSTART"  # Pseudo-opcode (start of encryption).
_ENCEND = "ENCEND"  # Pseudo-opcode (end of encryption).

INSTRUCTIONS = [
  ( "NOP",         0x00, ),
  ( "SELECTA",     0x01, _REG, ),
  ( "SELECTX",     0x02, _REG, ),
  ( "CLR",         0x03, _REG, ),
  ( "CLRA",        0x04, ),
  ( "CLRX",        0x05, ),
  ( "CLR0",        0x06, ),
  ( "COPY",        0x07, _REG, _REG, ),
  ( "COPYA",       0x08, _REG, ),
  ( "COPYX",       0x09, _REG, ),
  ( "LOAD",        0x0A, _REG, ),
  ( "LOADA",       0x0B, ),
  ( "LOADX",       0x0C, ),
  ( "ALOADX",      0x0D, ),
  ( "XSAVE",       0x0E, _REG, ),
  ( "XSAVEA",      0x0F, ),
  ( "COPY0",       0x10, _REG, ),
  ( "COPYI",       0x11, _IMM8, _REG, ),
  ( "SWAP",        0x12, _REG, _REG, ),
  ( "SWAPA",       0x13, _REG, ),
  ( "LEFT",        0x14, ),
  ( "RIGHT",       0x15, ),
  ( "FWRITE",      0x16, _REG, _FLOAT, ),
  ( "FWRITEA",     0x17, _FLOAT, ),
  ( "FWRITEX",     0x18, _FLOAT, ),
  ( "FWRITE0",     0x19, _FLOAT, ),
  ( "FREAD",       0x1A, _REG, ),
  ( "FREADA",      0x1B, ),
  ( "FREADX",      0x1C, ),
  ( "FREAD0",      0x1D, ),
  ( "ATOF",        0x1E, _ASCIIZ, ),
  ( "FTOA",        0x1F, _IMM8, ),
  ( "FSET",        0x20, _REG, ),
  ( "FADD",        0x21, _REG, ),
  ( "FSUB",        0x22, _REG, ),
  ( "FSUBR",       0x23, _REG, ),
  ( "FMUL",        0x24, _REG, ),
  ( "FDIV",        0x25, _REG, ),
  ( "FDIVR",       0x26, _REG, ),
  ( "FPOW",        0x27, _REG, ),
  ( "FCMP",        0x28, _REG, ),
  ( "FSET0",       0x29, ),
  ( "FADD0",       0x2A, ),
  ( "FSUB0",       0x2B, ),
  ( "FSUBR0",      0x2C, ),
  ( "FMUL0",       0x2D, ),
  ( "FDIV0",       0x2E, ),
  ( "FDIVR0",      0x2F, ),
  ( "FPOW0",       0x30, ),
  ( "FCMP0",       0x31, ),
  ( "FSETI",       0x32, _IMM8, ),
  ( "FADDI",       0x33, _IMM8, ),
  ( "FSUBI",       0x34, _IMM8, ),
  ( "FSUBRI",      0x35, _IMM8, ),
  ( "FMULI",       0x36, _IMM8, ),
  ( "FDIVI",       0x37, _IMM8, ),
  ( "FDIVRI",      0x38, _IMM8, ),
  ( "FPOWI",       0x39, _IMM8, ),
  ( "FCMPI",       0x3A, _IMM8, ),
  ( "FSTATUS",     0x3B, _REG, ),
  ( "FSTATUSA",    0x3C, ),
  ( "FCMP2",       0x3D, _REG, _REG, ),
  ( "FNEG",        0x3E, ),
  ( "FABS",        0x3F, ),
  ( "FINV",        0x40, ),
  ( "SQRT",        0x41, ),
  ( "ROOT",        0x42, _REG, ),
  ( "LOG",         0x43, ),
  ( "LOG10",       0x44, ),
  ( "EXP",         0x45, ),
  ( "EXP10",       0x46, ),
  ( "SIN",         0x47, ),
  ( "COS",         0x48, ),
  ( "TAN",         0x49, ),
  ( "ASIN",        0x4A, ),
  ( "ACOS",        0x4B, ),
  ( "ATAN",        0x4C, ),
  ( "ATAN2",       0x4D, _REG, ),
  ( "DEGREES",     0x4E, ),
  ( "RADIANS",     0x4F, ),
  ( "FMOD",        0x50, _REG, ),
  ( "FLOOR",       0x51, ),
  ( "CEIL",        0x52, ),
  ( "ROUND",       0x53, ),
  ( "FMIN",        0x54, _REG, ),
  ( "FMAX",        0x55, _REG, ),
  ( "FCNV",        0x56, _IMM8, ),
  ( "FMAC",        0x57, _REG, _REG, ),
  ( "FMSC",        0x58, _REG, _REG, ),
  ( "LOADBYTE",    0x59, _IMM8, ),
  ( "LOADUBYTE",   0x5A, _IMM8, ),
  ( "LOADWORD",    0x5B, _IMM16BE, ),
  ( "LOADUWORD",   0x5C, _IMM16BE, ),
  ( "LOADE",       0x5D, ),
  ( "LOADPI",      0x5E, ),
  ( "LOADCON",     0x5F, _IMM8, ),
  ( "FLOAT",       0x60, ),
  ( "FIX",         0x61, ),
  ( "FIXR",        0x62, ),
  ( "FRAC",        0x63, ),
  ( "FSPLIT",      0x64, ),
  ( "SELECTMA",    0x65, _REG, _IMM8, _IMM8, ),
  ( "SELECTMB",    0x66, _REG, _IMM8, _IMM8, ),
  ( "SELECTMC",    0x67, _REG, _IMM8, _IMM8, ),
  ( "LOADMA",      0x68, _IMM8, _IMM8, ),
  ( "LOADMB",      0x69, _IMM8, _IMM8, ),
  ( "LOADMC",      0x6A, _IMM8, _IMM8, ),
  ( "SAVEMA",      0x6B, _IMM8, _IMM8, ),
  ( "SAVEMB",      0x6C, _IMM8, _IMM8, ),
  ( "SAVEMC",      0x6D, _IMM8, _IMM8, ),
  ( "MOP",         0x6E, _IMM8, ),
  ( "SCALAR_SET",  0x6E, 0, ),
  ( "SCALAR_ADD",  0x6E, 1, ),
  ( "SCALAR_SUB",  0x6E, 2, ),
  ( "SCALAR_SUBR", 0x6E, 3, ),
  ( "SCALAR_MUL",  0x6E, 4, ),
  ( "SCALAR_DIV",  0x6E, 5, ),
  ( "SCALAR_DIVR", 0x6E, 6, ),
  ( "SCALAR_POW",  0x6E, 7, ),
  ( "EWISE_SET",   0x6E, 8, ),
  ( "EWISE_ADD",   0x6E, 9, ),
  ( "EWISE_SUB",   0x6E, 10, ),
  ( "EWISE_SUBR",  0x6E, 11, ),
  ( "EWISE_MUL",   0x6E, 12, ),
  ( "EWISE_DIV",   0x6E, 13, ),
  ( "EWISE_DIVR",  0x6E, 14, ),
  ( "EWISE_POW",   0x6E, 15, ),
  ( "MX_MULTIPLY", 0x6E, 16, ),
  ( "MX_IDENTITY", 0x6E, 17, ),
  ( "MX_DIAGONAL", 0x6E, 18, ),
  ( "MX_TRANSPOSE",0x6E, 19, ),
  ( "MX_COUNT",    0x6E, 20, ),
  ( "MX_SUM",      0x6E, 21, ),
  ( "MX_AVE",      0x6E, 22, ),
  ( "MX_MIN",      0x6E, 23, ),
  ( "MX_MAX",      0x6E, 24, ),
  ( "MX_COPYAB",   0x6E, 25, ),
  ( "MX_COPYAC",   0x6E, 26, ),
  ( "MX_COPYBA",   0x6E, 27, ),
  ( "MX_COPYBC",   0x6E, 28, ),
  ( "MX_COPYCA",   0x6E, 29, ),
  ( "MX_COPYCB",   0x6E, 30, ),
  ( "MX_DETERM",   0x6E, 31, ),
  ( "MX_INVERSE",  0x6E, 32, ),
  ( "MX_ILOADRA",  0x6E, 33, ),
  ( "MX_ILOADRB",  0x6E, 34, ),
  ( "MX_ILOADRC",  0x6E, 35, ),
  ( "MX_ILOADBA",  0x6E, 36, ),
  ( "MX_ILOADCA",  0x6E, 37, ),
  ( "MX_ISAVEAR",  0x6E, 38, ),
  ( "MX_ISAVEAB",  0x6E, 39, ),
  ( "MX_ISAVEAC",  0x6E, 40, ),
  ( "FFT",         0x6F, _IMM8, ),
  ( "WRBLK",       0x70, _IMM8, _MULTI_IMM32BE, ),
  ( "RDBLK",       0x71, _IMM8, ),
  ( "LOADIND",     0x7A, _REG, ),
  ( "SAVEIND",     0x7B, _REG, ),
  ( "INDA",        0x7C, _REG, ),
  ( "INDX",        0x7D, _REG, ),
  ( "FCALL",       0x7E, _IMM8, ),
  ( "EECALL",      0x7F, _EFUNC, ),
  ( "RET",         0x80, ),
  ( "BRA",         0x81, _REL8_LABEL, ),
  ( "BRACC",       0x82, _CC, _REL8_LABEL, ),
  ( "JMP",         0x83, _ABS16_LABEL, ),
  ( "JMPCC",       0x84, _CC, _ABS16_LABEL, ),
  ( "TABLE",       0x85, _IMM8, _MULTI_IMM32BE, ),
  ( "TABLEF",      0x85, _IMM8, _MULTI_FLOAT, ),
  ( "FTABLE",      0x86, _CC, _IMM8, _MULTI_IMM32BE, ),
  ( "FTABLEF",     0x86, _CC, _IMM8, _MULTI_FLOAT, ),
  ( "LTABLE",      0x87, _CC, _IMM8, _MULTI_IMM32BE, ),
  ( "LTABLEF",     0x87, _CC, _IMM8, _MULTI_FLOAT, ),
  ( "POLY",        0x88, _CC, _MULTI_FLOAT, ),
  ( "GOTO",        0x89, _IMM8, ),
  ( "RETCC",       0x8A, _CC, ),
  ( "LWRITE",      0x90, _REG, _IMM32BE, ),
  ( "LWRITEA",     0x91, _IMM32BE, ),
  ( "LWRITEX",     0x92, _IMM32BE, ),
  ( "LWRITE0",     0x93, _IMM32BE, ),
  ( "LREAD",       0x94, _REG, ),
  ( "LREADA",      0x95, ),
  ( "LREADX",      0x96, ),
  ( "LREAD0",      0x97, ),
  ( "LREADBYTE",   0x98, ),
  ( "LREADWORD",   0x99, ),
  ( "ATOL",        0x9A, _ASCIIZ, ),
  ( "LTOA",        0x9B, _IMM8, ),
  ( "LSET",        0x9C, _REG, ),
  ( "LADD",        0x9D, _REG, ),
  ( "LSUB",        0x9E, _REG, ),
  ( "LMUL",        0x9F, _REG, ),
  ( "LDIV",        0xA0, _REG, ),
  ( "LCMP",        0xA1, _REG, ),
  ( "LUDIV",       0xA2, _REG, ),
  ( "LUCMP",       0xA3, _REG, ),
  ( "LTST",        0xA4, _REG, ),
  ( "LSET0",       0xA5, ),
  ( "LADD0",       0xA6, ),
  ( "LSUB0",       0xA7, ),
  ( "LMUL0",       0xA8, ),
  ( "LDIV0",       0xA9, ),
  ( "LCMP0",       0xAA, ),
  ( "LUDIV0",      0xAB, ),
  ( "LUCMP0",      0xAC, ),
  ( "LTST0",       0xAD, ),
  ( "LSETI",       0xAE, _IMM8, ),
  ( "LADDI",       0xAF, _IMM8, ),
  ( "LSUBI",       0xB0, _IMM8, ),
  ( "LMULI",       0xB1, _IMM8, ),
  ( "LDIVI",       0xB2, _IMM8, ),
  ( "LCMPI",       0xB3, _IMM8, ),
  ( "LUDIVI",      0xB4, _IMM8, ),
  ( "LUCMPI",      0xB5, _IMM8, ),
  ( "LTSTI",       0xB6, _IMM8, ),
  ( "LSTATUS",     0xB7, _REG, ),
  ( "LSTATUSA",    0xB8, ),
  ( "LCMP2",       0xB9, _REG, _REG, ),
  ( "LUCMP2",      0xBA, _REG, _REG, ),
  ( "LNEG",        0xBB, ),
  ( "LABS",        0xBC, ),
  ( "LINC",        0xBD, _REG, ),
  ( "LDEC",        0xBE, _REG, ),
  ( "LNOT",        0xBF, ),
  ( "LAND",        0xC0, _REG, ),
  ( "LOR",         0xC1, _REG, ),
  ( "LXOR",        0xC2, _REG, ),
  ( "LSHIFT",      0xC3, _REG, ),
  ( "LMIN",        0xC4, _REG, ),
  ( "LMAX",        0xC5, _REG, ),
  ( "LONGBYTE",    0xC6, _IMM8, ),
  ( "LONGUBYTE",   0xC7, _IMM8, ),
  ( "LONGWORD",    0xC8, _IMM16BE, ),
  ( "LONGUWORD",   0xC9, _IMM16BE, ),
  ( "SETSTATUS",   0xCD, _IMM8, ),
  ( "SEROUT0",     0xCE, 0, _IMM8, ),
  ( "SEROUT1",     0xCE, 1, _ASCIIZ, ),
  ( "SEROUT2",     0xCE, 2, ),
  ( "SEROUT3",     0xCE, 3, ),
  ( "SEROUT4",     0xCE, 4, ),
  ( "SEROUT5",     0xCE, 5, _ASCIIZ, ),
  ( "SERIN",       0xCF, _IMM8, ),
  ( "SETOUT",      0xD0, _IMM8, ),
  ( "ADCMODE",     0xD1, _IMM8, ),
  ( "ADCTRIG",     0xD2, ),
  ( "ADCSCALE",    0xD3, _IMM8, ),
  ( "ADCLONG",     0xD4, _IMM8, ),
  ( "ADCLOAD",     0xD5, _IMM8, ),
  ( "ADCWAIT",     0xD6, ),
  ( "TIMESET",     0xD7, ),
  ( "TIMELONG",    0xD8, ),
  ( "TICKLONG",    0xD9, ),
  ( "EESAVE",      0xDA, _REG, _IMM8, ),
  ( "EESAVEA",     0xDB, _IMM8, ),
  ( "EELOAD",      0xDC, _REG, _IMM8, ),
  ( "EELOADA",     0xDD, _IMM8, ),
  ( "EEWRITE",     0xDE, _IMM8, _IMM8, _MULTI_IMM8),
  ( "EXTSET",      0xE0, ),
  ( "EXTLONG",     0xE1, ),
  ( "EXTWAIT",     0xE2, ),
  ( "STRSET",      0xE3, _ASCIIZ, ),
  ( "STRSEL",      0xE4, _IMM8, _IMM8, ),
  ( "STRINS",      0xE5, _ASCIIZ, ),
  ( "STRCMP",      0xE6, _ASCIIZ, ),
  ( "STRFIND",     0xE7, _ASCIIZ, ),
  ( "STRFCHR",     0xE8, _ASCIIZ, ),
  ( "STRFIELD",    0xE9, _IMM8, ),
  ( "STRTOF",      0xEA, ),
  ( "STRTOL",      0xEB, ),
  ( "READSEL",     0xEC, ),
  ( "STRBYTE",     0xED, ),
  ( "STRINC",      0xEE, ),
  ( "STRDEC",      0xEF, ),
  ( "SYNC",        0xF0, ),
  ( "READSTATUS",  0xF1, ),
  ( "READSTR",     0xF2, ),
  ( "VERSION",     0xF3, ),
  ( "IEEEMODE",    0xF4, ),
  ( "PICMODE",     0xF5, ),
  ( "CHECKSUM",    0xF6, ),
  ( "BREAK",       0xF7, ),
  ( "TRACEOFF",    0xF8, ),
  ( "TRACEON",     0xF9, ),
  ( "TRACESTR",    0xFA, _ASCIIZ, ),
  ( "TRACEREG",    0xFB, _REG, ),
  ( "READVAR",     0xFC, _IMM8, ),
  ( "RESET",       0xFF, ),
  ( "℘ALIGN",      _ALIGN, ),
  ( "℘ENCSTART",   _ENCSTART, ),
  ( "℘ENCEND",     _ENCEND, ),
]

class AbsRef:  # 16-bit absolute reference.
  def __init__(self, label):
    self.label = label

class RelRef:  # 8-bit relative reference.
  def __init__(self, label):
    self.label = label

class EFunc:  # EEPROM function reference (external linking).
  def __init__(self, name):
    self.name = name

class ItemInstr:
  def __init__(self, offset, pbytes, length):
    self.offset = offset
    self.pbytes = pbytes
    self.length = length

class Code:
  def __init__(self, src=None, offset=0):
    self.instr = []
    self.offset = offset  # Byte offset from the beginning of Code.
    self.labels = {}
    self.data = None
    self.enc_sections = []

    # Add instruction functions.
    for ins, *opcodes in INSTRUCTIONS:
      self.__dict__[ins] = partial(self.__instr, ins, opcodes)

    # Add some aliases.
    for i in range(128):
      self.__dict__['r%i' % i] = i

    if src:
      self.asm(src)

  def __instr(self, ins, opcodes, *args):
    args = list(args)
    #print(ins, opcodes, args)
    pbytes = []
    for op in opcodes:
      if type(op) is int:
        pbytes.append(op)
      elif type(op) is str:
        # In a weird case when the argument is a special object, just append it
        # to the queue. This might have unpredictable consequences, but...
        if args and (type(args[0]) in {EFunc, AbsRef, RelRef}):
          pbytes.append(args.pop(0))
        elif op == _REG or op == _IMM8 or op == _CC:
          v = (args.pop(0) + 0x100) & 0xff
          pbytes.append(v)
        elif op == _FLOAT:
          pbytes.append(pack(">f", args.pop(0)))
        elif op == _ASCIIZ:
          pbytes.append(args.pop(0))
          pbytes.append(0)  # Null byte.
        elif op == _IMM16BE:
          v = (args.pop(0) + 0x10000) & 0xffff
          pbytes.append(pack(">H", v))
        elif op == _IMM32BE:
          v = (args.pop(0) + 0x100000000) & 0xffffffff
          pbytes.append(pack(">I", v))
        elif op == _MULTI_IMM32BE:
          for arg in args:
            v = (arg + 0x100000000) & 0xffffffff
            pbytes.append(pack(">I", v))
        elif op == _MULTI_IMM16BE:
          raise Exception("impl: _MULTI_IMM16BE")
        elif op == _MULTI_IMM8:
          for arg in args:
            v = (arg + 0x100) & 0xff
            pbytes.append(v)
        elif op == _MULTI_FLOAT:
          raise Exception("impl: _MULTI_FLOAT")
        elif op == _REL8_LABEL:
          pbytes.append(RelRef(args.pop(0)))
        elif op == _ABS16_LABEL:
          pbytes.append(AbsRef(args.pop(0)))
        elif op == _EFUNC:
          slot = args.pop(0)
          # There is a small chance it's actually a byte.
          if type(slot) is int:
            v = (slot + 0x100) & 0xff
            pbytes.append(v)
          else:
            pbytes.append(EFunc(slot))
        elif op == _ALIGN:
          alignment = args.pop(0)
          diff = (self.offset + 1) % alignment  # +1 due to EEPROM image func size.
          if diff != 0:
            padding = alignment - diff
            pbytes.append(b"\0" * padding)  # \0 is NOP
        elif op == _ENCSTART:
          self.enc_start = self.offset
          self.enc_key = args.pop(0)
        elif op == _ENCEND:
          self.enc_sections.append((
            self.enc_start, self.offset, self.enc_key))
          self.enc_start = None
          self.enc_key = None
        else:
          raise Exception("Wrong op type? %s" % op)
      else:
        raise Exception("Instr type error?")

    self.__add_instr(pbytes)

  def __add_instr(self, pbytes):
    length = 0
    for pb in pbytes:
      if type(pb) is AbsRef:
        length += 2
      elif type(pb) is bytes:
        length += len(pb)
      elif type(pb) is str:
        length += len(bytes(pb, "utf-8"))
      else:
        length += 1

    self.instr.append(
        ItemInstr(self.offset, pbytes, length))

    self.offset += length

    return self.offset

  def LABEL(self, label):
    self.labels[label] = self.offset
    return label

  def finalize(self):
    self.data = bytearray(self.offset)
    self.imports = []
    i = 0
    for instr in self.instr:
      for pb in instr.pbytes:
        if type(pb) is AbsRef:
          abs_addr = self.labels[pb.label]
          self.data[i] = (abs_addr >> 8) & 0xff
          self.data[i+1] = abs_addr & 0xff
          i += 2
        elif type(pb) is RelRef:
          abs_addr = self.labels[pb.label]
          next_addr = instr.offset + instr.length
          diff = abs_addr - next_addr
          if diff < -128 or diff > 127:
            raise Exception("RelRef difference too large")
          self.data[i] = (diff + 256) & 0xff
          i += 1
        elif type(pb) is EFunc:
          self.data[i] = 0xff  # This is basically a placeholder.
          self.imports.append((pb.name, i))
          i += 1
        elif type(pb) is int:
          self.data[i] = (pb + 256) & 0xff
          i += 1
        elif type(pb) is bytes:
          self.data[i:i+len(pb)] = pb
          i += len(pb)
        elif type(pb) is str:
          data = bytes(pb, "utf-8")
          self.data[i:i+len(data)] = data
          i += len(data)
        else:
          raise Exception("???")

    return self.data

  # I would like to submit this to the 'biggest hack in history' competition.
  def asm(self, source, extra_locals=None):
    # String processing.
    lines = [ln.strip() for ln in source.splitlines()]
    for i, ln in enumerate(lines):
      if ln.startswith('!'):
        lines[i] = ln[1:]
        continue
      if ln.endswith(':'):
        lines[i] = "LABEL('%s')" % ln[:-1]
        continue
      s = ln.split(" ", 1) + [""]
      s[0] = s[0].upper()
      lines[i] = "%s(%s)" % tuple(s[:2])
    source = '\n'.join(lines)

    # Run as Python. Put self's methods in local namespace.
    cc = compile(source, "?pysm?", "exec")
    locals = {n: getattr(self, n) for n in self.__dir__() if not n.startswith('__')}
    if extra_locals:
      locals.update(extra_locals)
    exec(cc, globals(), locals)

class EEPROMFunction:
  def __init__(self, slot, code):
    if type(code) is str:
      c = Code()
      c.asm(code)
      code = c

    if type(code) is not Code:
      raise Exception("EEPROMFunction expects Code object or asm code")

    self.code = code
    self.data = code.finalize()
    self.slot = slot

  def get_data(self):
    return self.data

  def get_slot(self):
    return self.slot

class EEPROMImage:
  def __init__(self, slot, functions=None):
    self.slot = slot
    self.image = None
    self.functions = {}
    if functions:
      self.functions.update(functions)

    self.function_slots = None

  def get_data(self):
    if self.image is None:
      self.perform_linking()
    return self.image

  def get_slot(self):
    return self.slot

  def get_function_slot(self, name):
    if self.image is None:
      self.perform_linking()
    return self.function_slots[name]

  def perform_linking(self):
    print("...linking park")
    imports = []
    encrypted_sections = []
    self.function_slots = {}

    # Make sure the code is finalized and calc size.
    print("   ...compiling functions")
    total_sz = 0
    for code in self.functions.values():
      code.finalize()
      total_sz += (1 + len(code.data) + 3) & ~3  # Needs to be padded to 4 bytes.

    # Create the image and note the imports to be resolved.
    print("   ...flattening image")
    self.image = bytearray(total_sz)
    offset = 0
    for name, code in self.functions.items():
      self.image[offset] = len(code.data)
      self.image[1 + offset:1 + offset + len(code.data)] = code.data

      slot = self.slot + offset // 4
      self.function_slots[name] = slot
      print("      [%3i] %s (%i bytes)" % (slot, name, len(code.data)))

      for name, fix_offset in code.imports:
        imports.append((name, 1 + offset + fix_offset))

      for enc_start, enc_end, enc_key in code.enc_sections:
        encrypted_sections.append((1 + enc_start + offset, 1 + enc_end + offset, enc_key))

      offset += 1 + (len(code.data) + 3) & ~3  # Needs to be padded to 4 bytes.

    # Now that the function slot table is complete, resolve the imports.
    print("   ...linking")
    for name, fix_offset in imports:
      self.image[fix_offset] = self.function_slots[name]
      print("      fix 0x%.4x --> %s --> %i" % (fix_offset, name, self.function_slots[name]))

    # And apply encryption (for whatever it's actually worth).
    print("   ...encrypting")
    for enc_start, enc_end, enc_key in encrypted_sections:
      print("      encrypting 0x%.4x --> 0x%.4x (key 0x%.8x)" % (
        enc_start, enc_end, enc_key))
      i = enc_start
      while i < enc_end:
        v = unpack("<I", self.image[i:i+4])[0]
        v ^= enc_key
        self.image[i:i+4] = pack("<I", v)
        i += 4

    print("   ...final size: %i" % len(self.image))

# Note: This function doesn't care about potential 1-byte overwrite padding.
def generate_eeprom_data(name, data_obj):
  slot = data_obj.get_slot()

  data = data_obj.get_data()
  data_sz = len(data)

  s = ["byte %s[] = {\n" % name]
  for i in range(data_sz):
    if i % 16 == 0:
      s.append('  ')

    s.append("0x%.2x" % data[i])

    if i != data_sz - 1:
      s.append(", ")

    if (i + 1) % 16 == 0 or i == data_sz - 1:
      s.append("\n")


  s.append("};\n")
  s.append("const size_t %s_sz = %u;\n" % (name, data_sz));
  s.append("const byte %s_slot = %u;\n\n" % (name, slot));

  s = ''.join(s)
  return s

def generate_function_slot(var_name, data_obj, func_name):
  slot = data_obj.get_function_slot(func_name)
  return "byte %s = %i;" % (var_name, slot)

