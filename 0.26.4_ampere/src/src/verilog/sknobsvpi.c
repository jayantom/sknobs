#include <veriuser.h>
#include <acc_user.h>
#include <vpi_user.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sknobs.h"

////////////////////////////////////////////////////////////////////////////
// globals
static char buffer[SKNOBS_MAX_LENGTH]; // useful buffer for returning modified names

////////////////////////////////////////////////////////////////////////////
// expandName:
// take out the %m's and replace them by the module instance path
// take out the %d's and replace them with the value of i
// NOTE:
// This can be called only once in an API, otherwise all 
// char pointer start pointing to the same string, and all 
// of them point to the last string 
static char *expandName(char *name) {
  char c, *p = name, *q = buffer;
  int i = 3;
  while ((c = *p++)) {
    switch(c) {
    case '%':
      c = *p++;
      switch (c) {
      case 'm': {
        char ch, *mipname = tf_mipname();
        while ((ch = *mipname++))
          *q++ = ch;
        break;
      }
      case 'd': {
        int v = tf_getp(i++);
        char ch, *ptr, b[100];
        sprintf(b, "%0d", v);
        ptr = b;
        while ((ch = *ptr++))
          *q++ = ch;
        break;
      }
      default:
        assert(0);
      }
      break;
    default:
      *q++ = c;
    }
  }
  *q++ = 0;
  return buffer;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_init
int vpi_sknobs_init() {
  s_vpi_vlog_info vlog_info;
  static initted = 0;
  if (initted)
    return 1;
  vpi_get_vlog_info(&vlog_info);
  initted = 1;
  return sknobs_init(vlog_info.argc, vlog_info.argv);
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_exists_calltf
int vpi_sknobs_exists_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  s_vpi_value value;
  int result;
  char *name;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_exists(name);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_exists_sizetf
int vpi_sknobs_exists_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_value_calltf
int vpi_sknobs_get_value_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle defaultValueHandle;
  s_vpi_value value;
  long long result, defaultValue;
  char *name;
  char retBuffer[SKNOBS_MAX_LENGTH];

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  defaultValueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get default value
  defaultValue = 0L;
  if (defaultValueHandle) {
    value.format = vpiDecStrVal;
    vpi_get_value(defaultValueHandle, &value);
    defaultValue = atoll(value.value.str);
  }

  // search knobs database for result
  result = sknobs_get_value(name, defaultValue);

  // return result
  value.format = vpiHexStrVal;
  sprintf(retBuffer, "%llx", result);
  value.value.str = retBuffer;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_value_sizetf
int vpi_sknobs_get_value_sizetf() {
  return 64;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_set_value_calltf
int vpi_sknobs_set_value_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle valueHandle;
  s_vpi_value value;
  long long result, newValue;
  char *name;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  valueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get value
  value.format = vpiDecStrVal;
  vpi_get_value(valueHandle, &value);
  newValue = atoll(value.value.str);

  // search knobs database and set value
  sknobs_set_value(name, newValue);
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_string_calltf
int vpi_sknobs_get_string_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle defaultValueHandle;
  s_vpi_value value;
  char *result, *defaultValue;
  char *name;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  defaultValueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get default value
  defaultValue = "";
  if (defaultValueHandle) {
    value.format = vpiStringVal;
    vpi_get_value(defaultValueHandle, &value);
    defaultValue = value.value.str;
  }

  // search knobs database for result
  result = sknobs_get_string(name, defaultValue);

  // return result
  value.format = vpiStringVal;
  value.value.str = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_string_sizetf
int vpi_sknobs_get_string_sizetf() {
  return 8192*8;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterate_sizetf
int vpi_sknobs_iterate_sizetf() {
  return 64;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterate_calltf
int vpi_sknobs_iterate_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  s_vpi_value value;
  sknobs_iterator_p result;
  char *name;
  char retBuffer[SKNOBS_MAX_LENGTH];

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_iterate(name);

  // return result
  value.format = vpiHexStrVal;
  sprintf(retBuffer, "%llx", result);
  value.value.str = retBuffer;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);

  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterator_next_calltf
int vpi_sknobs_iterator_next_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  s_vpi_value value;
  char *name;
  int result;
  sknobs_iterator_p iterator;
  long long tmpll;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  
  // get iterator
  value.format = vpiHexStrVal;
  vpi_get_value(nameHandle, &value);
  tmpll = strtoll(value.value.str, (char **)NULL, 16);
  iterator = (sknobs_iterator_p)tmpll;

  // search knobs database for result
  result = sknobs_iterator_next(iterator);

  // return result
  value.format = vpiIntVal;
  value.value.integer = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterator_next_sizetf
int vpi_sknobs_iterator_next_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterator_get_string_calltf
int vpi_sknobs_iterator_get_string_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  s_vpi_value value;
  char *name, *result;
  sknobs_iterator_p iterator;
  long long tmpll;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  
  // get iterator
  value.format = vpiHexStrVal;
  vpi_get_value(nameHandle, &value);
  tmpll = strtoll(value.value.str, (char **)NULL, 16);
  iterator = (sknobs_iterator_p)tmpll;

  // search knobs database for result
  result = sknobs_iterator_get_string(iterator);

  // return result
  value.format = vpiStringVal;
  value.value.str = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_iterator_get_string_sizetf
int vpi_sknobs_iterator_get_string_sizetf() {
  return 8192*8;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_set_string_calltf
int vpi_sknobs_set_string_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle valueHandle;
  s_vpi_value value;
  char *name;
  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  valueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get value
  value.format = vpiStringVal;
  vpi_get_value(valueHandle, &value);

  // search knobs database and set value
  sknobs_set_string(name, value.value.str);
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_init_calltf
int vpi_sknobs_init_calltf() {
  vpiHandle sysTfHandle;
  s_vpi_value value;
  int result;
  result = vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_init_sizetf
int vpi_sknobs_init_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_close_calltf
int vpi_sknobs_close_calltf() {
  sknobs_close();
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_add_calltf
int vpi_sknobs_add_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle patternHandle;
  vpiHandle valueHandle;
  vpiHandle commentHandle;
  s_vpi_value value;
  int result;
  char *pattern, *newValue, *comment;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  patternHandle = vpi_scan(argIterator);
  valueHandle = vpi_scan(argIterator);
  commentHandle = vpi_scan(argIterator);
  
  // get pattern
  value.format = vpiStringVal;
  vpi_get_value(patternHandle, &value);
  pattern = expandName(value.value.str);

  // get value
  value.format = vpiStringVal;
  vpi_get_value(valueHandle, &value);
  //have to strdup it because newValue and comment pointed to the 
  //same string value.value.str so when comment is read, 
  //the newValue pointer also changes.
  newValue = strdup((const char *)value.value.str);

  // get comment
  comment = "";
  if (commentHandle) {
    value.format = vpiStringVal;
    vpi_get_value(commentHandle, &value);
    comment = value.value.str;
  }

  // search knobs database for result
  result = sknobs_add(pattern, newValue, comment);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  free(newValue);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_add_sizetf
int vpi_sknobs_add_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_calltf
int vpi_sknobs_load_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle commentHandle;
  s_vpi_value value;
  s_vpi_vlog_info vlog_info;
  int result;
  char *comment;

  //init still needed as the init does a whole bunch of things 
  //apart from loading argc,argv from command line. these include
  //looking for .knobsrc files, environmental variables, 
  //setting seed, and setting up debug info.
  //so should be called seperately, by the user
  //vpi_sknobs_init();
  vpi_get_vlog_info(&vlog_info);

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  commentHandle = vpi_scan(argIterator);
  
  // get comment
  comment = "";
  if (commentHandle) {
    value.format = vpiStringVal;
    vpi_get_value(commentHandle, &value);
    comment = value.value.str;
  }

  // search knobs database for result
  result = sknobs_load(vlog_info.argc, vlog_info.argv, comment);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_sizetf
int vpi_sknobs_load_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_string_calltf
int vpi_sknobs_load_string_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle bufferHandle;
  vpiHandle commentHandle;
  s_vpi_value value;
  int result;
  char *name, *newBuffer, *comment;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  bufferHandle = vpi_scan(argIterator);
  commentHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get newBuffer
  value.format = vpiStringVal;
  vpi_get_value(bufferHandle, &value);
  newBuffer = strdup(value.value.str);

  // get comment
  comment = "";
  if (commentHandle) {
    value.format = vpiStringVal;
    vpi_get_value(commentHandle, &value);
    comment = value.value.str;
  }

  // search knobs database for result
  result = sknobs_load_string(name, newBuffer, comment);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_string_sizetf
int vpi_sknobs_load_string_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_file_calltf
int vpi_sknobs_load_file_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle filenameHandle;
  s_vpi_value value;
  int result;
  char *filename;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  filenameHandle = vpi_scan(argIterator);
  
  // get filename
  value.format = vpiStringVal;
  vpi_get_value(filenameHandle, &value);
  filename = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_load_file(filename);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_file_sizetf
int vpi_sknobs_load_file_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_file_if_exists_calltf
int vpi_sknobs_load_file_if_exists_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle filenameHandle;
  s_vpi_value value;
  int result;
  char *filename;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  filenameHandle = vpi_scan(argIterator);
  
  // get filename
  value.format = vpiStringVal;
  vpi_get_value(filenameHandle, &value);
  filename = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_load_file_if_exists(filename);

  // return result
  value.format = vpiScalarVal;
  value.value.scalar = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_load_file_if_exists_sizetf
int vpi_sknobs_load_file_if_exists_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_find_file_calltf
int vpi_sknobs_find_file_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle filenameHandle;
  s_vpi_value value;
  char* result;
  char *filename;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  filenameHandle = vpi_scan(argIterator);
  
  // get filename
  value.format = vpiStringVal;
  vpi_get_value(filenameHandle, &value);
  filename = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_find_file(filename);

  // return result
  value.format = vpiStringVal;
  value.value.str = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_find_file_sizetf
int vpi_sknobs_find_file_sizetf() {
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_filename_calltf
int vpi_sknobs_get_filename_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle defaultValueHandle;
  s_vpi_value value;
  char *result, *defaultValue;
  char *name;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  defaultValueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get default value
  defaultValue = "";
  if (defaultValueHandle) {
    value.format = vpiStringVal;
    vpi_get_value(defaultValueHandle, &value);
    defaultValue = value.value.str;
  }

  // search knobs database for result
  result = sknobs_get_filename(name, defaultValue);

  // return result
  value.format = vpiStringVal;
  value.value.str = result;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_filename_sizetf
int vpi_sknobs_get_filename_sizetf() {
  return 8192*8;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_dynamic_value_calltf
int vpi_sknobs_get_dynamic_value_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle nameHandle;
  vpiHandle defaultValueHandle;
  s_vpi_value value;
  long long result, defaultValue;
  char *name;
  char retBuffer[SKNOBS_MAX_LENGTH];

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  nameHandle = vpi_scan(argIterator);
  defaultValueHandle = vpi_scan(argIterator);
  
  // get name
  value.format = vpiStringVal;
  vpi_get_value(nameHandle, &value);
  name = expandName(value.value.str);

  // get default value
  defaultValue = 0L;
  if (defaultValueHandle) {
    value.format = vpiDecStrVal;
    vpi_get_value(defaultValueHandle, &value);
    defaultValue = atoll(value.value.str);
  }

  // search knobs database for result
  result = sknobs_get_dynamic_value(name, defaultValue);

  // return result
  value.format = vpiHexStrVal;
  sprintf(retBuffer, "%llx", result);
  value.value.str = retBuffer;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_get_dynamic_value_sizetf
int vpi_sknobs_get_dynamic_value_sizetf() {
  return 64;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_set_seed_calltf
int vpi_sknobs_set_seed_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle valueHandle;
  s_vpi_value value;
  long newValue;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  valueHandle = vpi_scan(argIterator);
  
  // get value
  value.format = vpiDecStrVal;
  vpi_get_value(valueHandle, &value);
  newValue = atol(value.value.str);

  // search knobs database and set value
  sknobs_set_seed(newValue);
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_eval_calltf
int vpi_sknobs_eval_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle exprHandle;
  s_vpi_value value;
  long long result;
  char *expr;
  char retBuffer[SKNOBS_MAX_LENGTH];

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  exprHandle = vpi_scan(argIterator);
  
  // get expr
  value.format = vpiStringVal;
  vpi_get_value(exprHandle, &value);
  expr = expandName(value.value.str);

  // search knobs database for result
  result = sknobs_eval(expr);

  // return result
  value.format = vpiHexStrVal;
  sprintf(retBuffer, "%llx", result);
  value.value.str = retBuffer;
  vpi_put_value(sysTfHandle, &value, NULL, vpiNoDelay);
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_eval_sizetf
int vpi_sknobs_eval_sizetf() {
  return 64;
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_dump_calltf
int vpi_sknobs_dump_calltf() {
  vpi_sknobs_init();
  sknobs_dump();
}

////////////////////////////////////////////////////////////////////////////
// vpi_sknobs_save_calltf
int vpi_sknobs_save_calltf() {
  vpiHandle sysTfHandle;
  vpiHandle argIterator;
  vpiHandle filenameHandle;
  s_vpi_value value;
  long long result;
  char *filename;

  vpi_sknobs_init();

  // get handle to system function call
  sysTfHandle = vpi_handle(vpiSysTfCall, NULL);

  // get handles to arguments
  argIterator = vpi_iterate(vpiArgument, sysTfHandle);
  filenameHandle = vpi_scan(argIterator);
  
  // get filename
  value.format = vpiStringVal;
  vpi_get_value(filenameHandle, &value);
  filename = expandName(value.value.str);

  // search knobs database for result
  sknobs_save(filename);
}

////////////////////////////////////////////////////////////////////////////
// vpi_register_tfs
void vpi_register_tfs( void ) {
    s_vpi_systf_data systf_data;
    vpiHandle        systf_handle;

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_exists";
    systf_data.calltf      = vpi_sknobs_exists_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_exists_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_get_value";
    systf_data.calltf      = vpi_sknobs_get_value_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_get_value_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysTask;
    systf_data.sysfunctype = 0;
    systf_data.tfname      = "$sknobs_set_value";
    systf_data.calltf      = vpi_sknobs_set_value_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_get_string";
    systf_data.calltf      = vpi_sknobs_get_string_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_get_string_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_iterate";
    systf_data.calltf      = vpi_sknobs_iterate_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_iterate_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_iterator_next";
    systf_data.calltf      = vpi_sknobs_iterator_next_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_iterator_next_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_iterator_get_string";
    systf_data.calltf      = vpi_sknobs_iterator_get_string_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_iterator_get_string_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysTask;
    systf_data.sysfunctype = 0;
    systf_data.tfname      = "$sknobs_set_string";
    systf_data.calltf      = vpi_sknobs_set_string_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_init";
    systf_data.calltf      = vpi_sknobs_init_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_init_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_close";
    systf_data.calltf      = vpi_sknobs_close_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_add";
    systf_data.calltf      = vpi_sknobs_add_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_add_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_load";
    systf_data.calltf      = vpi_sknobs_load_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_load_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_load_string";
    systf_data.calltf      = vpi_sknobs_load_string_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_load_string_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_load_file";
    systf_data.calltf      = vpi_sknobs_load_file_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_load_file_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_load_file_if_exists";
    systf_data.calltf      = vpi_sknobs_load_file_if_exists_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_load_file_if_exists_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_find_file";
    systf_data.calltf      = vpi_sknobs_find_file_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_find_file_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_get_filename";
    systf_data.calltf      = vpi_sknobs_get_filename_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_get_filename_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_get_dynamic_value";
    systf_data.calltf      = vpi_sknobs_get_dynamic_value_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_get_dynamic_value_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_set_seed";
    systf_data.calltf      = vpi_sknobs_set_seed_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_eval";
    systf_data.calltf      = vpi_sknobs_eval_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = vpi_sknobs_eval_sizetf;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_dump";
    systf_data.calltf      = vpi_sknobs_dump_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );

    systf_data.type        = vpiSysFunc;
    systf_data.sysfunctype = vpiSysFuncSized;
    systf_data.tfname      = "$sknobs_save";
    systf_data.calltf      = vpi_sknobs_save_calltf;
    systf_data.compiletf   = 0;
    systf_data.sizetf      = 0;
    systf_data.user_data   = 0;
    systf_handle = vpi_register_systf( &systf_data );
    vpi_free_object( systf_handle );
}

////////////////////////////////////////////////////////////////////////////
// vlog_startup_routines
void (*vlog_startup_routines[])() = {
  vpi_register_tfs,
  0
};
