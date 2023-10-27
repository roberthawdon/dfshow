#define PLUGIN_NAME "Pause on Exec exit"
#define PLUGIN_VERSION "0.0.13"
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

void consume_line() {
    char buffer[1024];  // Temporary buffer
    fgets(buffer, sizeof(buffer), stdin);
}

void handlePostExecution(void* data) {
    printf("\r\nPress enter to return to Show...\n");
    consume_line();
}

void initPlugin() {
    registerEventListener(EVENT_POST_EXECUTION, handlePostExecution);
}

// void* post_execution_prompt() {
//     printf("\r\nPress enter to return to Show...\n");
//     consume_line();
//     return 0;
// }
// 
// Plugin* get_plugin() {
//     static Plugin plugin = {PLUGIN_TYPE_POST_EXECUTION, post_execution_prompt};
//     return &plugin;
// }
