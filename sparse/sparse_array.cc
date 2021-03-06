#include <assert.h>

#include "error_codes.h"

#include "full_matrix.h"
#include "full_util.h"
#include "sparse_array.h"

#include "../libpetey/linked.cc"

using namespace std;

namespace libpetey {
namespace libsparse {

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array() {
  nsparse=0;
  m=0;
  sparse_a=NULL;
}

template <class index_t, class scalar, class sparse_t>
sparse_array<index_t, scalar, sparse_t>::sparse_array(matrix_base<index_t, scalar> *other) {
  if (typeid(*other) == typeid(sparse_t)) {
    convert(* (sparse_t *) other);
  } else if (typeid(*other) == typeid(sparse_array<index_t, scalar, sparse_t>)) {
    copy(*(sparse_array<index_t, scalar, sparse_t> *) other);
  } else if (typeid(*other) == typeid(full_matrix<index_t, scalar>)) {
    convert(*(full_matrix<index_t, scalar> *) other);
  } else {
    fprintf(stderr, "Failed to initialize sparse array from base class\n");
  }

};

/*	
template <class index_t, class scalar, class sparse_t>
sparse_array<index_t, scalar> & sparse_array<index_t, scalar>::operator = (matrix_base<index_t, scalar, sparse_t> &other) {
  if (typeid(other) == typeid(sparse<index_t, scalar>)) {
    return *this=*((sparse<index_t, scalar> *) &other);
  } else if (typeid(other) == typeid(sparse_array<index_t, scalar>)) {
    return *this=*(sparse_array<index_t, scalar> *) &other;
  } else if (typeid(other) == typeid(full_matrix<index_t, scalar>)) {
    return *this=*(full_matrix<index_t, scalar> *) &other;
  }

};
*/
	
template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(sparse_t *s, long ns) {
  index_t m1, n1, n;
  sparse_a=new petey_pointer<sparse_t>[ns];
  s[0].dimensions(m, n);
  assert(m==n);
  for (long i=0; i<ns; i++) {
    s[i].dimensions(m1, n1);
    assert(m1==m);
    assert(n1==n);
    sparse_a[i]=s[i];
  }
  nsparse=ns;
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(sparse_t **s, long ns) {
  index_t m1, n1, n;
  sparse_a=new petey_pointer<sparse_t>[ns];
  s[0]->dimensions(m, n);
  assert(m==n);
  for (long i=0; i<ns; i++) {
    s[i]->dimensions(m1, n1);
    assert(m1==m);
    assert(n1==n);
    sparse_a[i]=s[i];
  }
  nsparse=ns;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::copy(sparse_array<index_t, real, sparse_t> & s, long l1, long l2) {
  m=s.dimension();
  if (l1<0) l1=0;
  if (l2<0) l2=s.nsparse-1;
  nsparse=l2-l1+1;
  sparse_a=new petey_pointer<sparse_t>[nsparse];
  for (long i=l1; i<=l2; i++) {
    sparse_a[i-l1]=s.sparse_a[i];
  }
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(sparse_array<index_t, real, sparse_t> & other) {
  copy(other);
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(sparse_array<index_t, real, sparse_t> & other, long l1, long l2) {
  copy(other, l1, l2);
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(full_matrix<index_t, real> & other) {
  index_t m1, n1;

  other.dimensions(m1, n1);
  assert(m1==n1);
  m=m1;
  nsparse=1;
  sparse_a=new petey_pointer<sparse_t>[nsparse];
  sparse_a[0]=new sparse_t();
  *sparse_a[0]=other;
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(char *filename) {
  FILE *fs;
  sparse_a=NULL;
  nsparse=0;
  fs=fopen(filename, "r");
  read(fs);
  fclose(fs);
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::~sparse_array() {
  clean();
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::clean() {
  if (sparse_a != NULL) {
    delete [] sparse_a;
  }
  nsparse=0;
  sparse_a=NULL;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::dimensions(index_t &i, index_t &j) const {
	i=m;
	j=m;
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(const sparse_array<index_t, real, sparse_t> &s) {
  nsparse=s.nsparse;
  m=s.m;
  sparse_a=new petey_pointer<sparse_t>[nsparse];
  for (long i=0; i<nsparse; i++) {
    sparse_a[i]=s.sparse_a[i];
  }
}

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> & sparse_array<index_t, real, sparse_t>::operator = (sparse_array<index_t, real, sparse_t> &s) {
  clean();
  copy(s);
  return *this;
}


template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::convert(sparse_t &s) {
  index_t m1, n1;
  s.dimensions(m1, n1);
  assert(m1==n1);
  clean();
  m=m1;
  nsparse=1;
  sparse_a=new petey_pointer<sparse_t>[nsparse];
  sparse_a[0]=new sparse_t();
  *sparse_a[0]=s;
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::sparse_array(sparse_t &s) {
  convert(s);
}

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> & sparse_array<index_t, real, sparse_t>::operator = (sparse_t &s) {
  convert(s);
  return *this;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::convert(full_matrix<index_t, real> &s) {
  index_t m1, n1;
  s.dimensions(m1, n1);
  assert(m1==n1);
  clean();
  m=m1;
  nsparse=1;
  sparse_a=new petey_pointer<sparse_t>[nsparse];
  sparse_a[0]=new sparse_t();
  *sparse_a[0]=s;
}

template <class index_t, class real, class sparse_t>
sparse_array<index_t, real, sparse_t>::operator real **() {
  real ** result;
  real ** cand;
  result=allocate_matrix<real, index_t>(m, m);
  cand=identity_matrix<real, index_t>(m, m);
  mat_mult_tt(cand, result);
  delete_matrix(cand);
  matrix_transpose<real, index_t>(result, m);

  return result;
} 

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> & sparse_array<index_t, real, sparse_t>::operator = (full_matrix<index_t, real> &s) {
  convert(s);
  return *this;
}

//**note: does not generate a copy!
template <class index_t, class real, class sparse_t>
sparse<index_t, real> * sparse_array<index_t, real, sparse_t>::get_el(long ind) {
  if (ind>=0 && ind<nsparse) {
    return (sparse<index_t, real> *) sparse_a[ind];
  } else {
    fprintf(stderr, "Index, %ld, out of bounds [%ld, %ld)\n", 
		(int64_t) ind, (int64_t) 0, (int64_t) nsparse);
    return NULL;
  }
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::vect_mult(real *cand, real *result) {
  real *med, *s;
  med=new real[m];

  sparse_a[0]->vect_mult(cand, result);
  for (long i=1; i<nsparse; i++) {
    sparse_a[i]->vect_mult(result, med);
    s=med;
    med=result;
    result=s;
  }
  //this check should probably be applied everywhere I use this trick
  //and result is not allocated inside the function...
  if (nsparse % 2 == 0) {
    s=result;
    result=med;
    med=s;
    for (index_t i=0; i<m; i++) result[i]=med[i];
  }
  delete [] med;
}

template <class index_t, class real, class sparse_t>
full_matrix<index_t, real> * sparse_array<index_t, real, sparse_t>::cmult(real *cand) {
  real *med, *s;
  med=new real[m];
  full_matrix<index_t, real> * result;

  result=new full_matrix<index_t, real>(nsparse, m);

  sparse_a[0]->vect_mult(cand, result->data[0]);
  for (long i=1; i<nsparse; i++) {
    sparse_a[i]->vect_mult(result->data[i-1], result->data[i]);
  }

  return result;
}

template <class index_t, class real, class sparse_t>
full_matrix<index_t, real> * sparse_array<index_t, real, sparse_t>::left_cmult(real *cor) {
  real *med, *s;
  med=new real[m];
  full_matrix<index_t, real> * result;

  result=new full_matrix<index_t, real>(nsparse, m);

  sparse_a[nsparse-1]->left_mult(cor, result->data[0]);
  for (long i=nsparse-2; i>=0; i++) {
    sparse_a[i]->left_mult(result->data[nsparse-i-2], result->data[nsparse-i-1]);
  }

  return result;
}

//this is more efficient because:
//1. no need to allocate any extra memory
//2. operations are done on smaller, contiguous blocks of memory: better paging
template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::mat_mult_tt(real **cand, 
		real **result) {
  printf("%6.1f%%", 0.);
  for (index_t i=0; i<m; i++) {
    printf("\b\b\b\b\b\b\b%6.1f%%", 100.*i/m);
    fflush(stdout);
    vect_mult(cand[i], result[i]);
  }
  printf("\b\b\b\b\b\b\b%6.1f%%\n", 100.);
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::mat_mult(real **cand, 
		real **result) {

  real *column1;
  real *column2;

  column1=new real[m];
  column2=new real[m];
  printf("%6.1f%%", 0.);
  for (index_t i=0; i<m; i++) {
    printf("\b\b\b\b\b\b\b%6.1f%%", 100.*i/m);
    fflush(stdout);
    for (index_t j=0; j<m; j++) column1[j]=cand[j][i];
    vect_mult(column1, column2);
    for (index_t j=0; j<m; j++) result[j][i]=column2[j];
  }
  printf("\b\b\b\b\b\b\b%6.1f%%\n", 100.);

  delete [] column1;
  delete [] column2;
  return;

  real **med;
  real **swap;

  med=allocate_matrix<real, index_t>(m, m);
  sparse_a[0]->mat_mult(cand, result, m);
  for (long i=1; i<nsparse; i++) {
    sparse_a[i]->mat_mult(result, med, m);
    swap=med;
    med=result;
    result=swap;
  }
  //this check should probably be applied everywhere I use this trick
  //and result is not allocated inside the function...
  if (nsparse % 2 == 0) {
    swap=result;
    result=med;
    med=swap;
    libpetey::copy_matrix(med, result, m, m);
  }
  delete_matrix(med);
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::mat_mult(sparse_t *cand, 
		sparse_t *result) {
  sparse_t *med;
  sparse_t *swap;
  med=new sparse_t(m, m);
  *result=*cand;
  result->transpose();

  //matrix multiplication (with transpose) is O(n^2); transposition is O(n log n)
  //should be faster using transposed operator after re-arranging the order
  //of operations:
  for (long i=0; i<nsparse; i++) {
    result->mat_mult_t(*sparse_a[i], *med);
    swap=med;
    med=result;
    result=swap;
  }
  //result is transposed (could fix it with more re-arrangement, but this is
  //more elegant...):
  result->transpose();

  /* this is how it was previously done (will keep it around until above is
  tested...)
  sparse_a[0]->mat_mult(*cand, *result);
  for (long i=1; i<nsparse; i++) {
    sparse_a[i]->mat_mult(*result, *med);
    swap=med;
    med=result;
    result=swap;
  }
  */

  if (nsparse % 2 == 1) {
    swap=result;
    result=med;
    med=swap;
    *result=*med;
  }
  delete med;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::left_m_mult(real **cor, 
		real **result) {

  //certain that this row-by-row method is more efficient, both in terms
  //of space and speed:
  for (index_t i=0; i<m; i++) {
    left_mult(cor[i], result[i]);
  }
  //(it's also a lot smaller...)

  return;

  real **med;
  real **swap;

  med=allocate_matrix<real, index_t>(m, m);
  sparse_a[nsparse-1]->mat_mult(cor, result, m);
  for (long i=nsparse-2; i>=0; i--) {
    sparse_a[i]->left_m_mult(result, med, m);
    swap=med;
    med=result;
    result=swap;
  }
  if (nsparse % 2 == 0) {
    swap=result;
    result=med;
    med=swap;
    libpetey::copy_matrix(med, result, m, m);
  }
  delete_matrix(med);
}
      

//to multiply two sparse arrays, simply concatenate them...
template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::mat_mult(
		sparse_array<index_t, real, sparse_t> &cand, 
		sparse_array<index_t, real, sparse_t> &result) {
  result.clean();

  result.nsparse=nsparse+cand.nsparse;
  result.sparse_a=new petey_pointer<sparse_t>[result.nsparse];
  assert(cand.m == m);
  result.m=m;

  //**note: matrices are in reverse order...
  for (long i=0; i<cand.nsparse; i++) {
    result.sparse_a[i]=cand.sparse_a[i];
  }
  for (long i=0; i<nsparse; i++) {
    result.sparse_a[i+cand.nsparse]=sparse_a[i];
  }

}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::reverse() {
  petey_pointer<sparse_t> swap;
  for (long i=0; i<nsparse/2; i++) {
    swap=sparse_a[i];
    sparse_a[i]=sparse_a[nsparse-i-1];
    sparse_a[nsparse-i-1]=swap;
  }
}

//to transpose the structure, transpose each of its members and reverse them:
template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::transpose() {
  for (long i=0; i<nsparse;i++) sparse_a[i]->transpose();
  reverse();
}

//does this work??
template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::get_row (index_t i, real *row) {
  real *s1;
  s1=new real[m];
  for (index_t k=0; k<m; k++) s1[k]=0;
  s1[i]=1;
  left_mult(s1, row);
  delete [] s1;
}

//does this work??
template <class index_t, class real, class sparse_t>
real *sparse_array<index_t, real, sparse_t>::operator () (index_t i) {
  real *row;
  row=new real[m];
  get_row(i, row);
  return row;
}

//does this work??
template <class index_t, class real, class sparse_t>
real sparse_array<index_t, real, sparse_t>::operator () (index_t i, index_t j) {
  real *row, result;
  row=(*this)(i);
  result=row[j];
  delete [] row;
  return result;
}

template <class index_t, class real, class sparse_t>
long sparse_array<index_t, real, sparse_t>::cel(real val, index_t i, index_t j) {
  return -1;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::asl_transform(real m, real b) {
  for (long i=0; i<nsparse; i++) sparse_a[i]->sl_transform(m, b);
  //reverse();
}


template <class index_t, class real, class sparse_t>
size_t sparse_array<index_t, real, sparse_t>::read(FILE *fs, long n) {
  size_t nred=0;
  size_t nred1;
  index_t m1, n1;

  if (n >= 0) {
    nsparse=n;
    sparse_a=new petey_pointer<sparse_t>[n];
    sparse_a[0]=new sparse_t();
    nred+=sparse_a[0]->read(fs);
    sparse_a[0]->dimensions(m, n1);
    assert(m==n1);
    for (long i=1; i<nsparse;i++) {
      sparse_a[i]=new sparse_t();
      nred+=sparse_a[i]->read(fs);
      sparse_a[i]->dimensions(m1, n1);
      assert(m==m1);
      assert(m==n1);
    }
  } else {
    linked_list<sparse_t*> sparse_list;
    sparse_t *sael;
    sparse_t **sa;
    sael=new sparse_t();
    nred=sael->read(fs);
    sael->dimensions(m, n1);
    assert(m==n1);
    sparse_list.add(sael);
    while (feof(fs) == 0) {
      sael=new sparse_t();
      nred1=sael->read(fs);
      if (nred1==0) break;
      sael->dimensions(m1, n1);
      assert(m==m1);
      assert(m==n1);
      sparse_list.add(sael);
      nred+=nred1;
    }
    sa=sparse_list.make_array(nsparse);
    sparse_a=new petey_pointer<sparse_t>[nsparse];
    for (long i=0; i<nsparse; i++) sparse_a[i]=sa[i];
    delete [] sa;
  }

  return nred;
}

template <class index_t, class real, class sparse_t>
size_t sparse_array<index_t, real, sparse_t>::read(FILE *fs) {
  return read(fs, -1);
}


template <class index_t, class real, class sparse_t>
size_t sparse_array<index_t, real, sparse_t>::write(FILE *fs) {
  size_t nwrit=0;
  for (long i=0; i<nsparse; i++) nwrit+=sparse_a[i]->write(fs);
  return nwrit;
}

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> * sparse_array<index_t, real, sparse_t>::mat_mult(matrix_base<index_t, real> *cand) {
  matrix_base<index_t, real> *result;
  index_t m1, n1;
  cand->dimensions(m1, n1);
  if (m != m1) {
    fprintf(stderr, "Matrix inner dimensions must match: [%dx%d]*[%dx%d]\n",
		      m, m, m1, n1);
    result=NULL;
  } else if (typeid(*cand) == typeid(full_matrix<index_t, real>)) {
    //if the multiplicand is not square, simply transform to full matrices:
    if (m1!=n1) {
      full_matrix<index_t, real> this1;
      this1=*this;
      result=this1.mat_mult(cand);
    } else {
      result=new full_matrix<index_t, real>(m, m);
      mat_mult(((full_matrix<index_t, real> *) cand)->data, 
			((full_matrix<index_t, real> *) result)->data);
    }
  } else if (typeid(*cand) == typeid(sparse<index_t, real>)) {
    //if multiplicand is not square, we transform both into full matrices:
    if (m1 != n1) {
      full_matrix<index_t, real> cand1;
      cand1=*(sparse<index_t, real> *) cand;
      //fuck, what a mess:
      result=mat_mult(&cand1);
    } else {
      sparse_array<index_t, real> cand1;
      cand1=*(sparse<index_t, real> *)cand;
      result=new sparse_array<index_t, real, sparse_t>();
      mat_mult(cand1, * (sparse_array<index_t, real> *) result);
    }
  } else if (typeid(*cand) == typeid(sparse_array<index_t, real>)) {
    result=new sparse_array<index_t, real, sparse_t>();
    mat_mult(*(sparse_array<index_t, real> *) cand, 
		* (sparse_array<index_t, real> *) result);
  } else if (typeid(*cand) == typeid(matrix_base<index_t, real>)) {
    matrix_base<index_t, real> *result;
    result=new matrix_base<index_t, real>(m, n1);
    //do nothing
  } else {
    fprintf(stderr, "sparse_array::mat_mult - error, matrix type not recognized\n");
    result=NULL;
  }

  return result;
}  

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> * sparse_array<index_t, real, sparse_t>::add(matrix_base<index_t, real> *b) {
  full_matrix<index_t, real> a;
  a=*this;
  return a.add(b);
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::scal_mult(real cand) {
  petey_pointer<sparse_t> *new_array;
  sparse_t *cand2;
  cand2=new sparse_t(m, m);
  cand2->identity();
  cand2->sl_transform(cand, 0);
  new_array=new petey_pointer<sparse_t>[nsparse+1];
  for (long i=0; i<nsparse; i++) new_array[i]=sparse_a[i];
  new_array[nsparse]=cand2;
  delete [] sparse_a;
  sparse_a=new_array;
  nsparse++;
}

template <class index_t, class real, class sparse_t>
real * sparse_array<index_t, real, sparse_t>::vect_mult(real *cand) {
  real *result;
  result=new real[m];
  vect_mult(cand, result);
  return result;
}
	
template <class index_t, class real, class sparse_t>
real * sparse_array<index_t, real, sparse_t>::left_mult(real *cor) {
  real *result;
  result=new real[m];
  left_mult(cor, result);

  return result;

}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::left_mult(real *cor, real *result) {
  real *med, *s;
  med=new real[m];

  sparse_a[nsparse-1]->left_mult(cor, result);
  for (long i=nsparse-2; i>=0; i--) {
    sparse_a[i]->left_mult(result, med);
    s=med;
    med=result;
    result=s;
  }
  if (nsparse % 2 == 0) {
    s=result;
    result=med;
    med=s;
    for (index_t i=0; i<m; i++) result[i]=med[i];
  }
  delete [] med;
}

template <class index_t, class real, class sparse_t>
matrix_base<index_t, real> *  sparse_array<index_t, real, sparse_t>::clone() {
  sparse_array<index_t, real, sparse_t> *result;
  result=new sparse_array<index_t, real, sparse_t>(*this);
  return result;
}

template <class index_t, class real, class sparse_t>
real sparse_array<index_t, real, sparse_t>::norm() {
  real **ful;
  real nm;
  ful=(real **) *this;
  nm=matrix_norm<real, index_t>(ful, m, m);
  delete_matrix(ful);
  return nm;
}

/*
template <class index_t, class data_t, class sparse_t>
sparse_array<index_t, data_t, sparse_t>::operator sparse<index_t, data_t>& () {
  sparse_t *result;
  result=new sparse_t(*this);
  return *result;
}

template <class index_t, class data_t, class sparse_t>
sparse_array<index_t, data_t, sparse_t>::operator sparse_array<index_t, data_t, sparse_t>& () {
  sparse_array<index_t, data_t, sparse_t> *result;
  result=new sparse_array<index_t, data_t, sparse_t>(*this);
  return *result;
}

template <class index_t, class data_t, class sparse_t>
sparse_array<index_t, data_t, sparse_t>::operator full_matrix<index_t, data_t>& () {
  full_matrix<index_t, data_t> *result;
  result=new full_matrix<index_t, data_t>(*this);
  return *result;
}
*/

template <class index_t, class real, class sparse_t>
int sparse_array<index_t, real, sparse_t>::scan(FILE *fs) {
  char line[200];
  int err;

  if (fgets(line, 200, fs)==NULL) return FILE_READ_ERROR;
  if (sscanf(line, "%ld", &nsparse)==0) return FILE_READ_ERROR;
  for (long i=0; i<nsparse; i++) {
    err=sparse_a[i]->scan(fs);
    if (err==FILE_READ_ERROR) break;
  }
  return err;
}

template <class index_t, class real, class sparse_t>
void sparse_array<index_t, real, sparse_t>::print(FILE *fs) {
  fprintf(fs, "%ld\n", nsparse);
  for (long i=0; i<nsparse; i++) sparse_a[i]->print(fs);
}

//template class sparse_array<int16_t, float, sparse_matrix>;
template class sparse_array<ind_t, float, sparse_matrix>;
template class sparse_array<ind_t, double, sparse<ind_t, double> >;

} //end namespace libsparse
} //end namespace libpetey

