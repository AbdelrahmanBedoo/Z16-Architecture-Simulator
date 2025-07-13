# Z16 Instruction Set Simulator (ISS)

## Overview

The Z16 Instruction Set Simulator (ISS) emulates a CPU by reading a binary machine code file for the Z16 (ZC16) architecture, disassembling its instructions into a human-readable format, and executing them as if they were running on actual hardware. 

## Project Requirements

- **Input Handling:**  
  - Read a ZC16 machine code file (`.bin`) containing encoded instructions and data.
  - The first instruction is assumed to be located at memory address `0x00000000`.

- **Instruction Processing:**  
  - **Decoding:** Each 16-bit instruction is decoded into a human-readable string that represents the actual Z16 instruction.
  - **Execution:** The simulator updates registers, modifies memory, or performs I/O operations via system calls (`ecall`).


- **Supported Instructions and Services:**  
  - **ecall 1:** Print an integer (the integer is stored in register `a0`).
  - **ecall 5:** Print a NULL-terminated string (the address of the string is stored in register `a0`).
  - **ecall 3:** Terminate the program.

## Build Instructions

### Prerequisites

- C++ compiler (supporting C++11 or later)
- CMake (optional, if using a CMake-based build system)
- Git (for version control)

### Compilation

To compile using a standard C++ compiler, run:

```bash
g++ -std=c++11 -o rvsim main.cpp
```

### Usage Guidelines

After compiling, run the simulator from the command line by providing the machine code file as an argument:

```bash
./rvsim path/to/machine_code_file.bin
```

The simulator will:

- **Load the machine code into memory.**
- **Display a full disassembly of the binary.**
- **Execute the instructions while showing a trace of the execution.**
- **Print the final state of the registers upon termination.**
- **Pring the memory**

## Design Overview

The simulator is implemented in C++ and is structured around the `Z16Simulator` class. Key components include:

### Memory Model
- A 64KB array represents the memory of the simulated machine.
- Memory operations are provided through helper functions for reading and writing bytes and words.

### Register File
- Eight 16-bit registers (`x0`–`x7`) are modeled using an `std::array<uint16_t, 8>`.
- These registers are also given ABI names (e.g., `t0`, `ra`, `sp`) for human-readable output.

### Instruction Decoding
- The `disassemble()` method converts each 16-bit instruction into a human-readable string.
- It handles various instruction formats such as R-type, I-type, B-type, etc., following the Z16 ISA specification.

### Instruction Execution
- The `executeInstruction()` method processes each instruction by updating registers, managing control flow (branching and jumping), and performing system calls (`ecall`).

### Control Flow
- The main execution loop in `runExecution()` fetches instructions sequentially from memory.
- It disassembles them, executes them, and prints execution traces.

### Testability
- Comprehensive test cases (at least 10) are developed to validate all aspects of the simulator, including instruction decoding, execution, and I/O services.
- An assembler available from the ZC16 ISA GitHub repository can be used to create test binaries.

## Group members and contributions

### Sama Emad
Implemented core simulation functions including:

- Memory Management: readByte, writeByte, readWord, and writeWord

- Execution Engine: runExecution and executeInstruction

- State Reporting: printFinalState for final register state output

### Abdelrahman Mohamed
Developed the disassembly and output features:

- Disassembly Module: disassemble and runFullDisassembly

- Data Handling: ASCII string detection and grouping of zero words

- System Call Processing: ecall service handling within executeInstruction

### Mai 

Developed the test cases and the test coverage matrix

## Project Challenges

### Instruction Decoding Complexity
- The variety of instruction formats required careful bit manipulation to correctly decode each field.

### Sign Extension and Immediate Handling
- Handling sign extension for immediate values in various instruction types was crucial for correct arithmetic and branching behavior.

### Branch Target Calculation
- Accurately calculating branch targets, particularly for B-type instructions with offset multiplications and adjustments, posed a significant challenge.

### Simulator Testing
- Creating comprehensive test cases that cover every instruction and edge case was essential for validating the simulator’s correctness.
- Debugging these cases helped refine the simulator’s execution flow and memory management.

## Conclusion

This project serves as a practical implementation of a RISC-style ISA simulator, offering insights into CPU design, instruction decoding, and low-level program execution. The simulator is intended as a learning tool, and further improvements or extensions can be made to enhance its functionality and performance.
