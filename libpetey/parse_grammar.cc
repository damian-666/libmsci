#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "grammar_t.h"

int main(int argc, char **argv) {
  FILE *fs;
  grammar_t grammar;
  char *line, *line2;
  parse_tree *pt;
  int nerr;

  fs=fopen(argv[1], "r");
  nerr=grammar.load(fs);
  fclose(fs);

  if (nerr>0) {
    fprintf(stderr, "The grammer in %s contained %d errors\n", argv[1], nerr);
  }

  while (feof(stdin)==0) {
    line=readline("*GG*");
    if (strlen(line)>1) add_history(line);
    //add back a string terminator character:
    line2=new char[strlen(line)+2];
    sprintf(line2, "%s\\", line);

    pt=grammar.parse(line2);

    if (pt==NULL) {
      printf("'%s' is not a valid expression\n");
    } else {
      pt->print();
      delete pt;
    }
    delete [] line;
    delete [] line2;
  }

  return nerr;

}
    
