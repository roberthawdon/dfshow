#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

typedef enum {
    PLUGIN_STRING,
    PLUGIN_WIDE_STRING,
    PLUGIN_INTEGER
} PluginDataType;

typedef union {
    const char* string_value;
    const wchar_t* wide_value;
    int int_value;
} PluginDataValue;

typedef struct {
    PluginDataType type;
    PluginDataValue value;
} PluginResult;

typedef PluginResult (*plugin_func)();

#endif

