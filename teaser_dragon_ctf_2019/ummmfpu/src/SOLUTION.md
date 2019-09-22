Solution
========

1. Well, learn the architecture - there are a lot of docs / libraries / etc on Micromega's website. Here's a short version though
  * There are 128 general purpose 32-bit registers, a 127-byte string buffer, a status register, and two alias-registers (`A` and `X`) which can be mapped to any other general-purpose registers.
  * The string buffer actually has three accompanying unnamed registers: one for length of the string currently in the buffer, one for start of selection (i.e. cursor placement) and one for length of selection. There are a couple of instructions to operate on the string as well.
  * There are also 3 matrix registers, which basically describe N-by-M matrices mapped to general purpose registers.
  * There is no (visible) stack, though there are two small hidden stacks for function calls and accumulator values (see `LEFT` and `RIGHT` instructions).
  * The FPU is programmable, meaning you can upload your functions to either EEPROM or Flash. For the sake of this task only EEPROM is used.
  * Apart from putting code in EEPROM/Flash, you can just send (SPI) opcodes to the FPU and it will immediately execute them.
  * The EEPROM can be re-programmed runtime from code - and yes, this is used in the task for SMC, obviously. One note here is that the EEPROM's slots are 32-bit (though the EEPROM in some cases behaves like each slot would be made of two 16-bit words, but that's not important for this task).
  * The FPU has some other bells and whistles, but they are irrelevant for this task.
2. Once you understand the structure of EEPROM (it's basically 1 byte length of function followed by the function's opcodes), you can write a disassembler.
3. There are 9 functions, 6 of them plaintext, and 3 encrypted (names are not in the image, I'm just using the ones I used to create the task for simplicity):
  * `start`: does some short initialization and calls `???1`
  * `pvs_push`: saves a value on a software-made stack (`r127` acts as stack pointer)
  * `pvs_pop`: restores a value from the stack
  * `write_slot`: writes 32-bit value to EEPROM slot
  * `read_slot`: reads a 32-bit value from an EEPROM slot
  * `recrypt`: XORs specified EEPROM slots with a 32-bit key
  * `???1` to `???3`: is encrypted
4. Analyzing the encryption prologue/epilogue should reveal that it basically calls `recrypt` once at the beginning of the function and once at the end. This should allow one to decrypt and disassemble the body of `???1` - `???3`.
5. Analyzing the 3 encrypted functions should reveal:
  * `load_magics`: loads 16 32-bit constants
  * `main`: does simple checks on the provided flag (length, `DrgnS{` prefix, `}` suffix) and calls `verify_flag`
  * `verify_flag`: does a set of simple transformations on the provided flag (matrix transposition, add 3, xor lcg()) and compares the result with the constants from `load_magics`
6. Reversing the values from `load_magics` based on the `verify_flag` transformations should yield the flag.

Apart from SMC there aren't really any gotchas, though I tried to use as many different (weird) instructions as possible in the code.

