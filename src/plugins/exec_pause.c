#define PLUGIN_NAME "Pause on Exec exit"
#define PLUGIN_VERSION "0.0.4"
#define PLUGIN_AUTHOR L"Robert Ian Hawdon"

#include <stdio.h>
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

void* post_execution_prompt() {
    printf("\r\nPress any key to return to 'show'...\n");
    getchar();
    return 0;
}

Plugin* get_plugin() {
    static Plugin plugin = {PLUGIN_TYPE_POST_EXECUTION, post_execution_prompt};
    return &plugin;
}
