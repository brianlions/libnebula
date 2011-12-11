/*
 * p_message.cc
 *
 *  Created on: Dec 10, 2011
 *  Author:     Brian Y. ZHANG
 *  Email:      brianlions at gmail dot com
 */

#ifndef USE_PRETTY_MESSAGE
#define USE_PRETTY_MESSAGE
#endif
#include <stdlib.h>
#include "nebula/pretty_message.h"

void foo()
{
  BEGIN_FUNC();

  END_FUNC();
}

int main(int argc, char ** argv)
{
  foo();
  exit(0);
}
