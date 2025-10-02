/*
**EXAMPLE PROGRAM** (in asm)
ADDi 100 50 X ; add 100 and 50 and store the result in in register X
SROr X 0x0000 ; store the contents of register X into memory address 0x0000
HLT
**EXAMPLE PROGRAM** (in hex machine code)
00 64 32 00 
11 00 00 00 
FF 00 00 00
*/


#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

typedef uint8_t byte;
typedef uint16_t word;
typedef bool bit;
typedef std::string string;

bit c_reg = 0; //carry register, will be 1 if last operation resulted in an ALU carry
bit neg_reg = 0; //negative register, will be 1 if the last operation resulted in a negative ALU result
bit z_reg = 0; //zero register, will be 1 if the last operation resulted in a ALU output of zero
bit b_reg = 0; //bool register, will be 1 if the last operation was true, otherwise, is 0 

byte current_page = 0;

word pc = 0; //65,536 possible values, 0 - 65,535 (inclusive)//

void print_hex(byte number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}
void print_hex(word number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}
void print_hex(bool number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}


word concat_hex(byte a, byte b) {
    return (static_cast<word>(a) << 8) | b;
}

void tick() {
    pc += 4;
}

class ROM {
    private:
        std::array<byte, 0x10000> MEM{};
            
    public:
        ROM(std::vector<byte> arr) {
            for (int i = 0; i < arr.size(); i++) {
                MEM.at(i) = arr.at(i);
            }
        }

        std::array<byte, 4> focus() {
            return {MEM.at(pc), MEM.at(pc+1), MEM.at(pc+2), MEM.at(pc+3)};
        }

};
class PWPM { //persistant and writable program memory
    private:
        std::array<byte, 0x10000> MEM{};
            
    public:
        PWPM(std::vector<byte> arr) {
            for (int i = 0; i < arr.size(); i++) {
                MEM.at(i) = arr.at(i);
            }
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

void init_screen(RAM& ram, byte ascii) {
    for (int i = 0xFFFF; i > 0xFCFF; i--) {
        ram.store(ascii, i);
    }
}

string screen(std::array<byte,0x10000>& mem) {
    std::stringstream out;
    const int cols = 32;
    const int rows = 24;
    int count = cols * rows; // 768
    int idx = 0;
    for (int i = 0xFFFF; i >= 0xFCFF; i--) {
        out << static_cast<char>(mem.at(i)) << ' ';
        if (mem.at(i) == 0) {
            out << "  ";
        }
        idx++;
        if ((idx % cols) == 0 && idx != count) out << '\n';
    }
    return out.str();
}

void print_scr(std::array<byte, 0x10000>& mem) {

    std::string scr = screen(mem);

    std::cout << scr;
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
        ROM page0 = ROM({
            0x1C, 0x30, 0xFF, 0xFF,
            0x1C, 0x30, 0xFC, 0xFF,
            0xFF, 0x00, 0x00, 0x00
        });
        ROM page1 = ROM({});
        PWPM page2 = PWPM({});
        PWPM page3 = PWPM({});

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

    public:
        std::array<Register, 0x10> reg;
        RAM ram;
        void run() {
            while (true) {    
                byte op = page_focus().at(0);
                byte A = page_focus().at(1);
                byte B = page_focus().at(2);
                byte dest = page_focus().at(3);

                byte res = 0;
                byte x = 0;
                byte y = 0;

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
                        init_screen(ram, x);
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
                    //JMPx
                    case 0x1F:
                        current_page = 0;
                        pc = concat_hex(B, dest);
                        continue;
                        break;
                    //JMPZx
                    case 0x20:
                        if (z_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPNx
                    case 0x21:
                        if (neg_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPCx
                    case 0x22:
                        if (c_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JNZx
                    case 0x23:
                        if (!z_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    
                    //JMPy
                    case 0x2F:
                        current_page = 1;
                        pc = concat_hex(B, dest);
                        continue;
                        break;
                    //JMPZy
                    case 0x30:
                        if (z_reg) {
                            current_page = 1;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPNy
                    case 0x31:
                        if (neg_reg) {
                            current_page = 1;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPCy
                    case 0x32:
                        if (c_reg) {
                            current_page = 1;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JNZy
                    case 0x33:
                        if (!z_reg) {
                            current_page = 1;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPTx
                    case 0x39:
                        if (b_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPFx
                    case 0x3A:
                        if (!b_reg) {
                            current_page = 0;
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;   break;
                    //JMP
                    case 0x4D:
                        pc = concat_hex(B, dest);
                        continue;
                        break;
                    //JMPZ
                    case 0x4E:
                        if (z_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPN
                    case 0x4F:
                        if (neg_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPC
                    case 0x50:
                        if (c_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JNZ
                    case 0x51:
                        if (!z_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPT
                    case 0x52:
                        if (b_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //JMPF
                    case 0x53:
                        if (!b_reg) {
                            pc = concat_hex(B, dest);
                            continue;
                        }
                        break;
                    //HLT
                    case 0xFF:
                        return;
                    default:
                        return;
                }
                tick();
                system("cls");
                print_scr(ram.MEM);
            }
        }
};

int main() {
    CPU main;
    main.run();


    return 0;
}