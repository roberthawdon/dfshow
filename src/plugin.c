/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2023  Robert Ian Hawdon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include "banned.h"
#include "plugin.h"
#include "plugin_interface.h"
#include "customtypes.h"
#include "common.h"

int pluginCount = -1;

PluginNode* pluginRegistry = NULL;

extern bool plugins;

pluginList *loadedPlugins;

Plugin* load_plugin(const char* path) {
    // Check if the loadedPlugins list has been allocated and act accordingly
    if (pluginCount == -1){
        loadedPlugins = malloc(sizeof(pluginList) *2);
    } else {
        loadedPlugins = realloc(loadedPlugins, (pluginCount + 1) * (sizeof(pluginList) + 1));
    }

    // Check if the file has the .so extension
    if (strcmp(path + strlen(path) - 3, ".so") != 0) {
        return NULL;
    }

    void* handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        return NULL;
    }

    pluginCount++;

    // Retrieve the name, version, and author of the plugin
    const char* (*get_name)() = (const char* (*)())dlsym(handle, "get_name");
    const char* (*get_version)() = (const char* (*)())dlsym(handle, "get_version");
    const wchar_t* (*get_author)() = (const wchar_t* (*)())dlsym(handle, "get_author");

    if (get_name && get_version && get_author) {
        setDynamicChar(&loadedPlugins[pluginCount].pluginName, "%s", get_name());
        setDynamicWChar(&loadedPlugins[pluginCount].pluginAuthor, L"%ls", get_author());
        snprintf(loadedPlugins[pluginCount].pluginVersion, 16, "%s", get_version());
    }

    Plugin* (*get_plugin)() = (Plugin* (*)())dlsym(handle, "get_plugin");
    if (!get_plugin) {
        dlclose(handle);
        return NULL;
    }

    if (pluginCount > -1) {
        plugins = true;
    }

    return get_plugin();
}

void freePluginList(pluginList *loadedPlugins, int count) {
    int i;
    for (i = 0; i < count; i++){
        free(loadedPlugins[i].pluginName);
        free(loadedPlugins[i].pluginAuthor);
    }
    free(loadedPlugins);
}

void loadPluginsFromDirectory(const char* directory_path) {
    DIR* dir = opendir(directory_path);
    if (!dir) {
        // perror("Failed to open directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if the loadedPlugins list has been allocated and act accordingly
        if (pluginCount == -1){
            loadedPlugins = malloc(sizeof(pluginList) *2);
        } else {
            loadedPlugins = realloc(loadedPlugins, (pluginCount + 1) * (sizeof(pluginList) + 1));
        }

        // Check if the file has the .so extension
        if (strstr(entry->d_name, ".so")) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);

            void* handle = dlopen(full_path, RTLD_LAZY);
            if (!handle) {
                // fprintf(stderr, "Failed to load plugin %s: %s\n", full_path, dlerror());
                continue;
            }

            // Attempt to retrieve the plugin function and execute it
            Plugin* (*get_plugin)() = (Plugin* (*)())dlsym(handle, "get_plugin");
            if (get_plugin) {

                pluginCount++;

                // Retrieve the name, version, and author of the plugin
                const char* (*get_name)() = (const char* (*)())dlsym(handle, "get_name");
                const char* (*get_version)() = (const char* (*)())dlsym(handle, "get_version");
                const wchar_t* (*get_author)() = (const wchar_t* (*)())dlsym(handle, "get_author");

                if (get_name && get_version && get_author) {
                    setDynamicChar(&loadedPlugins[pluginCount].pluginName, "%s", get_name());
                    setDynamicWChar(&loadedPlugins[pluginCount].pluginAuthor, L"%ls", get_author());
                    snprintf(loadedPlugins[pluginCount].pluginVersion, 16, "%s", get_version());
                }

                Plugin* plugin = get_plugin();
                registerPlugin(plugin);
                // Now you can use the plugin, e.g., execute it, register it, etc.
            } else {
                // fprintf(stderr, "Failed to find get_plugin function in %s\n", full_path);
            }
        }
    }
    closedir(dir);

    if (pluginCount > -1) {
        plugins = true;
    }
}

void registerPlugin(Plugin* plugin) {
    PluginNode* newNode = malloc(sizeof(PluginNode));
    newNode->plugin = plugin;
    newNode->next = pluginRegistry;
    pluginRegistry = newNode;  // Add to the front for simplicity.
}

PluginEntry* findPluginsByType(PluginDataType type) {
    PluginEntry* result = NULL;
    PluginEntry* currentResult = NULL;

    PluginNode* current = pluginRegistry;
    while (current) {
        if (current->plugin->type == type) {
            PluginEntry* newEntry = malloc(sizeof(PluginEntry));
            newEntry->plugin = current->plugin;
            newEntry->next = NULL;

            if (!result) {
                result = newEntry;
                currentResult = result;
            } else {
                currentResult->next = newEntry;
                currentResult = newEntry;
            }
        }
        current = current->next;
    }
    return result;
}

void freePluginEntry(PluginEntry* entry){
    while (entry) {
        PluginEntry* next = entry->next;
        free(entry);
        entry = next;
    }
}

void cleanupPluginRegistry() {
    PluginNode* current = pluginRegistry;
    while (current) {
        PluginNode* toFree = current;
        current = current->next;
        free(toFree);
    }
}
