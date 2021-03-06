#ifndef AGF_EVAL_H_INCLUDED
#define AGF_EVAL_H_INCLUDED

#include <stdio.h>

namespace libagf {

  //contingency table includes row and column subtotals
  template <class cls_t>
  nel_ta **build_contingency_table(cls_t *truth, 	//true class values
		cls_t *ret, 				//retrieved class values
		nel_ta n, 				//number of values
		cls_t &nclt, 				//number of classes in truth
		cls_t &nclr);				//number of classes in retrieved

  //calculate the uncertainty coefficient:
  template <typename cls_t, typename count_t>
  double uncertainty_coefficient(count_t **acc_mat, 	//contingency table
		cls_t nclt, 				//rows in contingency table
		cls_t nclr, 				//columns in contingency table
		double &ucr, 				//reverse uncertainty coeff.
		double &uct);				//symmetric coeff.
 
  //print out the contingency table: 
  template <class cls_t>
  void print_contingency_table(nel_ta **acc_mat, 	//contingency table
		cls_t nclt, 				//rows ("truth")
		cls_t nclr, 				//columns ("retrived")
		FILE *fs=stdout);			//print to this file
      
  //full evaluation of the accuracy of a classification retrieval
  //- contingency table, uncertainty coefficients, accuracy
  template <class cls_t>
  double class_eval(cls_t *truth, 			//true classes
		cls_t *ret, 				//retrived classes
		nel_ta n, 				//number of values
		FILE *fs=stdout);			//output file stream

  //basic evaluation of the accuracy of a classification retrieval
  //- uncertainty coefficient, accuracy
  template <class cls_t>
  double class_eval_basic(cls_t *truth, 			//true classes
		cls_t *ret, 				//retrived classes
		nel_ta n, 				//number of values
		FILE *fs=stdout,			//output file stream
		flag_a Hflag=0);			//don't print header

  //test the accuracy of the confidence ratings:
  //calculate table accuracy as a function of confidence rating
  template <class real, class cls_t>
  real ** con_acc_table(cls_t *truth, 			//true classes
		cls_t *cls, 				//retrived classes
		real *con, 				//confidence ratings
		nel_ta n, 				//number of values
		int nhist);				//size of table
							// (number of histogram bins)

  //two class case:
  template <class real, class cls_t>
  real ** con_acc_table2(cls_t *truth, 			//true classes
		real *r, 				//difference in conditional prob.
		nel_ta n, 				//number of values
		int nhist);				//half size of table

  //two class case:
  template <class real, class cls_t>
  real ** con_acc_table2(cls_t *truth, 			//true classes
		cls_t *cls, 				//retrived classes
		real *con, 				//confidence ratings
		nel_ta n, 				//number of values
		int nhist);				//half size of table

  //print the table accuracy vs. confidence rating:
  template <class real>
  void print_con_acc(real **table, 			//table acc (1st row) vs. conf (2nd row)
		int ncls, 				//number of class values
		int nhist, 				//size of table
		FILE *fs=stdout);			//output file stream

  //prints out a table of accuracies versus compute confidence ratings:
  template <class real, class cls_t>
  void check_confidence(cls_t *truth, 		//actual classes
		cls_t *cls, 			//retrieved classes
		real *con, 			//retrieved confidence ratings
		nel_ta n, 			//number of samples
		int nhist,			//size of table
		FILE *fs = stdout);		//print to this file stream

  template <typename real, typename cls_t>
  int validate_cond_prob(cls_t *class1,		//true classes
		  real **p,			//conditional prob.
		  nel_ta n,			//number of samples
		  cls_t ncls,			//number of classes
		  real &corr,			//Pearson coefficient
		  real &slope,			//slope
		  real &brier,			//Brier score
		  real &rms,			//sum of squares from best fit
		  real &norm,			//highest poss. value for sumsqr
		  FILE *fs=NULL);

  template <typename real, typename cls_t>
  int validate_cond_prob(cls_t *class1,		//true classes
		  real *p,			//conditional prob.
		  cls_t *class2,		//estimated classes
		  nel_ta n,			//number of samples
		  real &corr,			//Pearson coefficient
		  real &slope,			//slope
		  real &brier,			//Brier score
		  real &rms,			//sum of squares from best fit
		  real &norm,			//highest poss. value for sumsqr
		  FILE *fs=NULL);

} //end namespace libagf

#endif

