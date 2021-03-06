#ifndef AV_H_INCLUDED
#define AV_H_INCLUDED 1

#include <stdint.h>
#include <complex>

#include "matrix_base.h"

#ifndef FLEADING_UNDERSCORES
  #define FLEADING_UNDERSCORES
#endif
#ifndef FTRAILING_UNDERSCORES
  #define FTRAILING_UNDERSCORES _
#endif

#define CAT2(ARG1,ARG2,ARG3) ARG1##ARG2##ARG3
#define CAT3(ARG1,ARG2,ARG3) CAT2(ARG1,ARG2,ARG3)

#define FORTRAN_FUNC(FUNC_NAME) CAT3(FLEADING_UNDERSCORES,FUNC_NAME,FTRAILING_UNDERSCORES)

#define DEF_MAXNITER 500
#define DEF_TOL 0.00000001

extern "C" {
  void FORTRAN_FUNC(sarsvd)(int32_t *n, int32_t *nev, int32_t *ncv, float *v, 
		float *eval, int32_t *maxitr, float *tol, 
		const char which[2], void *parm);
  void FORTRAN_FUNC(darsvd)(int32_t *n, int32_t *nev, int32_t *ncv, double *v, 
		double *eval, int32_t *maxitr, double *tol,
		const char which[2], void *parm);
  void FORTRAN_FUNC(sarevd)(int32_t *n, int32_t *nev, int32_t *ncv, float *v, 
		float *eval, int32_t *maxitr, float *tol,
		const char which[2], void *parm);
  void FORTRAN_FUNC(darevd)(int32_t *n, int32_t *nev, int32_t *ncv, double *v, 
		double *eval, int32_t *maxitr, double *tol, 
		const char which[2], void *parm);
}

extern "C" {
  void FORTRAN_FUNC(av)(int32_t *nx, float *v, float *w, void *parm);
  void FORTRAN_FUNC(dav)(int32_t *nx, double *v, double *w, void *parm);
}

namespace libpetey {
namespace libsparse {

  //C++ glue functions to glue the Fortran wrapper functions...
  float ** 				//returned eigenvectors
  	cc_arsvd(int32_t n, 		//matrix side length
		int32_t ncv, 			//number of Arnoldi vectors
		int32_t nev, 			//number of eigenvalues
		float *eval,			//returned eigenvalues
		matrix_base<int32_t, float> *parm, //matrix
		const char which[2]="LM",
		int32_t maxitr=DEF_MAXNITER,
		float tol=DEF_TOL);

  double ** cc_arsvd(int32_t n, 
	int32_t ncv, int32_t nev, double *eval,
	matrix_base<int32_t, double> *parm,
		const char which[2]="LM",
		int32_t maxitr=DEF_MAXNITER,
		double tol=DEF_TOL);


  //**should** return a series of complex eigenvalues
  //...if I can figure out how to extract them...
  std::complex<float> ** cc_arevd(int32_t n,
		int32_t ncv,
		int32_t nev,
		std::complex<float> *eval,
		matrix_base<int32_t, float> *parm,
		const char which[2]="LM",
		int32_t maxitr=DEF_MAXNITER,
		float tol=DEF_TOL);

  std::complex<double> ** cc_arevd(int32_t n,
		int32_t ncv,
		int32_t nev,
		std::complex<double> *eval,
		matrix_base<int32_t, double> *parm,
		const char which[2]="LM",
		int32_t maxitr=DEF_MAXNITER,
		double tol=DEF_TOL);

} //end namespace libsparse
} //end namespace libpetey

#endif
