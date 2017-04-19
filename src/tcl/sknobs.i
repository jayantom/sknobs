%module sknobs

// This tells SWIG to treat char ** as a special case
%typemap(in) (int argc, char **argv) {
    Tcl_Obj **listobjv;
    int       nitems;
    int       i;
    if (Tcl_ListObjLength(interp, $input, &nitems) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, $input, &nitems, &listobjv) == TCL_ERROR) {
        return TCL_ERROR;
    }
    $2 = (char **) malloc((nitems+2)*sizeof(char *));
    $2[0] = "sknobs";
    for (i = 0; i < nitems; i++) {
      $2[i+1] = Tcl_GetStringFromObj(listobjv[i],0);
    }
    $2[i+1] = 0;
    $1 = nitems+1;
}

// This gives SWIG some cleanup code that will get called after the function call
%typemap(freearg) (int argc, char **argv) {
  if ($2) {
     free($2);
  }

}

#include "sknobs.h"

%{
#include "sknobs.h"
%}

