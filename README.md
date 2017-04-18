# sknobs

## Introduction

"Knobs” is a common term for a method of controlling various parameterizable aspects of a simulation.
However, the implementation and usage is far from common and usually not as complete as what is
going to be presented here. This implementation is feature rich and provides natural interfaces to
practically all languages that are likely to be used in a design verification environment.

The sknobs module processes command line arguments, builds a database of knob values and provides
access functions.  It is implemented in c and includes wrappers for common verification languages,
including python, tcl, verilog and go.

Knobs can be of the following form:
*  +k=a
*  +k=a~b                        /* choose value between a and b inclusive */
*  +k=a,b,c,d                    /* list with equal weightings */
*  +k=a:weighta,b:weightb        /* weighted list */

Knob files can be read in hierarchically similar to how verilog processes argument files.  
In addition you can specify multiple files which will be selected at random, thus providing 
the ability to select between different groups of related knob values.

* -f filename
* -f file1,file2,file3
* -f file1:weight1,file2:weight2,file3:weight3

Knob values can be locked down by prefixing the value with an additional =.
For example

+myImportantValue==100

Once this option is encounted during the linear search, the search will stop and will effective 
overide any values that were specified later in the input database.

Knobs can be specified using regular expressions, by default glob
style matching is used, however if the knob is specified with an extra
"+" then extended regular expressions can be used. For example:

  ++cpu[0-2]\\..*.cache.enable=1

This * will typically need to be escaped when used on a command line like so:

  ++cpu\[0-2\]\\.\*.cache.enable=1

The knobs database is loaded after a call to knobs_init, a seed for selected first randomly 
by using the time of day, this can be overridding by setting a SEED environment variable or by passing in an
+seed=n option on the command line.  After the seed has been determined a number of initialization files 
are tried.

  1. ~/.knobsrc
  2. starting with CWD read all *.knobsrc files.  Then work up file hierarchy to /.
  3. process environment variable SKNOBS
  4. process command line arguments

The module can be put into debug mode by setting the debug level environment variable SKNOBS_DEBUG.

  setenv SKNOBS_DEBUG 0  # quiet
  setenv SKNOBS_DEBUG 1  # very basic stuff
  setenv SKNOBS_DEBUG 2  # more
  setenv SKNOBS_DEBUG 3  # including information about random choices begin made.

There are two flavors for tokenizing strings containing knobs.  Knob-
containing-strings come from files and from the contents of envvar SKNOBS.
Knobs from the commandline are already tokenized by the shell.  The original
flavor used any of {space ' ', tab '\t', newline '\n'} to delimit knobs.  This
means that knobs in a file or in SKNOBS can not contain whitespace.  This
flavor is preserved as the default.  Another flavor exists to allow whitespace
by only using {carriage-return '\r', newline '\n'} as the set of delimiters.
The behavior is controlled by the SKNOBS_DELIMITER_FLAVOR environment variable.

  SKNOBS_DELIMITER_FLAVOR       Token string

  <none>                        " \t\n"
  sptbnl                        " \t\n"
  crnl                          "\r\n"

The knobs database is queried with four main functions:
  exists("name")
  get_string("name", defaultValue)
  get_value("name", defaultValue)
  get_dynamic_value("name", defaultValue)

The function get_value determines a value and saves it away so that a
call to an identical name will return exactly the same value.  If you
wish each call to return a newly selected random value then use
get_dynamic_value.

Values can be set in the database by the program by using the functions:
  set_string("name", "value")
  set_value("name", value)
although I expect this will rarely be used because knobs are usually
specified either in files or on the command line.

Examples of how to use each language wrapper can be seen by looking at
the test functions that are compiled in each of the language specific
subdirectories.

