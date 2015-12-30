#include <string.h>
#include <math.h>

#include <gsl/gsl_linalg.h>

#include "constrained.h"
#include "read_ascii_all.h"
#include "error_codes.h"

#include "svmkernel.h"
#include "svm_multi.h"
#include "agf_fconv.h"

using namespace libpetey;

namespace libagf {

  template <class real>
  int solve_cond_prob_1v1(real **r, int ncls, real *p) {
    int err;
    gsl_matrix *Q;
    gsl_vector *x;
    gsl_vector *b;

    //Wu and Lin 2004, JMLR 5:975 give linear system to solve for multi-class
    //probabilities in 1 vs. 1 case
    Q=gsl_matrix_alloc(ncls+1, ncls+1);
    x=gsl_vector_alloc(ncls+1);
    b=gsl_vector_alloc(ncls+1);

    //fill the matrix and the solution vector:
    //off-diagonals:
    for (int i=0; i<ncls; i++) {
      for (int j=i+1; j<ncls; j++) {
        real val=r[i][j]*(1-r[i][j]);
        gsl_matrix_set(Q, i, j, val);
	gsl_matrix_set(Q, j, i, val);
      }
    }

    //diagonals plus solution vector:
    for (int i=0; i<ncls; i++) {
      gsl_vector_set(b, i, 0);
      real val=0;
      for (int j=0; j<i; j++) val+=r[j][i]*r[j][i];
      for (int j=i+1; j<ncls; j++) val+=r[i][j]*r[i][j];
      gsl_matrix_set(Q, i, i, val);
      //normality constraint:
      gsl_matrix_set(Q, i, ncls, 1);
      gsl_matrix_set(Q, ncls, i, 1);
    }
    //rest of normality constraint:
    gsl_vector_set(b, ncls, 1);
    gsl_matrix_set(Q, ncls, ncls, 0);

    //use SVD solver:
    err=solver(Q, b, x);

    //re-assign GSL result back to standard floating point array:
    for (int i=0; i<ncls; i++) p[i]=gsl_vector_get(x, i);

    //clean up:
    gsl_matrix_free(Q);
    gsl_vector_free(x);
    gsl_vector_free(b);

    return err;
  }

  template <class real, class cls_t>
  svm_multi<real, cls_t>::svm_multi() {
    this->ncls=0;
    this->D=0;
    nsv_total=0;
    nsv=NULL;
    sv=NULL;
    coef=NULL;
    rho=NULL;
    probA=NULL;
    probB=NULL;
    label=NULL;
  }

  //initialize from LIBSVM model file:
  template <class real, class cls_t>
  svm_multi<real, cls_t>::svm_multi(char *file) {
    FILE *fs=fopen(file, "r");
    char *line=NULL;
    char **substr=NULL;
    int nsub;
    int nparam;		//ncls*(ncls-1)/2: number of binary classifiers
    int nsv1;		//should agree with nsv...
    int pfound=0;	//number of parameters for probability estimation read in (should be 2)
    char format[12];	//format code
    char fcode[4];	//floating point format code
    int lineno=0;

    get_format_code<real>(fcode);
    sprintf(format, "%%%s%%n", fcode);

    //this->mat=NULL;
    //this->b=NULL;
    //this->id=-1;
    
    probA=NULL;
    probB=NULL;

    if (fs==NULL) {
      fprintf(stderr, "svm2class: failed to open model file, %s\n", file);
      throw UNABLE_TO_OPEN_FILE_FOR_READING;
    }

    rho=0;
    param=new real[3];

    do {
      if (line!=NULL) delete [] line;
      if (substr!=NULL) delete [] substr;
      line=fget_line(fs, 1);
      lineno++;
      substr=split_string_destructive(line, nsub);
      //printf("svm2class: nsub=%d\n", nsub);
      //for (int i=0; i<nsub; i++) printf("%s ", substr[i]);
      //printf("\n");
      if (nsub == 0) continue;
      if (strcmp(substr[0], "SV")==0 && nsub<2) {
        fprintf(stderr, "svm_multi: error in initialization file, %s; unrecognized keywordi (%s)/not enough parameters\n", substr[0], file);
        fclose(fs);
        throw FILE_READ_ERROR;
      }
      if (strcmp(substr[0], "svm_type")==0) {
        if (strcmp(substr[1], "c_svc")!=0 && strcmp(substr[1], "nu-svc")!=0) {
          fprintf(stderr, "svm_multi: not a classifier SVM in file, %s (%s)\n", file, substr[1]);
	  fclose(fs);
	  throw PARAMETER_OUT_OF_RANGE;
        }
      } else if (strcmp(substr[0], "nr_class")==0) {
        this->ncls=atoi(substr[1]);
	if (this->ncls < 2) {
          fprintf(stderr, "svm_multi: one class classifiers not accepted (file, %s)\n", file);
	  fclose(fs);
	  throw PARAMETER_OUT_OF_RANGE;
        }
	nparam=this->ncls*(this->ncls-1)/2;
      } else if (strcmp(substr[0], "kernel_type")==0) {
        if (strcmp(substr[1], "linear")==0) {
	  kernel=&linear_basis<real>;
	  kernel_deriv=&linear_basis_deriv<real>;
	  param=NULL;
        } else if (strcmp(substr[1], "polynomial")==0) {
          kernel=&polynomial_basis<real>;
          kernel_deriv=&polynomial_basis_deriv<real>;
	} else if (strcmp(substr[1], "rbf")==0) {
          kernel=&radial_basis<real>;
          kernel_deriv=&radial_basis_deriv<real>;
	} else if (strcmp(substr[1], "sigmoid")==0) {
          kernel=&sigmoid_basis<real>;
          kernel_deriv=&sigmoid_basis_deriv<real>;
	} else {
          fprintf(stderr, "svm2class: basis function, %s, not recognized (file, %s)\n", substr[1], file);
	  fclose(fs);
	  throw PARAMETER_OUT_OF_RANGE;
        }
      } else if (strcmp(substr[0], "gamma")==0) {
        param[0]=atof(substr[1]);
      } else if (strcmp(substr[0], "coef0")==0) {
        param[1]=atof(substr[1]);
      } else if (strcmp(substr[0], "degree")==0) {
        param[2]=atof(substr[1]);
      } else if (strcmp(substr[0], "total_sv")==0) {
        nsv_total=atoi(substr[1]);
      } else if (strcmp(substr[0], "nr_sv")==0) {
        if (nsub<this->ncls) {
          fprintf(stderr, "svm_multi: error in initialization file: not enough parameters (nr_sv) (file, %s)\n", file);
	  fclose(fs);
	  throw FILE_READ_ERROR;
	}
	nsv=new nel_ta[this->ncls];
	for (int i=0; i<this->ncls; i++) nsv[i]=atoi(substr[i]);
      } else if (strcmp(substr[0], "rho")==0) {
        if (nsub<nparam+1) {
          fprintf(stderr, "svm_multi: error in initialization file: not enough parameters (rho) (file, %s)\n", file);
	  fclose(fs);
	  throw FILE_READ_ERROR;
	}
        rho=new real[nparam];
	for (int i=0; i<nparam; i++) rho[i]=atof(substr[i+1]);
      } else if (strcmp(substr[0], "probA")==0) {
        if (nsub<nparam+1) {
          fprintf(stderr, "svm_multi: error in initialization file: not enough parameters (probA) (file, %s)\n", file);
	  fclose(fs);
	  throw FILE_READ_ERROR;
	}
        probA=new real[nparam];
	for (int i=0; i<nparam; i++) probA[i]=atof(substr[i+1]);
	pfound++;
      } else if (strcmp(substr[0], "probB")==0) {
        if (nsub<nparam+1) {
          fprintf(stderr, "svm_multi: error in initialization file: not enough parameters (probB) (file, %s)\n", file);
	  fclose(fs);
	  throw FILE_READ_ERROR;
	}
        probB=new real[nparam];
	for (int i=0; i<nparam; i++) probB[i]=atof(substr[i+1]);
	pfound++;
      } else if (strcmp(substr[0], "label")==0) {
        if (nsub<this->ncls+1) {
          fprintf(stderr, "svm_multi: error in initialization file: not enough parameters (label) (file, %s)\n", file);
	  fclose(fs);
	  throw FILE_READ_ERROR;
	}
        label=new cls_t[this->ncls];
	for (int i=0; i<nparam; i++) label[i]=atoi(substr[i+1]);
      }
    } while (strcmp(substr[0], "SV")!=0);
    delete [] line;
    delete [] substr;

    dim_ta **ind=new dim_ta *[nsv_total];	//dimension indices
    real **raw=new real *[nsv_total];		//raw features data
    int nf[nsv_total];
    coef=new real*[nsv_total];
    coef[0]=new real[nsv_total*(this->ncls-1)];
    this->D=0;
    for (int i=0; i<nsv_total; i++) {
      int nread;		//number of item scanned
      int pos;			//position in line
      int rel;			//relative position in line

      line=fget_line(fs);
      coef[i]=coef[0]+i*(this->ncls-1);
      for (int j=0; j<this->ncls-1; j++) {
        nread=sscanf(line+pos, format, coef[i]+j, &rel);
	if (nread!=1) {
          fprintf(stderr, "svm_multi: error reading coefficients from %s line %d\n", file, lineno+i);
	  throw FILE_READ_ERROR;
	}
	pos+=rel;
      }
      nf[i]=scan_svm_features(line+pos, ind[i], raw[i]);
      if (nf[i]<=0) {
        fprintf(stderr, "svm_multi: error reading support vectors from %s line %d\n", file, lineno+i);
	throw FILE_READ_ERROR;
      }
      for (int j=0; j<nf[i]; j++) if (ind[i][j]>this->D) this->D=ind[i][j];
    }
    //transfer to more usual array and fill in missing values:
    real missing=0;
    sv=new real*[nsv_total];
    sv[0]=new real[nsv_total*this->D];
    for (int i=0; i<nsv_total; i++) {
      sv[i]=sv[0]+i*this->D;
      for (int j=0; j<this->D; j++) sv[i][j]=missing;
      for (int j=0; j<nf[i]; j++) sv[i][ind[i][j]-1]=raw[i][j];
      delete [] ind[i];
      delete [] raw[i];
    }
    delete [] ind;
    delete [] raw;
    delete [] line;

    fclose(fs);
  }

  template <class real, class cls_t>
  svm_multi<real, cls_t>::~svm_multi() {
    delete [] sv;
    delete [] coef;
    delete [] rho;
    delete [] nsv;
    if (probA!=NULL) delete [] probA;
    if (probB!=NULL) delete [] probB;
    delete [] label;
  }

  //raw decision values, on for each pair of classes:
  template <class real, class cls_t>
  real ** svm_multi<real, cls_t>::classify_raw(real *x) {
    real **result;
    real kv[nsv_total];
    nel_ta start[this->ncls];
    int si, sj;
    int p=0;

    for (int i=0; i<nsv_total; i++) kv[i]=(*kernel)(x, sv[i], this->D, param);

    start[0]=0;
    for (int i=1; i<this->ncls; i++) start[i]=start[i-1]+nsv[i-1];

    result=new real *[this->ncls-1];
    //wastes space, but it's simpler this way:
    result[0]=new real[this->ncls*this->ncls];

    for (int i=0; i<this->ncls; i++) {
      result[i]=result[0]+i*this->ncls;
      for (int j=i+1; j<this->ncls; j++) {
        result[i][j]=0;
	for (int k=0; k<nsv[i]; k++) result[i][j]+=coef[j-1][si+k]*kv[si+k];
	for (int k=0; k<nsv[j]; k++) result[i][j]+=coef[i][sj+k]*kv[sj+k];
	result[i][j] -= rho[p];
	p++;
      }
    }

    return result;
  }

  template <class real, class cls_t>
  cls_t svm_multi<real, cls_t>::classify(real *x, real *p, real *praw) {
    real **praw0;
    int k=0;

    praw0=classify_raw(x);
    if (probA!=NULL && probB!=NULL) {
      for (int i=0; i<this->ncls; i++) {
        for (int j=i+1; j<this->ncls; j++) {
          praw0[i][j]=1/(1+exp(probA[k]*praw0[i][j]+probB[k]));
	  k++;
	}
      }
      solve_cond_prob_1v1(praw0, this->ncls, p);
    } else {
      for (int i=0; i<this->ncls; i++) p[i]=0;
      for (int i=0; i<this->ncls; i++) {
        for (int j=i+1; j<this->ncls; j++) {
          if (praw0[i][j]>0) p[i]++; else p[j]++;
	}
      }
    }

    return choose_class(p, this->ncls);
  }

  template <class real, class cls_t>
  cls_t svm_multi<real, cls_t>::class_list(cls_t *cls) {
    for (cls_t i=0; i<this->ncls; i++) cls[i]=label[i];
    return this->ncls;
  }

  //we'll fill these in later once we figure out what they're supposed to do...
  template <class real, class cls_t>
  void svm_multi<real, cls_t>::print(FILE *fs, char *fbase, int depth) {
  }

  template <class real, class cls_t>
  int svm_multi<real, cls_t>::commands(multi_train_param &param, cls_t **clist, char *fbase) {
    //training to convert LIBSVM model to AGF model?
  }

  template class svm_multi<real_a, cls_ta>;


} //end namespace libagf

