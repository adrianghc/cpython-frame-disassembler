"""CPython Frame Disassembler - Debug tool for C extensions."""

# Re-export the C extension function
from .frame_disassembler import disassemble_frame

__version__ = "0.1.0"
__all__ = ["disassemble_frame"]
