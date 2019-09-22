#!/usr/bin/python3
from fpuc import *

def EncryptedCode(name, key, code):
  prolog = """
    # Preserve arguments.
    SELECTMA r16, 1, 8
    SELECTMB r24, 1, 8
    MX_COPYAB

    # Decrypt.
    LONGUBYTE EFunc("%(name)s")
    COPY0 r16
    LONGUWORD AbsRef("__encrypted_start")
    COPY0 r17
    LONGUWORD AbsRef("__encrypted_end")
    COPY0 r18
    SELECTA r18
    LSUB r17
    LWRITE r19, 0x%(key)x
    EECALL "recrypt"

    # Restore arguments.
    SELECTMA r16, 1, 8
    SELECTMB r24, 1, 8
    MX_COPYBA


    # Pass-through to the actual (decrypted hopefully) code.
    ℘ALIGN 4
    ℘ENCSTART 0x%(key).8x
  __encrypted_start:
  """ % {
      "name": name,
      "key": key
  }

  epilog = """
    ℘ALIGN 4
    ℘ENCEND
  __encrypted_end:

    # Preserve return value.
    COPY r0, r24

    # Recrypt.
    LONGUBYTE EFunc("%(name)s")
    COPY0 r16
    LONGUWORD AbsRef("__encrypted_start")
    COPY0 r17
    LONGUWORD AbsRef("__encrypted_end")
    COPY0 r18
    SELECTA r18
    LSUB r17
    LWRITE r19, 0x%(key).8x
    EECALL "recrypt"

    # Restore return value.
    COPY r24, r0
    RET
  """ % {
      "name": name,
      "key": key
  }

  # Replace any RET's to proper jumps.
  actual_code = []
  for ln in code.splitlines():
    stripped_ln = ln.strip()
    if stripped_ln == 'RET':
      ln = '    JMP "__encrypted_end"'
    actual_code.append(ln)

  actual_code = '\n'.join(actual_code)
  full_code = '\n\n'.join([prolog, actual_code, epilog])
  #print(full_code)

  return Code(full_code)

img = EEPROMImage(0, {
  "start": Code("""
    # Initialize PVSP
    SELECTA r127
    LSETI 92

    # Jump to real main
    EECALL "main"
    RET
  """),

  "main": EncryptedCode("main", 0x4ba9dc18, """
    # Check length.
    READVAR 14
    SELECTA r0
    LUCMPI 23
    BRACC 0x10, "wrong"

    # Check prefix.
    STRSEL 0, 6
    STRCMP "DrgnS{"
    BRACC 0x10, "wrong"

    # Check suffix.
    STRSEL 22, 1
    STRCMP "}"
    BRACC 0x10, "wrong"

    # Copy the flag to the registers.
    STRFCHR "{}"
    STRFIELD 2

    SELECTA r1
    LSETI 40

    CLR r2

  flag_copy_loop:
    SELECTA r0
    READVAR 17
    SAVEIND r1
    LINC r1
    LINC r2
    STRINC

    SELECTA r2
    LUCMPI 16
    JMPCC 0x50, "flag_copy_loop"

    EECALL "verify_flag"
    SELECTA r0
    LUCMPI 1
    BRACC 0x50, "wrong"

    STRSET "Well Done!"
    RET

  wrong:
    STRSET "Wrong flag."
    RET

  """),

  "load_magics": EncryptedCode("load_magics", 0x17db41dc, """
    SELECTX r72
    WRBLK 16, 0x7ae58502, 0x22b82d4f, 0x3d2900f9, 0x1337af79, 0x102e2716, 0x6e3e276d, 0x0dab7258, 0x2c42952c, 0x72d3889b, 0x110de1f0, 0x58a098d1, 0x79230535, 0x37d04671, 0x0965af85, 0x139195a6, 0x2b733562
    RET
  """),

  "verify_flag": EncryptedCode("verify_flag", 0x73d401ac, """
    EECALL "load_magics"

    SELECTMB r40, 4, 4
    SELECTMA r56, 4, 4
    MX_TRANSPOSE

    # This is a hack using the fact that denormal adding works the same way
    # as uint32 adding.
    CLR0
    LINC r0
    LINC r0
    LINC r0
    SCALAR_ADD

    # Init LCG (almost identical as glibc).
    LWRITE r2, 1103515245
    LWRITE r3, 0xa5a5a5a5
    LWRITE r4, 12345
    LWRITE r5, 0x7fffffff

    SELECTA r9
    LSETI 8
    SELECTA r10
    LSETI 16
    SELECTA r11
    LSETI 24

    # XOR the flag.
    SELECTA r1
    LSETI 56

    CLR r12

    SELECTA r13
    LSETI 72

  xor_loop:
    # Iterate LCG (output is in r3).
    SELECTA r3
    LMUL r2
    LADD r4
    LAND r5

    # Load character code.
    LOADIND r1

    # Clone byte (0x000000Nn --> 0xNnNnNnNn)
    SELECTA r0
    COPY0 r6
    COPY0 r7
    COPY0 r8
    SELECTA r6
    LSHIFT r9
    SELECTA r7
    LSHIFT r10
    SELECTA r8
    LSHIFT r11
    SELECTA r0
    LOR r6
    LOR r7
    LOR r8

    # XOR with LCG's output.
    LXOR r3
    SAVEIND r1

    # Constant time comparison.
    COPY0 r6
    LOADIND r13
    SELECTA r0
    LXOR r6
    SELECTA r12
    LOR r0

    # Iterate / check condition.
    LINC r13
    LINC r1
    SELECTA r1
    LUCMPI 72
    BRACC 0x50, "xor_loop"

    # Return.
    LINC r12
    COPY r12, r0
    RET
  """),

  # push reg[A]
  "pvs_push": Code("""
    SAVEIND r127
    LINC r127
    RET
  """),

  # reg[A] = pop
  "pvs_pop": Code("""
    LDEC r127
    LOADIND r127
    LSET0
    RET
  """),

  # Args:
  #  r16 EEPROM start slot
  #  r17 byte offset from start slot (4-byte aligned!)
  #  r18 size in bytes (4-byte aligned!)
  #  r19 32-bit XOR key
  "recrypt": Code("""
    !arg_start_slot = r16
    !arg_offset = r17
    !arg_size = r18
    !arg_key = r19

    # Preserve r24, r25 and r26.
    SELECTA r24
    EECALL "pvs_push"
    SELECTA r25
    EECALL "pvs_push"
    SELECTA r26
    EECALL "pvs_push"

    # Calculate the actual slot.
    !shift_by = r1
    SELECTA shift_by
    LSETI -2

    SELECTA arg_size
    LSHIFT shift_by

    SELECTA arg_offset
    LINC arg_offset
    LSHIFT shift_by

    SELECTA arg_start_slot
    LADD arg_offset

    !last_slot = r24
    SELECTA last_slot
    LSET arg_start_slot
    LADD arg_size

    !curr_slot = r25
    COPY arg_start_slot, curr_slot

    !xor_key = r26
    COPY arg_key, xor_key

  loop:
    !param_1 = r16
    !param_2 = r17

    COPY curr_slot, param_1
    EECALL "read_slot"

    SELECTA r0
    LXOR xor_key

    COPY r0, param_2
    COPY curr_slot, param_1
    EECALL "write_slot"

    LINC curr_slot
    LUCMP2 curr_slot, last_slot
    JMPCC 0x50, "loop"

    # Restore r26, r25 and r24.
    SELECTA r26
    EECALL "pvs_pop"
    SELECTA r25
    EECALL "pvs_pop"
    SELECTA r24
    EECALL "pvs_pop"

    RET
  """),

  # Write a 32-bit value to the specified slot.
  # Args:
  #  r16 EEPROM slot number
  #  r17 32-bit value to write
  "write_slot": Code("""
    !arg_slot = r16
    !arg_value = r17

    !shift_by = r2
    SELECTA shift_by
    LSETI 16

    SELECTA arg_slot
    LSHIFT shift_by

    # In reverse:
    # 03    - length of function
    # DB 00 - EESAVEA 0 (to be masked in)
    # 80    - RET
    !opcodes = r1
    SELECTA opcodes
    LWRITEA 0x8000DB03
    LOR arg_slot
    EESAVEA 0xff

    SELECTA arg_value
    EECALL 0xff

    RET
  """),

  # Read a 32-bit value from the specified slot.
  # Args:
  #  r16 EEPROM slot number
  "read_slot": Code("""
    !arg_slot = r16
    !ret_val = r0

    !shift_by = r2
    SELECTA shift_by
    LSETI 16

    SELECTA arg_slot
    LSHIFT shift_by

    # In reverse:
    # 03    - length of function
    # DD 00 - EELOADA 0 (to be masked in)
    # 80    - RET
    !opcodes = r1
    SELECTA opcodes
    LWRITEA 0x8000DD03
    LOR arg_slot
    EESAVEA 0xff

    SELECTA ret_val
    EECALL 0xff

    RET
  """),

})

s = generate_eeprom_data("task", img)
s += generate_function_slot("start_slot", img, "start")
#print()
#print(s)
with open("generated.h", "w") as f:
  f.write(s)

