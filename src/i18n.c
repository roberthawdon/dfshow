#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include "config.h"

void initI18n(){
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
}