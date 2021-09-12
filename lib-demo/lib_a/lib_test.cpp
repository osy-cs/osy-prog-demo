//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Testing program for static and dynamic library.
// Verion of library: see Makefile
//
//***************************************************************************

#include <stdio.h>

// Header file for library
#include "modules.h"

int main()
{
  // function from library
  module_1();

  // function from library
  module_2();
}
