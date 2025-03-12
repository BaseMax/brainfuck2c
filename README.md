# Brainfuck to C Transpiler

This project is a Brainfuck-to-C transpiler written in C. It converts Brainfuck source code into equivalent C code by following three main phases:

1. **Lexer Phase:**  
   Reads the input Brainfuck code and produces an array of tokens.

2. **Parser Phase:**  
   Converts the token stream into an Abstract Syntax Tree (AST), merging consecutive operations and handling loops recursively. It also detects and reports unmatched brackets.

3. **Generator Phase:**  
   Traverses the AST and prints out the corresponding C code.

The generated C code creates a memory tape of `TAPE_SIZE` cells and uses standard C I/O (`getchar`/`putchar`) for Brainfuck’s input/output.

## Features

- **Optimized Token Merging:** Consecutive operations (e.g., multiple `+` or `>`) are merged into a single statement.
- **Robust Error Handling:** Reports unmatched brackets and handles memory allocation errors.
- **Flexible Input:** Reads Brainfuck code from a file (provided as a command‑line argument) or from standard input.
- **Readable Output:** The generated C code is clean, well-indented, and easy to understand.

## Requirements

- A C99 compliant compiler (e.g., GCC)
- GCC (GNU Compiler Collection) for building the project

## Building the Transpiler

To compile the transpiler, run:

```bash
gcc -Wall -Wextra -pedantic -Werror -o brainfuck2c brainfuck2c.c
```

This command enables all warnings, treats warnings as errors, and produces an executable named brainfuck2c.

## Usage

You can run the transpiler by specifying a Brainfuck source file:

```bash
./brainfuck2c program.bf > program.c
```

### Compiling the Generated C Code

After generating the C code, compile it with:

```bash
gcc -Wall -Wextra -pedantic -Werror -o program program.c
```

Then run the resulting executable:

```bash
./program
```

## Code Structure

`brainfuck2c.c` - The main source file that implements the transpiler, organized into:

  - Lexer Phase: Contains token definitions and the lex() function.
  - Parser Phase: Implements the AST data structures and recursive parsing functions.
  - Generator Phase: Traverses the AST to generate equivalent C code.

## License

This project is distributed under the MIT License.

## Contributing

Contributions are welcome! Feel free to fork the repository, make improvements, and submit pull requests. For any issues or suggestions, please open an issue on GitHub.

Developed by Max Base (Seyyed Ali Mohammadiyeh).
