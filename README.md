# agon-ez80asm
## Source file format
Lines in the source file should have the following format:

    Label Operator Operand Comment

All fields are optional.

- Labels are defined as first token on a line, terminated by a colon (':')
- Operators are either Assembler directives, or ez80 opcodes and should start with whitespace. Assembler directives can start with a dot (.)
- Comments start with a semi-colon (;). A line with just a comment can start without whitespace

## Global labels

Global labels cannot have the same name as ez80 mnemonics, nor assembler directives.

Global labels are case-sensitive.
## Local labels
Local labels start with the '@' symbol, terminated by a colon (':') and define a temporary label between two global labels. As soon as a new global label starts, all local labels become out of scope and a new scoped local label space starts.

64 local labels are currently supported in each scope.

Local labels are case-sensitive.

## Anonymous labels
Anonymous labels are labeled '@@', terminated by a colon (':') and define a temporary label. As soon as a new anonymous label is defined, the previous is no longer reachable.
Code can use @f/@n or @b/@p, to get the label in the FORWARD/NEXT, or BACKWARD/PREVIOUS direction.

## Supported Assembler directives
All directives can be used with, or without leading dot (.)


| Directive               | Description                                                                             | Usage                                                                                                                                                                                                                                                                                                                                                                              |
|-------------------------|-----------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ALIGN                   | <br>Select alignment for the program counter,<br>pad bytes up to new boundary alignment | ALIGN \<boundary\><br><br>The boundary, expressed in bytes, must be a power of two (1,2,4,8 etc)<br><br>Example: ALIGN 256 ; align next instruction at boundary of 256                                                                                                                                                                                                             |
| ASSUME ADL              | Sets default ADL status                                                                 | Example: ASSUME ADL=1 ; set ADL mode to 1                                                                                                                                                                                                                                                                                                                                          |
| BLKB/BLKW/<br>BLKP/BLKL | Allocate a block with a number of Bytes/<br>16-bit words/24-bit Pointers/32-bit words   | BLKx \<number\> [, value]<br><br>Example: BLKB 16 ; allocate 16 uninitialized bytes<br>Example: BLKB 16, 0xFF ; allocate 16 bytes and initialize them to 0xFF                                                                                                                                                                                                                      |
| DB / DEFB<br>ASCII / BYTE      | Define byte(s)                                                                          | DB \| DEFB \| ASCII \<value\> \| string> [, ...]<br><br>Reserve and initialize a byte, or a list of bytes. Within a string, these escape characters are supported and converted to their respective ascii code:<br>\n \r \t \b \\\ \\' \\"<br><br>Strings are not automatically terminated by a null byte. You can use either an additional ',0' value, or use the ASCIZ directive |
| DW / DEFW               | Define 16-bit word(s)                                                                   | DW \| DEFW \<value\> [, ...]<br><br>Reserve and initialize a word/long value, or a list of values                                                                                                                                                                                                                                                                                  |
| DL                      | Define 24-bit word(s)                                                                   | DL \<value\> [, ...]                                                                                                                                                                                                                                                                                                                                                               |
| DS                      | Defines storage location in bytes                                                       | DS \<number\> [, value]<br><br>Example: DS 10 ; reserve 10 byte, no reserved value<br>Example: DS 10, 0xFF ; reserve 10 bytes, all value 0xFF                                                                                                                                                                                                                                      |
| EQU                     | Assign symbolic name to label                                                           | Example:<br>LABEL: EQU 0xFF                                                                                                                                                                                                                                                                                                                                                        |
| INCLUDE                 | Include file in source code                                                             | Allows the insertion of source code from another file into the current source file during assembly. The included file is assembled into the current source file immediately after the directive. When the EOF (End of File) of the included file is reached, the assembly resumes on the line after the INCLUDE directive<br><br>Example: INCLUDE "example.inc"                    |
| ORG                     | Define location counter origin.<br>No padding.                                          | Sets the assembler location counter to a specified value. The directive must be followed by an integer constant, which is the value of the new origin.<br>Example: ORG $40000                                                                                                                                                                                                      |
---

## Define label
    <label> .equ value

Store a label with the given value.

---
