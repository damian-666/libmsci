#ifndef SOLVE_LODE__H
#define SOLVE_LODE__H

#include <gsl/gsl_vector_complex.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_matrix_complex_double.h>

//solves systems of linear ordinary differential equations of the following form:
// dx/dt = A x
//where a is a matrix, x is a vector, and t is a scalar

//uses eigenvalue decomposition:
// dx/dt = V^{-1} L V x
// d(Vx)/dt = L V x
// Vx = exp(Lt) V x_0
// x = V^{-1} exp(Lt) V x_0



namespace libpetey {

  //performs the eigenvalue decomposition: A=v*eval*vinv
  int diagonalize_matrix(gsl_matrix *A0, 	//matrix to decompose
		gsl_matrix_complex *v, 		//right eigenvectors
		gsl_vector_complex *eval, 	//eigenvalues
		gsl_matrix_complex *vinv);	//left eigenvectors

  //check the decomposition:
  void check_eig_decomp(gsl_matrix_complex *v, 		//right eigenvectors
		gsl_vector_complex *eval, 	//eigenvalues
		gsl_matrix_complex *vinv,	//left eigenvectors
		gsl_matrix_complex *result);

  //tests the eigen-value decomposition with a random matrix:
  int test_lode(int n,				//size of problem
		  double tol);			//expected tolerance

  //computes: x=vinv*exp(t*eval)*v*x0
  int solve_lode(gsl_vector *x0, 		//initial conditions
		gsl_matrix_complex *v, 		//right eigenvectors
		gsl_vector_complex *eval,  	//eigenvalues
		gsl_matrix_complex *vinv,	//left eigenvectors
		double t, 			//integration time
		gsl_vector *x);			//final conditions

} //end namespace libpetey

#endif
