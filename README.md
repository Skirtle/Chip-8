# CHIP-8 emulator
###### Sources [CHIP-8 Wikipedia article](https://en.wikipedia.org/wiki/CHIP-8)

## What is CHIP-8?
CHIP-8 is an interpreted programming language created by Joseph Weisbacker for an 1802 microprocessor, designed to be easy to create programs for and using less memory than other languages

## Virtual machine description
### Memory
4096 bytes of memory, but the interpreter allocated the first 512. Uppermost 256 bytes are reserved for display refresh and 96 bytes below that are reserved for call stack, iunternal use, and other variables. Modern implementations do not need to avoid using the first 512 bytes. We will keep true to the original by leaving it blank, except for font data.
### Registers
16 8-bit registers, from V0 to VF, VF doubles as a flag for certain instructions, so try to avoid using it (the carry flag for addition, the 'no borrow' flag for subtraction, and set upon pixel collision for draw). The address register, I, is 12 bits. 
### Stack
Stores return addressess when subroutines are called. Original version allocated 48 bytes for up to 12 levels of nesting
### Timers
Two 60 Hz timers:
* Delay timer: Intended for timing events of a game. Can be set and read
* Sound timer: Intended for sound effects. When nonzero, beeping sound. Can only be set
### Input
Hex keyboard (16 keys, 0-F). 8, 4, 6, and 2 are used as directional inputs. There are three opcodes for inputs. One skips the next instruction if a specific key is pressed. Another skips if a key isn't pressed. The third waits for a key and stores it when pressed
### Graphics and sound
The display is monochrome 64x32 pixels. Graphics drawn via sprites (8 pixels wide and 1-15 in height). Sprite pixels XOR'd with screen pixels. VF set to 1 if any screen pixels are flipped from to set unset when a sprite is drawn and set to 0 otherwise. 

## Opcode table
### Symbols
* NNN: address
* NN: 8-bit constant
* N: 4-bit constant
* X and Y: 4-bit register identifier
* PC: Program counter
* I: 12-bit register, for memory address
* VN: One of the 16 avilable variables, from 0 to F

### Table
| OPcode | Type | C Psudocode | Explanation |
| :----: | :-----: | :-----------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| 0NNN | Call | | Calls machine code routine at address NNN |
| 00E0 | Display | disp_clear() | Clears the screen |
| 00EE | Flow | return; | Returns from a subroutine |
| 1NNN | Flow | goto NNN; | Jumps to address NNN |
| 2NNN | Flow | *(0xNNN)() | Calls subroutine at address NNN |
| 3XNN | Cond | if (Vx == NN) | Skips next instruction is VX equals NN |
| 4XNN | Cond | if (Vx != NN) | Skips next instruction if VX does not equal NN |
| 5XY0 | Cond | if (Vx == Vy) | Skips the nxet instruction if VX equals VY |
| 6XNN | Const | Vx = NN | Sets VX to NN |
| 7XNN | Const | Vx += NN | Adds NN to VX (carry flag not changed) |
| 8XY0 | Assig | Vx = Vy | Sets VX to the value of VY. |
| 8XY1 | BitOp | Vx \|= Vy | Sets VX to VX or VY. (bitwise OR operation). |
| 8XY2 | BitOp | Vx &= Vy | Sets VX to VX and VY. (bitwise AND operation). |
| 8XY3 | BitOp | Vx ^= Vy | Sets VX to VX xor VY. |
| 8XY4 | Math | Vx += Vy | Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not. |
| 8XY5 | Math | Vx -= Vy | VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not). |
| 8XY6 | BitOp | Vx >>= 1 | Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF. |
| 8XY7 | Math | Vx = Vy - Vx | Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX). |
| 8XYE | BitOp | Vx <<= 1 | Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset. |
| 9XY0 | Cond | if (Vx != Vy) | Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block). |
| ANNN | MEM | I = NNN | Sets I to the address NNN. |
| BNNN | Flow | PC = V0 + NNN | Jumps to the address NNN plus V0. |
| CXNN | Rand | Vx = rand() & NN | Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. |
| DXYN | Display | draw(Vx, Vy, N) | 	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen |
| EX9E | KeyOp | if (key() == Vx) | Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block). |
| EXA1 | KeyOp | if (key() != Vx) | Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed (usually the next instruction is a jump to skip a code block). |
| FX07 | Timer | Vx = get_delay() | Sets VX to the value of the delay timer. |
| FX0A | KeyOp | Vx = get_key() | A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing). |
| FX15 | Timer | delay_timer(Vx) | Sets the delay timer to VX. |
| FX18 | Sound | sound_timer(Vx) | Sets the sound timer to VX. |
| FX1E | MEM | I += Vx | Adds VX to I. VF is not affected. |
| FX29 | MEM | I = sprite_addr[Vx] | Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font. |
| FX33 | BCD | ```set_BCD(Vx) *(I+0) = BCD(3); *(I+1) = BCD(2); *(I+2) = BCD(1);``` | Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2. |
| FX55 | MEM | reg_dump(Vx, &I) | Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified. |
| FX65 | MEM | reg_load(Vx, &I) | Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified. |


## Font data
| Character | Data | 
| :----: | :-----: |
| 0 | 0xF0, 0x90, 0x90, 0x90, 0xF0 |
| 1 | 0x20, 0x60, 0x20, 0x20, 0x70 |
| 2 | 0xF0, 0x10, 0xF0, 0x80, 0xF0 |
| 3 | 0xF0, 0x10, 0xF0, 0x10, 0xF0 |
| 4 | 0x90, 0x90, 0xF0, 0x10, 0x10 |
| 5 | 0xF0, 0x80, 0xF0, 0x10, 0xF0 |
| 6 | 0xF0, 0x80, 0xF0, 0x90, 0xF0 |
| 7 | 0xF0, 0x10, 0x20, 0x40, 0x40 |
| 8 | 0xF0, 0x90, 0xF0, 0x90, 0xF0 |
| 9 | 0xF0, 0x90, 0xF0, 0x10, 0xF0 |
| A | 0xF0, 0x90, 0xF0, 0x90, 0x90 |
| B | 0xE0, 0x90, 0xE0, 0x90, 0xE0 |
| C | 0xF0, 0x80, 0x80, 0x80, 0xF0 |
| D | 0xE0, 0x90, 0x90, 0x90, 0xE0 |
| E | 0xF0, 0x80, 0xF0, 0x80, 0xF0 |
| F | 0xF0, 0x80, 0xF0, 0x80, 0x80 |

## Keypad format
|  |  |  |  |
| :-: | :-: | :-: | :-: |
| 1 | 2 | 3 | C |
| 4 | 5 | 6 | D |
| 7 | 8 | 9 | E |
| A | 0 | B | F |

##### Author: Dalton Kajander
##### POC: daltonkajander@yahoo.com
