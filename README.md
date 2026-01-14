# PasteFile Plugin for x32dbg
A robust plugin for x32dbg that allows users to paste binary file contents directly into the debugger's memory.
for example , you export a .bin file from .text section of memory , you can use this plugin to paste it back to memory. normally used to diffrent virant version of same base PE file.

## Features
- **Direct Memory Patching**: Load any binary file and write it to the debugged process's memory.
- **Flexible Addressing**: Supports pasting at the current cursor location (Dump view) or a specific memory address.
- **Detailed Feedback**: Provides byte-level statistics (Skipped/Patched/Failed) and error reporting.

## Usage
**Command**: `PasteFile`

**Syntax**:
```text
PasteFile <filename>,[address]
```

**Examples**:
- **Use Current Selection**:
  ```text
  PasteFile C:\Path\To\shellcode.bin
  ```
  *(Pastes content starting at the current address selected in the Dump view)*

- **Specify Address**:
  ```text
  PasteFile C:\Path\To\shellcode.bin,00401000
  ```
  *(Pastes content starting at 0x401000)*

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
- **"Memory patch failed"**?
  Check if the target memory address is writable. The plugin attempts to write byte-by-byte and reports failures.
- **File not found?**
  Use absolute paths (e.g., `C:\temp\file.bin`) to avoid ambiguity.
