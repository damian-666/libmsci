#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "parse_command_opts.h"

#include "av.h"
#include "sparse_array.h"

using namespace std;
using namespace libpetey;
using namespace libsparse;

int main(int argc, char **argv) {
  FILE *fs;
  char *infile;
  char *outfile;
  char c;
  sparse_matrix dummy;
  sparse_array<ind_t, float, sparse_matrix> A;
  long ncon;

  ind_t m, n;	//size of matrix
  int32_t i0, N;		//start point, number of matrices
  int32_t nev, ncv;		//number of eigenvectors, number of Arnoldi vectors

  float *v;		//eigenvectors
  float *eval;		//eigenvalues

  int32_t nvar;

  int sa_dir;

  int32_t maxiter=300;
  float tol=0.0;
  char which[2]={'L', 'M'};

  void *optarg[20];
  int flag[20];

  i0=0;
  N=-1;

  ncv=20;
  nev=4;

  sa_dir=-1;

  optarg[0]=&nev;
  optarg[1]=&ncv;
  optarg[2]=&i0;
  optarg[3]=&N;
  optarg[4]=&tol;
  optarg[5]=&maxiter;

  //parse the command line arguments:
  argc=parse_command_opts(argc, argv, "vAONtIWl", "%d%d%d%d%g%d%s%", optarg, flag, 1);
  if (flag[6] && strlen((char *) optarg[6])>=2) {
    which[0]=((char *) optarg[6])[0];
    which[1]=((char *) optarg[6])[1];
  }
  if (flag[7]) sa_dir=1;

  if (argc < 3) {
    fprintf(stdout, "\n");
    fprintf(stdout, "Syntax:  ev_sparse_array [-O start] [-N N] [-I maxI] [-t tol]\n");
    fprintf(stdout, "                         [-v nv] [-A nA] [-W which] [-l] infile outfile\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Performs a singular-value decomposition on an array of\n");
    fprintf(stdout, "sparse vectors\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "where:\n");
    fprintf(stdout, "  infile  =  input file name, binary dump of an array of sparse matrices\n");
    fprintf(stdout, "  outfile =  output file name for singular vectors\n");
    fprintf(stdout, "  start   =  starting point in array\n");
    fprintf(stdout, "  N       =  number of sparse matrices to use\n");
    fprintf(stdout, "  nv      =  number of singular vectors to compute [=%d]\n", nev);
    fprintf(stdout, "  nA      =  number of Arnoldi vectors to use [=%d]\n", ncv);
    fprintf(stdout, "  maxI    =  maximum number of iterations [=%d]\n", maxiter);
    fprintf(stdout, "  tol     =  tolerance [=%f]\n", tol);
    fprintf(stdout, "  which   =  which eigenvalues to return [=%2s]:\n", which);
    fprintf(stdout, "               LM = largest in magnitude\n");
    fprintf(stdout, "               SM = smallest in magnitude\n");
    fprintf(stdout, "               LA = largest algebraicly\n");
    fprintf(stdout, "               SA = smallest algebraicly\n");
    fprintf(stdout, "               BE = half from each end of the spectrum\n");
    fprintf(stdout, "  -l      =  return left eigenvalues [default is right]\n");
    return 1;
  }

  infile=argv[1];
  outfile=argv[2];

  //read in the array of sparse matrices:
  fprintf(stderr, "Reading file: %s\n", infile);
  fs=fopen(infile, "r");
  for (long i=0; i<i0; i++) dummy.read(fs);
  A.read(fs, N);
  fclose(fs);
  fprintf(stderr, "%ld sparse matrices read in\n", (int64_t) A.nel());
  A.dimensions(m, n);
 
  if (sa_dir > 0) {
    A.transpose();
    n=m;
  }

  v=new float[2*n*ncv];
  eval=new float[nev*2];

  //call the fortran program:
  FORTRAN_FUNC(sarevd)(&n, &nev, &ncv, v, eval, &maxiter, &tol, which, (void *) &A);

  //write the output:
  fs=fopen(outfile, "w");
  nvar=n;		//for compatibility with libagf
  fwrite(&nvar, sizeof(nvar), 1, fs);
  fwrite(v, sizeof(float), 2*n*nev, fs);
  fclose(fs);

  for (int i=0; i<nev; i++) {
    printf("%g + %g i\n", eval[i], eval[i+nev]);
  }
  printf("\n");

  //clean up:
  delete [] v;

}

