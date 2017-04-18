////////////////////////////////////////////////////////////////////////////
// C implementation of knobs
//
// $Revision: #1 $
// $Author: jscheid $
// $Date: 2010/04/28 $
// $Id: //depot/users/jscheid/src/sknobs/0.25/src/c/sknobs.c#1 $
//
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <fnmatch.h>
#include <glob.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sknobs.h"

////////////////////////////////////////////////////////////////////////////
// TODO:
// faster memory management

////////////////////////////////////////////////////////////////////////////
// defines
#define MAX_ARGV_SIZE 100000
#define MAX_FILENAME_SIZE 10000
#define MAX_CHOICES 100
#define HASH_TABLE_SIZE 1031
#define MAX_ITERATORS 10
#define RANDOM_STATE_SIZE 32

////////////////////////////////////////////////////////////////////////////
// structures
typedef struct sknob_t {
  char *pattern;
  char *value;
  char *comment;
  int use_regex; // 1 if knob specified using extended regular expression
  regex_t regex;
  struct sknob_t *next;
} sknob_s;

typedef struct sknob_iterator_t {
  char *name;
  sknob_s *knob;
} sknob_iterator_s;

typedef struct saved_value_t {
  char *name;
  unsigned long long value;
  struct saved_value_t *next;
} saved_value_s;

typedef struct string_list_t {
  char *s;
  struct string_list_t *next;
} string_list_s;

////////////////////////////////////////////////////////////////////////////
// global variables
sknob_s *sknob_list = 0; // main knob list
sknob_s *last_knob = 0; // pointer to last knob added to list

static int debug = 0; // debug level
saved_value_s *saved_sknob_values[HASH_TABLE_SIZE]; // hash table for saved values
static char buffer[SKNOBS_MAX_LENGTH], eval_buffer[SKNOBS_MAX_LENGTH];
static sknob_iterator_s sknobs_iterators[MAX_ITERATORS];
static int sknobs_current_iterator = 0;
static int sknobs_init_flag = 0;
static int sknobs_init_return_value = 0;
extern char **environ;
static char random_state[RANDOM_STATE_SIZE];
static char *sknobs_delimiters_sptbnl = " \t\n";
static char *sknobs_delimiters_crnl = "\r\n";
static char *sknobs_delimiters;

////////////////////////////////////////////////////////////////////////////
// check_sknobs_init
int check_sknobs_init(void) {
  if (!sknobs_init_flag)
    return sknobs_init(0, (char **)0);
  return sknobs_init_return_value;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_prinsknob_t
// print out contents of knob
static void sknobs_print(sknob_s *knob) {
  if (knob->comment)
    printf(" %s:", knob->comment);
  printf(" +");
  if (knob->use_regex)
    printf("(regexp)");
  printf("%s", knob->pattern);
  if (knob->value)
    printf("=%s", knob->value);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_random
// Uses private random state.
unsigned long long sknobs_get_random(void) {
  char *old_random_state;
  unsigned long long value, partial;
  old_random_state = setstate(random_state);
  // While random() returns long, the value is [0, 2**31-1].
  // Take the lower 16 bits from four calls to get a 64-bit value.
  value = 0;
  partial = (unsigned long long) (random() & 0x0000FFFFL);
  value |= partial << 0ULL;
  partial = (unsigned long long) (random() & 0x0000FFFFL);
  value |= partial << 16ULL;
  partial = (unsigned long long) (random() & 0x0000FFFFL);
  value |= partial << 32ULL;
  partial = (unsigned long long) (random() & 0x0000FFFFL);
  value |= partial << 48ULL;
  setstate(old_random_state);
  if (debug > 3)
    printf("info: sknobs: random: %lld\n", value);
  return value;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_add
// add a knob to the knobs database
// returns 0 on success
int sknobs_add(char *pattern, char *value, char *comment) {
  sknob_s *knob = (sknob_s*)malloc(sizeof(sknob_s));
  if (!knob) {
    printf("error: sknobs: unable to allocate memory\n");
    return 1;
  }
  if (pattern[0] == '+') {
    knob->use_regex = 1;
    if (strlen(pattern) == 1) {
      printf("warning: sknobs: encountered lone '+' (value=%s comment=%s)\n",
             value, comment);
    }
    else {
      if (regcomp(&knob->regex, pattern+1, REG_EXTENDED|REG_NOSUB) != 0) {
        printf("error: sknobs: unable to compile regular expression pattern: %s\n",
               pattern+1);
        free(knob);
        return 1;
      }
    }
  }
  else
    knob->use_regex = 0;
  knob->pattern = strdup(pattern);
  if (value)
    knob->value = strdup(value);
  else
    knob->value = 0;
  knob->comment = strdup(comment);
  knob->next = 0;
  if (last_knob) {
    last_knob->next = knob;
    last_knob = knob;
  }
  else
    sknob_list = last_knob = knob;
  if (debug > 1) {
    printf("info: sknobs: add: pattern=%s value=%s", pattern, value);
    sknobs_print(knob);
    printf("\n");
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// choose_from_weighted_list
// parse weighted list apart and pick one element
// returns chosen string
static char *choose_from_weighted_list(char *l) {
  char *choices[MAX_CHOICES];
  unsigned long long weights[MAX_CHOICES];
  char *p;
  unsigned long long total;
  unsigned long long a, w;
  unsigned int i, j;
  char *choice = 0;
  unsigned int count = 0;
  assert(strlen(l) < SKNOBS_MAX_LENGTH);
  strncpy(eval_buffer, l, SKNOBS_MAX_LENGTH);
  eval_buffer[SKNOBS_MAX_LENGTH-1] = '\0';
  // split on comma
  choices[count++] =  strtok(eval_buffer, ",");
  while((p = strtok(NULL, ",")))
    choices[count++] = p;
  // assign weights, 1 by default
  for (i=0; i<count; ++i) {
    weights[i] = 1;
    if (strtok(choices[i], ":")) {
      char *weight_str = strtok(NULL, ":");
      if (weight_str) {
        errno = 0;
        weights[i] = strtoull(weight_str, (char **)NULL, 0);
        if (errno) {
          printf("error: sknobs: choose_from_weighted_list: could not covert to ul %s from %s\n", weight_str, l);
        }
      }
    }
  }
  // calculate total of weight
  total = 0;
  for (i=0; i<count; ++i)
    total += weights[i];
  // special case if there is only one to choose from
  if (total == 1)
    return choices[0];
  // choose one
  w = sknobs_get_random() % total;
  a = 0;
  j = 0;
  while (a <= w) {
    choice = choices[j];
    a += weights[j++];
  }
  return choice;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_eval
// calculate value from knobs expression
// returns calculated value
//
// knobs can be of the following formats:
//   a
//   a-b
//   a,b,c
//   a:wa,b:wb,c:wc
unsigned long long sknobs_eval(char *expr) {
  char *value;
  char *left, *right;
  unsigned long long l, r;
  // pick one from possible weighted list
  value = choose_from_weighted_list(expr);
  if (debug > 2)
    printf("info: sknobs: eval: expr=%s, choice=%s\n", expr, value);
  // if no range return value
  left = strtok(value, "~");
  right = strtok(NULL, "~");
  errno = 0;
  l = strtoull(left, (char **)NULL, 0);
  if (errno) {
    printf("error: sknobs: eval: could not covert to ull %s from %s\n", left, expr);
  }
  if (!right)
    return l;
  errno = 0;
  r = strtoull(right, (char **)NULL, 0);
  if (errno) {
    printf("error: sknobs: eval: could not covert to ull %s from %s\n", left, expr);
  }
  // return value within range
  return l + sknobs_get_random() % (r-l+1);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_iterate
// search through the knobs database for the knob whose pattern matches name
sknobs_iterator_p sknobs_iterate(char *name) {
  sknob_iterator_s *sknob_iterator;
  check_sknobs_init();
  sknob_iterator = &sknobs_iterators[sknobs_current_iterator];
  sknobs_current_iterator = (sknobs_current_iterator + 1) % MAX_ITERATORS;
  assert(sknob_iterator);
  if (sknob_iterator->name)
    free (sknob_iterator->name);
  sknob_iterator->name = strdup(name);
  sknob_iterator->knob = 0;
  return sknob_iterator;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_next
// search through the knobs database for the knob whose pattern matches name
sknob_s *sknobs_get_next(sknobs_iterator_p iterator) {
  sknob_iterator_s *sknob_iterator = (sknob_iterator_s*)iterator;
  assert (iterator);
  // if this is the first time, start at the head of the list
  if (!sknob_iterator->knob)
    sknob_iterator->knob = sknob_list;
  else
    sknob_iterator->knob = sknob_iterator->knob->next;
  while (sknob_iterator->knob) {
    int match;
    if (sknob_iterator->knob->use_regex)
      match = (0 == regexec(&sknob_iterator->knob->regex, 
                            sknob_iterator->name, 
                            (size_t) 0, NULL, 0));
    else {
      match = (0 == fnmatch(sknob_iterator->knob->pattern, 
                            sknob_iterator->name, 
                            0));
    }
    if (match) {
      if (debug > 1)
	printf("info: sknobs: found match name=%s pattern=%s comment=%s\n", 
               sknob_iterator->name, 
               sknob_iterator->knob->pattern, 
               sknob_iterator->knob->comment);
      return sknob_iterator->knob;
    }
    sknob_iterator->knob = sknob_iterator->knob->next;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_find_last
// search through the knobs database for the knob whose pattern matches name
sknob_s *sknobs_find_last(char *name) {
  sknob_s *knob, *result = 0;
  sknob_iterator_s *iterator = sknobs_iterate(name);
  while ((knob = sknobs_get_next(iterator)))
    result = knob;
  return result;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_iterator_next
// search through the knobs database for the knob whose pattern matches name
int sknobs_iterator_next(sknobs_iterator_p iterator) {
  sknob_s *knob = sknobs_get_next(iterator);
  return knob != 0;
}

////////////////////////////////////////////////////////////////////////////
char *sknobs_expand_string(char *s) {
  char *value, *p, *q, c, *n=0, name_buffer[1024];
  enum {NORMAL, STARTMACRO, MACRO} state;
  // skip locking character
  while ((s[0] == '=')) 
    ++s;
  // expand any $
  p = s;
  q = buffer;
  state = NORMAL;
  while ((c = *p++)) {
    switch (state) {
    case NORMAL:
      if (c == '$')
	state = STARTMACRO;
      else
	*q++ = c;
      break;
    case STARTMACRO:
      if (c != '(') {
	printf("error: sknobs: incorrect macro format, missing (?\n");
	return 0;
      }
      state = MACRO;
      n = name_buffer;
      break;
    case MACRO:
      if (c == ')') {
        sknob_s *knob;
	*n = '\0';
	knob = sknobs_find_last(name_buffer);
	if (knob) {
	  value = knob->value;
	  // skip locking character
	  while (value[0] == '=') ++value;
	}
	else
	  value = "";
	// copy value to buffer
	n = value;
	while (*n) {
	  *q++ = *n;
	  n++;
	}
	state = NORMAL;
	break;
      }
      *n++ = c;
      break;
    }
  }
  if (state != NORMAL) {
    printf("error: sknobs: incorrect macro format, missing )?\n");
    return 0;
  }
  *q = '\0';
  return buffer;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_iterator_get_string
// return string value of iterator
char *sknobs_iterator_get_string(sknobs_iterator_p iterator) {
  sknob_s *knob;
  char *result;
  sknob_iterator_s *sknob_iterator = (sknob_iterator_s*)iterator;
  knob = sknob_iterator->knob;
  assert(knob);
  result = knob->value;
  return result;
  //  return sknobs_expand_string(result);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_iterator_get_value
// return integer value of iterator
unsigned long long sknobs_iterator_get_value(sknobs_iterator_p iterator) {
  sknob_s *knob;
  char *expr;
  unsigned long long result;
  sknob_iterator_s *sknob_iterator = (sknob_iterator_s*)iterator;
  knob = sknob_iterator->knob;
  assert(knob);
  expr = knob->value;
  assert(expr);
  result = sknobs_eval(expr);
  return result;
}

////////////////////////////////////////////////////////////////////////////
// hash
// find a hash value for the string
unsigned int hash(char *s) {
  int h, i;
  int l = strlen(s);
  for (h=l, i=0; i<l; ++i)
    h = ((h<<4)^(h>>28)) ^ s[i];
  return h;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_exists
// search through the knobs database for the knob whose pattern matches name
int sknobs_exists(char *name) {
  int result;
  sknobs_iterator_p iterator;
  check_sknobs_init();
  iterator = sknobs_iterate(name);
  result = 0;
  if (sknobs_iterator_next(iterator)) {
    result = 1;
    while (sknobs_iterator_next(iterator));
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_string
// search through the knobs database for the knob whose pattern matches name
char *sknobs_get_string(char *name, char *defaultValue) {
  char *value;
  sknob_s *knob;
  check_sknobs_init();
  knob = sknobs_find_last(name);
  if (knob)
    value = knob->value;
  else
    value = defaultValue;
  if (debug > 1)
    printf("info: sknobs: get_string: %s=%s\n", name, value);
  // if value is actually null then just return it
  if (!value)
    return 0;
  return value;
  //  return sknobs_expand_string(value);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_set_string
// search through the knobs database for the knob whose pattern matches name
void sknobs_set_string(char *name, char *value) {
  sknob_s *knob;
  check_sknobs_init();
  knob = sknobs_find_last(name);
  if (knob) {
    if (knob->value)
      free(knob->value);
    knob->value = strdup(value);
  }
  else {
    sknobs_add(name, value, "set_string");
  }
  if (debug > 1)
    printf("info: sknobs: set_string: %s=%s\n", name, value);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_dynamic_value
// first call sknobs_get_string and then convert to a value
// returns value of requested knob
unsigned long long sknobs_get_dynamic_value(char *name, unsigned long long defaultValue) {
  char *expr = sknobs_get_string(name, 0);
  unsigned long long value;
  check_sknobs_init();
  if (!expr)
    return defaultValue;
  value = sknobs_eval(expr);
  return value;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_value
// first call sknobs_get_string and then convert to a value
// returns value of requested knob
unsigned long long sknobs_get_value(char *name, unsigned long long defaultValue) {
  unsigned long long value;
  unsigned int i;
  saved_value_s *saved_value;
  check_sknobs_init();
  // first search for previously returned value
  i = hash(name) % HASH_TABLE_SIZE; 
  assert(i < HASH_TABLE_SIZE);
  saved_value = saved_sknob_values[i];
  while (saved_value) {
    if (0 == strcmp(name, saved_value->name)) {
      break;
    }
    saved_value = saved_value->next;
  }
  if (saved_value) {
    value = saved_value->value;
  }
  else {
    // get dynamic value
    value = sknobs_get_dynamic_value(name, defaultValue);
    // save value for next call
    saved_value = (saved_value_s*)malloc(sizeof(saved_value_s));
    assert(saved_value);
    saved_value->name = strdup(name);
    saved_value->value = value;
    saved_value->next = saved_sknob_values[i];
    saved_sknob_values[i] =  saved_value;
  }
  //.done
  if (debug > 1)
    printf("info: sknobs: get_value: %s=%lld\n", name, value);
  return value;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_set_value
// search through the knobs database for the knob whose pattern matches name
void sknobs_set_value(char *name, unsigned long long value) {
  sknob_s *knob;
  check_sknobs_init();
  snprintf(buffer, SKNOBS_MAX_LENGTH, "%lld", value);
  knob = sknobs_find_last(name);
  if (knob) {
    if (knob->value)
      free(knob->value);
    knob->value = strdup(buffer);
  }
  else 
    sknobs_add(name, buffer, "set_value");
  if (debug > 1)
    printf("info: sknobs: set_value: %s=%s\n", name, buffer);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_dump
// dump the contents of the knobs database
void sknobs_dump(void) {
  sknob_s *knob = sknob_list;
  char *filename;
  sknobs_iterator_p iterator;
  printf("info: sknobs: dumping knobs\n");
  while (knob) {
    printf("info: sknobs: knob:");
    sknobs_print(knob);
    printf("\n");
    knob = knob->next;
  }
  iterator = sknobs_iterate("sknobs.files");
  while (sknobs_iterator_next(iterator)) {
    filename = sknobs_iterator_get_string(iterator);
    printf("info: sknobs: file: %s\n", filename);
  }
}

////////////////////////////////////////////////////////////////////////////
// sknobs_save
// save the contents of the knobs database to a file
void sknobs_save(char *filename) {
  saved_value_s *saved_value;
  sknob_s *knob;
  int i;
  FILE *fp;
  fp  = fopen(filename, "w");
  if (!fp) {
    printf("error: sknobs: unable to open filename for writing: %s\n", filename);
    exit(1);
  }
  knob = sknob_list;
  while (knob) {
    char *value = knob->value;
    char *comment = knob->comment;
    if (!value)
      value = "";
    if (!comment)
      comment = "";
    fprintf(fp, "// %s\n+%s=%s\n", comment, knob->pattern, value);
    knob = knob->next;
  }
  for (i=0; i<HASH_TABLE_SIZE; ++i) {
    saved_value = saved_sknob_values[i];
    while (saved_value) {
      fprintf(fp, "// saved value\n+%s=%0lld\n", saved_value->name, saved_value->value);
      saved_value = saved_value->next;
    }
  }
  fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_uncomment
// remove comments from a buffer in place
// return 1 on error
static int sknobs_uncomment(char *lbuffer) {
  enum {NORMAL, SLASH, BLOCK, STAR, LINE} state = NORMAL;
  char c, *p = lbuffer;
  while ((c = *p)) {
    switch(state) {
    case NORMAL:
      if (c == '/')
	state = SLASH;
      break;
    case SLASH:
      if (c == '*') {
	*(p-1) = ' ';
	*p = ' ';
	state = BLOCK;
      }
      else if (c == '/') {
	*(p-1) = ' ';
	*p = ' ';
	state = LINE;
      }
      else {
	state = NORMAL;
      }
      break;
    case BLOCK:
      if (c == '*')
	state = STAR;
      *p = ' ';
      break;
    case STAR:
      if (c == '/')
	state = NORMAL;
      else if (c != '*')
	state = BLOCK;
      *p = ' ';
      break;
    case LINE:
      if (c == '\n')
	state = NORMAL;
      else
	*p = ' ';
    }
    ++p;
  }
  if (state == BLOCK) {
    printf("error: sknobs: block comment not closed\n");
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_set_seed
void sknobs_set_seed(unsigned seed) {
  char *old_random_state;
  if (debug > 0)
    printf("info: sknobs: seed: %d\n", seed);
  // Call srandom for others, but also generate our private state.
  srandom(seed);
  old_random_state = initstate(seed, random_state, RANDOM_STATE_SIZE);
  setstate(old_random_state);
  srand(seed);
  srand48((long) seed);
  sknobs_set_value("seed", seed);
}

////////////////////////////////////////////////////////////////////////////
// sknobs_set_seed_from_string
int sknobs_set_seed_from_string(char *s) {
  unsigned seed;
  unsigned long long full_value;
  errno = 0;
  full_value = strtoull(s, (char **)NULL, 0);
  if (errno) {
    printf("error: sknobs: sknobs_set_seed_from_string: could not covert to ull %s\n", s);
    return 1;
  }
  // Use the lower 32 bits.
  seed = full_value & 0x00000000FFFFFFFFULL;
  sknobs_set_seed(seed);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_add_string
// parse string that might contain "=" and add knob
// return 1 on error
int sknobs_add_string(char *s, char *comment) {
  char *pattern, *value;
  char *lbuffer;
  if (debug > 2)
    printf("sknobs_add_string: %s\n", s);
  if (!s) {
    printf("error: sknobs: sknobs_add_string given null string (comment=%s)\n",
           comment);
    return 1;
  }
  if (s[0] == '\0') {
    printf("error: sknobs: sknobs_add_string given empty string (comment=%s)\n",
           comment);
    return 1;
  }
  if (s[0] == '=') {
    printf("error: sknobs: sknobs_add_string can not start with '=': %s (comment=%s)\n",
           s, comment);
    return 1;
  }
  lbuffer = strdup(s);
  // treat +undefine+name specially
  if (0==strncmp(lbuffer, "undefine+", 9)) {
    if (sknobs_add("sknobs.undefines", lbuffer+9, comment))
      goto sknobs_add_string_error;
  }
  // treat +define+name=value specially
  if (0==strncmp(lbuffer, "define+", 7)) {
    if (sknobs_add("sknobs.defines", lbuffer+7, comment))
      goto sknobs_add_string_error;
    pattern = strtok(lbuffer+7, "=\n");
  }
  else {
    pattern = strtok(lbuffer, "=\n");
  }
  value = strtok(NULL, "\0");
  if (!value)
    value = "1";
  if (0==strcmp(pattern, "seed"))
    sknobs_set_seed_from_string(value);
  if (sknobs_add(pattern, value, comment))
    goto sknobs_add_string_error;
  free(lbuffer);
  return 0;

  sknobs_add_string_error:
    free(lbuffer);
    return 1;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_file_exists
// determine if file exists
// returns 1 if file exists
int sknobs_file_exists(char *filename) {
  struct stat lbuffer;
  int rc;
  rc = stat(filename, &lbuffer);
  return rc != -1;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_find_file
// work through search path looking for file
// returns full path to file
char *sknobs_find_file(char *filename) {
  sknobs_iterator_p iterator;
  char *search_path;
  static char filename_buffer[1024];
  char filename_buffer2[1024]; // hack
  strcpy(filename_buffer2, filename);
  iterator = sknobs_iterate("sknobs.search_path");
  while (sknobs_iterator_next(iterator)) {
    search_path  = sknobs_iterator_get_string(iterator);
    sprintf(filename_buffer, "%s/%s", search_path, filename_buffer2);
    if (debug > 1) printf("info: sknobs: trying file: %s\n", filename_buffer);
    if (sknobs_file_exists(filename_buffer)) {
      if (debug > 1) printf("info: sknobs: found file: %s\n", filename_buffer);
      // clean up iterator before returnin
      while (sknobs_iterator_next(iterator));
      return filename_buffer;
    }
  }
  // return orginal file if not found
  return filename;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_get_filename
char *sknobs_get_filename(char *name, char *defaultValue) {
  char *s = sknobs_get_string(name, defaultValue);
  return sknobs_find_file(s);
}

int sknobs_load_dashf(char *filestring) {
  char *filename;
  if (debug > 2) {
    printf("info: sknobs: parsing dash-f filestring: %s\n", filestring);
  }
  filename = choose_from_weighted_list(filestring);
  filename = sknobs_find_file(filename);
  if (sknobs_load_file(filename))
    return 1;
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_load
// load the knobs database from argc,argv
// return 1 on error
int sknobs_load(int argc, char *argv[], char *comment) {
  int i;
  enum {START, DASHF, IGNORE} argState = START;
  if (debug > 2) {
    for (i=1; i<argc; ++i)
      printf("sknobs_load: argv[%d]=%s\n", i, argv[i]);
  }
  for (i=1; i<argc; ++i) {
    char *arg = argv[i];
    switch(argState) {
    case START: {
      if (0==strcmp(arg, "-f"))
        // dash-f and filestring coming as two separate tokens.
	argState = DASHF;
      else if (0==strncmp(arg, "-f ", 3)) {
        // dash-f and filestring coming as one token.
        char *filestring = arg+3;
        if (sknobs_load_dashf(filestring))
          return 1;
      }
      else if (0==strcmp(arg, "-pli"))
	argState = IGNORE;
      else if (arg[0] == '+') {
        if (strlen(arg) == 1) {
          printf("warning: sknobs: encountered lone '+' (comment=%s)\n",
                 comment);
        }
        else {
          if (sknobs_add_string(&arg[1], comment)) { // drop +
            return 1;
          }
        }
      }
      else {
        sknobs_add("sknobs.files", arg, "file");
	if (debug > 1) printf("info: sknobs: adding file: %s\n", arg);
      }
      break;
    }
    case DASHF: {
      char *filestring = arg;
      if (sknobs_load_dashf(filestring))
        return 1;
      argState = START;
      break;
    }
    case IGNORE: {
      if (debug > 1) printf("info: sknobs: ignoring argument: %s\n", arg);
      argState = START;
      break;
    }
    }
  }
  if (argState != START) {
    printf("error: sknobs: missing filename for final -f\n");
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_load_string
// load knobs from a string.  tokenizes the string first then calls sknobs_load
// return 1 on error
int sknobs_load_string(char *name, char *s, char *comment) {
  // tokenize
  int argc = 0;
  char *argv[MAX_ARGV_SIZE];
  char *token;
  if (debug > 2) printf("sknobs_load_string: %s\n", s);
  argv[argc++] = name;
  token = strtok(s, sknobs_delimiters);
  while (token) {
    assert(argc < MAX_ARGV_SIZE);
    argv[argc++] = token;
    token = strtok(NULL, sknobs_delimiters);
  }
  if (sknobs_load(argc, argv, comment))
    return 1;
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_load_file
// opens and reads file, removes comments, then calls sknobs_load_string
// return 1 on error
int sknobs_load_file(char *filename) {
  FILE *fp = fopen(filename, "r");
  long size;
  char *contents;

  if (debug > 1)
    printf("info: sknobs: loading knobs from file: %s\n", filename);

  // open file
  if (!fp) {
    printf("error: sknobs: unable to open -f file: %s\n", filename);
    return 1;
  }
  // read entire contents of file
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  contents = (char *)malloc(size+1);
  assert(contents);
  fseek(fp, 0, SEEK_SET);
  fread(contents, size, 1, fp);
  contents[size] = '\0';
  fclose(fp);

  // remove comments
  if (sknobs_uncomment(contents)) {
    printf("error: sknobs: unable to uncomment file: %s\n", filename);
    goto sknobs_load_file_error;
  }
  if (sknobs_load_string(filename, contents, filename)) {
    printf("error: sknobs: unable to load file contents: %s\n", filename);
    goto sknobs_load_file_error;
  }

  free(contents);
  return 0;

 sknobs_load_file_error:
  free(contents);
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_load_file_if_exists
// check if file exists then call sknobs_load_file
// reads ~/.knobs
// return 1 on error
int sknobs_load_file_if_exists(char *filename) {
  if (sknobs_file_exists(filename))
    return sknobs_load_file(filename);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_load_knobsrc_files
static int sknobs_load_knobsrc_files(char *dir) {
  char pattern[MAX_FILENAME_SIZE];
  glob_t g;
  sprintf(pattern, "%s/*.knobsrc", dir);
  if (debug > 1)
    printf("info: sknobs: searching for knobsrc files: %s\n", pattern);
  if (0==glob(pattern, 0, 0, &g)) {
    int i;
    for (i=0; i<g.gl_pathc; ++i) {
      if (sknobs_load_file(g.gl_pathv[i])) {
	return 1;
      }
    }
  }
  globfree(&g);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_init
// called at the beginning to populate knobs database
// exits immediately if already called
// return 1 on error
int sknobs_init(int argc, char **argv) {
  char *v, *p;
  char *delimiter_flavor;
  char *home;
  char *sknobs;
  char *pwd;
  char filename[MAX_FILENAME_SIZE];
  int i;
  unsigned seed_value;
  char *seed_string;
  char **e;
  size_t path_size;
  char *getcwd_buffer;
  FILE *fp;

  // check if we're already initialized 
  if (sknobs_init_flag) {
    if (argc != 0) {
      // If sknobs_init has already been called, this call won't do anything,
      // so error if the caller tries to pass args.
      printf("error: sknobs: sknobs_init called a second time, with %d args\n",
             argc);
      return 1;
    }
    return sknobs_init_return_value;
  }

  // indicate that we are done with this
  sknobs_init_flag = 1;

  // initialize hash table
  for (i=0; i<HASH_TABLE_SIZE; ++i)
    saved_sknob_values[i] = 0;

  // string delimiter version
  delimiter_flavor = getenv("SKNOBS_DELIMITER_FLAVOR");
  if (delimiter_flavor) {
    if (!strcmp(delimiter_flavor, "sptbnl")) {
      sknobs_delimiters = sknobs_delimiters_sptbnl;
    }
    else if (!strcmp(delimiter_flavor, "crnl")) {
      sknobs_delimiters = sknobs_delimiters_crnl;
    }
    else {
      printf("error: sknobs: bad value for SKNOBS_DELIMITER_FLAVOR (sptbnl or crnl): %s\n",
             delimiter_flavor);
      return 1;
    }
  }
  else {
    // use original behavior
    sknobs_delimiters = sknobs_delimiters_sptbnl;
  }

  // look for debug
  v =  getenv("SKNOBS_DEBUG");
  if (v) {
    debug = atoi(v);
    if (debug > 1)
      printf("info: sknobs: SKNOBS_DEBUG=%d\n", debug);
  }
  if (debug > 1) {
    int i;
    printf("info: sknobs: init called with argc=%d\n", argc);
    for (i=0; i<argc; ++i)
      printf("info: sknobs: init called with argv[%d]=%s\n", i, argv[i]);
  }

  // HACK: if sknobs.save exists, just load that file and return
  if (sknobs_file_exists("sknobs.save")) {
    if (sknobs_load_file("sknobs.save")) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }
    sknobs_init_return_value = 0;
    return sknobs_init_return_value;
  }

  // Get current working directory.
  path_size = (size_t) pathconf(".", _PC_PATH_MAX);
  getcwd_buffer = (char *) malloc(path_size);
  assert(getcwd_buffer);
  pwd = getcwd(getcwd_buffer, path_size);
  if (pwd == NULL) {
    printf("error: sknobs: getcwd failed\n");
    sknobs_init_return_value = 1;
    return sknobs_init_return_value;
  }
  if (strlen(pwd) >= SKNOBS_MAX_LENGTH) {
    printf("error: sknobs: getcwd path too big\n");
    sknobs_init_return_value = 1;
    return sknobs_init_return_value;
  }

  // initialize search path first with current file hierarchy
  strncpy(buffer, pwd, SKNOBS_MAX_LENGTH);
  buffer[SKNOBS_MAX_LENGTH-1] = '\0';
  while (strlen(buffer)) {
    char *p;
    sknobs_add("sknobs.search_path", buffer, "PWD");
    sprintf(filename, "%s/.stopknobs", buffer);
    if (sknobs_file_exists(filename))
      break;
    p = strrchr(buffer, '/');
    if (!p)
      break;
    *p = '\0';
  }

  // determine the the seed
  seed_string = NULL;

  // check for explicit seed
  // check for environment variable
  p = getenv("SEED"); // look for SEED environment variable
  if (p) {
    seed_string = p;
  }
  // scan through command line and see if seed is specified there
  for (i=1; i<argc; ++i)
    if (0 == strncmp(argv[i], "+seed=", 6))
      seed_string = argv[i]+6;
  if (seed_string) {
    sknobs_set_seed_from_string(seed_string);
  }
  else {
    // No explicit seed.
    // Use /dev/urandom if available.
    // Standard in Linux.
    fp = fopen("/dev/urandom", "r");
    if (fp != NULL) {
      size_t read_size;
      read_size = fread(&seed_value, 4, 1, fp);
      if (read_size != 1) {
        printf("error: sknobs: urandom read failed\n");
        sknobs_init_return_value = 1;
        return sknobs_init_return_value;
      }
      if (fclose(fp)) {
        printf("error: sknobs: fclose on /dev/urandom failed\n");
        sknobs_init_return_value = 1;
        return sknobs_init_return_value;
      }
    }
    else {
      // This is a crude fallback, as multiple instances invoked during one
      // second will get similar seeds.  Hash with pid to get a bit of
      // variability.
      seed_value = (unsigned) time(0);
      seed_value ^= (unsigned) getpid();
    }
    sknobs_set_seed(seed_value);
  }

  // try to load ~/.knobsrc
  home = getenv("HOME");
  if (home) {
    sprintf(filename, "%s/.knobsrc", home);
    if (sknobs_load_file_if_exists(filename)) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }
  }

  // work through search path loading *.knobsrc files
  strncpy(buffer, pwd, SKNOBS_MAX_LENGTH);
  buffer[SKNOBS_MAX_LENGTH-1] = '\0';
  p = buffer+1;
  while ((p=strchr(p, '/'))) {
    *p = '\0';
    if (sknobs_load_knobsrc_files(buffer)) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }
    *p = '/';
    ++p;
  }
  if (sknobs_load_knobsrc_files(pwd)) {
    sknobs_init_return_value = 1;
    return sknobs_init_return_value;
  }
  // Done using pwd/getcwd_buffer.
  free(getcwd_buffer);

  // grab all environment variables and make knobs out of them
  ; // for some reason this is NULL
#ifdef __GNUC__
  for (e = environ; *e; ++e)
#else
  for (e = __environ; *e; ++e)
#endif
    if (sknobs_add_string(*e, "environment")) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }

  // look at special environment variable SKNOBS
  sknobs = getenv("SKNOBS");
  if (sknobs) {
    if (debug > 1)
      printf("info: sknobs: processing SKNOBS environment variable: %s\n", sknobs);
    if (strlen(sknobs) >= SKNOBS_MAX_LENGTH) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }
    // sknobs_load_string will modify the string
    strncpy(buffer, sknobs, SKNOBS_MAX_LENGTH);
    buffer[SKNOBS_MAX_LENGTH-1] = '\0';
    if(sknobs_load_string("SKNOBS", sknobs, "SKNOBS environment variable")) {
      sknobs_init_return_value = 1;
      return sknobs_init_return_value;
    }
  }

  // finally load knobs from command line
  if (sknobs_load(argc, argv, "command line")) {
    sknobs_init_return_value = 1;
    return sknobs_init_return_value;
  }

  if (debug > 2)
    sknobs_dump();

  // return success
  sknobs_init_return_value = 0;
  return sknobs_init_return_value;
}

////////////////////////////////////////////////////////////////////////////
// sknobs_close
void sknobs_close(void) {
  int i;
  sknob_s *knob;

  if (debug > 0) {
    printf("info: sknobs: close\n");
  }

  // free up knobs list
  knob = sknob_list;
  while (knob) {
    sknob_s *next = knob->next;
    free(knob->pattern);
    if (knob->value)
      free(knob->value);
    if (knob->comment)
      free(knob->comment);
    if (knob->use_regex)
      regfree(&knob->regex);
    free(knob);
    knob = next;
  }
  sknob_list = 0;
  last_knob = 0;

  // free up saved values
  for (i=0; i<HASH_TABLE_SIZE; ++i) {
    saved_value_s *saved_value = saved_sknob_values[i];
    while (saved_value) {
      saved_value_s *next = saved_value->next;
      free(saved_value->name);
      free(saved_value);
      saved_value = next;
    }
    saved_sknob_values[i] = 0;
  }
}
