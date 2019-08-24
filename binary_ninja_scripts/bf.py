#!/usr/bin/python3
#
# Binary Ninja Brainfuck architecture plugin
#   by Gynvael Coldwind
#
# Note: this was made after a BN training as an exercise - it's terrible and
# hacky as hell, but whatever.

from binaryninja import *
import json

INSTRUCTION_IL = {
  '>': lambda il: il.append(il.set_reg(2, "ptr", il.add(2, il.reg(2, "ptr"), il.const(2, 1)))),
  '<': lambda il: il.append(il.set_reg(2, "ptr", il.sub(2, il.reg(2, "ptr"), il.const(2, 1)))),
  '+': lambda il: il.append(il.store(1, il.reg(2, "ptr"), il.add(1, il.load(1, il.reg(2, "ptr")), il.const(1, 1)))),
  '-': lambda il: il.append(il.store(1, il.reg(2, "ptr"), il.sub(1, il.load(1, il.reg(2, "ptr")), il.const(1, 1)))),
  '.': lambda il: il.append(il.system_call()),
  ',': lambda il: il.append(il.system_call()),
}

# InstructionTextToken(InstructionTextTokenType.TextToken, "asdf ")
INSTRUCTION_TEXT = {
  '>': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "inc"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " "),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr") ],
  '<': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "dec"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " "),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr") ],
  '+': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "inc"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " ["),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr"),
         InstructionTextToken(InstructionTextTokenType.TextToken, "]") ],
  '-': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "dec"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " ["),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr"),
         InstructionTextToken(InstructionTextTokenType.TextToken, "]") ],
  '.': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "putc"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " "),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr") ],
  ',': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "getc"),
         InstructionTextToken(InstructionTextTokenType.TextToken, " "),
         InstructionTextToken(InstructionTextTokenType.RegisterToken, "ptr") ],
  '[': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "jfwdz ") ],
  ']': [ InstructionTextToken(InstructionTextTokenType.InstructionToken, "jbacknz ") ],
}

class Brainfuck(Architecture):
  jump_map = None
  jump_targets = None
  name = "bf"
  address_size = 4
  default_int_size = 1
  instr_alignment = 1
  max_instr_length = 1
  regs = {
    "ptr": RegisterInfo("ptr", 2),
    "fake_sp": RegisterInfo("fake_sp", 2),  # Fake Stack Pointer
  }
  stack_pointer = "fake_sp"

  def always_branch(self, data, addr):
    print("always_branch")
    return False

  def invert_branch(self, data, addr):
    print("invert_branch")
    return False

  def is_always_branch_patch_available(self, data, addr):
    print("is_always_branch_patch_available")
    return False

  def is_invert_branch_patch_available(self, data, addr):
    print("is_invert_branch_patch_available")
    return False

  def is_never_branch_patch_available(self, data, addr):
    print("is_never_branch_patch_available")
    return False

  def is_skip_and_return_value_patch_available(self, data, addr):
    print("is_skip_and_return_value_patch_available")
    return False

  def is_skip_and_return_zero_patch_available(self, data, addr):
    print("is_skip_and_return_zero_patch_available")
    return False

  def skip_and_return_value(self, data, addr, value):
    print("skip_and_return_value")
    return False

  def assemble(self, code, addr):
    print("assemble")
    return False

  def check_jump_map(self):
    if self.jump_map is None:
      with open("jumpmap.json", "r") as f:
        self.jump_map = json.load(f)
      itm = self.jump_map.items()
      self.jump_map = {}
      self.jump_targets = set()
      for k, v in itm:
        self.jump_map[int(k)] = v
        self.jump_targets.add(v)

  def get_instruction_info(self, data, addr):
    self.check_jump_map()
    b = data[0]
    result = InstructionInfo()
    result.length = 1

    if addr in self.jump_map:
      result.add_branch(BranchType.TrueBranch, self.jump_map[addr])
      result.add_branch(BranchType.FalseBranch, addr + 1)

    return result

  def get_instruction_text(self, data, addr):
    self.check_jump_map()
    if not data:
      return [], 0

    b = chr(data[0])
    tokens = INSTRUCTION_TEXT.get(b, [ InstructionTextToken(InstructionTextTokenType.TextToken, "nop") ])[:]

    if b in {'[', ']'}:
      if addr in self.jump_map:
        dest = self.jump_map[addr]
        tokens.append(InstructionTextToken(InstructionTextTokenType.PossibleAddressToken, "0x%.4x" % dest, dest))
      else:
        tokens.append(InstructionTextToken(InstructionTextTokenType.TextToken, "???"))

    return tokens, 1

  def get_instruction_low_level_il(self, data, addr, il):
    self.check_jump_map()

    text_section = il.source_function.view.sections[".text"]

    if addr == text_section.start:
      il.append(il.set_reg(2, "ptr", il.const(2, 0)))

    b = chr(data[0])
    if b in {'[', ']'}:
      true_label = il.get_label_for_address(Architecture['bf'], self.jump_map[addr])
      false_label = il.get_label_for_address(Architecture['bf'], addr + 1)

      if b == '[':
        cond = il.compare_equal(1, il.load(1, il.reg(1, "ptr")), il.const(1, 0))
      else:
        cond = il.compare_not_equal(1, il.load(1, il.reg(1, "ptr")), il.const(1, 0))
      il.append(il.if_expr(cond, true_label, false_label))
    else:
      f = INSTRUCTION_IL.get(b, lambda il_: il_.append(il_.nop()))
      f(il)

    if b == '.':
      il.source_function.set_comment_at(addr, "putc")
    elif b == ',':
      il.source_function.set_comment_at(addr, "getc")

    if addr == text_section.end - 1:
      il.append(il.trap(0))
      il.source_function.set_comment_at(addr, "the end")

    return 1

  def convert_to_nop(self, data, addr):
    return b' ' * len(data)

Brainfuck.register()

def fill_jump_map(bv, va):
  jump_map = {}
  data = bv.read(0, len(bv))
  stack = []
  for i, b in enumerate(data):
    b = chr(b)

    if b == '[':
      stack.append(va + i)
      continue

    if b == ']':
      if not stack:
        log_error("Jump map scanning failed - mispaired []")
        return
      start = stack.pop(-1)
      end = va + i

      jump_map[start] = end + 1
      jump_map[end] = start + 1
      continue

  return jump_map

class BrainfuckView(BinaryView):
  name = "Brainfuck"
  long_name = "Brainfuck View"

  def __init__(self, raw_bv):
    BinaryView.__init__(self, parent_view=raw_bv, file_metadata=raw_bv.file)
    self.raw_bv = raw_bv

  @classmethod
  def is_valid_for_data(self, raw_bv):
    # There isn't much we can do here, since bf files don't really have a magic
    # value or anything like that. So we just check if it's ASCII-only in the
    # first 1024 bytes.
    data = raw_bv.read(0, 1024)
    return all([((b >= 0x20 and b <= 0x7e) or b == 0xa or b == 0xd or b == 0x9)
                for b in data])

  def init(self):
    self.arch = Architecture['bf']
    self.platform = Architecture['bf'].standalone_platform
    data_len = len(self.raw_bv)
    self.add_auto_segment(0x40000, data_len, 0, data_len, SegmentFlag.SegmentContainsCode)
    self.add_auto_segment(0, 0x10000, 0, 0, SegmentFlag.SegmentWritable | SegmentFlag.SegmentReadable)
    self.add_auto_section(".text", 0x40000, data_len, SectionSemantics.ReadOnlyCodeSectionSemantics)
    self.add_auto_section(".data", 0, 0x10000, SectionSemantics.ReadWriteDataSectionSemantics)
    self.add_entry_point(0x40000)

    print(type(self.arch))

    jump_map = fill_jump_map(self.raw_bv, 0x40000)
    with open("jumpmap.json", "w") as f:
      json.dump(jump_map, f)
    return True

  def platform_is_executable(self):
    return True

  def perform_get_entry_point(self):
    return 0x40000

  def perform_get_address_size(self):
    return 4


BrainfuckView.register()


def main(argv):
  if len(argv) != 2:
    sys.exit("usage: python3 bf.py <fname>")

  log_to_stderr(LogLevel.InfoLog)

  bv = BinaryViewType.get_view_of_file(argv[1])

  # Print whatever for debug.
  f = bv.functions[0]
  for block in f:
    print("---")
    for i in block:
      print(str(i))


  bv.file.close()

if __name__ == '__main__':
  main(sys.argv)

