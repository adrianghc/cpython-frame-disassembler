""" CPython Frame Disassembler - Debugging tool for C extensions """

from .frame_disassembler import disassemble_frame

__version__ = "0.1.0"
__all__ = ["disassemble_frame"]
