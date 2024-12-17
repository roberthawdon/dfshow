/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2024  Robert Ian Hawdon

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
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <libconfig.h>
#include <dirent.h>
#include <errno.h>
#include <libintl.h>
#include <locale.h>
#include "menu.h"
#include "display.h"
#include "settings.h"
#include "common.h"
#include "colors.h"
#include "input.h"
#include "banned.h"
#include "i18n.h"

int lightColorPair[256];

int colorThemePos = 0;
int totalItemCount = 16;

int selectedItem;

int bgToggle = 0;

int themeModified = 0;

colorPairs colors[256];

char fgbgLabel[11];

char *colorNames[17];

extern MEVENT event;

extern char *errmessage;

extern int colormode;
extern int c;
extern int * pc;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern int viewMode;

menuDef *colorMenu;
int colorMenuSize = 0;
wchar_t *colorMenuLabel;
menuButton *colorMenuButtons;

menuButton *themeBuilderButtons;

void processListThemes(const char * pathName)
{
  DIR *dfDir;
  struct dirent *res;
  char currentPath[1024];
  char currentFile[1024];
  config_t cfg;
  config_setting_t *group;
  memcpy(currentPath, pathName, (strlen(pathName) + 1));
  dfDir = opendir ( currentPath );
  if (access (currentPath, F_OK) != -1){
    if (dfDir){
      while ( ( res = readdir (dfDir))){
        snprintf(currentFile, 1024, "%s/%s", currentPath, res->d_name);
        if (!check_dir(currentFile)){
          config_init(&cfg);
          config_read_file(&cfg, currentFile);
          group = config_lookup(&cfg, "theme");
          if (group){
            printf(" - %s\n", objectFromPath(currentFile));
          }
        }
      }
      free(res);
    }
  }
}

void listThemes()
{
  if (check_dir(DATADIR)) {
    printf("\nGlobal Themes:\n");
    processListThemes(DATADIR);
  }
  if (check_dir(dirFromPath(homeConfLocation))) {
    printf("\nPersonal Themes:\n");
    processListThemes(dirFromPath(homeConfLocation));
  }
}

int itemLookup(int menuPos){
  switch(menuPos){
  case 0:
    selectedItem = COMMAND_PAIR;
    break;
  case 1:
    selectedItem = DISPLAY_PAIR;
    break;
  case 2:
    selectedItem = ERROR_PAIR;
    break;
  case 3:
    selectedItem = INFO_PAIR;
    break;
  case 4:
    selectedItem = HEADING_PAIR;
    break;
  case 5:
    selectedItem = DANGER_PAIR;
    break;
  case 6:
    selectedItem = SELECT_PAIR;
    break;
  case 7:
    selectedItem = HILITE_PAIR;
    break;
  case 8:
    selectedItem = INPUT_PAIR;
    break;
  case 9:
    selectedItem = DIR_PAIR;
    break;
  case 10:
    selectedItem = SLINK_PAIR;
    break;
  case 11:
    selectedItem = DEADLINK_PAIR;
    break;
  case 12:
    selectedItem = EXE_PAIR;
    break;
  case 13:
    selectedItem = SUID_PAIR;
    break;
  case 14:
    selectedItem = SGID_PAIR;
    break;
  case 15:
    selectedItem = STICKY_PAIR;
    break;
  case 16:
    selectedItem = STICKY_OW_PAIR;
    break;
  default:
    selectedItem = -1;
    break;
  }
  return(selectedItem);
}

void setColorPairs(int pair, int foreground, int background, int bold){
  switch(pair){
  case COMMAND_PAIR:
    snprintf(colors[pair].name, 24, "command");
    break;
  case INFO_PAIR:
    snprintf(colors[pair].name, 24, "info");
    break;
  case INPUT_PAIR:
    snprintf(colors[pair].name, 24, "input");
    break;
  case SELECT_PAIR:
    snprintf(colors[pair].name, 24, "select");
    break;
  case DISPLAY_PAIR:
    snprintf(colors[pair].name, 24, "display");
    break;
  case DANGER_PAIR:
    snprintf(colors[pair].name, 24, "danger");
    break;
  case DIR_PAIR:
    snprintf(colors[pair].name, 24, "dir");
    break;
  case SLINK_PAIR:
    snprintf(colors[pair].name, 24, "symlink");
    break;
  case EXE_PAIR:
    snprintf(colors[pair].name, 24, "exec");
    break;
  case SUID_PAIR:
    snprintf(colors[pair].name, 24, "suid");
    break;
  case SGID_PAIR:
    snprintf(colors[pair].name, 24, "sgid");
    break;
  case HILITE_PAIR:
    snprintf(colors[pair].name, 24, "hilite");
    break;
  case ERROR_PAIR:
    snprintf(colors[pair].name, 24, "error");
    break;
  case HEADING_PAIR:
    snprintf(colors[pair].name, 24, "heading");
    break;
  case DEADLINK_PAIR:
    snprintf(colors[pair].name, 24, "deadlink");
    break;
  case STICKY_PAIR:
    snprintf(colors[pair].name, 24, "sticky");
    break;
  case STICKY_OW_PAIR:
    snprintf(colors[pair].name, 24, "sticky-ow");
    break;
  default:
    snprintf(colors[pair].name, 24, "undef-%i", pair);
    break;
  }
  if (COLORS < 9){
  checkColor:
    if ( foreground > 8 ){
      foreground = foreground - 8;
      bold = 1;
      goto checkColor;
    }
    if ( background > 8 ){
      background = background - 8;
      goto checkColor;
    }
  }
  colors[pair].foreground = foreground;
  colors[pair].background = background;
  colors[pair].bold = bold;
}

int useTheme(const char * confFile)
{
  config_t cfg;
  config_setting_t *root, *setting, *group;
  int e;

  config_init(&cfg);
  config_read_file(&cfg, confFile);
  root = config_root_setting(&cfg);

  group = config_setting_get_member(root, "common");

  if (!group){
    group = config_setting_add(root, "common", CONFIG_TYPE_GROUP);
  }

  config_setting_remove(group, "theme");

  setting = config_setting_add(group, "theme", CONFIG_TYPE_STRING);

  config_setting_set_string(setting, getenv("DFS_THEME"));

  e = config_write_file(&cfg, confFile);

  return(e);

}

void refreshColors(){
  int i;
  int foreground, background, bold;
  for ( i = 0; i < 256; i++ ){
    foreground = colors[i].foreground;
    background = colors[i].background;
    bold = colors[i].bold;
    if (COLORS < 9){
    checkColor:
      if ( foreground > 8 ){
        foreground = foreground - 8;
        bold = 1;
        goto checkColor;
      }
      if ( background > 8 ){
        background = background - 8;
        goto checkColor;
      }
    }
    init_pair(i, foreground, background);
    lightColorPair[i] = bold;
  }
}

void saveTheme(){
  config_t cfg;
  config_setting_t *root, *setting, *group, *array;
  int e, f, i, r;
  char filename[1024];
  char * rewrite;
  int curPos=0;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0,0, _("Save Colors - Enter pathname:")) + 1);
  move(0,curPos);
  rewrite = malloc(sizeof(char) * (strlen(dirFromPath(homeConfLocation)) +1));
  snprintf(rewrite, (strlen(dirFromPath(homeConfLocation)) +1), "%s/", dirFromPath(homeConfLocation));
  e = readline(filename, 1024, rewrite);
  free(rewrite);
  if ( e == 0 ){
    if (check_first_char(filename, "~")){
      rewrite = str_replace(filename, "~", getenv("HOME"));
      memcpy(filename, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
    config_init(&cfg);
    root = config_root_setting(&cfg);
    group = config_setting_add(root, "theme", CONFIG_TYPE_GROUP);
    for (i = 1; i < (totalItemCount + 2); i++){
      array = config_setting_add(group, colors[i].name, CONFIG_TYPE_ARRAY);
      setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
      config_setting_set_int(setting, colors[i].foreground);
      setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
      config_setting_set_int(setting, colors[i].background);
      setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
      config_setting_set_int(setting, colors[i].bold);
    }
  saveTheme:
    if (access(dirFromPath(filename), W_OK) == 0){
      if (check_file(filename)){
        curs_set(FALSE);
        r = commonConfirmMenu(0,0, _("File exists. Replace?"), false, -1);
        while(1)
          {
            switch(r)
              {
                case YES:
                  config_write_file(&cfg, filename);
                  setenv("DFS_THEME", objectFromPath(filename), 1);
                  break;
                default:
                  break;
              }
            break;
          }
        curs_set(TRUE);
        themeBuilder();
      } else {
        config_write_file(&cfg, filename);
        setenv("DFS_THEME", objectFromPath(filename), 1);
        themeModified = 0;
      }
    } else {
      if (errno == ENOENT){
        // curs_set(FALSE);
        // mk_dir(dirFromPath(filename));
        // config_write_file(&cfg, filename);
        // // topLineMessage("Error: Unable to write file");
        // curs_set(TRUE);
        f = createParentsInput(dirFromPath(filename));
        if (f == 1){
          createParentDirs(filename);
          goto saveTheme;
        } else {
          setDynamicChar(&errmessage, _("Error: %s"), strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, _("Error: %s"), strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
    config_destroy(&cfg);
  }
  themeBuilder();
}

int applyTheme(const char *filename){
  config_t cfg;
  config_setting_t *setting, *group, *array;
  int groupLen, i, h;
  setenv("DFS_THEME", objectFromPath(filename), 1);
  config_init(&cfg);
  if (config_read_file(&cfg, filename)){
    group = config_lookup(&cfg, "theme");
    groupLen = config_setting_length(group);
    for (i = 0; i < groupLen; i++){
      array = config_setting_get_elem(group, i);
      for (h = 0; h < 256; h++){
        if (!strcmp(colors[h].name, config_setting_name(array))){
          setting = config_setting_get_member(group, config_setting_name(array));
          colors[h].foreground = config_setting_get_int_elem(setting, 0);
          colors[h].background = config_setting_get_int_elem(setting, 1);
          colors[h].bold = config_setting_get_int_elem(setting, 2);
        }
      }

    }
    config_destroy(&cfg);
    refreshColors();
    return(0);
  } else {
    topLineMessage(_("Error parsing theme file."));
    return(1);
  }
  return(1);
}

void loadTheme(){
  int e;
  char filename[1024];
  char * rewrite;
  int curPos = 0;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0,0, _("Load Colors - Enter pathname:")) + 1 );
  move(0,curPos);
  rewrite = malloc(sizeof(char) * (strlen(dirFromPath(homeConfLocation)) + 1));
  snprintf(rewrite, (strlen(dirFromPath(homeConfLocation)) + 1), "%s/", dirFromPath(homeConfLocation));
  e = readline(filename, 1024, rewrite);
  free(rewrite);
  if ( e == 0 ){
    if (check_first_char(filename, "~")){
      rewrite = str_replace(filename, "~", getenv("HOME"));
      memcpy(filename, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
    if (check_file(filename) ){
      setenv("DFS_THEME_OVERRIDE", "TRUE", 1);
      themeModified = 0;
      applyTheme(filename);
    } else {
      curs_set(FALSE);
      topLineMessage(_("Error: Unable to read file"));
      // curs_set(TRUE);
    }
  }
  themeBuilder();
}

void loadAppTheme(const char *themeName)
{
  char * rewrite;
  // Ignore if the theme requested is called 'default'
  if (strcmp(themeName, "default") && strcmp(themeName, "\0")){
      rewrite = malloc(sizeof(char) * (strlen(dirFromPath(homeConfLocation)) + strlen(themeName) + 2));
      snprintf(rewrite, (strlen(dirFromPath(homeConfLocation)) + strlen(themeName) + 2), "%s/%s", dirFromPath(homeConfLocation), themeName);
      if (check_file(rewrite)){
        applyTheme(rewrite);
      } else {
        free(rewrite);
        rewrite = malloc(sizeof(char) * (strlen(DATADIR) + strlen(themeName) + 2));
        snprintf(rewrite, (strlen(DATADIR) + strlen(themeName) + 2), "%s/%s", DATADIR, themeName);
        if (check_file(rewrite)){
          applyTheme(rewrite);
        }
      }
      free(rewrite);
  } else {
    // Assume default
    setenv("DFS_THEME", "default", 1);
  }
}

void updateColorPair(int code, int location){
  int colorCode = -1;
  int colorBold = 0;
  themeModified = 1;
  switch(code){
  case 0:
    colorCode = COLOR_BLACK;
    break;
  case 1:
    colorCode = COLOR_RED;
    break;
  case 2:
    colorCode = COLOR_GREEN;
    break;
  case 3:
    colorCode = COLOR_YELLOW;
    break;
  case 4:
    colorCode = COLOR_BLUE;
    break;
  case 5:
    colorCode = COLOR_MAGENTA;
    break;
  case 6:
    colorCode = COLOR_CYAN;
    break;
  case 7:
    colorCode = COLOR_WHITE;
    break;
  case 8:
    if (COLORS > 8){
      colorCode = BRIGHT_BLACK;
    } else {
      colorCode = COLOR_BLACK;
      colorBold = 1;
    }
    break;
  case 9:
    if (COLORS > 8){
      colorCode = BRIGHT_RED;
    } else {
      colorCode = COLOR_RED;
      colorBold = 1;
    }
    break;
  case 10:
    if (COLORS > 8){
      colorCode = BRIGHT_GREEN;
    } else {
      colorCode = COLOR_GREEN;
      colorBold = 1;
    }
    break;
  case 11:
    if (COLORS > 8){
      colorCode = BRIGHT_YELLOW;
    } else {
      colorCode = COLOR_YELLOW;
      colorBold = 1;
    }
    break;
  case 12:
    if (COLORS > 8){
      colorCode = BRIGHT_BLUE;
    } else {
      colorCode = COLOR_BLUE;
      colorBold = 1;
    }
    break;
  case 13:
    if (COLORS > 8){
      colorCode = BRIGHT_MAGENTA;
    } else {
      colorCode = COLOR_MAGENTA;
      colorBold = 1;
    }
    break;
  case 14:
    if (COLORS > 8){
      colorCode = BRIGHT_CYAN;
    } else {
      colorCode = COLOR_CYAN;
      colorBold = 1;
    }
    break;
  case 15:
    if (COLORS > 8){
      colorCode = BRIGHT_WHITE;
    } else {
      colorCode = COLOR_WHITE;
      colorBold = 1;
    }
    break;
  case -1:
    colorCode = DEFAULT_COLOR;
    break;
  case -2:
    colorCode = DEFAULT_COLOR;
    colorBold = 1;
    break;
  }
  if (location == 0) {
    colors[itemLookup(colorThemePos)].foreground = colorCode;
  } else {
    colors[itemLookup(colorThemePos)].background = colorCode;
  }
  colors[itemLookup(colorThemePos)].bold = colorBold;
}

void theme_menu_inputs()
{
  char useThemeMessage[256];
  while(1)
    {
      *pc = getch10th();
      loop:
      if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON1_PRESSED){
          if (event.y == 0){
            // Setting key based on click
            *pc = menuHotkeyLookup(colorMenu, (menuButtonLookup(colorMenuButtons, colorMenuSize, event.x, event.y, 0, 0, true)), colorMenuSize);
            goto loop;
          } else {
            *pc = themeButtonAction(menuButtonLookup(themeBuilderButtons, 35, event.x, event.y, 0, 0, false));
            goto loop;
          }
        } else if (event.bstate & BUTTON5_PRESSED){
          goto down;
        } else if (event.bstate & BUTTON4_PRESSED){
          goto up;
        }
      } else if (*pc == '!'){
        updateColorPair(-1, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '?'){
        updateColorPair(-2, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '0'){
        updateColorPair(COLOR_BLACK, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '1'){
        updateColorPair(COLOR_RED, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '2'){
        updateColorPair(COLOR_GREEN, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '3'){
        updateColorPair(COLOR_YELLOW, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '4'){
        updateColorPair(COLOR_BLUE, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '5'){
        updateColorPair(COLOR_MAGENTA, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '6'){
        updateColorPair(COLOR_CYAN, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '7'){
        updateColorPair(COLOR_WHITE, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '8'){
        updateColorPair(BRIGHT_BLACK, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == '9'){
        updateColorPair(BRIGHT_RED, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'a'){
        updateColorPair(BRIGHT_GREEN, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'b'){
        updateColorPair(BRIGHT_YELLOW, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'c'){
        updateColorPair(BRIGHT_BLUE, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'd'){
        updateColorPair(BRIGHT_MAGENTA, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'e'){
        updateColorPair(BRIGHT_CYAN, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == 'f'){
        updateColorPair(BRIGHT_WHITE, bgToggle);
        refreshColors();
        themeBuilder();
      } else if (*pc == menuHotkeyLookup(colorMenu, "c_load", colorMenuSize)){
        loadTheme();
      } else if (*pc == menuHotkeyLookup(colorMenu, "c_quit", colorMenuSize)){
        curs_set(FALSE);
        return;
      } else if (*pc == menuHotkeyLookup(colorMenu, "c_save", colorMenuSize)){
        saveTheme();
      } else if (*pc == menuHotkeyLookup(colorMenu, "c_toggle", colorMenuSize)){
        if (bgToggle == 0){
          bgToggle = 1;
        } else {
          bgToggle = 0;
        }
        themeBuilder();
      } else if (*pc == menuHotkeyLookup(colorMenu, "c_use", colorMenuSize)){
        //
        if (themeModified == 1){
          curs_set(FALSE);
          topLineMessage(_("Save theme before using as default."));
        } else {
          if (access(dirFromPath(homeConfLocation), W_OK) != 0) {
            createParentDirs(homeConfLocation);
          }
          if (useTheme(homeConfLocation)){
            snprintf(useThemeMessage, 256, _("Default theme has been set to [%s]."), getenv("DFS_THEME"));
            topLineMessage(useThemeMessage);
          } else {
            topLineMessage(_("An error occurred setting the default theme."));
          }
        }
        themeBuilder();
      } else if (*pc == 258 || *pc ==10){
        down:
        if (colorThemePos < totalItemCount){
          colorThemePos++;
          setCursorPos(colorThemePos - 1);
        }
      } else if (*pc == 259){
        up:
        if (colorThemePos > 0) {
          colorThemePos--;
          setCursorPos(colorThemePos + 1);
        }
      } else if (*pc == 260 || *pc == 261){
        // Do Nothing
      }
    }
}

int themeButtonAction(const char * refLabel){
  int prevColorPos = 0;
  int newPos = -1;
  int output = -1;

  if (!strncmp(refLabel, "colorThemePos", 13)){
    sscanf(refLabel + 13, "%i", &newPos);
    prevColorPos = colorThemePos;
    colorThemePos = newPos;
    setCursorPos(prevColorPos);
    return output;
  } else {
    output = refLabel[0];
  }

  return output;
}

void addColorButton(int index, const char * refLabel, const char * label, int topX, int topY){
  snprintf(themeBuilderButtons[index].refLabel, 16, "%s", refLabel);
  themeBuilderButtons[index].topX = topX;
  themeBuilderButtons[index].bottomX = topX + strlen(label) - 1;
  themeBuilderButtons[index].topY = themeBuilderButtons[index].bottomY = topY;
  mvprintw(topY, topX, "%s", label);
}

void setDefaultTheme(){
  use_default_colors();
  lightColorPair[0] = 0; // Unused array value
  setColorPairs(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
  setColorPairs(INFO_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
  setColorPairs(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
  setColorPairs(SELECT_PAIR, BRIGHT_BLUE, DEFAULT_COLOR, 0);
  setColorPairs(DISPLAY_PAIR, COLOR_CYAN, DEFAULT_COLOR, 0);
  setColorPairs(DANGER_PAIR, BRIGHT_RED, DEFAULT_COLOR, 0);
  setColorPairs(DIR_PAIR, BRIGHT_MAGENTA, DEFAULT_COLOR, 0);
  setColorPairs(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(EXE_PAIR, BRIGHT_YELLOW, DEFAULT_COLOR, 0);
  setColorPairs(SUID_PAIR, DEFAULT_COLOR, COLOR_RED, 0);
  setColorPairs(SGID_PAIR, COLOR_BLACK, COLOR_GREEN, 1);
  setColorPairs(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(HEADING_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
  setColorPairs(DEADLINK_PAIR, BRIGHT_RED, DEFAULT_COLOR, 0);
  setColorPairs(STICKY_PAIR, COLOR_WHITE, COLOR_BLUE, 0);
  setColorPairs(STICKY_OW_PAIR, COLOR_BLACK, COLOR_GREEN, 0);

  setColorPairs(COLORMENU_PAIR_0, COLOR_BLACK, COLOR_WHITE, 0);
  setColorPairs(COLORMENU_PAIR_1, COLOR_RED, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_2, COLOR_GREEN, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_3, COLOR_YELLOW, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_4, COLOR_BLUE, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_5, COLOR_MAGENTA, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_6, COLOR_CYAN, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_7, COLOR_WHITE, DEFAULT_COLOR, 0);
  if (COLORS > 8){
    setColorPairs(COLORMENU_PAIR_8, BRIGHT_BLACK, COLOR_WHITE, 0);
    setColorPairs(COLORMENU_PAIR_9, BRIGHT_RED, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_A, BRIGHT_GREEN, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_B, BRIGHT_YELLOW, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_C, BRIGHT_BLUE, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_D, BRIGHT_MAGENTA, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_E, BRIGHT_CYAN, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_F, BRIGHT_WHITE, DEFAULT_COLOR, 0);
  } else {
    setColorPairs(COLORMENU_PAIR_8, COLOR_BLACK, COLOR_WHITE, 1);
    setColorPairs(COLORMENU_PAIR_9, COLOR_RED, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_A, COLOR_GREEN, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_B, COLOR_YELLOW, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_C, COLOR_BLUE, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_D, COLOR_MAGENTA, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_E, COLOR_CYAN, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_F, COLOR_WHITE, DEFAULT_COLOR, 1);
  }

  setColorPairs(DEFAULT_BOLD_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(DEFAULT_COLOR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);

  refreshColors();

}

void setColors(int pair)
{
  attron(COLOR_PAIR(pair));
  if (lightColorPair[pair]){
    attron(A_BOLD);
  } else {
    attroff(A_BOLD);
  }
}

void setCursorPos(int prev)
{
  setColors(DISPLAY_PAIR);
  if (prev >= 0){
    // attroff(A_REVERSE);
    mvprintw(prev + 2, 1, " ");
  }
  attron(A_REVERSE);
  mvprintw(colorThemePos + 2, 1, " ");
  attroff(A_REVERSE);
}

void themeBuilder()
{
  int i, x, y, b, tipMessageWidth, colorNameWidth, colorNameItems;
  // size_t colorNameWidthSize;
  char *tipMessage;

  colorNameItems = 0;

  colorNames[0]  = _("Command lines");
  colorNames[1]  = _("Display lines");
  colorNames[2]  = _("Error messages");
  colorNames[3]  = _("Information lines");
  colorNames[4]  = _("Heading lines");
  colorNames[5]  = _("Danger lines");
  colorNames[6]  = _("Selected block lines");
  colorNames[7]  = _("Highlight");
  colorNames[8]  = _("Text input");
  colorNames[9]  = _("Directories");
  colorNames[10] = _("Symbolic links");
  colorNames[11] = _("Orphened symbolic links");
  colorNames[12] = _("Executable files");
  colorNames[13] = _("Set user identification");
  colorNames[14] = _("Set group identification");
  colorNames[15] = _("Sticky bit directory");
  colorNames[16] = _("Sticky bit directory - other writable");

  colorNameItems = sizeof(colorNames)/sizeof(colorNames[0]);

  if (themeBuilderButtons){
    free(themeBuilderButtons);
  }

  themeBuilderButtons = malloc(sizeof(menuButton) * (colorNameItems + 18));

  viewMode = 2;
  clear();
  if (bgToggle){
    snprintf(fgbgLabel, 11, _("background"));
  } else {
    snprintf(fgbgLabel, 11, _("foreground"));
  }
  wPrintMenu(0,0,colorMenuLabel);

  colorNameWidth = 0;
  for (i = 0; i < colorNameItems; i++){
    setColors(i + 1);
    mvprintw(i + 2, 4, "%s", colorNames[i]);
    b = strlen(colorNames[i]) - 1;
    snprintf(themeBuilderButtons[i].refLabel, 16, "colorThemePos%i", i);
    themeBuilderButtons[i].topX = 4;
    themeBuilderButtons[i].bottomX = 4 + b;
    themeBuilderButtons[i].topY = themeBuilderButtons[i].bottomY = i + 2;
    if (b > colorNameWidth){
      colorNameWidth = strlen(colorNames[i]);
    }
  }

  if ((COLS / 2) < (colorNameWidth + 12)){
    x = colorNameWidth + 12;
  } else {
    x = COLS / 2;
  }

  i = colorNameItems;
  b = 2;
  setColors(DEFAULT_COLOR_PAIR);
  addColorButton(i, "!", _("!-Default      "), x, b); i++; b++;
  setColors(DEFAULT_BOLD_PAIR);
  addColorButton(i, "?", _("?-Default Bold "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_0);
  addColorButton(i, "0", _("0-Black        "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_1);
  addColorButton(i, "1", _("1-Red          "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_2);
  addColorButton(i, "2", _("2-Green        "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_3);
  addColorButton(i, "3", _("3-Brown        "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_4);
  addColorButton(i, "4", _("4-Blue         "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_5);
  addColorButton(i, "5", _("5-Magenta      "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_6);
  addColorButton(i, "6", _("6-Cyan         "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_7);
  addColorButton(i, "7", _("7-Light Gray   "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_8);
  addColorButton(i, "8", _("8-Dark Gray    "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_9);
  addColorButton(i, "9", _("9-Light Red    "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_A);
  addColorButton(i, "a", _("A-Light Green  "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_B);
  addColorButton(i, "b", _("B-Yellow       "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_C);
  addColorButton(i, "c", _("C-Light Blue   "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_D);
  addColorButton(i, "d", _("D-Light Magenta"), x, b); i++; b++;
  setColors(COLORMENU_PAIR_E);
  addColorButton(i, "e", _("E-Light Cyan   "), x, b); i++; b++;
  setColors(COLORMENU_PAIR_F);
  addColorButton(i, "f", _("F-White        "), x, b); i++; b++;

  y = LINES - 4;

  if (y <= 22){
    y = 22;
  }

  tipMessageWidth = setDynamicChar(&tipMessage, _("Select 0 to F for desired %s color"), fgbgLabel);

  if (COLS < tipMessageWidth){
    x = 0;
  } else {
    x = ((COLS / 2) - (tipMessageWidth / 2));
  }


  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(y, x, "%s", tipMessage);


  setCursorPos(-1);

}
