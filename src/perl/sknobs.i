%module sknobs

#include "Extern.h"
#include "perl.h"
#include "XSUB.h" 

// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
  AV *tempav;
  I32 len;
  int i;
  SV  **tv;
  if (!SvROK($input))
    croak("$input is not a reference.");
  if (SvTYPE(SvRV($input)) != SVt_PVAV)
    croak("$input is not an array.");
  tempav = (AV*)SvRV($input);
  len = av_len(tempav);
  $1 = (char **) malloc((len+2)*sizeof(char *));
  for (i = 0; i <= len; i++) {
    tv = av_fetch(tempav, i, 0);  
    $1[i] = (char *) SvPV(*tv,PL_na);
  }
  $1[i] = 0;
};

// This cleans up our char ** array after the function call
%typemap(freearg) char ** {
  free($1);
}

%rename("close") close_();

%inline %{

#include "sknobs.h"

static int init(char **argv) {
  int argc = 0, i, cargc=0;
  char **cargv;
  while (argv[argc])
    ++argc;
  cargv = (char**)malloc((argc+1) * sizeof(char *));
  cargv[cargc++] = "sknobs";
  for (i=0; i<argc; ++i)
    cargv[cargc++] = argv[i];
  int result = sknobs_init(cargc, cargv);
  free(cargv);
  return result;
}

static void close_() {
  sknobs_close();
}

static int add(char *pattern, char *value, char *comment) {
  return sknobs_add(pattern, value, comment);
}

static int load(char **argv, char *comment) {
  int argc = 0, i, cargc=0;
  char **cargv;
  while (argv[argc]) ++argc;
  cargv = (char**)malloc((argc+1) * sizeof(char *));
  cargv[cargc++] = "sknobs";
  for (i=0; i<argc; ++i)
    cargv[cargc++] = argv[i];
  int result = sknobs_load(cargc, cargv, comment);
  free(cargv);
  return result;
}

static int load_string(char *name, char *buffer, char *comment) {
  return sknobs_load_string(name, buffer, comment);
}

static int load_file(char *filename) {
  return sknobs_load_file(filename);
}

static int load_file_if_exists(char *filename) {
  return sknobs_load_file_if_exists(filename);
}

static int exists(char *name) {
  return sknobs_exists(name);
}

static void* iterate(char *name) {
  return sknobs_iterate(name);
}

static int iterator_next(void* iterator) {
  return sknobs_iterator_next(iterator);
}

static char *iterator_get_string(void* iterator) {
  return sknobs_iterator_get_string(iterator);
}

static char *get_string(char *name, char *defaultValue) {
  return sknobs_get_string(name, defaultValue);
}

static char *find_file(char *filename) {
  return sknobs_find_file(filename);
}

static char *get_filename(char *name, char *defaultValue) {
  return sknobs_get_filename(name, defaultValue);
}

static long get_value(char *name, long defaultValue) {
  return sknobs_get_value(name, defaultValue);
}

static long get_dynamic_value(char *name, long defaultValue) {
  return sknobs_get_dynamic_value(name, defaultValue);
}

static void set_string(char *name, char *value) {
  sknobs_set_string(name, value);
}

static void set_value(char *name, long value) {
  sknobs_set_value(name, value);
}

static void set_seed(long value) {
  sknobs_set_seed(value);
}

static void dump() {
  sknobs_dump();
}

static void save(char *filename) {
  sknobs_save(filename);
}

%}
