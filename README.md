# WLP4 to MIPS Machine Code Compiler

## Overview

This project is a compiler that converts WLP4 (a subset of C) code into MIPS machine code. The compilation process involves multiple steps, including scanning, parsing, semantic analysis, assembly language generation, and binary machine code creation. This README outlines the usage, installation, and contribution guidelines for the compiler.

## Features

- **Tokenization:** Utilizes WLP4 language recognition DFA and the Simplified Maximal Munch algorithm to scan WLP4 code into tokens.
- **Parsing and Syntax Checking:** Employs scanned tokens with the WLP4 SLR(1) language to construct a parse tree while conducting syntax checks.
- **Semantic Analysis:** Annotates types to the parse tree to perform semantic checks.
- **Assembly Language Generation:** Generates MIPS assembly language based on the type-annotated parse tree.
- **Assembly Scanning:** Scans MIPS assembly language using the same technique as in the initial tokenization step.
- **Binary Code Generation:** Converts scanned MIPS tokens into binary MIPS machine code.

## Contributing

Contributions to the WLP4 to MIPS Machine Code Compiler are welcome. To contribute:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes.
4. Submit a pull request with a clear description of your changes.
