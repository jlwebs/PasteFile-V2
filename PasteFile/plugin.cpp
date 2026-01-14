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
    _plugin_logputs("[" PLUGIN_NAME
                    "] Usage: PasteFile <filename>,[address] [end_address]");
    return false;
  }

  _plugin_logprintf("[" PLUGIN_NAME "] Command args count: %d\n", argc);
  for (int k = 0; k < argc; k++)
    _plugin_logprintf("Arg%d: '%s'\n", k, argv[k]);

  char *filename = argv[1];
  char *addrStr = nullptr;
  char *endAddrStr = nullptr;

  if (argc == 2) {
    // Try to split by comma: file,addr[,end]
    char *comma = strchr(argv[1], ',');
    if (comma) {
      *comma = 0;
      filename = argv[1];
      addrStr = comma + 1;

      char *comma2 = strchr(addrStr, ',');
      if (comma2) {
        *comma2 = 0;
        endAddrStr = comma2 + 1;
      }
      _plugin_logprintf("[" PLUGIN_NAME "] Detected comma-separated args.\n");
    }
    // Fallback: check for space (if user quoted "file addr")
    else {
      char *space = strrchr(argv[1], ' ');
      if (space) {
        *space = 0;
        filename = argv[1];
        addrStr = space + 1;
      }
    }
  } else {
    // Standard argv parsing: PasteFile file [addr] [end]
    if (argc > 2)
      addrStr = argv[2];
    if (argc > 3)
      endAddrStr = argv[3];
  }

  std::vector<uint8_t> data;
  if (!ReadAllData(filename, data)) {
    _plugin_logprintf("[" PLUGIN_NAME "] Failed to read file.\n");
    _plugin_logprintf("Target File: \"%s\"\n", filename);
    return false;
  }

  duint start = 0;
  if (addrStr) {
    start = DbgValFromString(addrStr);
    _plugin_logprintf("[" PLUGIN_NAME "] Using provided address: %p\n", start);
  } else {
    start = Script::Gui::Dump::SelectionGetStart();
    _plugin_logprintf("[" PLUGIN_NAME "] Using Dump selection start: %p\n",
                      start);
  }

  duint endLimit = (duint)-1;
  if (endAddrStr) {
    endLimit = DbgValFromString(endAddrStr);
    _plugin_logprintf("[" PLUGIN_NAME "] Using ending limit: %p\n", endLimit);
  }
  duint size = data.size();
  duint truncatedBytes = 0;

  if (endLimit != (duint)-1 && (start + size) > endLimit) {
    if (start >= endLimit) {
      _plugin_logprintf(
          "[" PLUGIN_NAME
          "] Start address %p is beyond end limit %p. Nothing to patch.\n",
          start, endLimit);
      return true;
    }
    duint newSize = endLimit - start;
    truncatedBytes = size - newSize;
    size = newSize;
    _plugin_logprintf(
        "[" PLUGIN_NAME
        "] Truncating patch size from %d to %d bytes (Limit: %p)\n",
        data.size(), size, endLimit);
  }

  _plugin_logprintf("[" PLUGIN_NAME "] Patching %d bytes at %p...\n", size,
                    start);

  if (DbgFunctions()->MemPatch(start, data.data(), size)) {
    _plugin_logprintf("[" PLUGIN_NAME "] Patch successful!\n");
    if (truncatedBytes > 0)
      _plugin_logprintf("[" PLUGIN_NAME "] Warning: %d bytes were truncated.\n",
                        truncatedBytes);
  } else {
    _plugin_logputs(
        "[" PLUGIN_NAME
        "] Patch failed! (DbgFunctions()->MemPatch returned false)");
    if (endLimit == (duint)-1) {
      _plugin_logputs("[" PLUGIN_NAME
                      "] Note: No end address was specified. The patch might "
                      "have attempted to write to invalid memory.");
    }
  }

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
