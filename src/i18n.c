#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>

void initI18n(){
  bindtextdomain("base", "./locales/");
  textdomain("base");
}