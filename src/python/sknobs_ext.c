////////////////////////////////////////////////////////////////////////////
// Python extension for knobs
//
// Using py3c for python2/python3 compatibility.  To remove python2 support and
// the dependency on py3c, follow:
// https://py3c.readthedocs.io/en/latest/guide-cleanup.html
//
// Also consider moving to cython or cffi for more permanent non-manual
// extension support.
////////////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <py3c.h>
#include <stdio.h>

#include "sknobs.h"

////////////////////////////////////////////////////////////////////////////
static PyObject*
init(PyObject *self, PyObject *args) {
  int argc ;
  char **argv ;
  int i;
  if (!PyList_Check(args)) {
    PyErr_SetString(PyExc_ValueError, "Expecting a list");
    return NULL;
  }
  argc = PyList_Size(args);
  argv = (char **) malloc((argc+1)*sizeof(char *));
  for (i=0; i<argc; ++i) {
    PyObject *s = PyList_GetItem(args, i);
    if (!PyStr_Check(s)) {
      free(argv);
      PyErr_SetString(PyExc_ValueError, "List items must be strings");
      return NULL;
    }
    argv[i] = PyStr_AsString(s);
  }
  argv[i] = 0;
  return PyInt_FromLong(sknobs_init(argc, argv)); 
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
exists(PyObject *self, PyObject *args) {
  char *name = PyStr_AsString(args);
  return PyInt_FromLong(sknobs_exists(name)); 
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
get_value(PyObject *self, PyObject *args) {
  char *name ;
  unsigned long long defaultValue ;
  if(!PyArg_ParseTuple(args,(char *)"sL:get_value",&name,&defaultValue)) 
    return NULL;
  return PyLong_FromUnsignedLongLong(sknobs_get_value(name, defaultValue)); 
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
get_string(PyObject *self, PyObject *args) {
  char *name, *defaultValue;
  if(!PyArg_ParseTuple(args,(char *)"ss:get_string",&name, &defaultValue)) 
    return NULL;
  return PyStr_FromString(sknobs_get_string(name, defaultValue));
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
set_value(PyObject *self, PyObject *args) {
  char *name ;
  unsigned long long value ;
  if(!PyArg_ParseTuple(args,(char *)"sL:set_value",&name,&value)) 
    return NULL;
  sknobs_set_value(name, value); 
  Py_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
set_seed(PyObject *self, PyObject *args) {
  long value ;
  if(!PyArg_ParseTuple(args,(char *)"l:set_value",&value)) 
    return NULL;
  sknobs_set_seed(value); 
  Py_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
add(PyObject *self, PyObject *args) {
  char *name, *value;
  if(!PyArg_ParseTuple(args,(char *)"ss:add",&name, &value)) 
    return NULL;
  sknobs_add(name, value, "python");
  Py_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
set_string(PyObject *self, PyObject *args) {
  char *name, *value;
  if(!PyArg_ParseTuple(args,(char *)"ss:set_string",&name, &value)) 
    return NULL;
  sknobs_set_string(name, value);
  Py_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
get_all_strings(PyObject *self, PyObject *args) {
  char *pattern = PyStr_AsString(args);
  sknobs_iterator_p iterator = sknobs_iterate(pattern);
  PyObject *pl = PyList_New(0);
  while (sknobs_iterator_next(iterator))
    PyList_Append(pl, PyStr_FromString(sknobs_iterator_get_string(iterator)));
  return pl;
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
find_file(PyObject *self, PyObject *args) {
  char *filename = PyStr_AsString(args);
  return PyStr_FromString(strdup(sknobs_find_file(filename)));
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
get_filename(PyObject *self, PyObject *args) {
  char *name, *defaultValue;
  if(!PyArg_ParseTuple(args,(char *)"ss:get_string",&name, &defaultValue)) 
    return NULL;
  return PyStr_FromString(sknobs_get_filename(name, defaultValue));
}

////////////////////////////////////////////////////////////////////////////
static PyObject *
save(PyObject *self, PyObject *args) {
  char *filename = PyStr_AsString(args);
  sknobs_save(filename);
  Py_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////////////
static PyMethodDef sknobs_funcs[] = {
  {"init",            (PyCFunction)init,            METH_O,       "init(argv): initialize\n"},
  {"exists",          (PyCFunction)exists         , METH_O,       "exists(name): determine if name has a value\n"},
  {"get_value",       (PyCFunction)get_value,       METH_VARARGS, "get_value(name, default): get integer value of knob\n"},
  {"get_string",      (PyCFunction)get_string,      METH_VARARGS, "get_string(name, default): get string value of knob\n"},
  {"get_all_strings", (PyCFunction)get_all_strings, METH_O,       "get_all_strings(name): get list of strings\n"},
  {"add",             (PyCFunction)add,             METH_VARARGS, "add(name, value): add knob\n"},
  {"set_value",       (PyCFunction)set_value,       METH_VARARGS, "set_value(name, value): set integer value of knob\n"},
  {"set_seed",        (PyCFunction)set_seed,        METH_VARARGS, "set_seed(value): set global seed\n"},
  {"set_string",      (PyCFunction)set_string,      METH_VARARGS, "set_string(name, value): set string value of knob\n"},
  {"find_file",       (PyCFunction)find_file,       METH_O,       "find_file(name): search for file\n"},
  {"get_filename",    (PyCFunction)get_filename,    METH_VARARGS, "get_filename(name, default): get filename from value of knob\n"},
  {"save",            (PyCFunction)save,            METH_O,       "save(filename): save knobs database to file\n"},
  {NULL}
};

////////////////////////////////////////////////////////////////////////////
static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT, /* m_base */
  "sknobs",              /* m_name */
  NULL,                  /* m_doc */
  -1,                    /* m_size */
  sknobs_funcs           /* m_methods */
};

////////////////////////////////////////////////////////////////////////////
MODULE_INIT_FUNC(sknobs)
{
  PyObject *m;
  m = PyModule_Create(&moduledef);
  if (m == NULL) {
    return NULL;
  }
  return m;
}
