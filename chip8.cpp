/* 
Author: Dalton Kajander
POC: daltonkajander@yahoo.com

Source of information: https://en.wikipedia.org/wiki/CHIP-8#Memory, https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
*/
#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

#define MAX_MEMORY_BYTES 4096
#define MAX_REGISTERS 16
#define STACK_ADDRESS_COUNT 24
#define TIMER_HZ 60
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define BLACK " "
#define WHITE "■"
#define MAX_FILE_SIZE 3896
#define DEBUG 0
#define NANOSECONDS_PER_FRAME 1666666
#define SPRITE_MEMORY_LOCATION (uint16_t) 0x050

class Character {
public:
	uint8_t *pixels = nullptr;
	char character;
	Character(char c, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4, uint8_t p5) {
		character = c;
		pixels = (uint8_t*) malloc(sizeof(uint8_t) * 4);
		if (!pixels) {return;}
		pixels[0] = p1;
		pixels[1] = p2;
		pixels[2] = p3;
		pixels[3] = p4;
		pixels[4] = p5;
	}
};

class Register {
public:
	uint8_t v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf;
	Register() {
		this->v0 = 0; this->v1 = 0; this->v2 = 0; this->v3 = 0;
		this->v4 = 0; this->v5 = 0; this->v6 = 0; this->v7 = 0;
		this->v8 = 0; this->v9 = 0; this->va = 0; this->vb = 0;
		this->vc = 0; this->vd = 0; this->ve = 0; this->vf = 0;
	}

	void  set_register(int reg, uint8_t value) {
		switch (reg) {
			case 0x0:
				this->v0 = value;
				break;
			case 0x1:
				this->v1 = value;
				break;
			case 0x2:
				this->v2 = value;
				break;
			case 0x3:
				this->v3 = value;
				break;
			case 0x4:
				this->v4 = value;
				break;
			case 0x5:
				this->v5 = value;
				break;
			case 0x6:
				this->v6 = value;
				break;
			case 0x7:
				this->v7 = value;
				break;
			case 0x8:
				this->v8 = value;
				break;
			case 0x9:
				this->v9 = value;
				break;
			case 0xA:
				this->va = value;
				break;
			case 0xB:
				this->vb = value;
				break;
			case 0xC:
				this->vc = value;
				break;
			case 0xD:
				this->vd = value;
				break;
			case 0xE:
				this->ve = value;
				break;
			case 0xF:
				this->vf = value;
				break;
		}

	}

	uint8_t get_register(int reg) {
		int val = 0;
		switch (reg) {
			case 0x0:
				val = this->v0;
				break;
			case 0x1:
				val = this->v1;
				break;
			case 0x2:
				val = this->v2;
				break;
			case 0x3:
				val = this->v3;
				break;
			case 0x4:
				val = this->v4;
				break;
			case 0x5:
				val = this->v5;
				break;
			case 0x6:
				val = this->v6;
				break;
			case 0x7:
				val = this->v7;
				break;
			case 0x8:
				val = this->v8;
				break;
			case 0x9:
				val = this->v9;
				break;
			case 0xA:
				val = this->va;
				break;
			case 0xB:
				val = this->vb;
				break;
			case 0xC:
				val = this->vc;
				break;
			case 0xD:
				val = this->vd;
				break;
			case 0xE:
				val = this->ve;
				break;
			case 0xF:
				val = this->vf;
				break;
		}
		return val;
	}
};

class Stack {
public:
	Stack() : Stack(STACK_ADDRESS_COUNT) {}

	Stack(int max_address_count) {
		set_max_address_count(max_address_count);
		if (this->max_address_count != 0) this->stack = (uint16_t*) malloc(sizeof(uint16_t) * max_address_count);
		if (!this->stack) { Stack(0); }
		for (int i = 0; i < this->max_address_count; i++) { this->stack[i] = 0; }
	}

	~Stack() {
		if (this->stack != nullptr) {
			free(this->stack);
			this->stack = nullptr;
		}
	}

	void set_max_address_count(int address_count) { this->max_address_count = address_count; }
	int get_max_address_count() { return this->max_address_count; }

	uint16_t pop() {
		if (this->is_empty()) return (uint16_t) NULL;
		this->stack_pointer--;
		return this->stack[this->stack_pointer + 1];
	}
	uint16_t peak() {
		if (this->is_empty()) return (uint16_t) NULL;
		return this->stack[this->stack_pointer];
	}
	bool push(uint16_t value) {
		if (this->is_full()) return false;
		this->stack_pointer++;
		this->stack[this->stack_pointer] = value;
		return true;
	}
	bool is_empty() { return this->stack_pointer < 0; }
	bool is_full() { return (this->stack_pointer == (this->max_address_count - 1)); }

	int max_address_count;
private:
	uint16_t *stack = nullptr;
	int stack_pointer = -1;

};

void load_font_sprites(uint8_t *memory) {
    if (DEBUG) std::cout << "Loading font data into memory\n";

    Character sprite_array[16] = {
        Character('0', 0xF0, 0x90, 0x90, 0x90, 0xF0),
        Character('1', 0x20, 0x60, 0x20, 0x20, 0x70),
        Character('2', 0xF0, 0x10, 0xF0, 0x80, 0xF0),
        Character('3', 0xF0, 0x10, 0xF0, 0x10, 0xF0),
        Character('4', 0x90, 0x90, 0xF0, 0x10, 0x10),
        Character('5', 0xF0, 0x80, 0xF0, 0x10, 0xF0),
        Character('6', 0xF0, 0x80, 0xF0, 0x90, 0xF0),
        Character('7', 0xF0, 0x10, 0x20, 0x40, 0x40),
        Character('8', 0xF0, 0x90, 0xF0, 0x90, 0xF0),
        Character('9', 0xF0, 0x90, 0xF0, 0x10, 0xF0),
        Character('A', 0xF0, 0x90, 0xF0, 0x90, 0x90),
        Character('B', 0xE0, 0x90, 0xE0, 0x90, 0xE0),
        Character('C', 0xF0, 0x80, 0x80, 0x80, 0xF0),
        Character('D', 0xE0, 0x90, 0x90, 0x90, 0xE0),
        Character('E', 0xF0, 0x80, 0xF0, 0x80, 0xF0),
        Character('F', 0xF0, 0x80, 0xF0, 0x80, 0x80)
    };
    uint16_t starting_memory = SPRITE_MEMORY_LOCATION;

    for (int i = 0; i < 16; i++) {
        if (DEBUG) std::cout << "\t" << sprite_array[i].character << ":";
        for (int j = 0; j < 5; j++) {
            if (DEBUG) std::cout << " 0x" << std::uppercase << std::hex << +sprite_array[i].pixels[j];
            memory[starting_memory + (5 * i) + j] = sprite_array[i].pixels[j];
        }
        if (DEBUG) std::cout << "\n";
    }

	if (DEBUG) std::cout << "Loaded font sprites\n";
}

void set_8bit_memory_at_address(uint8_t* mem_buffer, int address, uint8_t value) {
    mem_buffer[0x200 + address] = value;
}

void set_16bit_memory_at_address(uint8_t* mem_buffer, int address, uint16_t value) {
    set_8bit_memory_at_address(mem_buffer, address, (value & 0xFF00) >> 8);
    set_8bit_memory_at_address(mem_buffer, address + 1, value & 0x00FF);
}

void clear_terminal() {
	//std::cout << "Clearing screen\n";
	std::cout << "\033[2J\033[1;1H";
}

void clear_screen(bool** display_buffer) {
	for (int i = 0; i < DISPLAY_HEIGHT; i++) {
		for (int j = 0; j < DISPLAY_WIDTH; j++) {
			display_buffer[i][j] = false;
		}
	}
}

void print_display(bool** display_buffer) {
    for (int i = 0; i < DISPLAY_HEIGHT; ++i) {
        for (int j = 0; j < DISPLAY_WIDTH; ++j) {
            std::cout << (display_buffer[i][j] ? WHITE : BLACK);
        }
        std::cout << "\n";
    }
}

void draw(bool **display_buffer, uint8_t *memory_buffer, Register *reg, uint8_t vx, uint8_t vy, uint8_t n, uint16_t I) {
    for (int row = 0; row < n; ++row) {
        uint8_t sprite_row = memory_buffer[I + row];

        for (int col = 0; col < 8; ++col) {
            bool pixel = (sprite_row >> (7 - col)) & 1;

            int x = (vx + col) % DISPLAY_WIDTH;
            int y = (vy + row) % DISPLAY_HEIGHT;

            if (pixel) { 
				display_buffer[y][x] ^= true; 
				if (display_buffer[y][x]) reg->set_register(0xF, 0x0001);
			}
        }
    }
}

void load_rom(std::string filename, uint8_t *memory) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cout << "Could not open file " << filename << "\n";
		return;
	}

	int index = 0;
    uint8_t byte;

    while (file.read(reinterpret_cast<char*>(&byte), 1)) {  // Read 1 byte at a time
        set_8bit_memory_at_address(memory, index, byte);
		index++;

        if (index >= MAX_FILE_SIZE) break;
	}

    if (DEBUG)  std::cout << "Finished reading the file.\n";

    file.close();

}

int main() {
	// Memory: 4096 bytes
	uint8_t *memory_buffer = (uint8_t*) malloc(sizeof(uint8_t) * MAX_MEMORY_BYTES);
	if (!memory_buffer) {
		std::cout << "Failed to reserved " << MAX_MEMORY_BYTES << " bytes for memory. Exitting\n";
		return -1;
	}
	if (DEBUG) std::cout << "Set " << MAX_MEMORY_BYTES << " bytes for memory\n";

	// Display memory
	bool **display_buffer = (bool**) malloc(sizeof(bool*) * DISPLAY_HEIGHT);
	if (display_buffer == NULL) {
		std::cout << "Failed to reserve display buffer memory. Exitting.\n";
		free(memory_buffer);
		
		return -1;
	}
	for (int i = 0; i < DISPLAY_HEIGHT; i++)  {
		display_buffer[i] = (bool*) malloc(sizeof(bool) * DISPLAY_WIDTH);
		if (display_buffer[i] == NULL) {
			std::cout << "Failed to reserve display buffer memory. Exitting.\n";
			free(memory_buffer);
			
			for (i = i - 1; i > 0; i--) {
				free(display_buffer[i]);
			}
			free(display_buffer);
			return -1;
		}
	}
	for (int i = 0; i < DISPLAY_HEIGHT; i++) {
		for (int j = 0; j < DISPLAY_WIDTH; j++) {
			display_buffer[i][j] = false;  // Set all pixels to black initially
		}
	}
	if (DEBUG) std::cout << "Set " << DISPLAY_WIDTH << "x" << DISPLAY_HEIGHT << " display buffer\n";

	// Registers: 16 8-bit registers, V0-VF
	Register reg16 = Register();
	if (DEBUG) std::cout << "Set " << MAX_REGISTERS << " bytes for variable registers\n";
	
	// Stack: RCA 1802 version allocates 48 bytes for 12 levels of nesting
	Stack stack = Stack();
	if (DEBUG) std::cout << "Set " << stack.get_max_address_count() * sizeof(uint16_t) << " bytes for the stack\n";

	// Index register (memory addreses)
	uint16_t I = 0x200;
	if (DEBUG) std::cout << "Set 2 bytes for variable registers\n";

	// Program counter
	int PC = 0;

	// Start loading data
	load_font_sprites(memory_buffer);

	// Load ROM
	load_rom("2-ibm-logo.ch8", memory_buffer);

	// Random number between 0 and 255
	std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0x00, 0xFF);

	
	bool halt_flag = false;
	clear_terminal();
	while (!halt_flag) {
		// Fetch
		// Instructions are 2 bytes each, so we need to combine the two
		uint16_t instruction = (memory_buffer[0x200 + PC] << 8) + memory_buffer[0x200 + PC + 1];
		if (DEBUG) std::cout << "Instruction: 0x" << std::hex << instruction << ", PC = " << PC << "\n";
		PC += 2;

		// Decode
		uint16_t NNN = instruction & 0x0FFF;
		uint16_t NN = instruction & 0x00FF;
		uint16_t N = instruction & 0x000F;
		uint16_t X = (instruction & 0x0F00) >> 8;
		uint16_t Y = (instruction & 0x00F0) >> 4;

		bool key_pressed = false;
		switch ((instruction & 0xF000) >> 12) {
			case 0x0:
				if (instruction == 0x00E0) { clear_screen(display_buffer); } // Clear the display
				else if (instruction == 0x00EE) { PC = stack.pop(); } // Return
				//else if (instruction == 0x0000) { halt_flag = true; }
				break;
			case 0x1:
				// 1NNN, jump to address NNN
				PC = NNN;
				if (DEBUG) std::cout << "\tJumping to address " << +PC << "\n";
				break;
			case 0x2:
				// 2NNN, call subroutine at address NNN and push current PC to stack
				stack.push(PC);
				PC = NNN;
				break;
			case 0x3:
				// 3XNN, if vx == NN: skip next instruction
				if (reg16.get_register(X) == (0x00FF & instruction)) PC += 2;
				break;
			case 0x4:
				// 4XNN, if vx != NN: skip next instruction
				if (reg16.get_register(X) != (0x00FF & instruction)) PC += 2;
				break;
			case 0x5:
				// 5XY0, if vx != NN: skip next instruction
				if (reg16.get_register(X) == reg16.get_register(Y)) PC += 2;
				break;
			case 0x6:
				// 6XNN, set VX to NN
				reg16.set_register(X, NN);
				if (DEBUG) std::cout << "\tSetting V" << X << " to " << NN << "\n";
				break;
			case 0x7:
				reg16.set_register(X, NN + reg16.get_register(X));
				if (DEBUG) std::cout << "\tAdding " << NN << " to V" << X << "\n";
				break;
			case 0x8: // Math
				if (N == 0) { reg16.set_register(X, reg16.get_register(Y)); } //  VX = VY
				else if (N == 0x1) { reg16.set_register(X, reg16.get_register(Y) | reg16.get_register(X)); } // VX |= VY, OR
				else if (N == 0x2) { reg16.set_register(X, reg16.get_register(Y) & reg16.get_register(X)); } // VX &= VY, AND
				else if (N == 0x3) { reg16.set_register(X, reg16.get_register(Y) ^ reg16.get_register(X)); } // VX ^= VY, XOR
				else if (N == 0x4) { // VX += VY. VF = 1 if overflow
					reg16.set_register(0xF, (int) reg16.get_register(X) + (int) reg16.get_register(Y) > 255);
					reg16.set_register(X, reg16.get_register(X) + reg16.get_register(Y));
				} 
				else if (N == 0x5) { // VX -= VY. VF = 1 if VX >= VY
					reg16.set_register(0xF, reg16.get_register(X) >= reg16.get_register(Y));
					reg16.set_register(X, reg16.get_register(X) - reg16.get_register(Y)); 
				} 
				else if (N == 0x6) { // VX >>= 1. VF = LSB of VX prior to shift
					reg16.set_register(0xF, 0x0001 & reg16.get_register(X));
					reg16.set_register(X, reg16.get_register(X) >> 1);
				} 
				else if (N == 0x7) { // VX = VY - VX. VF = 1 if VY >= VX
					reg16.set_register(0xF, reg16.get_register(Y) >= reg16.get_register(X));
					reg16.set_register(X,  reg16.get_register(Y) - reg16.get_register(X));
				} 
				else if (N == 0xE) { // VX <<= 1. VF = MSB of VX prior to shift
					reg16.set_register(0xF, 0x8000 & reg16.get_register(X));
					reg16.set_register(X, reg16.get_register(X) << 1);
				} 
				break;
			case 0x9:
				// 9XY0, skip next if VX != VY
				if (reg16.get_register(X) != reg16.get_register(Y)) PC += 2;
				break;
			case 0xA:
				I = NNN;
				if (DEBUG) std::cout << "\tSetting I to " << +I << "\n";
				break;
			case 0xB:
				// BNNN, jump to NNN + V0
				PC = (NNN) + reg16.get_register(0);
				break;
			case 0xC:
				// CXNN, set VX to the bitwise AND of a random number (0x00 - 0xFF) and NN
				reg16.set_register(X, distrib(gen) & 0x00FF & instruction);
				break;
			case 0xD:
				// DXYN, draw at X,Y with height of N
				if (DEBUG) std::cout << "\tDrawing to display at " << +reg16.get_register(X) << ", " << +reg16.get_register(Y) << "\n";
				draw(display_buffer, memory_buffer, &reg16, reg16.get_register(X), reg16.get_register(Y), N, I);
				clear_terminal();
				print_display(display_buffer);
				break;
			case 0xE: // Keys
				if (NN == 0x9E && ((reg16.get_register(X) & 0x000F) == key_pressed)) PC += 2; // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
				else if (NN == 0xA1 && ((reg16.get_register(X) & 0x000F) != key_pressed)) PC += 2; // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed
				break;
			case 0xF:
				break;
		}

		std::this_thread::sleep_for(std::chrono::nanoseconds(NANOSECONDS_PER_FRAME));

		
	}

	free(memory_buffer);
	for (int i = 0; i < DISPLAY_HEIGHT; i++) free(display_buffer[i]);
	free(display_buffer);
	return 0;
}