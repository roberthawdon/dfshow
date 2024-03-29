void generateDefaultShowMenus();
void refreshShowMenuLabels();
void unloadShowMenuLabels();
int touchType();
time_t touchTimeInput(int type);
int symLinkLocation();
void linktext_input(int selected, int symbolic);
void link_key_menu_inputs();
void modify_key_menu_inputs();
void modify_owner_input();
void modify_group_input();
void modify_permissions_input();
void modify_context_menu_inputs();
void modify_context_input(int mode);
void show_directory_input();
void directory_view_menu_inputs();
void global_menu_inputs();
char * execute_argument_input(const char *exec);
void copy_file_input(char *file, mode_t mode);
int replace_file_confirm_input(char *filename);
void rename_file_input(char *file);
void edit_file_input();
void make_directory_input();
void touch_file_input();
void delete_file_confirm_input(char *file);
void delete_directory_confirm_input(char *directory);
void delete_multi_file_confirm_input(results* ob);
void copy_multi_file_input(results* ob, char *input);
void copy_multi_file(results* ob, char *dest);
void rename_multi_file_input(results* ob, char *input);
int huntCaseSelectInput();
void huntInput(int selected, int charcase);
void refreshDirectory(char *sortmode, int origlineStart, int origselected, int destructive);
