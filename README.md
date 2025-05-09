# SICXE-Assembler

A C-based two-pass assembler for the SIC/XE architecture. This project demonstrates symbol table construction, opcode handling, and object code generation. Ideal for educational use in systems programming or compiler construction courses.



## Project Structure

```
SIC_XE Program (PORTFOLIO)/
├── directives.c
├── directives.h
├── errors.c
├── errors.h
├── headers.h
├── main.c
├── opcodes.c
├── opcodes.h
├── symbols.c
├── symbols.h
├── test0.sic               # Sample SIC/XE assembly source file
├── test0.lst               # Generated listing file
├── test0.obj               # Generated object file
├── Example test0.lst       # Reference listing output
├── Example test0.obj       # Reference object output
├── SIC_XE                  # Compiled output binary
└── Project3.dSYM/          # Debug symbols directory (macOS)
```

---



## Core C Files

### `main.c`
Implements the main assembler logic:
- Pass 1: Symbol table creation and location counter
- Pass 2: Object code generation and listing file output
- Handles format detection and flag computation

### `opcodes.c`
Handles:
- Opcode-to-hex translation
- Instruction format determination
- Mnemonic validation

### `symbols.c`
Manages:
- Symbol insertion and lookup
- Duplicate symbol detection
- Address resolution

### `directives.c`
Handles:
- Assembly directives such as START, END, BYTE, WORD, RESW, and RESB
- Literal management (if implemented)

### `errors.c`
Supports:
- Error reporting for invalid instructions, undefined symbols, and format mismatches

---



## Sample Files

### `test0.sic`
An input file containing SIC/XE assembly code:
- Instructions, labels, and directives
- Various addressing modes and program structure

### `test0.lst`
A listing file showing:
- Memory addresses
- Generated object code
- Source code lines with resolved symbols and errors

### `test0.obj`
An object file including:
- Header (H), Text (T), and End (E) records
- Suitable for SIC/XE loader execution or simulation

---



## How to Run Program

Compile the program using `gcc`:

    gcc -o SIC_XE main.c errors.c symbols.c opcodes.c directives.c

Then run the assembler with a `.sic` input file:

    ./SIC_XE test0.sic

Output files `test0.lst` and `test0.obj` will be created in the same directory.

---



## Future Enhancements
- Extended support for format 4 and indexed addressing
- Literal pools and base-relative addressing
- Integrated loader/linker system
- GUI-based simulator or web interface

---



## License
This project is provided for educational purposes and does not currently include a formal license.

---



## Author
**Trace**
