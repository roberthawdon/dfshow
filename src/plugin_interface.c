#include "plugin_interface.h"

extern int registryCount;

void registerEventListener(AppEvent event, EventListener listener) {
    eventRegistry[registryCount].event = event;
    eventRegistry[registryCount].listener = listener;
    registryCount++;
}

