#pragma once

/* TYPE DEFINITIONS */
typedef unsigned int uint_t;
typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned long long ullong_t;

typedef struct {
  char subString[256];
} splitStrStruct;

typedef struct node {
  int val;
  struct node * next;
} node_t;

typedef struct {
  char refLabel[16];
  wchar_t displayLabel[32];
  int hotKey;
  int displayLabelSize;
  int startPos;
} menuDef;

typedef struct {
  char refLabel[16];
  int topX;
  int topY;
  int bottomX;
  int bottomY;
} menuButton;

typedef struct {
  char refLabel[16];
  int linePos;
  int screenPos;
} settingsOrder;

typedef struct {
  char refLabel[16];
  wchar_t textLabel[64];
} settingSection;

typedef struct {
  int type;
  int storeType;
  char refLabel[16];
  char sectionRef[16];
  wchar_t textLabel[64];
  char section[16];
  int invert;
  int intSetting;
  char *charSetting;
  int maxValue;
} settingIndex;

typedef struct {
  int index;
  char refLabel[16];
  char value[16];
} t1CharValues;

typedef struct {
  int index;
  char refLabel[16];
  char settingLabel[16];
  int value;
  int boolVal;
} t2BinValues;

typedef struct {
  int index;
  char refLabel[16];
  char settingLabel[16];
} t3TextValues;

