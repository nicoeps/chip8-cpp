#include <fstream>
#include <cstring>

#include "chip8.hpp"

void Chip8::init() {
    srand(time(NULL));

    opcode = 0;
    I = 0;
    pc = 0x200;
    sp = 0;

    memset(gfx, 0, sizeof(gfx));
    memset(stack, 0, sizeof(stack));
    memset(V, 0, sizeof(V));
    memset(memory, 0 ,sizeof(memory));
    memset(key, 0, sizeof(key));
    key[0x10] = 0x10;
    
    const uint8_t chip8_font[80] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (uint8_t i = 0; i < 80; ++i) {
        memory[i] = chip8_font[i];
    }
}

void Chip8::load(std::string path) {
    std::ifstream rom;
    rom.open(path, std::ifstream::binary);
    
    if (rom) {
        rom.seekg(0, rom.end);
        int buffer_size = rom.tellg();
        rom.seekg(0, rom.beg);

        char* buffer = new char [buffer_size];

        rom.read(buffer, buffer_size);
        rom.close();

        for (int i=0; i < buffer_size; ++i) {
            memory[i + 0x200] = buffer[i];
        }

        delete[] buffer;
    }
}

void Chip8::emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    uint8_t op = (opcode & 0xF000) >> 12;
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;

    uint8_t random_byte;
    uint16_t pixel;
    uint16_t pos;
    uint16_t xx;
    uint16_t yy;

    switch (op) {
        case 0x0:
            switch (n) {
                // 00E0: Clear the display
                case 0x0:
                    memset(gfx, 0, sizeof(gfx));
                    break;

                // 00EE: Return from a subroutine
                case 0xE:
                    pc = stack[--sp];
                    break;
            }
            break;

        // 1nnn: Jump to location nnn 
        case 0x1:
            pc = nnn - 2;
            break;

        // 2nnn: Call subroutine at nnn
        case 0x2:
            stack[sp++] = pc;
            pc = nnn - 2;
            break;

        // 3xkk: Skip next instruction if Vx = kk
        case 0x3:
            if (V[x] == kk) {
                pc += 2;
            }
            break;

        // 4xkk: Skip next instruction if Vx != kk
        case 0x4:
            if (V[x] != kk) {
                pc += 2;
            }
            break;

        // 5xy0: Skip next instruction if Vx = Vy
        case 0x5:
            if (V[x] == V[y]) {
                pc += 2;
            }
            break;

        // 6xkk: Set x = kk
        case 0x6:
            V[x] = kk;
            break;

        // 7xkk: Set Vx = Vx + kk
        case 0x7:
            V[x] += kk;
            break;
        
        case 0x8:
            switch (n) {
                // 8xy0: Set Vx = Vy
                case 0x0:
                    V[x] = V[y];
                    break;

                // 8xy1: Set Vx = Vx OR Vy
                case 0x1:
                    V[x] = V[x] | V[y];
                    break;

                // 8xy2: Set Vx = Vx AND Vy
                case 0x2:
                    V[x] = V[x] & V[y];
                    break;

                // 8xy3: Set Vx = Vx XOR Vy
                case 0x3:
                    V[x] = V[x] ^ V[y];
                    break;

                // 8xy4: Set Vx = Vx + Vy, set VF = carry
                case 0x4:
                    V[0xF] = V[y] > (0xFF - V[x]);
                    V[x] += V[y];
                    break;

                // 8xy5: Set Vx = Vx - Vy, set VF = NOT borrow
                case 0x5:
                    V[0xF] = V[x] >= V[y];
                    V[x] -= V[y];
                    break;

                // 8xy6: Set Vx = Vx SHR 1
                case 0x6:
                    V[0xF] = V[x] & 0x0001;
                    V[x] >>= 1;
                    break;

                // 8xy7: Set Vx = Vy - Vx, set VF = NOT borrow
                case 0x7:
                    V[0xF] = V[y] >= V[x];
                    V[x] = V[y] - V[x];
                    break;

                // 8xyE: Set Vx = Vx SHL 1
                case 0xE:
                    V[0xF] = V[x] >> 7;
                    V[x] <<= 1;
                    break;
            }
            break;

        // 9xy0: Skip next instruction if Vx != Vy
        case 0x9:
            if (V[x] != V[y]) {
                pc += 2;
            }
            break;

        // Annn: Set I = nnn
        case 0xA:
            I = nnn;
            break;

        // Bnnn: Jump to location nnn + V0
        case 0xB:
            pc = nnn + V[0x0] - 2;
            break;
            
        // Cxkk: Set Vx = random byte AND kk
        case 0xC:
            random_byte = rand() % 256;
            V[x] = random_byte & kk;
            break;

        // Dxyn: Display n-byte sprite starting at memory location I
        // at (Vx, Vy), set VF = collision
        case 0xD:
            xx = V[x];
            yy = V[y];

            V[0xF] = 0;
            for (uint8_t y_index = 0; y_index < n; ++y_index) {
                pixel = memory[I + y_index];

                for (uint8_t x_index = 0; x_index < 8; ++x_index) {
                    pos = (xx + x_index) % 64 + ((yy + y_index) % 32) * 64;
                    if ((pixel & (0x0080 >> x_index)) != 0) {
                        if (gfx[pos] == 0xFF) {
                            V[0xF] = 1;
                        }
                        gfx[pos] ^= 0xFF;
                    }
                }
            }
            break;
        
        case 0xE:
            switch (kk) {
                // Ex9E: Skip next instruction if
                // key with the value of Vx is pressed
                case 0x9E:
                    if (key[V[x]] != 0) {
                        pc += 2;
                    }
                    break;

                // ExA1: Skip next instruction if
                // key with the value of Vx is not pressed
                case 0xA1:
                    if (key[V[x]] == 0) {
                        pc += 2;
                    }
                    break;
            }
            break;
        
        case 0xF:
            switch (kk) {
                // Fx07: Set Vx = delay timer value
                case 0x07:
                    V[x] = delay_timer;
                    break;

                // Fx0A: Wait for a key press, store the value of the key in Vx
                case 0x0A:
                    if (key[0x10] < 0x10) {
                        V[x] = key[0x10];
                    } else {
                        pc -= 2;
                    }
                    break;

                // Fx15: Set delay timer = Vx
                case 0x15:
                    delay_timer = V[x];
                    break;

                // Fx18: Set sound timer = Vx
                case 0x18:
                    sound_timer = V[x];
                    break;

                // Fx1E: Set I = I + Vx
                case 0x1E:
                    I += V[x];
                    break;

                // Fx29: Set I = location of sprite for digit Vx
                case 0x29:
                    I = (uint16_t) V[x] * 5;
                    break;

                // Fx33: Store BCD representation of Vx
                // in memory locations I, I+1, and I+2
                case 0x33:
                    memory[I] = V[x] / 100;
                    memory[I+1] = (V[x] / 10) % 10;
                    memory[I+2] = (V[x] % 100) % 10;
                    break;

                // Fx55: Store registers V0 through Vx
                // in memory starting at location I
                case 0x55:
                    for (uint8_t i = 0; i <= x; ++i) {
                        memory[I+i] = V[i];
                    }
                    break;

                // Fx65: Read registers V0 through Vx
                // from memory starting at location I
                case 0x65:
                    for (uint8_t i = 0; i <= x; ++i) {
                        V[i] = memory[I+i];
                    }
                    break;
            }
            break;
    }
    pc += 2;
}
