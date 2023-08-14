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
#include "banned.h"
#include "plugin_interface.h"
#include "customtypes.h"
#include "common.h"

int pluginCount = -1;

pluginList *loadedPlugins;

plugin_func load_plugin(const char* path) {
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

    plugin_func (*get_plugin)() = (plugin_func (*)())dlsym(handle, "get_plugin");
    if (!get_plugin) {
        dlclose(handle);
        return NULL;
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
