#include <stdio.h>
#include <assert.h>

#include "sknobs.h"

////////////////////////////////////////////////////////////////////////////
// main
int main(int argc, char *argv[]) {
  long v;
  char *s;
  char **fl;
  int i;
  sknobs_iterator_p iterator;
  if (sknobs_init(argc, argv)) {
    printf("error: knobs: unable to init\n");
    return 1;
  }
  printf("seed: %ld\n", sknobs_get_value("seed", 0));
  sknobs_set_string("stu", "1:2,10~12:5");
  sknobs_set_string("xyz", "=there");
  sknobs_set_string("abc", "7~9");
  // Should be OK.
  assert(sknobs_add_string("bareequals=", "") == 0);
  // Should be error.
  assert(sknobs_add_string("=nopattern", ""));
  // Should be error.
  assert(sknobs_add_string("=", ""));
  // Should be error.
  assert(sknobs_add_string("", ""));
  // Should be error.
  assert(sknobs_add_string(0, ""));
  sknobs_dump();
  printf("static:\n");
  for (i=0; i<10; ++i)
    printf("abc=%ld\n", sknobs_get_value("abc", 2));
  printf("dynamic:\n");
  for (i=0; i<10; ++i)
    printf("abc=%ld\n", sknobs_get_dynamic_value("abc", 2));
  for (i=0; i<10; ++i)
    printf("stu=%ld\n", sknobs_get_dynamic_value("stu", 2));
  printf("xyz=%s\n", sknobs_get_string("xyz", "hi"));
  printf("find_file: .knobsrc: %s\n", sknobs_find_file(".knobsrc"));
  printf("derefxyz=%s\n", sknobs_get_string("derefxyz", "$(xyz).test.$(abc)"));
  printf("ttt=%s\n", sknobs_get_string("ttt", "ttt not found"));
  sknobs_add("list", "first", "");
  sknobs_add("list", "second", "");
  sknobs_add("list", "third", "");
  iterator = sknobs_iterate("list");
  while(sknobs_iterator_next(iterator))
    printf("s from list = %s\n", sknobs_iterator_get_string(iterator));
  sknobs_save("testsknobs.saved");
  sknobs_close();

  return 0;
}
