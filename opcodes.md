# Opcode table
## Symbols
* NNN: address
* NN: 8-bit constant
* N: 4-bit constant
* X and Y: 4-bit register identifier
* PC: Program counter
* I: 12-bit register, for memory address
* VN: One of the 16 avilable variables, from 0 to F

## Table
| OPcode |  Type   | C Psudocode         | Explanation                                                                                                                                                                  |
| :----: | :-----: | :-----------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| 0NNN   | Call    |                     | Calls machine code routine at address NNN                                                                                                                                    |
| 00E0   | Display | disp_clear()        | Clears the screen                                                                                                                                                            |
| 00EE   | Flow    | return;             | Returns from a subroutine                                                                                                                                                    |
| 1NNN   | Flow    | goto NNN;           | Jumps to address NNN                                                                                                                                                         |
| 2NNN   | Flow    | *(0xNNN)()          | Calls subroutine at address NNN                                                                                                                                              |
| 3XNN   | Cond    | if (Vx == NN)       | Skips next instruction is VX equals NN                                                                                                                                       |
| 4XNN   | Cond    | if (Vx != NN)       | Skips next instruction if VX does not equal NN                                                                                                                               |
| 5XY0   | Cond    | if (Vx == Vy)       | Skips the nxet instruction if VX equals VY                                                                                                                                   |
| 6XNN   | Const   | Vx = NN             | Sets VX to NN                                                                                                                                                                |
| 7XNN   | Const   | Vx += NN            | Adds NN to VX (carry flag not changed)                                                                                                                                       |
| 8XY0   | Assig   | Vx = Vy             | Sets VX to the value of VY.                                                                                                                                                  |
| 8XY1   | BitOp   | Vx |= Vy            | Sets VX to VX or VY. (bitwise OR operation).                                                                                                                                 |
| 8XY2   | BitOp   | Vx &= Vy            | Sets VX to VX and VY. (bitwise AND operation).                                                                                                                               |
| 8XY3   | BitOp   | Vx ^= Vy            | Sets VX to VX xor VY.                                                                                                                                                        |
| 8XY4   | Math    | Vx += Vy            | Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.                                                                                          |
| 8XY5   | Math    | Vx -= Vy            | VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).                                    |
| 8XY6   | BitOp   | Vx >>= 1            | Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.                                                                         |
| 8XY7   | Math    | Vx = Vy - Vx        | Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).                                                   |
| 8XYE   | BitOp   | Vx <<= 1            | Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.                                        |
| 9XY0   | Cond    | if (Vx != Vy)       | Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block).                                                           |
| ANNN   | MEM     | I = NNN             | Sets I to the address NNN.                                                                                                                                                   |
| BNNN   | Flow    | PC = V0 + NNN       | Jumps to the address NNN plus V0.                                                                                                                                            |
| CXNN   | Rand    | Vx = rand() & NN    | Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.                                                                            |
| DXYN   | Display | draw(Vx, Vy, N)     | See below                                                                                                                                                                    |
| EX9E   | KeyOp   | if (key() == Vx)    | Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block).                |
| EXA1   | KeyOp   | if (key() != Vx)    | Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed (usually the next instruction is a jump to skip a code block).            |
| FX07   | Timer   | Vx = get_delay()    | Sets VX to the value of the delay timer.                                                                                                                                     |
| FX0A   | KeyOp   | Vx = get_key()      | A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).          |
| FX15   | Timer   | delay_timer(Vx)     | Sets the delay timer to VX.                                                                                                                                                  |
| FX18   | Sound   | sound_timer(Vx)     | Sets the sound timer to VX.                                                                                                                                                  |
| FX1E   | MEM     | I += Vx             | Adds VX to I. VF is not affected.                                                                                                                                            |
| FX29   | MEM     | I = sprite_addr[Vx] | Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font.                |
| FX33   | BCD     | See below           | Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.|
| FX55   | MEM     | reg_dump(Vx, &I)    | Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.           |
| FX65   | MEM     | reg_load(Vx, &I)    | Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified. |
