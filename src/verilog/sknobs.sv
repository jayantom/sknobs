package sknobs;

//import "DPI-C" function int sknobs_init(input int argc, input string argv[]);
//import "DPI-C" function int sknobs_init(input int argc, input string argv[]);
import "DPI-C" function int sknobs_init(input int argc);
import "DPI-C" function void sknobs_close();

import "DPI-C" function int sknobs_add(input string pattern, input string value, input string comment);

//import "DPI-C" function int sknobs_load(input int argc, input string argv[], input string comment);

import "DPI-C" function int sknobs_load_string(input string name, input string buffer, input string comment);
import "DPI-C" function int sknobs_load_file(input string filename);
import "DPI-C" function int sknobs_load_file_if_exists(input string filename);

import "DPI-C" function int sknobs_exists(input string name);

import "DPI-C" function chandle sknobs_iterate(input string name);
import "DPI-C" function int sknobs_iterator_next(input chandle iterator);
import "DPI-C" function longint sknobs_iterator_get_value(input chandle iterator);
import "DPI-C" function string sknobs_iterator_get_string(input chandle iterator);

import "DPI-C" function string sknobs_get_string(input string name, input string default_value);
import "DPI-C" function string sknobs_find_file(input string filename);
import "DPI-C" function string sknobs_get_filename(input string filename, input string default_value);

import "DPI-C" function longint sknobs_get_value(input string name, input longint default_value);
import "DPI-C" function longint sknobs_get_dynamic_value(input string  name, input longint default_value);

import "DPI-C" function void sknobs_set_string(input string name, input string value);
import "DPI-C" function void sknobs_set_value(input string name, input longint value);

import "DPI-C" function longint sknobs_eval(input string expr);

import "DPI-C" function void sknobs_dump();

import "DPI-C" function void sknobs_save(input string filename);

  function bit exists(string name);
    exists = sknobs_exists(name);
  endfunction // bit

  function chandle iterate(string name);
     iterate = sknobs_iterate(name);
  endfunction // chandle
   
  function int iterator_next(chandle iterator);
     iterator_next = sknobs_iterator_next(iterator);
  endfunction // int
   
  function longint iterator_get_value(chandle iterator);
     iterator_get_value = sknobs_iterator_get_value(iterator);
  endfunction // longint
   
  function string iterator_get_string(chandle iterator);
     iterator_get_string = sknobs_iterator_get_string(iterator);
  endfunction // string

  function string get_string(string name, string default_value="unspecified");
    get_string = sknobs_get_string(name, default_value);
    // TBD figure out how to use the reporter here, chicken and egg problem
    $display("%0d:I:sknobs: +%s=%0d", $time, name, get_string);
  endfunction
  
  function string find_file(string filename);
    find_file = sknobs_find_file(filename);
    $display("%0d:I:sknobs: find_file: %s=%s", $time, filename, find_file);
  endfunction

  function string get_filename(string filename, string default_value="unspecified");
    get_filename = sknobs_get_filename(filename, default_value);
  endfunction

  function longint get_value(string name, longint default_value=0);
    get_value = sknobs_get_value(name, default_value);
    // TBD figure out how to use the reporter here, chicken and egg problem
    $display("%0d:I:sknobs: +%s=%0d", $time, name, get_value);
  endfunction
  
  function longint get_dynamic_value(string name, longint default_value=0);
    get_dynamic_value = sknobs_get_dynamic_value(name, default_value);
  endfunction

  function void set_value(string name, longint value);
    void'(sknobs_set_value(name, value));
  endfunction

  function void set_string(string name, string value);
    void'(sknobs_set_string(name, value));
  endfunction // void

  function longint eval(string expr);
    eval = sknobs_eval(expr);
  endfunction // longint
endpackage
