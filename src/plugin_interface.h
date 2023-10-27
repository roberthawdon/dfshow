#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

typedef void (*EventListener)(void* data);

// Deprecated, moving to AppEvents
typedef enum {
    PLUGIN_TYPE_NULL,
    PLUGIN_TYPE_POST_EXECUTION,
} PluginDataType;

typedef enum {
    EVENT_NULL,
    EVENT_MENU_ADDITION,
    EVENT_POST_EXECUTION,
} AppEvent;

typedef struct {
  AppEvent event;
  EventListener listener;
} EventEntry;

// typedef union {
//     const void;
//     const void* return_value;
// } PluginDataValue;

typedef struct {
    PluginDataType type;
//    PluginDataValue value;
    void* (*execute)();
} Plugin;

void registerEventListener(AppEvent event, EventListener listener);

#endif

