#include "virtual_memory_handler.hpp"

bit c_reg = 0; //carry register, will be 1 if last operation resulted in an ALU carry
bit neg_reg = 0; //negative register, will be 1 if the last operation resulted in a negative ALU result
bit z_reg = 0; //zero register, will be 1 if the last operation resulted in a ALU output of zero
bit b_reg = 0; //bool register, will be 1 if the last operation was true, otherwise, is 0 

byte current_page = 0;

word pc = 0; //65,536 possible values, 0 - 65,535 (inclusive)//
word stack_ptr = 0;



bool close = false;

void tick() {
    pc += 4;
}

class ROM {
    private:
            
    public:
        std::array<byte, 0x10000> MEM{};
        ROM(const string &path) {
            load_memory(path, MEM);
        }

        std::array<byte, 4> focus() {
            return {MEM.at(pc), MEM.at(pc+1), MEM.at(pc+2), MEM.at(pc+3)};
        }
        byte load(word addr) { // returns the value at an address of the RAM
            return MEM.at(addr);
        }

};
class PWPM { //persistant and writable program memory
    private:
            
    public:
        std::array<byte, 0x10000> MEM{};
        PWPM(const string &path) {
            load_memory(path, MEM);
        }

        std::array<byte, 4> focus() {
            return {MEM.at(pc), MEM.at(pc+1), MEM.at(pc+2), MEM.at(pc+3)};
        }

        void store(byte value, word addr) {
            MEM.at(addr) = value;
        }

        byte load(word addr) {
            return MEM.at(addr);
        }
};


class RAM {
    private:
            
    public:
        std::array<byte, 0x10000> MEM{};
        void store(byte value, word addr) {
            MEM.at(addr) = value;
        }

        byte load(word addr) { // returns the value at an address of the RAM
            return MEM.at(addr);
        }
};

void fill_screen(RAM& ram, byte ascii) {
    for (int i = 0xFFFF; i > 0xFCFF; i--) {
        ram.store(ascii, i);
    }
}

screen_buffer screen(std::array<byte,0x10000>& mem) {
    
    screen_buffer fin;

    word pos = 0xFFFF;

   std::stringstream tmp_row;

    for (int y = 0; y < ROWS; y++) {
        tmp_row.str("");
        for (int x = 0; x < COLS; x++) {
            tmp_row << mem.at(pos);
            pos--;
        }
        fin.at(y) = tmp_row.str();
    }

    return fin;
}

void push_stack(RAM& mem, byte value) {
    mem.store(value, stack_ptr);
    stack_ptr++;
}

byte pop_stack(RAM& mem) {
    byte val = mem.load(--stack_ptr);
    mem.store(0, stack_ptr + 1);
    return val;
}

byte ALU(byte A, byte B, byte OP/*im only gonna use the first 4 bits*/) {
    byte result;
    switch (OP){
        //add no carry (ADD)
        case 0x0:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = A + B;
            neg_reg = result & 0x80;
            if (result == 0x0) {
                z_reg = 1;
            }
            return result;
        //add yes carry (ADDC)
        case 0x1:
            b_reg = 0;
            z_reg = 0;
            if ((static_cast<int>(A) + static_cast<int>(B)) > 0xFF) {
                c_reg = 1;
            }
            else {
                c_reg = 0;
            }
            result = A + B;
            neg_reg = result & 0x80;
            if (result == 0x0) {
                z_reg = 1;
            }
            return result;
        //subtract (SUB)
        case 0x2:
            b_reg = 0;
            result = A - B;
            c_reg = 0;
            z_reg = 0;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //XOR
        case 0x3:
            b_reg = 0;
            result = A ^ B;
            c_reg = 0;
            z_reg = 0;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //AND
        case 0x4:
            b_reg = 0;
            result = A & B;
            c_reg = 0;
            z_reg = 0;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //NOT
        case 0x5:
            b_reg = 0;
            result = ~A;
            c_reg = 0;
            z_reg = 0;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //shift left
        case 0x6:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = A << B;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //shift right
        case 0x7:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = A >> B;
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return A >> B;
        //increment no carry
        case 0x8:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = ALU(A,1,0);
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //increment yes carry
        case 0x9:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = ALU(A, 1, 1);
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //decrement
        case 0xa:
            b_reg = 0;
            c_reg = 0;
            z_reg = 0;
            result = ALU(A, 1, 3);
            neg_reg = result & 0x80;
            if (result == 0) {
                z_reg = true;
            }
            return result;
        //greater than or equal to
        case 0xB:
            b_reg = A >= B;
            break;
        //greater than
        case 0xC:
            b_reg = A > B;
            break;
        //less than or equal to
        case 0xD:
            b_reg = A <= B;
            break;
        //less than
        case 0xE:
            b_reg = A < B;
            break;
        //equal
        case 0xF:
            b_reg = A == B;
            break;
        default:
            exit(1);
    }
    return 0x00;
}

class Register {
    private:
        byte storage;
        char name;
    public:
        void save(byte value) {
            storage = value;
        }
        byte load() {
            return storage;
        }
};



class CPU {
    private:
        ROM page0 = ROM("../Memory/pageX.bin");
        ROM page1 = ROM("../Memory/pageY.bin");
        PWPM page2 = PWPM("../Memory/pageZ.bin");
        PWPM page3 = PWPM("../Memory/pageW.bin");
        byte KEYRMSB = 0b00000000;
        byte KEYRLSB =  0;

        std::array<byte, 4> page_focus() {
            switch (current_page) {
                case 0:
                    return page0.focus();
                case 1:
                    return page1.focus();
                case 2:
                    return page2.focus();
                case 3:
                    return page3.focus();
            }
            return page0.focus();
        }
        byte shift = false;
        byte ctrl = false;
        byte down = false;
        byte just_pressed = false;

        void update_KEYR() {
            shift = false;
            ctrl = false;
            down = false;

            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
                ctrl = true;
            }
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                shift = true;
            }
            int key = GetKeyPressed();

            switch (key) {
                case KEY_BACKSPACE:
                    key = 8;
                    break;
                case KEY_TAB:
                    key = 9;
                    break;
                case KEY_ENTER:
                    key = 13;
                    break;
                case KEY_ESCAPE:
                    key = 27;
                    break;
                
                case KEY_UP:
                    key = 24;
                    break;
                case KEY_DOWN:
                    key = 25;
                    break;
                case KEY_RIGHT:
                    key = 26;
                    break;
                case KEY_LEFT:
                    key = 28;
                    break;
            }

            if (key != 0) {
                if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT && key != KEY_LEFT_CONTROL && key!= KEY_RIGHT_CONTROL && key != KEY_LEFT_ALT && key != KEY_RIGHT_ALT){
                    KEYRLSB = key;
                    just_pressed = true;
                }
            }

            for (int i = 0; i < 256; i++) {
                if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT && key != KEY_LEFT_CONTROL && key!= KEY_RIGHT_CONTROL && key != KEY_LEFT_ALT && key != KEY_RIGHT_ALT){
                    down = true;
                }
            }

            KEYRMSB = ((ctrl << 7) | (shift << 6) | (down << 5));
        }

    public:
        std::array<Register, 0x10> reg;
        RAM ram;
        void push_changes() {
            save_memory("../Memory/pageZ.bin", page2.MEM);
            save_memory("../Memory/pageW.bin", page3.MEM);
        }

        void run() {
            update_KEYR();
            byte op = page_focus().at(0);
            byte A = page_focus().at(1);
            byte B = page_focus().at(2);
            byte dest = page_focus().at(3);

            byte res = 0;
            byte x = 0;
            byte y = 0;
            word addr;
            byte page;


            switch (op){
                //ADDi
                case 0x00:
                    res = ALU(A, B, 0x00);
                    reg.at(dest).save(res);
                    break;
                //ADDr
                case 0x01:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0);
                    reg.at(dest).save(res);
                    break;
                //ADDiC
                case 0x02:
                    res = ALU(A, B, 1);
                    reg.at(dest).save(res);
                    break;
                //ADDrC
                case 0x03:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 1);
                    reg.at(dest).save(res);
                    break;
                //SUBi
                case 0x04:
                    res = ALU(A, B, 0x02);
                    reg.at(dest).save(res);
                    break;
                //SUBr
                case 0x05:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0x02);
                    reg.at(dest).save(res);
                    break;
                //XORi
                case 0x06:
                    res = ALU(A, B, 0x3);
                    reg.at(dest).save(res);
                    break;
                //XORr
                case 0x07:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0x3);
                    reg.at(dest).save(res);
                    break;
                //ANDi
                case 0x08:
                    res = ALU(A, B, 0x4);
                    reg.at(dest).save(res);
                    break;
                //ANDr
                case 0x09:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0x4);
                    reg.at(dest).save(res);
                    break;
                //NOTi
                case 0x0A:
                    res = ALU(A, B, 0x5);
                    reg.at(dest).save(res);
                    break;
                //NOTr
                case 0x0B:
                    x = reg.at(A).load();
                    res = ALU(x, 0, 0x5);
                    reg.at(dest).save(res);
                    break;
                //SLi
                case 0x0C:
                    res = ALU(A, B, 0x6);
                    reg.at(dest).save(res);
                    break;
                //SLr
                case 0x0D:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0x6);
                    reg.at(dest).save(res);
                    break;

                //SRi
                case 0x0E:
                    res = ALU(A, B, 0x7);
                    reg.at(dest).save(res);
                    break;
                //SRr
                case 0x0F:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    res = ALU(x, y, 0x7);
                    reg.at(dest).save(res);
                    break;
                //INC
                case 0x10:
                    x = reg.at(A).load();
                    res = ALU(x, 0, 0x8);
                    reg.at(A).save(res);
                    break;
                //INCC
                case 0x11:
                    x = reg.at(A).load();
                    res = ALU(x, 0, 0x9);
                    reg.at(A).save(res);
                    break;
                //DEC
                case 0x12:
                    x = reg.at(A).load();
                    res = ALU(x, 0, 0xa);
                    reg.at(A).save(res);
                    break;
                //GTEi
                case 0x13:
                    ALU(A, B, 0xB);
                    break;
                //GTEr
                case 0x14:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    ALU(x, y, 0xB);
                    break;
                //GNEi
                case 0x15:
                    ALU(A, B, 0xC);
                    break;
                //GNEr
                case 0x16:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    ALU(x, y, 0xC);
                    break;
                //LTEi
                case 0x17:
                    ALU(A, B, 0xD);
                    break;
                //LTEr
                case 0x18:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    ALU(x, y, 0xD);
                    break;
                //LNEi
                case 0x19:
                    ALU(A, B, 0xE);
                    break;
                //LNEr
                case 0x1A:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    ALU(x, y, 0xE);
                    break;
                //CLRr
                case 0x1B:
                    x = reg.at(A).load();
                    fill_screen(ram, x);
                    break;
                //STOi
                case 0x1C:
                    ram.store(A, concat_hex(B,dest));
                    break;
                //STOr
                case 0x1D:
                    ram.store(reg.at(A).load(), (concat_hex(B, dest)));
                    break;
                //LD
                case 0x1E:
                    reg.at(dest).save(ram.load(concat_hex(A, B)));
                    break;
                //JMP
                case 0x1F:
                    current_page = A;
                    pc = concat_hex(B, dest) - 0x04;
                    return;
                    break;
                //JMPZ
                case 0x20:
                    if (z_reg) {
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        return;
                    }
                    break;
                //JMPN
                case 0x21:
                    if (neg_reg) {
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        return;
                    }
                    break;
                //JMPC
                case 0x22:
                    if (c_reg) {
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        return;
                    }
                    break;
                //JNZ
                case 0x23:
                    if (!z_reg) {
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        return;
                    }
                    break;
                //RET
                case 0x24:
                    page = pop_stack(ram);
                    x = pop_stack(ram);
                    y = pop_stack(ram);
                    addr = concat_hex(y, x);
                    current_page = page;
                    pc = addr - 0x04;
                    break;
                //STORr
                case 0x25:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    addr = concat_hex(x,y);
                    ram.store(reg.at(A).load(), addr);
                    break;
                //STORi
                case 0x26:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    addr = concat_hex(x, y);
                    ram.store(A, addr);
                    break;
                //LDR
                case 0x27:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    addr = concat_hex(x,y);
                    reg.at(dest).save(ram.load(addr));
                    break;
                //CLRi
                case 0x28:
                    fill_screen(ram, A);
                    break;
                //PUSHi
                case 0x29:
                    push_stack(ram, A);
                    break;
                //PUSHr
                case 0x2A:
                    x = reg.at(A).load();
                    push_stack(ram, x);
                    break;
                //POP
                case 0x2B:
                    reg.at(dest).save(pop_stack(ram));
                    break;
                //SPTR
                case 0x2C:
                    reg.at(B).save(static_cast<byte>(stack_ptr >> 8));
                    reg.at(dest).save(static_cast<byte>(stack_ptr));
                    break;
                //DRWr
                case 0x2D:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    addr = 0xFFFF - ((32 * y) + x);
                    ram.store(reg.at(dest).load(), addr);
                    break;
                //DRWi
                case 0x2E:
                    addr = 0xFFFF - ((32 * B) + A);
                    ram.store(dest, addr);
                    break;
                //JMPT
                case 0x2F:
                    if (b_reg){
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        break;
                    }
                    break;
                //JMPT
                case 0x30:
                    if (!b_reg){
                        current_page = A;
                        pc = concat_hex(B, dest) - 0x04;
                        break;
                    }
                    break;
                //SPiz
                case 0x31:
                    page2.store(A, concat_hex(B, dest));
                    break;
                //SPiz
                case 0x32:
                    x = reg.at(A).load();
                    page2.store(x, concat_hex(B, dest));
                    break;
                //SPRiz
                case 0x33:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    page2.store(A, concat_hex(x, y));
                    break;
                //SPRrz
                case 0x34:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    page2.store(reg.at(A).load(), concat_hex(x, y));
                    break;
                //SPiw
                case 0x35:
                    page3.store(A, concat_hex(B, dest));
                    break;
                //SPiw
                case 0x36:
                    x = reg.at(A).load();
                    page3.store(x, concat_hex(B, dest));
                    break;
                //SPRiw
                case 0x37:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    page3.store(A, concat_hex(x, y));
                    break;
                //SPRrw
                case 0x38:
                    x = reg.at(B).load();
                    y = reg.at(dest).load();
                    page3.store(reg.at(A).load(), concat_hex(x, y));
                    break;
                //LDx
                case 0x39:
                    reg.at(dest).save(page0.load(concat_hex(A, B)));
                    break;
                //LDRx
                case 0x3A:
                    addr = concat_hex(reg.at(A).load(), reg.at(B).load());
                    reg.at(dest).save(page0.load(addr));
                    break;
                //LDy
                case 0x3B:
                    reg.at(dest).save(page1.load(concat_hex(A, B)));
                    break;
                //LDRy
                case 0x3C:
                    addr = concat_hex(reg.at(A).load(), reg.at(B).load());
                    reg.at(dest).save(page1.load(addr));
                    break;
                //LDz
                case 0x3D:
                    reg.at(dest).save(page2.load(concat_hex(A, B)));
                    break;
                //LDRz
                case 0x3E:
                    addr = concat_hex(reg.at(A).load(), reg.at(B).load());
                    reg.at(dest).save(page2.load(addr));
                    break;
                //LDw
                case 0x3F:
                    reg.at(dest).save(page3.load(concat_hex(A, B)));
                    break;
                //LDRw
                case 0x40:
                    addr = concat_hex(reg.at(A).load(), reg.at(B).load());
                    reg.at(dest).save(page3.load(addr));
                    break;
                //CALL
                case 0x41:
                    addr = pc + 0x4; //next line addr
                    x = deconcat_hex(addr).at(0); // MSB
                    y = deconcat_hex(addr).at(1); //LSB
                    push_stack(ram, x);
                    push_stack(ram, y);
                    push_stack(ram, current_page);
                    pc = concat_hex(B, dest) - 0x04;
                    current_page = A;
                    break;
                //MOVi
                case 0x42:
                    reg.at(dest).save(A);
                    break;
                //MOVr
                case 0x43:
                    x = reg.at(A).load();
                    reg.at(dest).save(x);
                    break;
                //GKEYr
                case 0x44:
                    reg.at(dest).save(KEYRLSB);
                    break;
                //GKEYm
                case 0x45:
                    ram.store(KEYRLSB, concat_hex(B, dest));
                    break;
                //KEYD
                case 0x46:
                    reg.at(dest).save(0);
                    b_reg = false;
                    if (down) {
                        reg.at(dest).save(KEYRLSB);
                        b_reg = true;
                    }
                    break;
                //KEYP
                case 0x47:
                    b_reg = false;
                    if (just_pressed) {
                        reg.at(dest).save(KEYRLSB);
                        b_reg = true;
                        just_pressed = false;
                    }
                    break;
                //SHFT
                case 0x48:
                    b_reg = KEYRMSB & 0x40;
                    break;
                //CTRL
                case 0x49:
                    b_reg = KEYRMSB & 0x80;
                    break;
                //NOP
                case 0x4A:
                    for (int i = 0; i < 1000; i++) {

                    }
                    break;
                //EQi
                case 0x4B:
                    x = reg.at(A).load();
                    ALU(x, B, 0xF);
                    break;
                //EQr
                case 0x4C:
                    x = reg.at(A).load();
                    y = reg.at(B).load();
                    ALU(x, y, 0xF);
                    break;
                //EQm
                case 0x4D:
                    x = reg.at(A).load();
                    y = ram.load(concat_hex(B, dest));
                    ALU(x, y, 0xF);
                    break;
                //HOP
                case 0x4E:
                    if (A == 0) {
                        pc += concat_hex(B, dest);
                    }
                    else {
                        pc -= concat_hex(B, dest);
                    }
                    break;
                //HOPZ
                case 0x4F:
                    if (z_reg){
                        if (A == 0) {
                            pc += concat_hex(B, dest);
                        }
                        else {
                            pc -= concat_hex(B, dest);
                        }
                    }
                    break;
                //HOPZ
                case 0x50:
                    if (neg_reg){
                        if (A == 0) {
                            pc += concat_hex(B, dest);
                        }
                        else {
                            pc -= concat_hex(B, dest);
                        }
                    }
                    break;
                //HOPZ
                case 0x51:
                    if (c_reg){
                        if (A == 0) {
                            pc += concat_hex(B, dest);
                        }
                        else {
                            pc -= concat_hex(B, dest);
                        }
                    }
                    break;
                //HNZ
                case 0x52:
                    if (!z_reg){
                        if (A == 0) {
                            pc += concat_hex(B, dest);
                        }
                        else {
                            pc -= concat_hex(B, dest);
                        }
                    }
                    break;
                //HLT
                case 0xFF:
                    close = true;
                    break;
                default:
                    close = true;
                }
        }
};

int main() {
    InitWindow(790, 725, "CORTEX-8");
    CPU main;

    screen_buffer scr_buff;

    Font mono = LoadFontEx("../src/robotomono.ttf", 32, 0, 0);

    int font_size = 45;

    main.run();
    tick();

    while (!WindowShouldClose() && close == false) {
        ClearBackground(BLACK);
        scr_buff = screen(main.ram.MEM);
        BeginDrawing();

        // print_hex(main.ram.load(0xfffe));

        for (int y = 0; y < ROWS; y++) {
            DrawTextEx(mono, scr_buff.at(y).c_str(), {0, float(y) * (font_size/1.5f) - 8}, font_size, 5, GREEN);
        }

        EndDrawing();

        main.run();
        tick();
        // print_hex(pc);
    }

    main.push_changes();

    UnloadFont(mono);

    CloseWindow();

    return 0;
}