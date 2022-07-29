
 int sknobs_init(int argc, char **argv);
 void sknobs_close(void);

 int sknobs_add(char *pattern, char *value, char *comment);
 int sknobs_prepend(char *pattern, char *value, char *comment);
 int sknobs_load(int argc, char *argv[], char *comment);
 int sknobs_load_string(char *name, char *buffer, char *comment);
 int sknobs_load_file(char *filename);
 int sknobs_load_file_if_exists(char *filename);

 int sknobs_exists(char *name);

typedef void *sknobs_iterator_p;
 sknobs_iterator_p sknobs_iterate(char *name);
 int sknobs_iterator_next(sknobs_iterator_p iterator);
 char *sknobs_iterator_get_string(sknobs_iterator_p iterator);

 char *sknobs_get_string(char *name, char *defaultValue);
 char *sknobs_find_file(char *filename);
 char *sknobs_get_filename(char *name, char *defaultValue);

 unsigned long long sknobs_get_value(char *name, unsigned long long defaultValue);
 unsigned long long sknobs_get_dynamic_value(char *name, unsigned long long defaultValue);

 void sknobs_set_string(char *name, char *value);
 void sknobs_set_value(char *name, unsigned long long value);
 void sknobs_set_seed(long value);

 unsigned long long sknobs_eval(char *expr);

 void sknobs_dump(void);
 void sknobs_save(char *filename);

