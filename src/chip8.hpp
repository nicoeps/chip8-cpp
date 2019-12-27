class Chip8 {
    private:
        uint16_t opcode;
        uint16_t I;
        uint16_t pc;

        uint8_t memory[4096];
        uint8_t V[16];

        uint16_t stack[16];
        uint16_t sp;

    public:
        uint8_t gfx[64*32];
        uint8_t key[17];

        uint8_t delay_timer;
        uint8_t sound_timer;

        void init();
        void load(std::string path);
        void emulateCycle();
};
