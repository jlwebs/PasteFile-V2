# PasteFile Plugin for x32dbg
A robust plugin for x32dbg that allows users to paste binary file contents directly into the debugger's memory.
for example , you export a .bin file from .text section of memory , you can use this plugin to paste it back to memory. normally used to diffrent virant version of same base PE file.

## Features
- **Direct Memory Patching**: Load any binary file and write it to the debugged process's memory.
- **Flexible Addressing**: Supports pasting at the current cursor location (Dump view) or a specific memory address.
- **Detailed Feedback**: Provides byte-level statistics (Skipped/Patched/Failed) and error reporting.

## Usage
**Commands**: `PasteFile`, `CopyIN`, `PasteOut`

### 1. PasteFile
Directly paste a file's content to memory.

**Syntax**:
```text
PasteFile <filename>,[address]
```

**Examples**:
- **Use Current Selection**:
  ```text
  PasteFile C:\Path\To\shellcode.bin
  ```
- **Specify Address**:
  ```text
  PasteFile C:\Path\To\shellcode.bin,00401000
  ```

### 2. CopyIN & PasteOut (Two-Step Patching)
Useful for extracting a specific chunk from a binary file (e.g., a memory dump) and pasting it elsewhere.

#### CopyIN
Reads a segment from a binary file into an internal memory buffer.

**Syntax**:
```text
CopyIN <filename>,<begin_attr>,<end_attr>,[base]
```
- **begin_attr / end_attr**: Virtual addresses defining the range to copy.
- **base**: The base address of the file (default: `401000`). Used to calculate file offsets (`offset = attr - base`).

**Example**:
```text
CopyIN C:\Dump.bin, 401000, 402000
```
*(Reads data corresponding to 401000-402000 from Dump.bin. If base is not provided, you will be prompted to confirm default 401000.)*

#### PasteOut
Pastes the data currently stored in the internal buffer to memory.

**Syntax**:
```text
PasteOut [begin_attr]
```

**Examples**:
- **Paste to Cursor**:
  ```text
  PasteOut
  ```
  *(Pastes to the current address selected in the CPU Disassembly view)*

- **Paste to Specific Address**:
  ```text
  PasteOut 405000
  ```

*Note: Ensure you use absolute paths for the filename.*

## Compilation Environment
This project has been updated and verified to build successfully in the following environment:

- **OS**: Windows 10/11
- **IDE**: Visual Studio 2022 Community
- **Platform Toolset**: v143
- **Windows SDK Version**: 10.0.18362.0
- **Target Architecture**: x86 (32-bit)
- **Configuration**: Release

## Build Instructions
1. Open `PasteFile.sln` in Visual Studio 2022.
2. Select **Release** configuration and **x86** platform.
3. If prompted, retarget the solution to your installed Windows SDK version (tested with 10.0.18362.0).
4. Build the solution (`Ctrl+Shift+B`).
5. The output plugin will be located at: `bin\x32\PasteFile.dp32`.

## Installation
1. Close x32dbg.
2. Copy `bin\x32\PasteFile.dp32` to the plugins directory of x32dbg:
   - Example: `D:\x32dbg\release\x32\plugins\`
3. Start x32dbg. Check the log window for "PasteFile" verification messages.

## Troubleshooting
- **Plugin not loading?**
  Ensure you are using the 32-bit version of the plugin (`.dp32`) with x32dbg.
- **"Patch failed!" with no end address?**
  If you do not specify an `end_address` and the file content is larger than the valid memory region (e.g., exceeds the current section), the **entire patch operation will be aborted** by the debugger kernel to prevent corruption. Nothing will be written.
  **Recommendation**: Always specify the `end_address` (3rd argument). This enables the plugin to safely **truncate** the data and write only what fits into the target range.
