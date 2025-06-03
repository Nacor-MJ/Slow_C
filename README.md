# Slow_C

A simple C-like language compiler/interpreter project. This project is organized for clarity and modularity, with source code, headers, libraries, and tests separated into dedicated folders.

## Project Structure

```
Slow_C/
├── Makefile           # Build instructions
├── README.md          # Project documentation
├── build/             # Build artifacts and temporary files
├── include/           # Header files
├── lib/               # Third-party and library source files
├── src/               # Main source code
├── tests/             # Test files
```

## Building

1. Make sure you have GCC and Make installed (MSYS2 recommended on Windows).
2. Open your terminal in the project root directory.
3. Run:
   ```sh
   make
   ```
4. The output binary will be named `slow_c`.

## Running

To run the compiler/interpreter on a test file:

```sh
./slow_c tests/idk.c
```

## Main Components

- **src/**: Core compiler/interpreter logic (lexer, parser, IR, codegen, etc.)
- **include/**: Project headers and third-party headers (e.g., stb_ds.h)
- **lib/**: Third-party source files (e.g., stb_ds.c)
- **tests/**: Example and test programs
- **build/**: Temporary build files and assembly output

## Dependencies

- [stb_ds.h](https://github.com/nothings/stb) (public domain, included in `include/` and `lib/`)
- Standard C library

## Contributing

Pull requests and issues are welcome! Please keep code modular and document major changes.

## License

This project is open source. See individual files for license details (e.g., stb_ds.h is public domain).
