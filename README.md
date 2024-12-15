// ---------------------- The complete task of Program Analysis part is done -------------------------------- //

##  DEBUGGER PROJECT

## Overview
This debugger is a command-line tool for analyzing and debugging executable programs. It supports setting breakpoints, stepping through instructions, inspecting registers, and disassembling functions or entire programs. It is implemented using C++ and leverages Linux system calls like `ptrace` for debugging functionality.

## Features
- **Set Breakpoints**: Add breakpoints to specific memory addresses.
- **Instruction Stepping**: Step through instructions one at a time.
- **Continue Execution**: Resume execution until the next breakpoint or program completion.
- **Disassembly**: Disassemble specific functions or the entire program using `objdump`.
- **Inspect Registers**: View the values of CPU registers during execution.
- **Breakpoint Listing**: Display all active breakpoints.

## Commands
| Command                | Description                                      |
|------------------------|--------------------------------------------------|
| `next` / `nexti`       | Step to the next instruction.                   |
| `break <address>`      | Set a breakpoint at the specified address.      |
| `continue` / `c`       | Continue execution until the next breakpoint.   |
| `exit`                 | Exit the debugger.                              |
| `infobreak` / `i b`    | List all breakpoints.                           |
| `info registers` / `i r` | List all register values.                     |
| `disass` / `dis`       | Disassemble a function or the entire program.   |

## Usage

1. Compile the debugger:
   ```bash
   g++ -o debugger debugger.cpp -std=c++11
   ```

2. Run the debugger with an executable file:
   ```bash
   ./debugger path/to/executable
   ```

3. Use the commands listed above to debug the program interactively.

## Example

### Setting a Breakpoint
```
break 0x123456
```
This sets a breakpoint at the address `0x123456`.

### Viewing Registers
```
info registers
```
Displays the current values of the CPU registers.

### Disassembling Functions
```
disass
```
Follow the prompts to disassemble a specific function or the entire program.

### Continuing Execution
```
continue
```
Resumes execution of the target program.

## Implementation Details
The debugger makes extensive use of the `ptrace` system call to control and observe the target program. It allows:
- **Tracing Execution**: Using `PTRACE_TRACEME`, `PTRACE_CONT`, and `PTRACE_SINGLESTEP` to manage program execution.
- **Breakpoint Management**: Writing `INT 3` (`0xCC`) trap instructions to memory.
- **Register Access**: Reading and modifying registers with `PTRACE_GETREGS` and `PTRACE_SETREGS`.
- **Memory Inspection**: Using `PTRACE_PEEKTEXT` and `PTRACE_POKETEXT` to read and modify memory.

### Dependencies
- `ptrace` for tracing the target program.
- `objdump` for disassembly.

### File Structure
- **Main Program (`main`)**: Handles user input and debugging session.
- **Command Handlers**: Implements functionality for each debugger command.
- **Breakpoint Management**: Tracks active breakpoints in a `std::map`.

## Limitations
- This debugger is designed for educational purposes and may not handle complex scenarios like multi-threaded programs.
- Works only on Linux systems due to its reliance on `ptrace` and Linux-specific features.



