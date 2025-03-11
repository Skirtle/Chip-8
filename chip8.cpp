/* 
Author: Dalton Kajander
POC: daltonkajander@yahoo.com

Source of information: https://en.wikipedia.org/wiki/CHIP-8#Memory, https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
*/
#include <iostream>

#define MAX_MEMORY_BYTES 4096
#define MAX_REGISTERS 16
#define STACK_ADDRESS_COUNT 24
#define TIMER_HZ 60
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

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
    std::cout << "Loading font data into memory\n";

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
    uint16_t starting_memory = 0x050;

    for (int i = 0; i < 16; i++) {
        std::cout << "\t" << sprite_array[i].character << ":";
        for (int j = 0; j < 5; j++) {
            std::cout << " 0x" << std::uppercase << std::hex << +sprite_array[i].pixels[j];
            memory[starting_memory + (5 * i) + j] = sprite_array[i].pixels[j];
        }
        std::cout << "\n";
    }

	std::cout << "Loaded font sprites\n";
}

int main() {
	// Memory: 4096 bytes
	uint8_t *memory_buffer = (uint8_t*) malloc(sizeof(uint8_t) * MAX_MEMORY_BYTES);
	if (!memory_buffer) {
		std::cout << "Failed to reserved " << MAX_MEMORY_BYTES << " bytes for memory. Exitting\n";
		return -1;
	}
	std::cout << "Set " << MAX_MEMORY_BYTES << " bytes for memory\n";

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
	std::cout << "Set " << DISPLAY_WIDTH << "x" << DISPLAY_HEIGHT << " display buffer\n";

	// Registers: 16 8-bit registers, V0-VF
	Register reg16 = Register();
	std::cout << "Set " << MAX_REGISTERS << " bytes for variable registers\n";
	
	// Stack: RCA 1802 version allocates 48 bytes for 12 levels of nesting
	Stack stack = Stack();
	std::cout << "Set " << stack.get_max_address_count() * sizeof(uint16_t) << " bytes for the stack\n";

	// Index register (memory addreses)
	uint16_t I = 0x00;
	std::cout << "Set 2 bytes for variable registers\n";

	// Program counter
	int PC = -1;

	// Start loading data
	load_font_sprites(memory_buffer);

	

	free(memory_buffer);
	for (int i = 0; i < DISPLAY_HEIGHT; i++) free(display_buffer[i]);
	free(display_buffer);
	return 0;
}