
#include <stdio.h>
#include <math.h>

#include <stdint.h>

#include "supernewton.h"
#include "agf_lib.h"

#include "tree_lg.h"
#include "peteys_tmpl_lib.h"

#include "gsl/gsl_rng.h"
//#include "nr.h"

using namespace std;
using namespace libpetey;

namespace libagf {

  //a set of programs designed to find the class borders
  //and use these to perform classifications...

  long agf_global_borders_maxiter=BORDERS_MAXITER;

  //estimate difference in conditional prob.:
  template <class real>
  real agfrfunc(real *x, 			//test point
		void *param1, 		//parameters: sample locations etc.
		real *drdx) {		//gradient of diff. in cond. prob.
    //parameters for calculating the cond. prob.:
    bordparam<real> *bparam=(bordparam<real> *) param1;		//contains training data
    agfparam<real> *aparam=(agfparam<real> *) bparam->rparam;	//parameters for AGF

    //diagnostics:
    agf_diag_param diag_param;
    real f;
 
    //return value: 
    real d;

    if (aparam->k>0) {
      d=dgrad(bparam->train, bparam->D, bparam->n, bparam->ind, x, 
		aparam->var, aparam->k, aparam->W, drdx, &diag_param);

      if (diag_param.f < aparam->min_f) aparam->min_f=diag_param.f;
      else if (diag_param.f > aparam->max_f) aparam->max_f=diag_param.f;
      aparam->total_f+=diag_param.f;
    } else {
      d=dgrad(bparam->train, bparam->D, bparam->n, bparam->ind, x, 
		aparam->var, aparam->W, drdx, &diag_param);

    }

    //fill in the diagnostics:
    if (diag_param.nd < aparam->min_nd) aparam->min_nd=diag_param.nd;
    else if (diag_param.nd > aparam->max_nd) aparam->max_nd=diag_param.nd;
    aparam->total_nd+=diag_param.nd;

    if (diag_param.W < aparam->min_W) aparam->min_W=diag_param.W;
    else if (diag_param.W > aparam->max_W) aparam->max_W=diag_param.W;
    aparam->total_W+=diag_param.W;

    aparam->ncall++;

    return d;
  }

  //initialize all the parameters for AGF algorithm:
  template <class real>
  int agfparam_init(agfparam<real> *param,
			real var[2], 		//variance range
			nel_ta k, 		//number of nearest neighbours
			real W,			//total of weights
			int stickyflag) {	//0 = both brackets fully floating
	  					//1 = first bracket to be reset
						//2 = second bracket to be reset
						//3 = both brackets to be reset
    int err;

    switch (stickyflag) {
      case(1):
        param->var0[0]=var[0];
	param->var0[1]=-1;
	break;
      case(2):
	param->var0[0]=-1;
	param->var0[1]=var[1];
	break;
      case(3):
        param->var0[0]=var[0];
	param->var0[1]=var[1];
	break;
      default:
	param->var0[0]=-1;
	param->var0[1]=-1;
    }

    param->var[0] = var[0];
    param->var[1] = var[1];
    param->k = k;
    param->W = W;

    //zero diagnostics:
    param->min_nd=agf_global_weights_maxiter+2;	//two more than max allowable
    param->max_nd=0;
    param->total_nd=0;

    param->min_f=1;		//maximum allowable
    param->max_f=0;
    param->total_f=0;

    param->min_W=1000*W;		//maximum allowable
    param->max_W=0;
    param->total_W=0;

    param->ncall=0;

    return 0;
  }

  //initialize all the parameters for AGF algorithm:
  template <class real>
  int agfbordparam_init(bordparam<real> *param,
			real **train, 		//locations of training samples
						//sorted by class
			dim_ta D, 		//number of dimensions
			nel_ta n, 		//number of training samples
			nel_ta clind,		//indexes to first sample of second class
			real var[2], 		//variance range
			nel_ta k, 		//number of nearest neighbours
			real W,			//total of weights
			int smallflag) {	//for small datasets
    agfparam<real> *aparam;
    real var1[2];
    int stickyflag=0;
    int err;

    //avoid side-effects:
    var1[0]=var[0];
    var1[0]=var[1];

    if (var[0] <= 0 || var[1] <= 0) {
      //calculate the averages and standard deviations:
      real std[D];
      real ave[D];
      real vart;

      calc_norm(train, D, n, ave, std);

      //if the initial filter variance is not set, set it to the total
      //variance of the data:
      //variance brackets are then fully "sticky..."
      vart=0;
      for (dim_ta i=0; i<D; i++) vart+=std[i]*std[i];
      if (var[0] <= 0) {
        aparam->var[0]=vart/pow(n, 2./D);
        fprintf(stderr, "Using %10.3g for lower filter variance bracket\n\n", var1[0]);
	stickyflag|=1;
      }
      if (var[1] <= 0) {
        aparam->var[1]=vart;
        fprintf(stderr, "Using %10.3g for upper filter variance bracket\n\n", var1[1]);
	stickyflag|=2;
      }
    }

    //transfer training data to function parameters:
    err=bordparam_init(param, train, D, n, clind, smallflag);
    if (err!=0) return err;

    //parameters for AGF algorithm:
    aparam=new agfparam<real>;
    agfparam_init(aparam, var1, k, W, stickyflag);
    param->rparam=aparam;

    return 0;
  }

  template <typename real>
  void agfparam_reset_var(bordparam<real> *param) {
    if (param->var0[0]>0) param->var[0]=param->var0[0];
    if (param->var0[1]>0) param->var[1]=param->var0[1];
  }

  template <class real>
  void agfbordparam_clean(bordparam<real> *param) {
    bordparam_clean(param);
    delete (agfparam<real> *) param->rparam;
  }

  template <class real>
  void print_agfborddiag(bordparam<real> *param1, FILE *fs) {
    agfparam<real> *param=(agfparam<real> *) param1->rparam;

    //print out the diagnostics:
    fprintf(fs, "\n");
    fprintf(fs, "diagnostic parameter          %8s   %8s   %8s\n",
                  "min", "max", "average");
    fprintf(fs, "\n");
    fprintf(fs, "iterations in agf_calc_w:      %8d   %8d %10.3g\n",
                  param->min_nd, param->max_nd,
                  (float) param->total_nd/(float) param->ncall);
    if (param->k>0) {
      fprintf(fs, "value of f:                    %10.3g %10.3g %10.3g\n",
                  param->min_f, param->max_f, param->total_f/param->ncall);
    }
    fprintf(fs, "value of W:                    %10.2f %10.2f %10.2f\n",
                  param->min_W, param->max_W, param->total_W/param->ncall);
    fprintf(fs, "\n");
    fprintf(fs, "total number of calls: %d\n", param->ncall);
    fprintf(fs, "\n");
  }

  //when we have a large number of training samples:
  template <class real>
  nel_ta find_class_borders(real **x, 		//location of samples, sorted by class
			dim_ta nvar, 		//number of dimensions
			nel_ta ntrain,		//number of training vectors
			nel_ta clind, 		//index of start of second class
			nel_ta nsamp, 		//number of samples to take
			real var[2],		//initial filter width
			nel_ta k, 		//number of nearest neighbours to use
			real wc, 		//"critical" weight
			real tol,		//desired tolerance
			real ** border, 	//returned border vectors	
			real ** gradient,	//returned gradient vectors
			real rthresh) {		//threshold for discrimination border

    nel_ta ns2;
    bordparam<real> param;

    agfbordparam_init(&param, x, nvar, ntrain, clind, var, k, wc);
    ns2=sample_class_borders(&agfrfunc, &oppositesample, &param, nsamp, nvar, tol, 
			agf_global_borders_maxiter, border, gradient, rthresh);
    print_agfborddiag(&param, stdout);
    agfbordparam_clean(&param);
    return ns2;
  }

  //when we have a small number of training samples:
  template <class real>
  nel_ta find_class_borders_small(real **x,		//location of samples, sorted by class
			dim_ta nvar, 		//number of dimensions
			nel_ta ntrain,		//number of training vectors
			nel_ta clind, 		//index of start of second class
			nel_ta &nsamp, 		//number of samples to take (returns actual number found)
			real var[2],		//initial filter width
			nel_ta k, 		//number of nearest neighbours to use
			real wc, 		//"critical" weight
			real tol,		//desired tolerance
			real ** border, 	//returned border vectors	
			real ** gradient,	//returned gradient vectors
			real rthresh) {		//threshold for discrimination border

    nel_ta ns2;
    bordparam<real> param;

    agfbordparam_init(&param, x, nvar, ntrain, clind, var, k, wc, 1);
    ns2=sample_class_borders(&agfrfunc, &oppositesample_small, &param, nsamp, nvar, 
			tol, agf_global_borders_maxiter, border, gradient, rthresh);
    print_agfborddiag(&param, stdout);
    agfbordparam_clean(&param);

    return ns2;
  }

  //much more elegant version:
  template <class cls_t>
  int parse_partition(int argc, char **argv, cls_t ncls, cls_t *map) {
    cls_t cls=0;
    int strptr;
    int nread;
    cls_t lower, upper;

    for (cls_t i=0; i<ncls; i++) map[i]=-1;

    for (int i=0; i<argc; i++) {
      if (argv[i][0]==PARTITION_SYMBOL) {
        cls++;
        continue;
      }
      nread=sscanf(argv[i], "%d%n", &lower, &strptr);
      if (nread<=0) {
        fprintf(stderr, "parse_partition: failed to scan class value in argument %d\n", i);
        return COMMAND_OPTION_PARSE_ERROR;
      }
      if (lower < 0 || lower >= ncls) {
        fprintf(stderr, "parse_partition: argument %d; class value out-of-range (%d: must be between [%d, %d]\n", i, lower, 0, ncls-1);
        return PARAMETER_OUT_OF_RANGE;
      }
      if (argv[i][strptr]=='-') {
        nread=sscanf(argv[i]+strptr+1, "%d", &upper);
        if (nread<=0) {
          fprintf(stderr, "parse_partition: failed to scan upper bound in argument %d\n", i);
          return COMMAND_OPTION_PARSE_ERROR;
        }
        if (upper < 0 || upper >= ncls) {
          fprintf(stderr, "parse_partition: argument %d; upper bound out-of-range (%d: must be between [%d, %d]\n", i, upper, 0, ncls-1);
          return PARAMETER_OUT_OF_RANGE;
        }
        if (upper<lower) {
          fprintf(stderr, "parse_partition: range in argument %d; lower bound (%d) must be less than upper (%d)\n", i, lower, upper);
          return PARAMETER_OUT_OF_RANGE;
        }
        for (cls_t j=lower; j<=upper; j++) {
          if (map[j]!=-1) {
            fprintf(stderr, "parse_partition: warning, rewriting old map %d->%d with %d->%d (argument %d)\n", j, map[j], j, cls, i);
          }
          map[j]=cls;
        }
      } else {
        if (map[lower]!=-1) {
          fprintf(stderr, "parse_partition: warning, rewriting old map %d->%d with %d->%d (argument %d)\n", lower, map[lower], lower, cls, i);
        }
        map[lower]=cls;
      }
    }

    return 0;
  }

  //(**efficiently) applying partitions to a set of classes:
  template <class cls_t>
  void apply_partition(cls_t *cls, nel_ta n, cls_t *map) {
    for (cls_t i=0; i<n; i++) {
      cls[i]=map[cls[i]];
    }
  }

  template int agfparam_init<float>(agfparam<float> *, float *, nel_ta, float, int);
  template int agfparam_init<double>(agfparam<double> *, double *, nel_ta, double, int);

  template int agfbordparam_init<float>(bordparam<float> *, 
		  float **, dim_ta, nel_ta, nel_ta, 
		  float[2], nel_ta, float, int);
  template int agfbordparam_init<double>(bordparam<double> *, 
		  double **, dim_ta, nel_ta, nel_ta, 
		  double[2], nel_ta, double, int);

  template void agfbordparam_clean<float>(bordparam<float> *);
  template void agfbordparam_clean<double>(bordparam<double> *);

  template nel_ta find_class_borders<float>(float **x, 
			dim_ta nvar, 
			nel_ta ntrain,
			nel_ta clind, 
			nel_ta nsamp, 
			float var[2],
			nel_ta k, 
			float wc, 
			float tol,
			float ** border, 	
			float ** gradient,
			float rthresh);

  template nel_ta find_class_borders<double>(double **x, 
			dim_ta nvar, 
			nel_ta ntrain,
			nel_ta clind, 
			nel_ta nsamp, 
			double var[2],
			nel_ta k, 
			double wc, 
			double tol,
			double ** border, 	
			double ** gradient,
			double rthresh);

  template nel_ta find_class_borders_small<float>(float **x, 
			dim_ta nvar, 
			nel_ta ntrain,
			nel_ta clind, 
			nel_ta &nsamp, 
			float var[2],
			nel_ta k, 
			float wc, 
			float tol,
			float ** border, 	
			float ** gradient,
			float rthresh);

  template nel_ta find_class_borders_small<double>(double **x, 
			dim_ta nvar, 
			nel_ta ntrain,
			nel_ta clind, 
			nel_ta &nsamp, 
			double var[2],
			nel_ta k, 
			double wc, 
			double tol,
			double ** border, 	
			double ** gradient,
			double rthresh);

  template int parse_partition<cls_ta>(int, char **, cls_ta, cls_ta *);

  template void apply_partition<cls_ta>(cls_ta *, nel_ta n, cls_ta *);

} //end namespace libagf

