#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "full_util.h"
//#include "peteys_tmpl_lib.h"

#include "agf_lib.h"

using namespace std;
using namespace libpetey;

namespace libagf {

  template <class real, class cls_t>
  classifier_obj<real, cls_t>::classifier_obj() {
    ncls=0;
    D=0;
    D1=0;
    mat=NULL;
    b=NULL;
    name=NULL;
  }

  template <class real, class cls_t>
  classifier_obj<real, cls_t>::~classifier_obj() {
    if (name!=NULL) delete [] name;
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::n_class() {
    return ncls;
  }

  template <class real, class cls_t>
  dim_ta classifier_obj<real, cls_t>::n_feat() {
    return D1;
  }

  template <class real, class cls_t>
  dim_ta classifier_obj<real, cls_t>::n_feat_t() {
    if (mat==NULL) D=n_feat();
    return D;
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::classify(real *x, real &p, real *praw) {
    cls_t result;
    real p1[ncls];
    result=classify(x, p1, praw);
    p=p1[result];
    return result;
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::classify(real *x, real *p, real *praw) {
    cls_t result;
    real p1, p2;
    result=classify(x, p1, praw);
    p2=(1-p1)/(ncls-1);
    for (cls_t i=0; i<result; i++) p[i]=p2;
    p[result]=p1;
    for (cls_t i=result+1; i<ncls; i++) p[i]=p2;
    return result;
  }

  //default version, take the multi-p version and find the right p:
  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::batch_classify(real **x, cls_t *c, real *p, nel_ta n, dim_ta nvar) {
    real **p2;
    if (n==0) return;
    //printf("classifier_obj->batch_classify (default): performing classifications with %d test vectors for %d classes\n", n, ncls);

    p2=new real *[n];
    p2[0]=new real[n*ncls];
    for (nel_ta i=1; i<n; i++) p2[i]=p2[0]+i*ncls;
    batch_classify(x, c, p2, n, nvar);
    for (nel_ta i=0; i<n; i++) {
      if (c[i]>=0 && c[i]<=ncls-1) {
        p[i]=p2[i][c[i]];
      } else {
        fprintf(stderr, "classifier_obj->batch_classify: error, class value (%d) out of range [0, %d)\n", c[i], ncls);
        p[i]=0;
      }
    }
    delete [] p2[0];
    delete [] p2;
  }

  //default version, just apply classify over and over:
  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::batch_classify(real **x, cls_t *c, real **p, nel_ta n, dim_ta nvar) {
    for (nel_ta i=0; i<n; i++) c[i]=classify(x[i], p[i]);
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::ltran(real **mat1, real *b1, dim_ta d1, dim_ta d2, int flag) {
    int err2=0;

    //find out the "raw" dimensions: 
    if (mat==NULL) n_feat();
    //we don't necessarily know offhand what the dimensionality of the problem is: 
    //printf("ltran: d1=%d; d2=%d\n", d1, d2);
    if (flag) err2=ltran_model(mat1, b1, d1, d2);
    mat=mat1;
    b=b1;
    if (d2!=D1 && D1!=-1) {
      fprintf(stderr, "classifier_obj: second dimension of trans. mat. does not that of borders data: %d vs. %d\n", d2, D1);
      return DIMENSION_MISMATCH;
    }
    //this is very clear:
    D1=d2;
    D=d1;

    return err2;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {
     fprintf(stderr, "classifier_obj: you tried to allocate the abstract base class\n");
     throw PARAMETER_OUT_OF_RANGE;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::copy_ltran(classifier_obj<real, cls_t> *other) {
    if (other->mat==NULL) {
      return 0;
    } else {
      assert(D1==other->D1);
      mat=other->mat;
      b=other->b;
      D=other->D;
      return 1;
    }
  }

  template <class real, class cls_t>
  real * classifier_obj<real, cls_t>::do_xtran(real *x) {
    real *xtran;
    if (mat!=NULL) {
      real tmp;
      xtran=new real[D1];
      //linearly transform the test point:
      for (dim_ta j=0; j<D1; j++) xtran[j]=0;
      for (dim_ta i=0; i<D; i++) {
        tmp=x[i]-b[i];
        for (dim_ta j=0; j<D1; j++) xtran[j]=xtran[j]+tmp*mat[i][j];
      }
      //xtran=left_vec_mult(x, mat, this->D, D1);
    } else {
      xtran=x;
    }
    return xtran;
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::classify_t(real *x, real &p, real *praw) {
    cls_t c;
    real *xtran;
    xtran=do_xtran(x);
    c=classify(xtran, p, praw);
    if (mat!=NULL) delete [] xtran;
    return c;
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::classify_t(real *x, real *p, real *praw) {
    cls_t c;
    real *xtran;
    xtran=do_xtran(x);
    c=classify(xtran, p, praw);
    if (mat!=NULL) delete [] xtran;
    return c;
  }

  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::batch_classify_t(real **x, cls_t *c, real *p, nel_ta n, dim_ta nvar) {
    real *xtran[n];
    for (int i=0; i<n; i++) xtran[i]=do_xtran(x[i]);
    batch_classify(xtran, c, p, n, nvar);
    if (mat!=NULL) for (nel_ta i=0; i<n; i++) delete [] xtran[i];
  }

  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::batch_classify_t(real **x, cls_t *c, real **p, nel_ta n, dim_ta nvar) {
    real *xtran[n];
    for (int i=0; i<n; i++) xtran[i]=do_xtran(x[i]);
    batch_classify(xtran, c, p, n, nvar);
    if (mat!=NULL) for (nel_ta i=0; i<n; i++) delete [] xtran[i];
  }

  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::print(FILE *fs, char *fbase, int depth) {
    fprintf(stderr, "classifier_obj: you tried to use the print method in a derived class that doesn't support it\n");
    throw INTERNAL_ERROR;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::commands(multi_train_param &p, cls_t **list, char *f) {
    fprintf(stderr, "classifier_obj: you tried to use the commands method in a derived class that doesn't support it\n");
    throw INTERNAL_ERROR;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::max_depth(int cur) {return 1;}

  //does butt-kiss--want to pass that on to the "oneclass" object:
  template <class real, class cls_t>
  void classifier_obj<real, cls_t>::set_id(cls_t *id) {}


  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::class_list(cls_t *cls) {
    for (cls_t i=0; i<ncls; i++) cls[i]=i;
    return ncls;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::load(FILE *file) {
    fprintf(stderr, "classifier_obj:load method not defined for this subclass\n");
    throw INTERNAL_ERROR;
  }

  template <class real, class cls_t>
  int classifier_obj<real, cls_t>::save(FILE *file) {
    fprintf(stderr, "classifier_obj:save method not defined for this subclass\n");
    throw INTERNAL_ERROR;
  }

  template <typename real, typename cls_t>
  int classifier_obj<real, cls_t>::get_code(int **code, char **model) {
    return 0;
  }

  //an ugly hack (followed by a whole bunch of nothing...):
  template <typename real, typename cls_t>
  void classifier_obj<real, cls_t>::train(real **train, cls_t *cls, nel_ta ntrain, int type, real *param) {
    fprintf(stderr, "classifier_obj: calibrate method not defined for this sub-class!\n");
  }

  template <class real, class cls_t>
  cls_t classifier_obj<real, cls_t>::collect_binary_classifiers(binaryclassifier<real, cls_t> **list) {
    return 0;
  }

  //a whole bunch of nothing:
  template <class real, class cls_t>
  oneclass<real, cls_t>::oneclass(cls_t cl) {
    cls=cl;
    this->ncls=1;
  }

  //more nothing:
  template <class real, class cls_t>
  oneclass<real, cls_t>::~oneclass() {}

  template <class real, class cls_t>
  cls_t oneclass<real, cls_t>::classify(real *x, real &p, real *praw) {
    p=1;
    return cls;
  }

  template <class real, class cls_t>
  cls_t oneclass<real, cls_t>::classify(real *x, real *p, real *praw) {
    p[0]=1;
    return cls;
  }

  template <class real, class cls_t>
  void oneclass<real, cls_t>::batch_classify(real **x, cls_t *c, real *p, nel_ta n, dim_ta nvar) {
    for (nel_ta i=0; i<n; i++) {
      c[i]=cls;
      p[i]=1;
    }
  }

  template <class real, class cls_t>
  void oneclass<real, cls_t>::batch_classify(real **x, cls_t *c, real **p, nel_ta n, dim_ta nvar) {
    for (nel_ta i=0; i<n; i++) {
      c[i]=cls;
      p[i][0]=1;
    }
  }

  template <class real, class cls_t>
  int oneclass<real, cls_t>::ltran_model(real **mat1, real *b1, dim_ta d1, dim_ta d2) {return 0;}

  template <class real, class cls_t>
  cls_t oneclass<real, cls_t>::class_list(cls_t *cls1) {
    cls1[0]=cls;
    return this->ncls;
  }

  template <class real, class cls_t>
  int oneclass<real, cls_t>::max_depth(int cur) {return 0;}

  template <class real, class cls_t>
  void oneclass<real, cls_t>::print(FILE *fs, char *fbase, int depth) {
    for (int i=0; i<depth; i++) fprintf(fs, "  ");
    fprintf(fs, "%d\n", cls);
  }

  //doesn't do much, but makes code more elegant:
  template <class real, class cls_t>
  int oneclass<real, cls_t>::commands(multi_train_param &param, cls_t **clist, char *fbase) {
    return class_list(*clist);
  }

  //doesn't do much, but makes code more elegant:
  template <class real, class cls_t>
  int oneclass<real, cls_t>::load(FILE *fs) {
    return (fscanf(fs, "%d", &cls)==1);
  }

  //doesn't do much, but makes code more elegant:
  template <class real, class cls_t>
  int oneclass<real, cls_t>::save(FILE *fs) {
    return (fprintf(fs, "%4d", cls)==1);
  }

  template class classifier_obj<float, cls_ta>;
  template class classifier_obj<double, cls_ta>;
  template class oneclass<real_a, cls_ta>;

}

