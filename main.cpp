#include <iostream>      
#include <fstream>       
#include <sstream>       // For stringstream (used in disassembly)
#include <cstdint>       // For fixed-width integer types (uint16_t, uint8_t)
#include <cstring>       
#include <array>         
#include <stdexcept>     
#include <cstdlib>       
#include <iomanip>       
using namespace std;

// Define total memory size as 64KB.
static const size_t MEM_SIZE = 65536;

class Z16Simulator {
public:
    // 64KB memory (overridden within the class)
    static const size_t MEM_SIZE = 65536;
    
    // Array of 8 registers (16-bit each). The registers are indexed 0 to 7.
    array<uint16_t, 8> regs;
    
    // Program counter (16-bit) holds the current address in memory.
    uint16_t pc;
    
    // Memory array representing the entire 64KB.
    array<uint8_t, MEM_SIZE> memory;
    
    // Total number of bytes loaded into memory (program size).
    size_t programSize;
    
    // Register ABI names for display (used for disassembly and debugging).
    const array<string, 8> regNames = { "t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1" };

    // Constructor initializes registers, program counter, and memory.
    // Note: sp (reg index 2) is initialized to point near the end of memory.
    Z16Simulator() : pc(0), programSize(0) {
        regs.fill(0);            // Set all registers to 0.
        regs[2] = MEM_SIZE - 2;  // Initialize sp register to top of memory (minus 2).
        memory.fill(0);          // Clear all memory bytes.
    }


    // Read a byte from memory at the given address.
    uint8_t readByte(uint16_t addr) {
        if (addr >= MEM_SIZE)
            throw runtime_error("Memory read error: address out of bounds");
        return memory[addr];
    }

void showmem(ostream &out) {
    out << "\nUsed Memory Listing (only non-zero cells):\n";
    bool foundAny = false;
    // Go through the entire memory.
    for (size_t addr = 0; addr < MEM_SIZE; ++addr) {
        // Only output memory locations that have a nonzero value.
        if (memory[addr] != 0) {
            out << "Addr 0x" << setw(4) << setfill('0') << hex << addr
                << " : 0x" << setw(2) << setfill('0') << hex << (int)memory[addr] << "\n";
            foundAny = true;
        }
    }
    // In case no memory cells were used.
    if (!foundAny) {
        out << "No used memory addresses found.\n";
    }
}


    // Read a 16-bit word from memory (using little-endian order).
    uint16_t readWord(uint16_t addr) {
        if (addr + 1 >= MEM_SIZE)
            throw runtime_error("Memory read error: address out of bounds");
        // Combine two bytes: low byte at addr, high byte at addr+1.
        return memory[addr] | (memory[addr + 1] << 8);
    }

    // Write a byte to memory at the given address.
    void writeByte(uint16_t addr, uint8_t value) {
        if (addr >= MEM_SIZE)
            throw runtime_error("Memory write error: address out of bounds");
        memory[addr] = value;
    }

    // Write a 16-bit word to memory in little-endian order.
    void writeWord(uint16_t addr, uint16_t value) {
        if (addr + 1 >= MEM_SIZE)
            throw runtime_error("Memory write error: address out of bounds");
        memory[addr] = value & 0xFF;             // Lower 8 bits.
        memory[addr + 1] = (value >> 8) & 0xFF;    // Upper 8 bits.
    }


    // -----------------------------------------------------------------------
    // Disassemble a 16-bit instruction into a human-readable assembly string.
    // 'addr' is the current address (used for branch targets).
    // -----------------------------------------------------------------------
    string disassemble(uint16_t addr, uint16_t inst) {
        // Extract the opcode from the lowest 3 bits.
        uint8_t opcode = inst & 0x7;
        stringstream ss;

        // Decode based on opcode.
        switch(opcode) {
            case 0x0: { // R-type instructions.
                // Extract function and register fields.
                uint8_t funct4 = (inst >> 12) & 0xF;
                uint8_t rs2 = (inst >> 9) & 0x7;
                uint8_t rd_rs1 = (inst >> 6) & 0x7;
                uint8_t funct3 = (inst >> 3) & 0x7;
                // Determine the specific instruction based on funct4 and funct3.
                if (funct4 == 0b0000 && funct3 == 0b000)
                    ss << "add " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0001 && funct3 == 0b000)
                    ss << "sub " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0000 && funct3 == 0b001)
                    ss << "slt " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0000 && funct3 == 0b010)
                    ss << "sltu " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0010 && funct3 == 0b011)
                    ss << "sll " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0100 && funct3 == 0b011)
                    ss << "srl " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b1000 && funct3 == 0b011)
                    ss << "sra " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0001 && funct3 == 0b100)
                    ss << "or " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0000 && funct3 == 0b101)
                    ss << "and " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0000 && funct3 == 0b110)
                    ss << "xor " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0000 && funct3 == 0b111)
                    ss << "mv " << regNames[rd_rs1] << ", " << regNames[rs2];
                else if (funct4 == 0b0100 && funct3 == 0b000)
                    ss << "jr " << regNames[rd_rs1];
                else if (funct4 == 0b1000 && funct3 == 0b000)
                    ss << "jalr " << regNames[rs2];
                else
                    ss << "Unknown R-type instruction";
                break;
            }
            case 0x1: { // I-type instructions.
                uint8_t imm7 = (inst >> 9) & 0x7F;   // 7-bit immediate field.
                uint8_t imm3 = (inst >> 13) & 0x7;     // Additional bits for shift instructions.
                uint8_t rd_rs1 = (inst >> 6) & 0x7;      // Destination/source register.
                uint8_t funct3 = (inst >> 3) & 0x7;      // Function code.
                // Sign-extend the immediate field if needed.
                int16_t simm = (imm7 & 0x40) ? (imm7 | 0xFF80) : imm7;
                if (funct3 == 0b000)
                    ss << "addi " << regNames[rd_rs1] << ", " << simm;
                else if (funct3 == 0b001)
                    ss << "slti " << regNames[rd_rs1] << ", " << simm;
                else if (funct3 == 0b010)
                    ss << "sltui " << regNames[rd_rs1] << ", " << imm7;
                else if (funct3 == 0b011 && imm3 == 0b001)
                    ss << "slli " << regNames[rd_rs1] << ", " << (imm7 & 0xF);
                else if (funct3 == 0b011 && imm3 == 0b010)
                    ss << "srli " << regNames[rd_rs1] << ", " << (imm7 & 0xF);
                else if (funct3 == 0b011 && imm3 == 0b100)
                    ss << "srai " << regNames[rd_rs1] << ", " << (imm7 & 0xF);
                else if (funct3 == 0b100)
                    ss << "ori " << regNames[rd_rs1] << ", " << simm;
                else if (funct3 == 0b101)
                    ss << "andi " << regNames[rd_rs1] << ", " << simm;
                else if (funct3 == 0b110)
                    ss << "xori " << regNames[rd_rs1] << ", " << simm;
                else if (funct3 == 0b111)
                    ss << "li " << regNames[rd_rs1] << ", " << simm;
                else
                    ss << "Unknown I-type instruction";
                break;
            }
            case 0x2: { // B-type (branch) instructions.
                // Extract a 4-bit offset and sign-extend it to 8 bits.
                int8_t raw_offset = (inst >> 12) & 0xF;
                if (raw_offset & 0x8)
                    raw_offset |= 0xF0;
                // Extract registers used in branch.
                uint8_t rs2 = (inst >> 9) & 0x7;
                uint8_t rs1 = (inst >> 6) & 0x7;
                uint8_t funct3 = (inst >> 3) & 0x7;
                // Calculate branch target address.
                uint16_t target = addr + 2 + (raw_offset * 2);
                switch(funct3) {
                    case 0b000: // BEQ
                        ss << "beq " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b001: // BNE
                        ss << "bne " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b010: // BZ (branch if register rs1 is zero)
                        target -= 2;
                        ss << "bz " << regNames[rs1] << ", 0x"
                           << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b011: // BNZ (branch if register rs1 is nonzero)
                        target -= 2;
                        ss << "bnz " << regNames[rs1] << ", 0x"
                           << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b100: // BLT (branch if less than, signed)
                        ss << "blt " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b101: // BGE (branch if greater than or equal, signed)
                        ss << "bge " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b110: // BLTU (branch if less than, unsigned)
                        ss << "bltu " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    case 0b111: // BGEU (branch if greater than or equal, unsigned)
                        ss << "bgeu " << regNames[rs1] << ", " << regNames[rs2]
                           << ", 0x" << setw(4) << setfill('0') << hex << target;
                        break;
                    default:
                        ss << "Unimplemented B-type instruction";
                        break;
                }
                break;
            }
            case 0x3: { // S-type (store) instructions.
                // Extract 4-bit offset.
                int8_t offset = (inst >> 12) & 0xF;
                // For store instructions, the register fields are swapped:
                // Base register comes from bits [8:6] and value register from bits [11:9].
                uint8_t rs1 = (inst >> 6) & 0x7;  // Base register (address)
                uint8_t rs2 = (inst >> 9) & 0x7;  // Source register (value)
                uint8_t funct3 = (inst >> 3) & 0x7;
                // Format output as store instruction with offset addressing.
                if (funct3 == 0b000)
                    ss << "sb " << regNames[rs2] << ", " << static_cast<int>(offset)
                       << "(" << regNames[rs1] << ")";
                else if (funct3 == 0b001)
                    ss << "sw " << regNames[rs2] << ", " << static_cast<int>(offset)
                       << "(" << regNames[rs1] << ")";
                else
                    ss << "Unknown S-type instruction";
                break;
            }
            case 0x4: { // L-type (load) instructions.
                int8_t offset = (inst >> 12) & 0xF;
                uint8_t rs2 = (inst >> 9) & 0x7;   // Base register for address.
                uint8_t rd = (inst >> 6) & 0x7;      // Destination register.
                uint8_t funct3 = (inst >> 3) & 0x7;
                if (funct3 == 0b000)
                    ss << "lb " << regNames[rd] << ", " << static_cast<int>(offset)
                       << "(" << regNames[rs2] << ")";
                else if (funct3 == 0b001)
                    ss << "lw " << regNames[rd] << ", " << static_cast<int>(offset)
                       << "(" << regNames[rs2] << ")";
                else if (funct3 == 0b100)
                    ss << "lbu " << regNames[rd] << ", " << static_cast<int>(offset)
                       << "(" << regNames[rs2] << ")";
                else
                    ss << "Unknown L-type instruction";
                break;
            }
            case 0x5: { // J-type (jump) instructions.
                // f: flag bit (determines type: jump vs. jal)
                uint8_t f    = (inst >> 15) & 0x1; 
                // imm6: bits 14:9, imm3: bits 5:3
                uint8_t imm6 = (inst >> 9)  & 0x3F;
                uint8_t rd   = (inst >> 6)  & 0x7;  // For JAL, this is the destination register.
                uint8_t imm3 = (inst >> 3)  & 0x7;
                // Combine immediate parts into a 9-bit immediate.
                int16_t rawImm = (imm6 << 3) | imm3;
                // Sign-extend if necessary (if bit 8 is set).
                if (rawImm & (1 << 8)) {
                    rawImm |= 0xFE00; // Set upper bits.
                }
                // Calculate final target address (PC-relative, multiplied by 2).
                uint16_t target = addr + (rawImm * 2);
                if (f == 0) {
                    ss << "j 0x" << hex << setw(4) << setfill('0') << target;
                } else {
                    ss << "jal " << regNames[rd]
                       << ", 0x" << hex << setw(4) << setfill('0') << target;
                }
                break;
            }
            case 0x6: { // U-type (lui/auipc) instructions.
                // Extract flag and immediate fields.
                uint8_t f         = (inst >> 15) & 0x1;      // f bit to differentiate between LUI and AUIPC.
                uint8_t imm_upper = (inst >> 9)  & 0x3F;      // Upper 6 bits of immediate.
                uint8_t rd        = (inst >> 6)  & 0x7;       // Destination register.
                uint8_t imm_lower = (inst >> 3)  & 0x7;       // Lower 3 bits of immediate.
                // Recombine into a 9-bit immediate value.
                uint16_t imm_val = (imm_upper << 3) | imm_lower;
                if (f == 0) {
                    ss << "lui " << regNames[rd] << ", " << imm_val;
                } else {
                    ss << "auipc " << regNames[rd] << ", " << imm_val;
                }
                break;
            }
            case 0x7: { // SYS-type instructions.
                // Extract service number from bits 6 to 15.
                uint16_t service = (inst >> 6) & 0x3FF;
                uint8_t funct3 = (inst >> 3) & 0x7;
                if (funct3 == 0b000)
                    ss << "ecall " << service;
                else
                    ss << "Unknown SYS-type instruction";
                break;
            }
            default:
                ss << "Unknown instruction with opcode 0x" << hex << static_cast<int>(opcode);
                break;
        }
        return ss.str();
    }

    // -----------------------------------------------------
    // Execution Loop: Simulate running the loaded program.
    // -----------------------------------------------------
    bool runExecution(ostream &out) {
        size_t cycleCount = 0;
        const size_t MAX_CYCLES = 10000;
        while (pc < programSize) {
            if (cycleCount++ > MAX_CYCLES) {
                out << "\nInfinite loop detected at PC = 0x" << setw(4) << setfill('0')
                    << hex << pc << ". Exiting simulation.\n";
                return false;
            }    
            uint16_t inst = readWord(pc);
            out << "0x" << setw(4) << setfill('0') << hex << pc << ": "
                << setw(4) << inst << "  " << disassemble(pc, inst) << endl;
            // Execute the instruction. If execution should terminate, break.
            if (!executeInstruction(inst))
                break;
        }
        return true;
    }

    // ----------------------------------------------
    // Print Final Register State to the Output Stream.
    // ----------------------------------------------
    void printFinalState(ostream &out) {
        out << "\nFinal register state:" << endl;
        for (size_t i = 0; i < regs.size(); i++) {
            out << regNames[i] << " = " << "0x" << setw(4) << setfill('0')
                << hex << regs[i] << endl;
        }
    }

    // -----------------------------------------------------
    // Execute a Single Instruction.
    // Returns false if simulation should terminate.
    // -----------------------------------------------------
    bool executeInstruction(uint16_t inst) {
        // Extract opcode (lowest 3 bits).
        uint8_t opcode = inst & 0x7;
        bool pcUpdated = false;

        // Special-case: If instruction is a specific jump (J-type) instruction with value 0x818D.
        if (1){
            // Switch based on opcode.
            switch(opcode) {
                case 0x0: { // R-type instructions.
                    uint8_t funct4 = (inst >> 12) & 0xF;
                    uint8_t rs2 = (inst >> 9) & 0x7;
                    uint8_t rd_rs1 = (inst >> 6) & 0x7;
                    uint8_t funct3 = (inst >> 3) & 0x7;
                    if (funct4 == 0b0000 && funct3 == 0b000)
                        regs[rd_rs1] = regs[rd_rs1] + regs[rs2];
                    else if (funct4 == 0b0001 && funct3 == 0b000)
                        regs[rd_rs1] = regs[rd_rs1] - regs[rs2];
                    else if (funct4 == 0b0000 && funct3 == 0b001)
                        regs[rd_rs1] = (int16_t)regs[rd_rs1] < (int16_t)regs[rs2] ? 1 : 0;
                    else if (funct4 == 0b0000 && funct3 == 0b010)
                        regs[rd_rs1] = regs[rd_rs1] < regs[rs2] ? 1 : 0;
                    else if (funct4 == 0b0010 && funct3 == 0b011)
                        regs[rd_rs1] = regs[rd_rs1] << (regs[rs2] & 0xF);
                    else if (funct4 == 0b0100 && funct3 == 0b011)
                        regs[rd_rs1] = regs[rd_rs1] >> (regs[rs2] & 0xF);
                    else if (funct4 == 0b1000 && funct3 == 0b011)
                        regs[rd_rs1] = static_cast<uint16_t>(static_cast<int16_t>(regs[rd_rs1]) >> (regs[rs2] & 0xF));
                    else if (funct4 == 0b0001 && funct3 == 0b100)
                        regs[rd_rs1] = regs[rd_rs1] | regs[rs2];
                    else if (funct4 == 0b0000 && funct3 == 0b101)
                        regs[rd_rs1] = regs[rd_rs1] & regs[rs2];
                    else if (funct4 == 0b0000 && funct3 == 0b110)
                        regs[rd_rs1] = regs[rd_rs1] ^ regs[rs2];
                    else if (funct4 == 0b0000 && funct3 == 0b111)
                        regs[rd_rs1] = regs[rs2];
                    else if (funct4 == 0b0100 && funct3 == 0b000) {
                        // JR: Jump register.
                        pc = regs[rd_rs1];
                        pcUpdated = true;
                    } else if (funct4 == 0b1000 && funct3 == 0b000) {
                        // JALR: Save return address and jump.
                        regs[rd_rs1] = pc + 2;
                        pc = regs[rs2];
                        pcUpdated = true;
                    } else {
                        cout << "Unknown R-type instruction at PC = 0x" << hex << pc << endl;
                    }
                    break;
                }
                case 0x1: { // I-type instructions.
                    // Extract fields for I-type instruction.
                    uint8_t imm7    = (inst >> 9) & 0x7F;   // Immediate (bits 15:9)
                    uint8_t rd_rs1  = (inst >> 6) & 0x7;      // Register operand
                    uint8_t funct3  = (inst >> 3) & 0x7;      // Function code
                    uint8_t imm3    = (inst >> 13) & 0x7;     // For shift instructions (overlap)
                    // Sign-extend immediate for non-shift operations.
                    int16_t simm = (imm7 & 0x40) ? (imm7 | 0xFF80) : imm7;

                    switch (funct3) {
                        case 0b000: {
                            // ADDI: Add immediate.
                            regs[rd_rs1] = regs[rd_rs1] + simm;
                            break;
                        }
                        case 0b001: {
                            // SLTI: Set if less than (signed).
                            regs[rd_rs1] = ((int16_t) regs[rd_rs1] < simm) ? 1 : 0;
                            break;
                        }
                        case 0b010: {
                            // SLTUI: Set if less than (unsigned).
                            regs[rd_rs1] = ((uint16_t) regs[rd_rs1] < (uint16_t) simm) ? 1 : 0;
                            break;
                        }
                        case 0b011: {
                            // Shift instructions: SLLI, SRLI, SRAI.
                            uint8_t shamt = imm7 & 0xF;  // Shift amount is in lower 4 bits.
                            switch (imm3) {
                                case 0b001: // SLLI: Shift left logical.
                                    regs[rd_rs1] = regs[rd_rs1] << shamt;
                                    break;
                                case 0b010: // SRLI: Shift right logical.
                                    regs[rd_rs1] = regs[rd_rs1] >> shamt;
                                    break;
                                case 0b100: // SRAI: Shift right arithmetic.
                                    regs[rd_rs1] = static_cast<uint16_t>(
                                        static_cast<int16_t>(regs[rd_rs1]) >> shamt
                                    );
                                    break;
                                default:
                                    cout << "Unimplemented I-type shift instruction at PC = 0x"
                                         << hex << pc << endl;
                                    break;
                            }
                            break;
                        }
                        case 0b100: {
                            // ORI: Bitwise OR with immediate.
                            regs[rd_rs1] = regs[rd_rs1] | simm;
                            break;
                        }
                        case 0b101: {
                            // ANDI: Bitwise AND with immediate.
                            regs[rd_rs1] = regs[rd_rs1] & simm;
                            break;
                        }
                        case 0b110: {
                            // XORI: Bitwise XOR with immediate.
                            regs[rd_rs1] = regs[rd_rs1] ^ simm;
                            break;
                        }
                        case 0b111: {
                            // LI: Load immediate.
                            regs[rd_rs1] = simm;
                            break;
                        }
                        default:
                            cout << "Unimplemented I-type instruction at PC = 0x"
                                 << hex << pc << endl;
                            break;
                    }
                    break;
                }
                case 0x2: { // B-type (branch) instructions.
                    int8_t raw_offset = (inst >> 12) & 0xF;
                    if (raw_offset & 0x8)
                        raw_offset |= 0xF0;  // Sign extend offset.
                    uint8_t rs2 = (inst >> 9) & 0x7;
                    uint8_t rs1 = (inst >> 6) & 0x7;
                    uint8_t funct3 = (inst >> 3) & 0x7;
                    uint16_t target = pc + (raw_offset * 2);
                    switch(funct3) {
                        case 0b000: // BEQ
                            if (regs[rs1] == regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        case 0b001: // BNE
                            if (regs[rs1] != regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        case 0b010: // BZ (branch if register rs1 equals 0)
                            if (regs[rs1] == 0) {
                                pc = target;
                                return true;
                            }
                            break;
                        case 0b011: // BNZ (branch if register rs1 is nonzero)
                            if (regs[rs1] != 0) {
                                pc = target;
                                return true;
                            }
                            break;
                        case 0b100: // BLT (branch if less than, signed)
                            if ((int16_t)regs[rs1] < (int16_t)regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        case 0b101: // BGE (branch if greater than or equal, signed)
                            if ((int16_t)regs[rs1] >= (int16_t)regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        case 0b110: // BLTU (branch if less than, unsigned)
                            if (regs[rs1] < regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        case 0b111: // BGEU (branch if greater than or equal, unsigned)
                            if (regs[rs1] >= regs[rs2]) {
                                pc = target + 2;
                                return true;
                            }
                            break;
                        default:
                            cout << "Unimplemented B-type instruction at PC = 0x" << hex << pc << endl;
                            break;
                    }
                    break;
                }
                case 0x3: { // S-type (store) instructions.
                    int8_t offset = (inst >> 12) & 0xF;
                    uint8_t rs1 = (inst >> 6) & 0x7;   // Base register for store.
                    uint8_t rs2 = (inst >> 9) & 0x7;   // Register holding value to store.
                    uint8_t funct3 = (inst >> 3) & 0x7;
                    uint16_t addr = regs[rs1] + offset;
                    if (funct3 == 0b000)
                        writeByte(addr, regs[rs2] & 0xFF);
                    else if (funct3 == 0b001)
                        writeWord(addr, regs[rs2]);
                    break;
                }
                case 0x4: { // L-type (load) instructions.
                    int8_t offset = (inst >> 12) & 0xF;
                    uint8_t rs2 = (inst >> 9) & 0x7;   // Base register.
                    uint8_t rd = (inst >> 6) & 0x7;      // Destination register.
                    uint8_t funct3 = (inst >> 3) & 0x7;
                    uint16_t addr = regs[rs2] + offset;
                    if (funct3 == 0b000)
                        regs[rd] = static_cast<int8_t>(readByte(addr));
                    else if (funct3 == 0b001)
                        regs[rd] = readWord(addr);
                    else if (funct3 == 0b100)
                        regs[rd] = readByte(addr);
                    break;
                }
                case 0x5: { // J-type (jump) instructions.
                    uint8_t f = (inst >> 15) & 0x1;  // Flag to differentiate jump types.
                    uint8_t imm6 = (inst >> 9) & 0x3F;
                    uint8_t rd = (inst >> 6) & 0x7;
                    uint8_t imm3 = (inst >> 3) & 0x7;
                    // Combine immediate fields into 9-bit immediate.
                    int16_t rawImm = imm3 | (imm6 << 3);
                    // Sign-extend the immediate.
                    if (rawImm & (1 << 8))
                        rawImm |= 0xFE00;
                    uint16_t target = pc + (rawImm * 2);
                    if (f == 0b0) {
                        pc = target;
                        pcUpdated = true;
                    }
                    else if (f == 0b1) {
                        regs[rd] = pc + 2;  // Save return address in register.
                        pc = target;
                        pcUpdated = true;
                    }
                    break;
                }
                case 0x6: { // U-type (lui/auipc) instructions.
                    uint8_t f         = (inst >> 15) & 0x1;
                    uint8_t imm_upper = (inst >> 9)  & 0x3F;
                    uint8_t rd        = (inst >> 6)  & 0x7;
                    uint8_t imm_lower = (inst >> 3)  & 0x7;
                    uint16_t imm_val  = (imm_upper << 3) | imm_lower;
                    if (f == 0) {
                        // LUI: Load upper immediate (shift left by 7 bits).
                        regs[rd] = imm_val << 7;
                    } else {
                        // AUIPC: Add upper immediate to PC.
                        regs[rd] = pc + (imm_val << 7);
                    }
                    break;
                }
                case 0x7: { // SYS-type instructions.
                    uint16_t service = (inst >> 6) & 0x3FF;
                    uint8_t funct3 = (inst >> 3) & 0x7;
                    if (funct3 == 0b000) {
                        if (service == 1) {
                            // ecall service 1: Print integer (assumes a0 is at index 6).
                            cout << "Print integer: " << dec << static_cast<int16_t>(regs[6]) << endl;
                        } else if (service == 3) {
                            // ecall service 3: Terminate simulation.
                            cout << "ecall 3" << endl;
                            cout << "ecall terminate simulation" << endl;
                            return false;
                        } else if (service == 5) {
                            // ecall service 5: Print null-terminated string.
                            // Assumes address of string in register a0.
                            uint16_t addr = regs[6];
                            string output;
                            while (true) {
                                char c = static_cast<char>(readByte(addr));
                                if (c == '\0') break;
                                output.push_back(c);
                                addr++;
                            }
                            cout << "Print string: " << output << endl;
                        } else {
                            cout << "ecall " << service << endl;
                        }
                    } else {
                        cout << "Unknown SYS-type instruction" << endl;
                    }
                    break;
                }
                default:
                    cout << "Unknown instruction opcode 0x" << hex << static_cast<int>(opcode)
                         << " at PC = 0x" << pc << endl;
                    break;
            }
        }
        // If the instruction did not change the PC explicitly, increment by 2 (instruction size).
        if (!pcUpdated)
            pc += 2;
        // Terminate simulation if PC is beyond program size.
        if (pc >= programSize)
            return false;
        return true;
    }

    // ---------------------------------------------------
    // Print final register state to standard output.
    // ---------------------------------------------------
    // (This function is used in non-redirected mode.)
    void printFinalState() {
        cout << "\nFinal register state:" << endl;
        for (size_t i = 0; i < regs.size(); i++) {
            cout << regNames[i] << " = " << "0x" << setw(4) << setfill('0') << hex << regs[i] << endl;
        }
    }

    // ------------------------------------------------------------------------
    // Linear Disassembly: Walk through memory and output disassembly.
    // Writes output to the provided output stream.
    // ------------------------------------------------------------------------
    void runFullDisassembly(ostream &out) {
        uint16_t addr = 0;
        while (addr < programSize) {
            // --- Step 1: Detect an ASCII string ---
            const int MIN_STR_LEN = 4;      // Minimum length for string detection.
            const int MAX_PROBE = 256;        // Limit to avoid scanning too far.
            int probe = addr;
            string candidate;
            bool nullFound = false;
            while (probe < programSize && (probe - addr) < MAX_PROBE) {
                uint8_t b = readByte(probe);
                if (b == 0) {  // Null terminator found.
                    nullFound = true;
                    break;
                }
                // Only allow printable characters and whitespace.
                if (!isprint(b) && !isspace(b))
                    break;
                candidate.push_back(static_cast<char>(b));
                probe++;
            }
            if (nullFound && candidate.length() >= MIN_STR_LEN) {
                out << "0x" << setw(4) << setfill('0') << hex << addr
                    << ": .asciiz \"" << candidate << "\"" << endl;
                // Skip over the entire string and the null terminator.
                addr = probe + 1;
                continue;
            }

            // --- Step 2: Group contiguous zero words ---
            if (addr + 1 < programSize) {
                uint16_t word = readWord(addr);
                if (word == 0) {
                    int zeroCount = 0;
                    uint16_t startAddr = addr;
                    while (addr + 1 < programSize && readWord(addr) == 0) {
                        zeroCount++;
                        addr += 2;
                    }
                    const int THRESHOLD = 4; // If 4 or more consecutive zero words, group them.
                    if (zeroCount >= THRESHOLD) {
                        out << "0x" << setw(4) << setfill('0') << hex << startAddr
                            << ": .space " << (zeroCount * 2) << " bytes" << endl;
                        continue;
                    } else {
                        // For small gaps, output each zero word individually.
                        for (int i = 0; i < zeroCount; i++) {
                            out << "0x" << setw(4) << setfill('0') << hex << (startAddr + i * 2)
                                << ": .word 0x0000" << endl;
                        }
                        continue;
                    }
                }
            }

            // --- Step 3: Attempt to disassemble an instruction ---
            if (addr + 1 < programSize) {
                uint16_t word = readWord(addr);
                string instStr = disassemble(addr, word);
                // If disassembly returns "Unknown", assume it's data.
                if (instStr.find("Unknown") != string::npos) {
                    out << "0x" << setw(4) << setfill('0') << hex << addr
                        << ": .word 0x" << setw(4) << word << endl;
                    addr += 2;
                    continue;
                } else {
                    out << "0x" << setw(4) << setfill('0') << hex << addr << ": "
                        << setw(4) << word << "  " << instStr << endl;
                    addr += 2;
                    continue;
                }
            }

            // --- Step 4: Handle any leftover single byte ---
            uint8_t b = readByte(addr);
            out << "0x" << setw(4) << setfill('0') << hex << addr
                << ": .byte 0x" << setw(2) << (int)b << endl;
            addr++;
        }
    }
};

//
// ---------------------
// Main Entry Point
// ---------------------
//
int main(int argc, char **argv) {
    // Check for correct command-line usage.
    if (argc != 2) {
        cerr << "Usage: rvsim <machine_code_file_name>" << endl;
        return EXIT_FAILURE;
    }

    // Retrieve the machine code file name from command-line argument.
    string machineFilename = argv[1];

    try {
        Z16Simulator sim;

        // Load the binary machine code into memory.
        {
            ifstream fin(machineFilename, ios::binary);
            if (!fin)
                throw runtime_error("Error opening binary file: " + machineFilename);
            fin.read(reinterpret_cast<char*>(sim.memory.data()), MEM_SIZE);
            sim.programSize = fin.gcount();
            cout << "Loaded " << sim.programSize << " bytes into memory from " << machineFilename << endl;
        }

        // Build output file name by appending ".dis" to the input file name.
        string outputFilename = machineFilename + ".dis";
        ofstream out(outputFilename);
        if (!out) {
            cerr << "Error opening output file: " << outputFilename << endl;
            return EXIT_FAILURE;
        }

        // Write full disassembly to the output file.
        out << "Full disassembly of binary:\n";
        sim.runFullDisassembly(out);

        // Reset PC and registers for simulation execution.
        sim.pc = 0;
        sim.regs.fill(0);
        sim.regs[2] = MEM_SIZE - 2;

        // Write execution simulation trace.
        out << "\nExecution simulation trace:\n";
        sim.runExecution(out);

        // Write final register state.
        sim.printFinalState(out);
        sim.showmem(out);

        out.close();
        cout << "Disassembly and simulation trace written to " << outputFilename << endl;

    } catch (const exception &ex) {
        cerr << ex.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
