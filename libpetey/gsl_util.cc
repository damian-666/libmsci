#include <stdio.h>
#include <stdio.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_matrix_complex_double.h>

#include "randomize.h"

namespace libpetey {

  //random debugging code...
  void print_gsl_matrix(FILE *fs, gsl_matrix * mat) {
    for (int i=0; i<mat->size1; i++) {
      for (int j=0; j<mat->size2; j++) {
        fprintf(fs, "%10.5g ", gsl_matrix_get(mat, i, j));
      }
      fprintf(fs, "\n");
    }
  }

  //random debugging code...
  void print_gsl_matrix_complex(FILE *fs, gsl_matrix_complex * mat) {
    for (int i=0; i<mat->size1; i++) {
      for (int j=0; j<mat->size2; j++) {
        fprintf(fs, "%12.5g+%12.5gi ", 
			GSL_REAL(gsl_matrix_complex_get(mat, i, j)),
			GSL_IMAG(gsl_matrix_complex_get(mat, i, j)));
      }
      fprintf(fs, "\n");
    }
  }

  void random_gsl_matrix(gsl_matrix *A) {
    for (int i=0; i<A->size1; i++) {
      for (int j=0; j<A->size2; j++) {
	gsl_matrix_set(A, i, j, ranu());
      }
    }
  }
  gsl_matrix *random_gsl_matrix(int m, int n) {
    gsl_matrix *A=gsl_matrix_alloc(m, n);
    random_gsl_matrix(A);
    return A;
  }

  void gsl_throw_handler(const char *reason, const char * file, int line, int gsl_errno) {
    fprintf(stderr, "Error in %s, line %d: %s\n", file, line, reason);
    throw gsl_errno;
  }

  void gsl_continue_handler(const char *reason, const char * file, int line, int gsl_errno) {
    fprintf(stderr, "Error in %s, line %d: %s\n", file, line, reason);
  }

}

