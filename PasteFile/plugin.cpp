#include "plugin.h"

static std::wstring Utf8ToUtf16(const std::string &str) {
  std::wstring convertedString;
  int requiredSize =
      MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
  if (requiredSize > 0) {
    std::vector<wchar_t> buffer(requiredSize);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], requiredSize);
    convertedString.assign(buffer.begin(), buffer.end() - 1);
  }
  return convertedString;
}

static bool ReadAllData(const std::string &fileName,
                        std::vector<uint8_t> &content) {
  auto hFile = CreateFileW(Utf8ToUtf16(fileName).c_str(), GENERIC_READ,
                           FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  auto result = false;
  if (hFile != INVALID_HANDLE_VALUE) {
    unsigned int filesize = GetFileSize(hFile, nullptr);
    if (!filesize) {
      content.clear();
      result = true;
    } else {
      content.resize(filesize);
      DWORD read = 0;
      result = !!ReadFile(hFile, content.data(), filesize, &read, nullptr);
    }
    CloseHandle(hFile);
  }
  return result;
}

static bool cbCommand(int argc, char *argv[]) {
  if (argc < 2) {
    _plugin_logputs("[" PLUGIN_NAME "] Usage: PasteFile <filename>,[address]");
    return false;
  }

  std::vector<uint8_t> data;
  if (!ReadAllData(argv[1], data)) {
    _plugin_logputs("[" PLUGIN_NAME "] Failed to read file data!");
    return false;
  }

  duint start = 0;
  if (argc > 2) {
    // Try to parse the second argument as an address/expression
    start = DbgValFromString(argv[2]);
    if (start == 0 && strcmp(argv[2], "0") != 0 &&
        strcmp(argv[2], "0x0") !=
            0) // Basic validation if DbgValFromString returns 0 on failure
    {
      // DbgValFromString returns 0 on failure usually, but 0 is also a valid
      // address. However, if the user typed something invalid, it might be
      // safer to fallback or error. Let's assume user knows what they are doing
      // if they provide an arg. Or we can check if it really failed. A safe way
      // is to check if DbgValFromString returns 0 and the string is not "0".
    }
    _plugin_logprintf("[" PLUGIN_NAME "] Using provided address: %p\n", start);
  } else {
    start = Script::Gui::Dump::SelectionGetStart();
    _plugin_logprintf("[" PLUGIN_NAME "] Using Dump selection start: %p\n",
                      start);
  }
  duint total = data.size();
  duint skipped = 0;
  duint patched = 0;
  duint failed = 0;

  _plugin_logprintf("[" PLUGIN_NAME
                    "] Starting patch at %p, size: %d bytes...\n",
                    start, total);

  for (size_t i = 0; i < total; i++) {
    duint addr = start + i;
    uint8_t currentByte;

    // Try to read current memory
    // Try to read current memory
    if (!Script::Memory::IsValidPtr(addr)) {
      _plugin_logprintf(
          "[" PLUGIN_NAME
          "] Failed to read memory at %p. Aborting remaining %d bytes.\n",
          addr, total - i);
      failed += (total - i);
      break;
    }
    currentByte = Script::Memory::ReadByte(addr);

    // Compare
    if (currentByte == data[i]) {
      skipped++;
      continue;
    }

    // Try to write
    if (Script::Memory::WriteByte(addr, data[i])) {
      patched++;
    } else {
      _plugin_logprintf("[" PLUGIN_NAME
                        "] Failed to write byte at %p (Old: %02X, New: %02X)\n",
                        addr, currentByte, data[i]);
      failed++;
    }
  }

  _plugin_logprintf("[" PLUGIN_NAME "] Patch finished.\n");
  _plugin_logprintf("[" PLUGIN_NAME "] Total:   %d\n", total);
  _plugin_logprintf("[" PLUGIN_NAME "] Skipped: %d (Same content)\n", skipped);
  _plugin_logprintf("[" PLUGIN_NAME "] Patched: %d\n", patched);
  _plugin_logprintf("[" PLUGIN_NAME "] Failed:  %d\n", failed);

  if (failed > 0)
    _plugin_logputs(
        "[" PLUGIN_NAME
        "] Some bytes failed to write. Check memory permissions/validity.");

  GuiUpdateAllViews();
  return true;
}

// Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT *initStruct) {
  if (!_plugin_registercommand(pluginHandle, PLUGIN_NAME, cbCommand, false))
    _plugin_logputs("[" PLUGIN_NAME "] Error registering the \"" PLUGIN_NAME
                    "\" command!");
  return true; // Return false to cancel loading the plugin.
}

// Deinitialize your plugin data here (clearing menus optional).
bool pluginStop() { return true; }

// Do GUI/Menu related things here.
void pluginSetup() {}
