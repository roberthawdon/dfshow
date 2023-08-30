#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

typedef enum {
    PLUGIN_TYPE_NULL,
    PLUGIN_TYPE_POST_EXECUTION,
} PluginDataType;

// typedef union {
//     const void;
//     const void* return_value;
// } PluginDataValue;

typedef struct {
    PluginDataType type;
//    PluginDataValue value;
    void* (*execute)();
} Plugin;

#endif

