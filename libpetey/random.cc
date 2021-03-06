#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parse_command_opts.h"
#include "randomize.h"
#include "error_codes.h"

//part of the test suite

//generates a list of random numbers

using namespace libpetey;

int main(int argc, char **argv) {
  int32_t n;
  double num;
  double offset=0;
  double scale_factor=1;
  void *opt[10];
  int flag[10];

  opt[0]=&scale_factor;
  opt[1]=&offset;

  argc=parse_command_opts(argc, argv, "vmg?S", "%lg%lg%%%", opt, flag, 1);
  if (argc<0) {
    fprintf(stderr, "random: error parsing command line\n");
    exit(FATAL_COMMAND_OPTION_PARSE_ERROR);
  }

  if (argc < 2 || flag[3]) {
    printf("Usage: random [-g] [-v var] [-m mean] n\n");
    printf("n     = number of deviates\n");
    printf("var   = variance (Gaussian)/span (uniform)\n");
    printf("mean  = mean (Guassian)/minimum (uniform)\n");
    printf("-g    = Gaussian deviates\n");
    printf("-?    = print this help screen\n");
    return 0;
  }

  sscanf(argv[1], "%d", &n);

  ran_init();

  if (flag[4]) {
    unsigned long seed;
    if (n==1) {
      seed=seed_from_clock();
      printf("%u\n", seed);
    } else {
      char *command=new char [strlen(argv[0])+6];
      sprintf(command, "%s -S 1", argv[0]);
      for (int32_t i=0; i<n; i++) {
        system(command);
      }
      delete [] command;
    }
  } else {
    for (int32_t i=0; i<n; i++) {
      if (flag[2]) {
        num=rang();
      } else {
        num=ranu();
      }
      printf("%lg\n", scale_factor*num+offset);
    }
  }

  ran_end();

  return 0;

}

  
