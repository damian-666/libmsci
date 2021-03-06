#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "error_codes.h"

namespace libpetey {

  void get_format(const char *format, int ind, char *formi) {
    int k=0;
    int i=0;
    int j;

    for (i=1; format[i]!='\0' && k<ind; i++) {
      if (format[i]=='%') k++;
    }
    if (format[i]=='\0' && k!=ind) {
      //means your code needs fixin'
      fprintf(stderr, "parse_command_opts: Error in command option format specification\n");
      exit(-1);
    }

    formi[0]=format[i-1];
    for (j=i; format[j] != '\0' && format[j] != '%'; j++) {
      formi[j-i+1]=format[j];
    }
    formi[j-i+1]='\0';

  } 
 
 
  #define FORMLEN 10

  //returns number of arguments still left
  //if there is a non-fatal error, returns -(# of found options)
  //currently the following format codes are recognized:
  //%d, %g, %ld, %lg, %c, %s
  //a flag or switch is denoted by a '%' with nothing following
  //
  //to have the parameters read directly into named variables,
  //use the following indirection:
  //
  //float parm0;
  //void *pall[NOPTS];
  //pall[0]=(void *) &parm0;
  //
  //since strings are allocated within the function
  //we can only assign them after calling the function:
  //
  //char *parm2;
  //parse_command_opts(...);
  //parm2=(char *) pall[2];
  //
  //
  //bits in opts:
  //first bit: allow whitespace between option and argument
  //second bit: do not flag unrecognized options or remove from list
  //third bit: ignore numbers (allows negative numbers in regular argument list)
  //
  //[**note: removes from list arguments already parsed]
  //edit: now shifts non-option arguments to front of list
  //
  //to blank out an existing option while keeping everything else the same,
  //replace it with a space--spaces are skipped

  int parse_command_opts(int argc,	//number of command line args 
		char **argv, 		//arguments passed to command line
		const char *code, 	//code for each option
		const char *format, 	//format code for each option
		void **parm,		//returned parameters
                int *flag,		//flags
		int fopts=0) {		//option flags

    int nleft;
    char opt;
    char *optarg;
    int err=1;
    int ncon;

    char form[FORMLEN];

    int nopts=strlen(code);
    int rflag[argc];

    int inflag;

    int allopts;
    int wsflag;

    wsflag=fopts % 2;
    inflag=(fopts >> 2) % 2;
    allopts=(fopts >> 1) % 2;

    //printf("%d %d %d\n", inflag, allopts, wsflag);

    for (int i=0; i<nopts; i++) flag[i]=0;

    for (int i=1; i<argc; i++) {
      rflag[i]=1;
      if (argv[i][0]=='-') {
        opt=argv[i][1];
        if (inflag && (opt >= '0' && opt <= '9')) continue;
        int j=0;
        for (j=0; j<nopts; j++) {
          if (opt==code[j] && code[j]!=' ') {
            rflag[i]=0;
            flag[j]=1;
            get_format(format, j, form);
            //printf("format: %s\n", form);
            if (form[1]!='\0') {
              if (argv[i][2]=='\0') {
                if (wsflag == 0) {
                  fprintf(stderr, "parse_command_opts: Command option, -%c, missing argument.\n", opt);
                  err=-1;
                  parm[i]=NULL;
                } else {
                  if (i+1 >= argc) {
                    fprintf(stderr, "parse_command_opts: Error parsing command option: -%c\n", opt);
                    err=-1;
                    parm[i]=NULL;
                    break;
                  }
                  optarg=argv[i+1];
                  i++;
                  rflag[i]=0;
                }
              } else {
                optarg=argv[i]+2;
              }
              if (form[1]=='s') {
                //copy the string...
                //(is it better to copy it or just assign it?? valgrind complains...)
                parm[j]=new char [strlen(optarg)+1];
	        strcpy((char *) parm[j], optarg);
                //printf("%s\n", optarg);
                ncon=1;
              } else if (form[1]=='c') {
                * (char *) parm[j]=optarg[0];
	        ncon=1;
              } else if (form[1]=='d') {
                ncon=sscanf(optarg, form, (int32_t *) parm[j]);
              } else if (form[1]=='g') {
                ncon=sscanf(optarg, form, (float *) parm[j]);
              } else if (form[1]=='l') {
	        if (form[2]=='d') {
                  ncon=sscanf(optarg, form, (int64_t *) parm[j]);
	        } else if (form[2]=='g') {
                  ncon=sscanf(optarg, form, (double *) parm[j]);
                }
              }
	    
              if (ncon==0) {
                err=-1;
                fprintf(stderr, "parse_command_opts: Error parsing command option -%c\n", code[j]);
              }
            }
            break;
          }
        }
        if (j >=nopts) {
          if (allopts==0) {
            rflag[i]=0;
            fprintf(stderr, "parse_command_opts: Unrecognized command line option %s\n", argv[i]);
            err=-1;
          }
        }
      }
    }

    //prune out optional arguments:
    //edit: push them to the back (opposite of getopt--just to be contrary!)
    nleft=1;
    char **optall=new char *[argc];
    for (int i=1; i<argc; i++) {
      if (rflag[i]) {
        argv[nleft]=argv[i];
        nleft++;
      } else {
        optall[i-nleft]=argv[i];
      }
    }

    for (int i=nleft; i<argc; i++) argv[i]=optall[i-nleft];
    delete [] optall;
        
    return err*nleft;

  }

} //end namespace libpetey

