#include <vera_directc.h>

#include "knobs.h"

void dc_knobs_init() {
  static int initted = 0;
  if (!initted) {
    knobs_init(0, 0);
  }
  initted = 1;
}

vec32 result[2];  // return value

vec32 *dc_knobs_exists(char *name) {
  dc_knobs_init();
  result[0].c = 0;
  result[1].d = knobs_exists(name);
  return &result[0];
}

vec32 *dc_knobs_get_value(char *name, vec32 *defaultValue) {
  long long value;
  dc_knobs_init();
  value =  (long long)defaultValue[1].d << 32 | defaultValue[0].d;
  value = knobs_get_value(name, value);
  result[0].c = 0;
  result[0].d = value;
  result[1].c = 0;
  result[1].d = value >> 32;
  return &result[0];
}

void dc_knobs_set_value(char *name, vec32 *value) {
  long long v;
  dc_knobs_init();
  v =  (long long)value[1].d << 32 | value[0].d;
  knobs_set_value(name, v);
}

char *dc_knobs_get_string(char *name, char *defaultValue) {
  dc_knobs_init();
  return knobs_get_string(name, defaultValue);
}

void dc_knobs_set_string(char *name, char *value) {
  dc_knobs_init();
  knobs_set_string(name, value);
}
