/*
  DF-SHOW - A clone of 'SHOW' directory browser from DF-EDIT by Larry Kroeker
  Copyright (C) 2018-2019  Robert Ian Hawdon

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
#include "common.h"
#include "colors.h"

int lightColorPair[256];
// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

int colorThemePos = 0;
int totalItemCount = 14;

int selectedItem;

int bgToggle = 0;

colorPairs colors[256];

char fgbgLabel[11];

char errmessage[256];

extern int colormode;
extern int c;
extern int * pc;

extern char globalConfLocation[128];
extern char homeConfLocation[128];

menuDef *colorMenu;
int colorMenuSize = 0;
wchar_t *colorMenuLabel;

void processListThemes(const char * pathName)
{
  DIR *dfDir;
  struct dirent *res;
  char currentPath[1024];
  char currentFile[1024];
  config_t cfg;
  config_setting_t *group;
  strcpy(currentPath, pathName);
  dfDir = opendir ( currentPath );
  if (access (currentPath, F_OK) != -1){
    if (dfDir){
      while ( ( res = readdir (dfDir))){
        sprintf(currentFile, "%s/%s", currentPath, res->d_name);
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
  default:
    selectedItem = -1;
    break;
  }
  return(selectedItem);
}

void setColorPairs(int pair, int foreground, int background, int bold){
  switch(pair){
  case 1:
    strcpy(colors[pair].name, "command");
    break;
  case 2:
    strcpy(colors[pair].name, "info");
    break;
  case 3:
    strcpy(colors[pair].name, "input");
    break;
  case 4:
    strcpy(colors[pair].name, "select");
    break;
  case 5:
    strcpy(colors[pair].name, "display");
    break;
  case 6:
    strcpy(colors[pair].name, "danger");
    break;
  case 7:
    strcpy(colors[pair].name, "dir");
    break;
  case 8:
    strcpy(colors[pair].name, "symlink");
    break;
  case 9:
    strcpy(colors[pair].name, "exec");
    break;
  case 10:
    strcpy(colors[pair].name, "suid");
    break;
  case 11:
    strcpy(colors[pair].name, "sgid");
    break;
  case 12:
    strcpy(colors[pair].name, "hilite");
    break;
  case 13:
    strcpy(colors[pair].name, "error");
    break;
  case 14:
    strcpy(colors[pair].name, "heading");
    break;
  case 15:
    strcpy(colors[pair].name, "deadlink");
    break;
  default:
    sprintf(colors[pair].name, "undef-%i", pair);
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
  int e, f, i;
  char filename[1024];
  char * rewrite;
  move(0,0);
  clrtoeol();
  printMenu(0,0, "Save Colors - Enter pathname:");
  move(0,30);
  rewrite = malloc(sizeof(char) * strlen(dirFromPath(homeConfLocation) +1));
  sprintf(rewrite, "%s/", dirFromPath(homeConfLocation));
  e = readline(filename, 1024, rewrite);
  free(rewrite);
  if ( e == 0 ){
    if (check_first_char(filename, "~")){
      rewrite = str_replace(filename, "~", getenv("HOME"));
      strcpy(filename, rewrite);
      free(rewrite);
    }
    config_init(&cfg);
    root = config_root_setting(&cfg);
    group = config_setting_add(root, "theme", CONFIG_TYPE_GROUP);
    for (i = 1; i < 16; i++){
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
        printMenu(0,0, "File exists. Replace? (!Yes/!No)");
        while(1)
          {
            *pc = getch();
            switch(*pc)
              {
              case 'y':
                config_write_file(&cfg, filename);
                setenv("DFS_THEME", objectFromPath(filename), 1);
                //No Break, drop through to default
              default:
                curs_set(TRUE);
                themeBuilder();
                break;
              }
            break;
          }
      } else {
        config_write_file(&cfg, filename);
        setenv("DFS_THEME", objectFromPath(filename), 1);
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
          sprintf(errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
        }
      } else {
        sprintf(errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
      }
    }
    config_destroy(&cfg);
  }
  themeBuilder();
}

int applyTheme(const char *filename){
  config_t cfg;
  config_setting_t *root, *setting, *group, *array;
  int groupLen, i, h;
  setenv("DFS_THEME", objectFromPath(filename), 1);
  config_init(&cfg);
  config_read_file(&cfg, filename);
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
}

void loadTheme(){
  int e;
  char filename[1024];
  char * rewrite;
  move(0,0);
  clrtoeol();
  printMenu(0,0, "Load Colors - Enter pathname:");
  move(0,30);
  rewrite = malloc(sizeof(char) * strlen(dirFromPath(homeConfLocation) +1));
  sprintf(rewrite, "%s/", dirFromPath(homeConfLocation));
  e = readline(filename, 1024, rewrite);
  free(rewrite);
  if ( e == 0 ){
    if (check_first_char(filename, "~")){
      rewrite = str_replace(filename, "~", getenv("HOME"));
      strcpy(filename, rewrite);
      free(rewrite);
    }
    if (check_file(filename) ){
      setenv("DFS_THEME_OVERRIDE", "TRUE", 1);
      applyTheme(filename);
    } else {
      curs_set(FALSE);
      topLineMessage("Error: Unable to read file");
      curs_set(TRUE);
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
      sprintf(rewrite, "%s/%s", dirFromPath(homeConfLocation), themeName);
      if (check_file(rewrite)){
        applyTheme(rewrite);
      } else {
        free(rewrite);
        rewrite = malloc(sizeof(char) * (strlen(DATADIR) + strlen(themeName) + 2));
        sprintf(rewrite, "%s/%s", DATADIR, themeName);
        if (check_file(rewrite)){
          applyTheme(rewrite);
        }
      }
      free(rewrite);
    }
}

void updateColorPair(int code, int location){
  int colorCode = -1;
  int colorBold = 0;
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
  while(1)
    {
      *pc = getch10th();
      if (*pc == '!'){
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
      } else if (*pc == 258 || *pc ==10){
        if (colorThemePos < totalItemCount){
          colorThemePos++;
          themeBuilder();
        }
      } else if (*pc == 259){
        if (colorThemePos > 0) {
          colorThemePos--;
          themeBuilder();
        }
      } else if (*pc == 260 || *pc == 261){
        // Do Nothing
      }
    }
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
  setColorPairs(SGID_PAIR, BRIGHT_BLACK, COLOR_GREEN, 0);
  setColorPairs(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(HEADING_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
  setColorPairs(DEADLINK_PAIR, BRIGHT_RED, DEFAULT_COLOR, 0);

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
  // endwin();
  // printf("CP: %i\n", pair);
  attron(COLOR_PAIR(pair));
  if (lightColorPair[pair]){
    attron(A_BOLD);
  } else {
    attroff(A_BOLD);
  }
}

void themeBuilder()
{
  clear();
  if (bgToggle){
    strcpy(fgbgLabel, "background");
  } else {
    strcpy(fgbgLabel, "foreground");
  }
  wPrintMenu(0,0,colorMenuLabel);

  setColors(COMMAND_PAIR);
  mvprintw(2, 4, "Command lines");
  setColors(DISPLAY_PAIR);
  mvprintw(3, 4, "Display lines");
  setColors(ERROR_PAIR);
  mvprintw(4, 4, "Error messages");
  setColors(INFO_PAIR);
  mvprintw(5, 4, "Information lines");
  setColors(HEADING_PAIR);
  mvprintw(6, 4, "Heading lines");
  setColors(DANGER_PAIR);
  mvprintw(7, 4, "Danger lines");
  setColors(SELECT_PAIR);
  mvprintw(8, 4, "Selected block lines");
  setColors(HILITE_PAIR);
  mvprintw(9, 4, "Highlight");
  setColors(INPUT_PAIR);
  mvprintw(10, 4, "Text input");
  setColors(DIR_PAIR);
  mvprintw(11, 4, "Directories");
  setColors(SLINK_PAIR);
  mvprintw(12, 4, "Valid symbolic links");
  setColors(DEADLINK_PAIR);
  mvprintw(13, 4, "Invalid symbolic links");
  setColors(EXE_PAIR);
  mvprintw(14, 4, "Executable files");
  setColors(SUID_PAIR);
  mvprintw(15, 4, "Set-user identification");
  setColors(SGID_PAIR);
  mvprintw(16, 4, "Set-group identification");

  setColors(DEFAULT_COLOR_PAIR);
  mvprintw(2, 45, "!-Default      ");
  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(3, 45, "?-Default Bold ");
  setColors(COLORMENU_PAIR_0);
  mvprintw(4, 45, "0-Black        ");
  setColors(COLORMENU_PAIR_1);
  mvprintw(5, 45, "1-Red          ");
  setColors(COLORMENU_PAIR_2);
  mvprintw(6, 45, "2-Green        ");
  setColors(COLORMENU_PAIR_3);
  mvprintw(7, 45, "3-Brown        ");
  setColors(COLORMENU_PAIR_4);
  mvprintw(8, 45, "4-Blue         ");
  setColors(COLORMENU_PAIR_5);
  mvprintw(9, 45, "5-Magenta      ");
  setColors(COLORMENU_PAIR_6);
  mvprintw(10, 45, "6-Cyan         ");
  setColors(COLORMENU_PAIR_7);
  mvprintw(11, 45, "7-Light Gray   ");
  setColors(COLORMENU_PAIR_8);
  mvprintw(12, 45, "8-Dark Gray    ");
  setColors(COLORMENU_PAIR_9);
  mvprintw(13, 45, "9-Light Red    ");
  setColors(COLORMENU_PAIR_A);
  mvprintw(14, 45, "A-Light Green  ");
  setColors(COLORMENU_PAIR_B);
  mvprintw(15, 45, "B-Yellow       ");
  setColors(COLORMENU_PAIR_C);
  mvprintw(16, 45, "C-Light Blue   ");
  setColors(COLORMENU_PAIR_D);
  mvprintw(17, 45, "D-Light Magenta");
  setColors(COLORMENU_PAIR_E);
  mvprintw(18, 45, "E-Light Cyan   ");
  setColors(COLORMENU_PAIR_F);
  mvprintw(19, 45, "F-White        ");

  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(22, 22, "Select 0 to F for desired %s color", fgbgLabel);

  curs_set(TRUE);
  move(colorThemePos + 2, 1);


}
