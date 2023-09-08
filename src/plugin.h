#include "customtypes.h"
#include "plugin_interface.h"

typedef struct PluginNode {
  Plugin* plugin;
  struct PluginNode* next;
} PluginNode;

typedef struct PluginEntry {
  Plugin* plugin;
  struct PluginEntry* next;
} PluginEntry;

Plugin* load_plugin(const char* path);
void freePluginList(pluginList *loadedPlugins, int count);
void loadPluginsFromDirectory(const char* directory_path);
void registerPlugin(Plugin* plugin);
PluginEntry* findPluginsByType(PluginDataType type);
void cleanupPluginRegistry();
void freePluginEntry(PluginEntry* entry);
