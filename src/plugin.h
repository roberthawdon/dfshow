#include "customtypes.h"
#include "plugin_interface.h"

typedef struct PluginNode {
  Plugin* plugin;
  struct PluginNode* next;
} PluginNode;

Plugin* load_plugin(const char* path);
void freePluginList(pluginList *loadedPlugins, int count);
void loadPluginsFromDirectory(const char* directory_path);
void registerPlugin(Plugin* plugin);
Plugin* findPluginByType(PluginDataType type);
void cleanupPluginRegistry();
