#define PLUGIN_NAME "Dummy Plugin"
#define PLUGIN_VERSION "1.0.0"
#define PLUGIN_AUTHOR L"Robert Ian Hawdon"

#include <wchar.h>
#include "../plugin_interface.h"

const char* get_name() {
    return PLUGIN_NAME;
}

const char* get_version() {
    return PLUGIN_VERSION;
}

const wchar_t* get_author() {
    return PLUGIN_AUTHOR;
}

void* execute_plugin() {
    return 0;
}

PluginResult* get_plugin() {
    static PluginResult plugin = {PLUGIN_INTEGER, execute_plugin};
    return &plugin;
}
