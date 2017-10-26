#include <math.h>
#include <string.h>
#include <assert.h>

#include "read_ascii_all.h"
#include "error_codes.h"
#include "full_util.h"
#include "intersection.h"

#include "agf_lib.h"

using namespace libpetey;

namespace libagf {
  //fuck it, this shit is Rube-Goldberg enough, I can't see how a couple globabl 
  //variables can make things much worse:
  void * global_svm_helper;
  FILE * global_svm_allinone;

  template <typename real, typename cls_t>
  svm_helper<real> * unite_support_vectors(binaryclassifier<real, cls_t> **list, cls_ta n) {
    svm_helper<real> *helper;
    svm2class<real, cls_t> *classifier;
    svm2class2<real, cls_t> *classnew[n];
    nel_ta nsv[n];
    real **sv[n];				//all the support vectors
    real ** unn;			//union of support vectors
    int nsv_total;			//size of union
    int *ind[n];				//indices into union
    int nparam;				//how many kernel paramters are there?
    dim_ta D;
    nel_ta k;		//counter

    helper=new svm_helper<real>();
    D=list[0]->n_feat_t();
    classifier=(svm2class<real, cls_t> *) list[0];
    helper->kernel=classifier->classifier->kernel;
    helper->kernel_deriv=classifier->classifier->kernel_deriv;

    if (helper->kernel==&linear_basis<real>) {
      nparam=0;
    } else if (helper->kernel==&polynomial_basis<real>) {
      nparam=3;
    } else if (helper->kernel==&radial_basis<real>) {
      nparam=1;
    } else if (helper->kernel==&sigmoid_basis<real>) {
      nparam=2;
    } else {
      fprintf(stderr, "unite_support_vectors: warning, kernel not recognized; assuming 0 parameters\n");
      nparam=0;
    }

    helper->param=NULL;
    if (nparam>0) helper->param=new real[nparam];
    for (int i=0; i<nparam; i++) helper->param[i]=classifier->classifier->param[i];

    k=0;
    for (int i=0; i<n; i++) {
      //cast to binary support vector machine (old type):
      classifier=(svm2class<real, cls_t> *) list[i];
      if (helper->kernel==classifier->classifier->kernel) {
        for (int m=0; m<nparam; m++) {
          if (helper->param[m]!=classifier->classifier->param[m]) {
            fprintf(stderr, "unite_support_vectors: all binary SVMs must have the same kernel parameters\n");
            throw DIMENSION_MISMATCH;
	  }
	}
      } else {
        fprintf(stderr, "unite_support_vectors: all binary SVMs must have the same kernel function\n");
        throw DIMENSION_MISMATCH;
      }
      nsv[i]=classifier->classifier->nsv_total;		//number of support vectors
      //pointer to all the support vectors in this machine:
      sv[i]=classifier->classifier->sv;
      //allocate new binary support vector machine (new type) and start passing coefficients:
      classnew[i]=new svm2class2<real, cls_t>(D);
      //indexes into unified list of support vectors:
      classnew[i]->ind=new int[nsv[i]];
      //union function will fill these:
      ind[i]=classnew[i]->ind;
      //copy all coefficients:
      classnew[i]->probA=classifier->classifier->probA[0];
      classnew[i]->probB=classifier->classifier->probB[0];
      classnew[i]->coef=new real[nsv[i]+1];
      for (int j=0; j<nsv[i]; j++) classnew[i]->coef[j]=classifier->classifier->coef[j][0];
      classnew[i]->coef[nsv[i]]=classifier->classifier->rho[0];
      classnew[i]->helper=helper;
      //copy the name:
      classnew[i]->name=new char[strlen(classifier->classifier->name)+1];
      strcpy(classnew[i]->name, classifier->classifier->name);
    }

    //function to find union of vectors:
    nsv_total=unify_vectors(sv, nsv, n, D, unn, ind);

    //copy union since results from above are copied by pointer only:
    helper->sv=copy_matrix<real>(unn, nsv_total, D);

    //delete old binary SVMs and replace them with new ones:
    for (int i=0; i<n; i++) {
      delete list[i];
      list[i]=classnew[i];
    }

    //allocate space for kernel values, etc:
    helper->kval=new real[nsv_total];
    helper->flag=new int[nsv_total];
    helper->test=new real[D];

    return helper;
  }

  template svm_helper<float> *unite_support_vectors<float, cls_ta>(binaryclassifier<float, cls_ta> **, cls_ta);
  template svm_helper<double> *unite_support_vectors<double, cls_ta>(binaryclassifier<double, cls_ta> **, cls_ta);

  template <typename real>
  svm_helper<real>::svm_helper() {
    nsv=0;
    sv=NULL;
    flag=NULL;
    kval=NULL;
    test=NULL;
    D=-1;
  }

  template <typename real>
  svm_helper<real>::svm_helper(FILE *fs) {
    int nparam;
    char *line;
    char **sub;			//sub-strings
    int nsub;

    //read in basis function type:
    line=fget_line(fs, 1);
    if (strcmp(line, "linear")==0) {
      kernel=&linear_basis<real>;
      kernel_deriv=&linear_basis_deriv<real>;
      nparam=0;
    } else if (strcmp(line, "polynomial")==0) {
      kernel=&polynomial_basis<real>;
      kernel_deriv=&polynomial_basis_deriv<real>;
      nparam=3;
    } else if (strcmp(line, "radial")==0) {
      kernel=&radial_basis<real>;
      kernel_deriv=&radial_basis_deriv<real>;
      nparam=1;
    } else if (strcmp(line, "sigmoid")==0) {
      kernel=&sigmoid_basis<real>;
      kernel_deriv=&sigmoid_basis_deriv<real>;
      nparam=2;
    } else {
      fprintf(stderr, "svm_helper: kernel type, %s, not recognized\n");
      throw FILE_READ_ERROR;
    }
    delete [] line;

    //read in basis function parameters:
    line=fget_line(fs, 1);
    sub=split_string_destructive(line, nsub);
    if (nsub < nparam) {
      fprintf(stderr, "svm_helper: not enough parameters found for kernel function\n");
      fprintf(stderr, "            %d found, %d expected\n", nsub, nparam);
      throw FILE_READ_ERROR;
    }
    param=new real[nparam];
    for (int i=0; i<nparam; i++) {
      if (sscanf(sub[i], "%g", param+i)!=1) {
        fprintf(stderr, "svm_helper: error scanning kernel parameters\n");
        throw FILE_READ_ERROR;
      }
    }
    delete [] sub;
    delete [] line;

    //read in support vectors:
    sv=scan_matrix<real>(fs, nsv, D);

    //allocate space for kernel values:
    kval=new real[nsv];
    flag=new int[nsv];
    test=new real[D];
  }

  template <typename real>
  svm_helper<real>::~svm_helper() {
    if (sv!=NULL) {
      delete_matrix(sv);
      delete [] kval;
      delete [] flag;
      delete [] test;
      delete [] param;
    }
  }

  template <typename real>
  int svm_helper<real>::save(FILE *fs) {
    int nparam;
    if (kernel==&linear_basis<real>) {
      nparam=0;
      fprintf(fs, "linear\n");
    } else if (kernel==&polynomial_basis<real>) {
      nparam=3;
      fprintf(fs, "polynomial\n");
    } else if (kernel==&radial_basis<real>) {
      nparam=1;
      fprintf(fs, "radial\n");
    } else if (kernel==&sigmoid_basis<real>) {
      nparam=2;
      fprintf(fs, "sigmoid\n");
    } else {
      fprintf(stderr, "svm_helper::save: kernel not recognized; cannot save\n");
      throw PARAMETER_OUT_OF_RANGE;
    }
    for (int i=0; i<nparam; i++) fprintf(fs, " %g", param[i]);
    fprintf(fs, "\n");

    print_matrix(fs, sv, nsv, D);
    
  }

  template <typename real>
  void svm_helper<real>::register_point(real *x) {
    if (compare_vectors(x, test, D)!=0) {
      for (int i=0; i<D; i++) test[i]=x[i];
      for (int i=0; i<nsv; i++) flag[i]=0;
    }
  }

  template <typename real>
  real svm_helper<real>::get_kernel(nel_ta index) {
    if (flag[index]==0) {
      flag[index]=1;
      kval[index]=(* kernel) (test, sv[index], D, param);
    }
    return kval[index];
  }

  template <typename real>
  real svm_helper<real>::get_kernel_deriv(nel_ta index, real *deriv) {
    return (* kernel_deriv) (test, sv[index], D, param, deriv);
  }

  template <class real>
  int svm_helper<real>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {
    real **sv2;
    if (d1!=D) {
      fprintf(stderr, "svm_helper: first dimension (%d) of trans. mat. does not agree with that of support vectors (%d)\n", d1, D);
      return DIMENSION_MISMATCH;
    }
    //apply constant factor:
    for (nel_ta i=0; i<nsv; i++) {
      for (dim_ta j=0; j<D; j++) {
        sv[i][j]=sv[i][j]-b1[j];
      }
    }

    sv2=matrix_mult(sv, mat1, nsv, d1, d2);

    D=d2;

    delete [] sv[0];
    delete [] sv;
    sv=sv2;

    return 0;
  }

  template class svm_helper<float>;
  template class svm_helper<double>;

  template <typename real, typename cls_t>
  svm2class2<real, cls_t>::svm2class2(dim_ta ndim) {
    this->D=ndim;
    this->D1=ndim;
    helper=NULL;
    ind=NULL;
    nsv=0;
    coef=NULL;
  }

  template <typename real, typename cls_t>
  svm2class2<real, cls_t>::svm2class2(char *name) {
    FILE *fs;
    char *line;
    this->name=new char[strlen(name)+1];
    strcpy(this->name, name);
    do {
      line=fget_line(global_svm_allinone);
      if (line==NULL) {
        fprintf(stderr, "svm2class2: error in input file\n");
	throw FILE_READ_ERROR;
      }
    } while (strcmp(line, name)!=0);

    //the sin:
    fs=global_svm_allinone;
    helper=(svm_helper<real> *) global_svm_helper;

    fscanf(fs, "%d", &nsv);
    fscanf(fs, "%g %g", &probA, &probB);
    ind=new int[nsv];
    coef=new real[nsv];
    for (nel_ta i=0; i<nsv; i++) {
      fscanf(fs, "%g", coef+i);
    }
    for (nel_ta i=0; i<nsv; i++) {
      fscanf(fs, "%d", ind+i);
    }
  }

  template <typename real, typename cls_t>
  svm2class2<real, cls_t>::~svm2class2() {
    if (coef!=NULL) {
      delete [] coef;
      delete [] ind;
    }
  }

  template <typename real, typename cls_t>
  int svm2class2<real, cls_t>::save(FILE *fs) {
    fprintf(fs, "%s\n", this->name);
    fprintf(fs, "%d\n", nsv);
    fprintf(fs, "%g %g\n", probA, probB);
    for (nel_ta i=0; i<nsv; i++) {
      fprintf(fs, " %g", coef+i);
    }
    fprintf(fs, "\n");
    for (nel_ta i=0; i<nsv; i++) {
      fscanf(fs, " %d", ind+i);
    }
    fprintf(fs, "\n");
    return 0;
  }

  template <typename real, typename cls_t>
  real svm2class2<real, cls_t>::R(real *x, real *praw) {
    real result;
    real kv;
    cls_t swp;
    int sgn=1;		//sign is not reversed relative to LIBSVM implementation
    int p=0;

    result=coef[nsv];
    helper->register_point(x);
    for (int k=0; k<nsv; k++) {
      kv=helper->get_kernel(ind[k]);
      result+=coef[k]*kv;
    }
    if (praw!=NULL) praw[0]=result;
    result=1./(1+exp(probA*result+probB));

    return sgn*(2*result-1);
  } 

  template <typename real, typename cls_t>
  real svm2class2<real, cls_t>::R_deriv(real *x, real *drdx) {
    real result;
    real kv;
    cls_t swp;
    real t1, t2;
    int sgn=1;		//sign is not reversed relative to LIBSVM implementation

    real deriv[this->D1];
    real drdx1[this->D1];

    result=coef[nsv];
    helper->register_point(x);
    for (int k=0; k<nsv; k++) {
      kv=helper->get_kernel_deriv(ind[k], deriv);
      result+=coef[k]*kv;
      for (dim_ta m=0; m<this->D1; m++) drdx1[m]+=coef[k]*deriv[m];
    }

    t1=exp(result*probA+probB);
    t2=probA*t1/(1+t1)/(1+t1);			//derivative of sigmoid function
    for (dim_ta k=0; k<this->D1; k++) drdx1[k]*=-2*t2;
    result=2/(1+t1)-1;

    //printf("drdx=");
    for (dim_ta k=0; k<this->D1; k++) {
      drdx[k]=sgn*drdx1[k];
      //printf("%g ", drdx[k]);
    }
    //printf("\nr=%g\n", result);

    return sgn*result;

  }

  template <typename real, typename cls_t>
  int svm2class2<real, cls_t>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {
    if (this->mat == NULL) {
      //the classifier now has a different number of features:
      this->D1=d2;
      this->D=d2;
    } else {
      //assert(mat1==this->mat && b1==this->b);
      //from the outside, the classifier looks like it has the same number of
      //features as before normalization:
      assert(this->D1==d2);
    }

    return helper->ltran_model(mat1, b1, d1, d2);

  }

  template class svm2class2<float, cls_ta>;
  template class svm2class2<double, cls_ta>;

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class() {
    classifier=NULL;
    dflag=0;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class(char *modfile) {
    cls_t *clist;
    this->name=new char[strlen(modfile)+1];
    strcpy(this->name, modfile);
    classifier=new svm_multi<real, cls_t>(modfile);
    this->ncls=classifier->n_class();
    if (this->ncls != 2) {
      fprintf(stderr, "svm2class: only binary classifiers accepted (file, %s)\n", modfile);
      throw PARAMETER_OUT_OF_RANGE;
    }
    dflag=1;
    clist=new cls_t[this->ncls];
    classifier->class_list(clist);
    if (clist[0]==0 && clist[1]==1) {
      ind1=1;
      ind2=0;
    } else {
      ind1=0;
      ind2=1;
    }
    label1=clist[ind2];
    label2=clist[ind1];
    //a lot of book-keeping, dammit:
    this->D=classifier->n_feat_t();
    this->D1=classifier->n_feat();
    delete [] clist;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::svm2class(svm_multi<real, cls_t> *other, cls_t i, cls_t j, int cflag) {
    cls_t *clist;
    cls_t oncls=other->n_class();
    if (cflag) {
      classifier=new svm_multi<real, cls_t>(other);
      dflag=1;
    } else {
      classifier=other;
      dflag=0;
    }
    //a lot of book-keeping, dammit:
    this->D=other->n_feat_t();
    this->D1=other->n_feat();
    this->ncls=2;
    if (i==j || i>=oncls || j>=oncls) throw PARAMETER_OUT_OF_RANGE;
    ind1=i;
    ind2=j;
    clist=new cls_t[classifier->n_class()];
    classifier->class_list(clist);
    label2=clist[i];
    label1=clist[j];
    delete [] clist;
  }

  template <class real, class cls_t>
  svm2class<real, cls_t>::~svm2class() {
    if (dflag) delete classifier;
  }

  //some book-keeping:
  template <class real, class cls_t>
  cls_t svm2class<real, cls_t>::class_list(cls_t *list) {
    list[0]=label1;
    list[1]=label2;
    return this->ncls;
  }

  template <class real, class cls_t>
  real svm2class<real, cls_t>::R(real *x, real *praw) {
    return classifier->R(x, ind1, ind2, praw);
  }

  template <class real, class cls_t>
  real svm2class<real, cls_t>::R_deriv(real *x, real *drdx) {
    real r=classifier->R_deriv(x, ind1, ind2, drdx);
    return r;
    real drdx2[this->D1];
    real r2=R(x);
    this->R_deriv_num(x, 0.005, drdx2);
    printf("%g %g\n", r, r2);
    for (int i=0; i<this->D1; i++) {
      printf("%g %g\n", drdx[i], drdx2[i]);
    }
    printf("\n");
  }

  template <class real, class cls_t>
  int svm2class<real, cls_t>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {
    return classifier->ltran_model(mat1, b1, d1, d2);
  }

  template <class real, class cls_t>
  real svmrfunc(real *x, void *param, real *deriv) {
    bordparam<real> *p1=(bordparam<real> *) param;
    svm2class<real, cls_t> *p2=(svm2class<real, cls_t> *) p1->rparam;
    return p2->R_deriv(x, deriv);
  }

  template class svm2class<float, cls_ta>;
  template class svm2class<double, cls_ta>;

  template float svmrfunc<float, cls_ta>(float *, void *, float *);
  template double svmrfunc<double, cls_ta>(double *, void *, double *);

}
