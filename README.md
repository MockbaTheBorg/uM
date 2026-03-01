# uM - MUMPS Monitor for Microcontrollers

A MUMPS (M) language interpreter for microcontrollers, loosely based on the 1984 M(umps) Standard and Z80 Micromumps for CP/M.

## Features

### Implemented
- **Pattern Matching** - Full pattern matching with type codes:
  - `A` - Alphabetic characters
  - `N` - Numeric digits
  - `U` - Uppercase letters
  - `L` - Lowercase letters
  - `P` - Punctuation
  - `C` - Control characters
  - `E` - Any character
  - Quantifiers: `n`, `n.`, `n.m`, `.`, `.m`
  - Alternation: `(pattern1,pattern2,...)`
  - Literals: `"quoted strings"`

- **Variables** - Local variables with subscript support
- **Global Variables** - B-tree based storage in GLOBALS.DAT
- **Functions** - Built-in MUMPS functions:
  - `$A()` - ASCII value
  - `$C()` - Character from ASCII
  - `$D()` - Data existence
  - `$E()` - Extract substring
  - `$F()` - Find substring
  - `$J()` - Justify string
  - `$L()` - String length
  - `$O()` - Order (next/previous subscript)
  - `$P()` - Piece (substring by delimiter)
  - `$Q()` - Query next
  - `$R()` - Random number
  - `$S()` - String replace

- **Commands** - Standard MUMPS commands:
  - `SET`, `WRITE`, `READ`, `KILL`
  - `IF`, `ELSE`, `FOR`, `WHILE`, `DO`, `GOTO`
  - `QUIT`, `RETURN`, `HALT`
  - `NEW`, `XECUTE`

### Architecture
```
src/
  mumps.c      - Main interpreter entry point
  defines.h   - Global definitions and constants
  infra.h     - Infrastructure (I/O, memory)
  symtbl.h    - Symbol table management
  expr.h      - Expression parser with pattern matching
  functions.h - Built-in functions
  commands.h  - Command implementations
  svars.h     - Special variables
  global.h    - Global variable storage
refs/
  global.c    - GLOBALS.DAT B-tree management tool
```

## Building

```bash
make
```

This produces the `mumps` executable.

## Usage

Run the interpreter:
```bash
./mumps
```

### GLOBALS.DAT Management

The `refs/global.c` tool manages the global variable database:

```bash
# Create new database (200 blocks default)
refs/global GLOBALS.DAT -C 200

# Read a global
refs/global GLOBALS.DAT -W "^NAME"
refs/global GLOBALS.DAT -W "^NAME" -k "subscript1,subscript2"

# Set a global
refs/global GLOBALS.DAT -S "^NAME" -v "value"
refs/global GLOBALS.DAT -S "^NAME" -k "1,2" -v "value"

# Delete a global
refs/global GLOBALS.DAT -K "^NAME"
refs/global GLOBALS.DAT -K "^NAME" -k "1,2"
```

## Database Format

- Block size: 768 bytes
- Block pointers: 24-bit middle-endian
- B-tree structure with directory blocks and data blocks
- Canonical subscript encoding for proper sorting

## Version

Version: 06.Oct.14

## Author

Marcelo Dantas  
marcelo.f.dantas@gmail.com
