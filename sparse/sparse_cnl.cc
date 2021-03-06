#include <math.h>
    
#include "sparse_array.h"
#include "full_matrix.h"
    
#include "sparse_element.h"

namespace libpetey {
  namespace libsparse {
    
    template <class index_t, class data_t>
    matrix_base<index_t, data_t> & sparse<index_t, data_t>::operator = (full_matrix<index_t, data_t> &other) {
      from_full(other.data, other.m, other.n);
      return *this;
    }
    
    template <class index_t, class data_t>
    void sparse<index_t, data_t>::convert(sparse_array<index_t, data_t> &other, data_t neps) {
      sparse<index_t, data_t> cand;
      eps=neps;
      matrix=NULL;		//prevent seg. fault
      other.dimensions(m, n);
      cand.identity(m, n);
      other.mat_mult(&cand, this);
    }
    
    template <class index_t, class data_t>
    matrix_base<index_t, data_t> & sparse<index_t, data_t>::operator = (sparse_array<index_t, data_t> &other) {
      convert(other);
      return *this;
    }
    
    template <class index_t, class data_t>
    sparse<index_t, data_t>::sparse(sparse_array<index_t, data_t> &other) {
      sparse_log=stderr;
      convert(other);
    }
    
    /*
    template <class index_t, class data_t>
    sparse<index_t, data_t>::operator sparse<index_t, data_t>& () {
      sparse<index_t, data_t> *result;
      result=new sparse<index_t, data_t>(*this);
      return *result;
    }
    
    template <class index_t, class data_t>
    sparse<index_t, data_t>::operator sparse_array<index_t, data_t>& () {
      sparse_array<index_t, data_t> *result;
      result=new sparse_array<index_t, data_t>(*this);
      return *result;
    }
    
    template <class index_t, class data_t>
    sparse<index_t, data_t>::operator full_matrix<index_t, data_t>& () {
      full_matrix<index_t, data_t> *result;
      result=new full_matrix<index_t, data_t>(*this);
      return *result;
    
    }
    */
    
    
    template <class index_t, class data_t>
    matrix_base<index_t, data_t> * sparse<index_t, data_t>::mat_mult(
    		    matrix_base<index_t, data_t> *cand) {
      index_t m1, n1;
      cand->dimensions(m1, n1);
      if (m1 != n) {
        fprintf(stderr, "Matrix multiply: inner dimensions don't match:\n");
        fprintf(stderr, "  sparse [%d x %d] * matrix [%d x %d]\n", m, n, m1, n1);
        return NULL;
      }
      //printf("sparse_cnl: outputting matrix multiplicand\n");
      //cand->print(stdout);
    
      if (typeid(*cand) == typeid(full_matrix<index_t, data_t>)) {
        full_matrix<index_t, data_t> *cand2;
        full_matrix<index_t, data_t> *result;
        cand2=(full_matrix<index_t, data_t> *) cand;
        result=new full_matrix<index_t, data_t>(m, n1);
        mat_mult(cand2->data, result->data, n1);
        printf("sparse_cnl: outputting matrix product operation\n");
        result->print(stdout);
        return result;
      } else if (typeid(*cand) == typeid(sparse<index_t, data_t>)) {
        if (m!=n || m1 != n1) {
          sparse<index_t, data_t> *result;
          result=new sparse<index_t, data_t>;
          mat_mult(*((sparse<index_t, data_t> *) cand), *result);
          return result;
        } else {
          sparse_array<index_t, data_t> *tss;
          //sparse_array<index_t, data_t> *result;
          sparse_array<index_t, data_t> cand2((sparse<index_t, data_t> *) cand, 1);
          tss=new sparse_array<index_t, data_t>(this, 1);
          return tss->mat_mult(&cand2);
        }
    
      } else if (typeid(*cand) == typeid(sparse_array<index_t, data_t>)) {
        if (m!=n) {
          full_matrix<index_t, data_t> cand1(*(full_matrix<index_t, data_t> *) cand);
          return mat_mult(&cand1);
        } else {
          sparse_array<index_t, data_t> *tss;
          sparse_array<index_t, data_t> *result;
    
          result=new sparse_array<index_t, data_t>();
          tss=new sparse_array<index_t, data_t>(this, 1);
    
          tss->mat_mult(* (sparse_array<index_t, data_t> *) cand, *result);
          //dangling pointer...
          delete tss;
          return result;
        }
      } else if (typeid(*cand) == typeid(matrix_base<index_t, data_t>)) {
        matrix_base<index_t, data_t> *result;
        result=new matrix_base<index_t, data_t>(m, n1);
        //do nothing
        return result;
      } else {
        fprintf(stderr, "sparse_cnl::mat_mult - error, matrix type not recognized\n");
        return NULL;
      }
    }
    
    
    template <class index_t, class data_t>
    data_t * sparse<index_t, data_t>::vect_mult(data_t *cand) {
      data_t *result;
      result=new data_t[m];
      vect_mult(cand, result);
      return result;
    }
    
    template <class index_t, class data_t>
    matrix_base<index_t, data_t> * sparse<index_t, data_t>::add(matrix_base<index_t, data_t> *b) {
      index_t m1, n1;
      b->dimensions(m1, n1);
      if (m != m1 || n != n1) {
        fprintf(stderr, "Matrix add: matrix dimensions don't match:\n");
        fprintf(stderr, "  sparse [%d x %d] + matrix [%d x %d]\n", m, n, m1, n1);
        return NULL;
      }
      if (typeid(*b) == typeid(full_matrix<index_t, data_t>)) {
        full_matrix<index_t, data_t> *result;
        result=new full_matrix<index_t, data_t>();
        *result=* (full_matrix<index_t, data_t> *) b;
        full_add(result->data);
        return result;
      } else if (typeid(*b) == typeid(sparse<index_t, data_t>)) {
        sparse<index_t, data_t> *result;
        result=new sparse<index_t, data_t>(*this);
        result->sparse_add(*(sparse<index_t, data_t> *) b);
        return result;
      } else if (typeid(*b) == typeid(sparse_array<index_t, data_t>)) {
        full_matrix<index_t, data_t> *result;
        result=new full_matrix<index_t, data_t>();
        *result=*(sparse_array<index_t, data_t> *) b;
        full_add(result->data);
        return result;
      } else if (typeid(*b) == typeid(matrix_base<index_t, data_t>)) {
        matrix_base<index_t, data_t> *result;
        result=new matrix_base<index_t, data_t>(m, n);
        return result;
      } else {
        fprintf(stderr, "sparse_cnl::add - error, matrix type not recognized\n");
        return NULL;
      }
    }
    
    template <class index_t, class data_t>
    void sparse<index_t, data_t>::scal_mult(data_t m) {
      sl_transform(m, 0.);
    }
    
    template <class index_t, class data_t>
    data_t * sparse<index_t, data_t>::left_mult(data_t *cor) {
      data_t *result;
      result=new data_t[n];
      left_mult(cor, result);
      return result;
    }
    
    template <class index_t, class data_t>
    matrix_base<index_t, data_t> * sparse<index_t, data_t>::clone() {
      sparse<index_t, data_t> *result;
      result=new sparse<index_t, data_t>(*this);
      return result;
    }
    
    template <class index_t, class data_t>
    data_t sparse<index_t, data_t>::norm() {
      data_t result=0;
      update();
      for (index_t i=0; i<nel; i++) result+=matrix[i].value*matrix[i].value;
      return sqrt(result);
    }
    
    //some instantiations:
    //template class sparse<int16_t, float>;
    template class sparse<int32_t, float>;
    template class sparse<int32_t, double>;
    //

  } //end namespace libsparse
} //end namespace libpetey

