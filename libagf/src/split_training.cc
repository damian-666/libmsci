#include <math.h>
#include <string.h>
#include <stdio.h>

#include <gsl/gsl_linalg.h>

#include "randomize.h"
#include "peteys_tmpl_lib.h"
#include "full_util.h"
#include "agf_lib.h"

using namespace std;
using namespace libagf;
using namespace libpetey;

//should just do this in sed...

int main(int argc, char *argv[]) {
  FILE *fs;
  FILE *diagfs;			//print diagnostics to this file stream

  char **line;
  nel_ta nline;		//number of lines

  agf_command_opts opt_args;

  //parse the command line arguments:
  exit_value=agf_parse_command_opts(argc, argv, "d:f:0HR", &opt_args);
  if (exit_value==FATAL_COMMAND_OPTION_PARSE_ERROR) return exit_value;

  //print help page if there are not enough mandatory arguments:
  if ((opt_args.stdinflag && (opt_args.fflag && argc<2 || opt_args.div=0 && argc<1))
  		|| (opt_args.fflag && argc<3) || (opt_args.div > 0 && argc<2)) {

    printf("\n");
    printf("Purpose:    Splits an ASCII file with one sample per line into training and\n");
    //(should just do this in sed)
    printf("            test segments\n");
    printf("\n");
    printf("Syntax:	split_training [-d ndiv] [-f frac] [-H] [-R]\n");
    printf("                          [input] train [test]]\n");
    printf("\n");
    printf("arguments:\n");
    printf("  input        ASCII input file\n");
    printf("  train        ASCII file containing training data or base name of each\n");
    printf("               segment in n-fold cross-validation\n");
    printf("  test         base name for test data\n");
    printf("\n");
    printf("file options:\n");
    printf("  -0           read from stdin (ASCII or no ordinate data (-C) only)\n");
    printf("  -H           no dimension header\n");
    printf("\n");
    printf("  -R           data separation works by random selection rather than\n");
    printf("                 simple separation\n");
    printf("  -d ndiv      number of separate output files\n");
    printf("  -f frac      separate into test and training (over-rides -d)\n");
    printf("\n");
    return INSUFFICIENT_COMMAND_ARGS;
  }

  diagfs=stderr;
  ran_init();

  if (opt_args.stdinflag==0) {
    if (opt_args.asciiflag || opt_args.Cflag) {
      vecfile=new char[strlen(argv[0])+1];
      sprintf(vecfile, "%s", argv[0]);
    } else {
      vecfile=new char[strlen(argv[0])+5];
      sprintf(vecfile, "%s.vec", argv[0]);
    }
  }

  if (opt_args.svd>0 || opt_args.normflag || opt_args.selectflag || opt_args.normfile!=NULL) {
    if (opt_args.normfile==NULL) {
      if (opt_args.stdoutflag) {
        fprintf(stderr, "agf_preprocess: please specify normalization file with -a\n");
        exit(INSUFFICIENT_COMMAND_ARGS);
      } else {
        opt_args.normfile=new char[strlen(argv[1])+5];
        sprintf(opt_args.normfile, "%s.std", argv[1]);
      }
    }
    command=new char [strlen(opt_args.normfile)+50+4*argc+strlen(argv[0])];
    sprintf(command, "%s%s%s -a %s", AGF_COMMAND_PREFIX,
		AGF_LTRAN_COM, AGF_OPT_VER, opt_args.normfile);
    if (opt_args.asciiflag) {
      strcat(command, " -A");
      if (opt_args.Mflag) {
        strcat(command, " -M");		//svm format
      }
    }
    if (opt_args.stdinflag) {
      strcat(command, " -0");		//read from stdin
    }
    strcat(command, " -1");		//always write to stdout
    //normalization by average and std. dev.:
    if (opt_args.normflag) strcat(command, " -n");
    //singular value decomposition:
    if (opt_args.svd!=0) {
      sprintf(command+strlen(command), " -S %d", opt_args.svd);
    }
    //for some versions of getopt, mandatory arguments have to be at end:
    if (opt_args.selectflag) strcat(command, " -F");
    if (opt_args.stdinflag==0) sprintf(command+strlen(command), " %s", vecfile);
    if (opt_args.selectflag) {
      int i0=2;
      if (opt_args.stdinflag && opt_args.stdoutflag) {
        i0=0;
      } else if (opt_args.stdinflag || opt_args.stdoutflag) {
        i0=1;
      } else if (opt_args.fflag) {
        i0=3;
      }
      for (int i=i0; i<argc; i++) {
        sprintf(command+strlen(command), " %s", argv[i]);
      }
    }
    fprintf(stderr, "%s\n", command);
    fs=popen(command, "r");
    //this is a fucking mess....
  } else {
    if (opt_args.stdinflag) {
      fs=stdin;
    } else {
      fs=fopen(vecfile, "r");
    }
  }
      
  if (fs==NULL) {
    fprintf(stderr, "agf_preprocess: Unable to open coordinate data for reading\n");
    exit(UNABLE_TO_OPEN_FILE_FOR_READING);
  }

  //read in training data and set the output file names:
  //fprintf(stderr, "agf_preprocess: Reading training data\n");
  if (opt_args.stdoutflag==0) outbase=argv[1];
  if (opt_args.asciiflag) {
    cls_ta *cls;
    if (opt_args.Mflag) {
      ntrain=read_svm(fs, train, cls, nvar);
    } else {
      ntrain=read_lvq(fs, train, cls, nvar, opt_args.Cflag*2+opt_args.Hflag);
    }
    ord=(void *) cls;
    if (command!=NULL) pclose(fs); else fclose(fs);
    
  } else {
    int32_t nt1, nv1;

    train=read_matrix<real_a, int32_t>(fs, nt1, nv1);
    if (nt1 == -1) {
      fprintf(stderr, "agf_preprocess: Error reading coordinate data\n");
      exit(FILE_READ_ERROR);
    }
    ntrain=nt1;
    nvar=nv1;
    if (command!=NULL) pclose(fs); else fclose(fs);

    if (opt_args.stdoutflag==0) {
      outvec=new char[strlen(outbase)+5];
      if (opt_args.Cflag) {
        sprintf(outvec, "%s", outbase);
      } else {
        sprintf(outvec, "%s.vec", outbase);
      }
    }

    if (opt_args.Cflag==0) {
      clsfile=new char[strlen(argv[0])+5];
      if (opt_args.Lflag) {
        sprintf(clsfile, "%s.dat", argv[0]);
      } else {
        sprintf(clsfile, "%s.cls", argv[0]);
      }
    }

    ord=NULL;

    if (opt_args.Cflag==0) {
      outcls=new char[strlen(outbase)+5];
      if (opt_args.Lflag) {
        sprintf(outcls, "%s.dat", outbase);
      } else {
        sprintf(outcls, "%s.cls", outbase);
      }

      ord=(void *) read_clsfile<cls_ta>(clsfile, ntrain2);
      if (ntrain2 == -1) {
        fprintf(stderr, "agf_preprocess: Error reading file: %s\n", clsfile);
        exit(FILE_READ_ERROR);
      }
      if (ord==NULL) {
          fprintf(stderr, "agf_preprocess: Unable to open file for reading: %s\n", clsfile);
          exit(UNABLE_TO_OPEN_FILE_FOR_READING);
      }
      fprintf(diagfs, "%d class labels found in file: %s\n", ntrain2, clsfile);
      if (ntrain2!=ntrain) {
        fprintf(stderr, "agf_preprocess: Sample count mismatch\n");
        exit(SAMPLE_COUNT_MISMATCH);
      }
    }
  }

  /*FLAG*/
  //this is NOT conducive to easy maintenance:
  if (opt_args.stdinflag==0) {
    argv++;
    argc--;
  }
  if (opt_args.stdoutflag==0) {
    argv++;
    argc--;
  }

  if (opt_args.fflag) {
    testvec=new char[strlen(argv[0])+5];
    sprintf(testvec, "%s.vec", argv[0]);
    testcls=new char[strlen(argv[0])+5];
    if (opt_args.Lflag) {
      sprintf(testcls, "%s.dat", argv[0]);
    } else {
      sprintf(testcls, "%s.cls", argv[0]);
    }
    argv++;
    argc--;
  }

  nres=ntrain;
  all=train[0];		//need this when it's time to clean up

  if (opt_args.Cflag==0 && opt_args.Lflag==0) {
    //fprintf(stderr, "agf_preprocess: mapping class labels\n");
    cls_ta *cls=(cls_ta *) ord;
    cls_ta *clsmap;
    cls_ta ncls1;
    //count the number of classes:
    ncls=1;
    for (cls_ta i=0; i<ntrain; i++) if (cls[i]>=ncls) ncls=cls[i]+1;

    int err;

    if (argc>0 && opt_args.selectflag==0) {
      nel_ta ntnew=0;

      clsmap=new cls_ta[ncls];
      err=parse_partition(argc, argv, ncls, clsmap);
      if (err!=0) {
        fprintf(stderr, "agf_preprocess: error parsing class partition\n");
        exit(err);
      }
      apply_partition(cls, ntrain, clsmap);
      ncls1=1;
      for (cls_ta i=0; i<ncls; i++) if (clsmap[i]>=ncls1) ncls1=clsmap[i]+1;
      ncls=ncls1;
      for (nel_ta i=0; i<ntrain; i++) {
        if (cls[i]>=0) {
          cls[ntnew]=cls[i];
          train[ntnew]=train[i];
          ntnew++;
        }
      }
      nres=ntnew;
      delete [] clsmap;
    }

    //compress the class labels:
    if (opt_args.Uflag) {
      clsmap=new cls_ta[ncls];
      for (cls_ta i=0; i<ncls; i++) clsmap[i]=0;
      //(could use the old clsmap if it exists...)
      for (nel_ta i=0; i<ntrain; i++) clsmap[cls[i]]=1;
      ncls1=0;
      for (cls_ta i=0; i<ncls; i++) {
        if (clsmap[i]!=0) {
          clsmap[i]=ncls1;
          ncls1++;
        }
      }
      apply_partition(cls, ntrain, clsmap);
      ncls=ncls1;
      delete [] clsmap;
    }

  }


  if (opt_args.Pflag) {
    //fprintf(stderr, "agf_preprocess: calculating covariance matrix\n");
      real_a **cov;
      real_a diff1, diff2;

      cov=zero_matrix<real_a, int32_t>(nvar, nvar);

      ave=new real_a[nvar];

      for (dim_ta i=0; i<nvar; i++) {
        ave[i]=0;
        for (nel_ta j=0; j<ntrain; j++) ave[i]+=train[j][i];
	ave[i]/=ntrain;
      }
      for (dim_ta i=0; i<nvar; i++) {
	for (dim_ta j=0; j<nvar; j++) {
          for (nel_ta k=0; k<ntrain; k++) {
            diff1=train[k][i]-ave[i];
	    diff2=train[k][j]-ave[j];
	    cov[i][j]+=diff1*diff2;
	  }
        }
      }
      for (dim_ta i=0; i<nvar; i++) {
        for (dim_ta j=0; j<nvar; j++) {
          cov[i][j]/=(ntrain-1);
	  printf("%12.6g ", cov[i][j]);
	}
	printf("\n");
      }

      delete_matrix(train);
      train=cov;
      nres=nvar;
      opt_args.Cflag=1;		//don't output ordinate data
      delete [] ave;
  }

  //randomly permute the data:
  if (opt_args.zflag && opt_args.Pflag==0) {
    //fprintf(stderr, "agf_preprocess: randomizing data\n");
    real_a **tnew=new real_a *[nres];
    long *rind=randomize(nres);
    for (nel_ta i=0; i<nres; i++) {
      tnew[i]=train[rind[i]];
    }
    delete [] train;
    train=tnew;
    if (opt_args.Cflag==0) {
      void *cnew=new char[nres*ordsize];
      for (nel_ta i=0; i<nres; i++) {
        for (size_t j=0; j<ordsize; j++) {
          ((char *) cnew)[ordsize*i+j]=((char *) ord)[ordsize*rind[i]+j];
        }
      }
      //stupid way of doing things (this whole program is a fucking mess...)
      if (opt_args.Lflag) {
        real_a *ord2=(real_a *) ord;
        delete [] ord2;
      } else {
        cls_ta *cls=(cls_ta *) ord;
        delete [] cls;
      }
      ord=cnew;
    }
    delete [] rind;
    //randomize_vec(train, nvar, nres, cls);
  }

  if (opt_args.Bflag && opt_args.Pflag==0 && opt_args.Cflag==0) {
    //fprintf(stderr, "agf_preprocess: sorting ordinates\n");
    if (opt_args.Lflag) {
      long *sind;
      real_a *ord2=(real_a *) ord;
      real_a **tnew=new real_a *[nres];
      real_a *onew=new real_a[nres];
      sind=heapsort(ord2, nres);
      for (nel_ta i=0; i<nres; i++) {
        tnew[i]=train[sind[i]];
        onew[i]=ord2[sind[i]];
      }
      delete [] train;
      delete [] ord2;
      train=tnew;
      ord=(void *) onew;
      delete [] sind;
    } else {
      nel_ta *cind=sort_classes(train, nres, (cls_ta *) ord, ncls);
      fprintf(diagfs, "Class locations:\n");
      for (cls_ta i=0; i<ncls; i++) {
        fprintf(diagfs, "%d: %d %d\n", i, cind[i], cind[i+1]-cind[i]+1);
      }
      delete [] cind;
    }
  }

  //write the results to a file:
  if (opt_args.fflag && opt_args.Pflag!=1) {
    //fprintf(stderr, "agf_preprocess: splitting input data and writing to files\n");
    //randomize not only the division, but the numbers:
    if (opt_args.Rflag) {
      FILE *trainvecfs;
      FILE *trainclsfs;
      FILE *testvecfs;
      FILE *testclsfs;
      trainvecfs=fopen(outvec, "w");
      testvecfs=fopen(testvec, "w");
      if (opt_args.Hflag==0) {
        fwrite(&nvar, sizeof(nvar), 1, trainvecfs);
      }
      if (opt_args.Cflag==0) {
        trainclsfs=fopen(outcls, "w");
        testclsfs=fopen(testcls, "w");
      }
      if (opt_args.Hflag==0) fwrite(&nvar, sizeof(nvar), 1, testvecfs);
      for (nel_ta i=0; i<nres; i++) {
        if (ranu() < opt_args.ftest) {
          fwrite(train[i], sizeof(real_a), nvar, testvecfs);
          if (opt_args.Cflag==0) fwrite(((char *) ord)+i*ordsize, ordsize, 1, testclsfs);
        } else {
          fwrite(train[i], sizeof(real_a), nvar, trainvecfs);
          if (opt_args.Cflag==0) fwrite(((char *) ord)+i*ordsize, ordsize, 1, trainclsfs);
        }
      }
      fclose(trainvecfs);
      fclose(testvecfs);
      if (opt_args.Cflag==0) {
        fclose(trainclsfs);
        fclose(testclsfs);
      }
    } else {
      size_t l1, l2;
      //l1=size_t(nres*(1-opt_args.ftest));	//don't do this...
      l2=size_t(nres*opt_args.ftest);
      l1=nres-l2;
      fs=fopen(outvec, "w");
      if (opt_args.Hflag==0) {
        fwrite(&nvar, sizeof(nvar), 1, fs);
      }
      for (nel_ta i=0; i<l1; i++) {
        fwrite(train[i], sizeof(real_a), nvar, fs);
      }
      fclose(fs);
      if (opt_args.Cflag==0) {
        fs=fopen(outcls, "w");
        fwrite(ord, ordsize, l1, fs);
        fclose(fs);
      }
      fs=fopen(testvec, "w");
      if (opt_args.Hflag==0) {
        fwrite(&nvar, sizeof(nvar), 1, fs);
      }
      for (nel_ta i=l1; i<nres; i++) {
        fwrite(train[i], sizeof(real_a), nvar, fs);
      }
      fclose(fs);
      if (opt_args.Cflag==0) {
        fs=fopen(testcls, "w");
        fwrite(((char *) ord)+l1*ordsize, ordsize, l2, fs);
        fclose(fs);
      }
    }
    delete [] testvec;
    delete [] testcls;
  } else if (opt_args.div>0) {
    char *outfile;
    int rann;
    outfile=new char[strlen(outbase)+8];
    if (opt_args.Rflag) {
      FILE **outvfs;
      FILE **outcfs;
      outvfs=new FILE * [opt_args.div];
      outcfs=new FILE * [opt_args.div];
      for (int32_t i=0; i<opt_args.div; i++) {
        sprintf(outfile, "%s-%2.2d.vec", outbase, i);
        outvfs[i]=fopen(outfile, "w");
        if (opt_args.Hflag==0) {
          fwrite(&nvar, sizeof(nel_ta), 1, outvfs[i]);
        }
        if (opt_args.Cflag==0) {
          if (opt_args.Lflag) {
            sprintf(outfile, "%s-%2.2d.dat", outbase, i);
          } else {
            sprintf(outfile, "%s-%2.2d.cls", outbase, i);
          }
          outcfs[i]=fopen(outfile, "w");
        }
      }
  //  clind=sort_classes(vec, nvar, cls, n, ncl);
      for (nel_ta i=0; i<nres; i++) {
        rann=ranu()*opt_args.div;
        fwrite(train[i], sizeof(float), nvar, outvfs[rann]);
        if (opt_args.Cflag==0) {
          fwrite(((char *) ord)+i*ordsize, ordsize, 1, outcfs[rann]);
        }
      }

      for (int32_t i=0; i<opt_args.div; i++) {
        fclose(outvfs[i]);
        if (opt_args.Cflag==0) fclose(outcfs[i]);
      }
    } else {
      nel_ta l1, l2;
      l1=0;
      for (int32_t i=0; i<opt_args.div; i++) {
        sprintf(outfile, "%s-%2.2d.vec", outbase, i);
        fs=fopen(outfile, "w");
        if (opt_args.Hflag==0) fwrite(&nvar, sizeof(nel_ta), 1, fs);
        l2=(i+1)*nres/opt_args.div;
        for (nel_ta j=l1; j<l2; j++) {
          fwrite(train[j], sizeof(real_a), nvar, fs);
        }

        fclose(fs);
        if (opt_args.Cflag==0) {
          if (opt_args.Lflag) {
            sprintf(outfile, "%s-%2.2d.dat", outbase, i);
          } else {
            sprintf(outfile, "%s-%2.2d.cls", outbase, i);
          }
          fs=fopen(outfile, "w");
          fwrite(((char *) ord)+l1*ordsize, ordsize, l2-l1, fs);
          fclose(fs);
        }
        l1=l2;
      }
    }
    delete [] outfile;
  } else {
    //fprintf(stderr, "agf_preprocess: writing output data\n");
    if (opt_args.asciiflag) {
      if (opt_args.stdoutflag) {
        fs=stdout;
      } else {
        fs=fopen(outbase, "w");
      }
      print_lvq_svm(fs, train, (cls_ta *) ord, nres, nvar, opt_args.Mflag, opt_args.Hflag);
      if (opt_args.stdoutflag==0) fclose(fs);
    } else {
      if (opt_args.stdoutflag) {
        fs=stdout;
      } else {
        fs=fopen(outvec, "w");
      }
      if (fs==NULL) {
        fprintf(stderr, "Unable to open file for writing: %s\n", outvec);
        return UNABLE_TO_OPEN_FILE_FOR_WRITING;
      }
      fwrite(&nvar, sizeof(nvar), 1, fs);
      for (nel_ta i=0; i<nres; i++) {
        fwrite(train[i], sizeof(real_a), nvar, fs);
      }
      if (opt_args.stdoutflag==0) fclose(fs);
      if (opt_args.Cflag==0) {
        fs=fopen(outcls, "w");
        if (fs==NULL) {
          fprintf(stderr, "Unable to open file for writing: %s\n", outvec);
          return UNABLE_TO_OPEN_FILE_FOR_WRITING;
        }
        fwrite(ord, ordsize, nres, fs);
        fclose(fs);
      }
    }
  }

  //clean up:
  delete [] all;
  delete [] train;

  if (opt_args.Cflag==0) {
    if (opt_args.Lflag) {
      real_a *dum=(real_a *) ord;
      delete [] dum;			//is this valid??
    } else {
      cls_ta *cls=(cls_ta *) ord;
      delete [] cls;
    }
  }
  if (clsfile!=NULL) delete [] clsfile;
  if (outcls!=NULL) delete [] outcls;

  if (vecfile!=NULL) delete [] vecfile;
  if (outvec!=NULL) delete [] outvec;

  if (command!=NULL) delete [] command;

  if (opt_args.normfile!=NULL) delete [] opt_args.normfile;

  ran_end();

  return exit_value;

}


