# CPython Frame Disassembler

A debugging tool for Python C extensions that shows the current Python bytecode location during C-level debugging.

## Purpose

When debugging C extensions with gdb or VSCode, you lose track of where you are in the Python code. This tool prints the current Python frame's bytecode, making it easy to orient yourself in the CPython interpreter.

## Installation

### Requirements
- Python 3.9+
- C++ compiler with C++11 support
- Python development headers

### Build from source
```bash
pip install build
python -m build
pip install dist/*.whl
```

### Development installation
```bash
pip install -e .
```

## Usage

### Basic usage in Python code
```python
import frame_disassembler

# Add this where you want to see the bytecode
frame_disassembler.disassemble_frame(marker="CHECKPOINT")
```

Output:
```
CHECKPOINT BEGIN line=42 /path/to/file.py
[bytecode disassembly]
CHECKPOINT END line=42 /path/to/file.py
```

### Debugging C extensions

The main use case is debugging C extensions in VSCode or gdb:

1. Set a breakpoint in your C extension
2. When the breakpoint hits, step out (Shift-F11) 4 times to reach the interpreter level
3. Step over (F10) until you're above the `disassemble_frame` call
4. Now you see exactly where you are in the Python code
5. Use F10/F11 to navigate to the problem area

The bytecode output serves as a landmark in the CPython interpreter, solving the problem of getting lost in interpreter code that all looks the same.

## Implementation

The tool uses a C++ extension with:
- RAII for automatic memory management
- GIL state management for thread safety
- Error state preservation across calls
- Detection and rejection of no-GIL Python builds (incompatible with debugging)

## History

Originally developed for PySide debugging where it has been used in production for years. This standalone version was extracted by Adrian Georg Herrmann with contributions from Christian Tismer.

## License

[To be determined]
