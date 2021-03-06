#ifndef MATRIX_BASE_H
#define MATRIX_BASE_H

#include <stdio.h>
#include <typeinfo>

#include "matrix_base.h"

//"null" matrix: i.e. pre-compute the dimensions for error-checking purposes

namespace libpetey {
  namespace libsparse {

    template <class index_t, class scalar_t, class vector_t>
    class sc_mat_t:public sc_t, public matrix_base<index_t, scalar_t> {
      public:
        virtual ~sc_mat_t();

        //vector multiplication:
        virtual vector_t * vect_mult2(vector_t *cand)=0;
        virtual vector_t * left_mult2(vector_t *cor)=0;

        //the above are good for the sparse calculator, but not for solvers
        virtual void vect_mult2(vector_t &cand, vector_t &result)=0;
        virtual void left_mult2(vector_t &cor, vector_t &result)=0;

        virtual void get_row2(index_t i, vector_t &row)=0;
        virtual vector_t * get_row2(index_t i)=0;

    };

    template <class index_t, class scalar_t, class vector_t>
    class sc_sparse_t:public sc_mat_t<index_t, scalar_t, vector_t>,
		public sparse<index_t, scalar_t> {
      public:
        sc_sparse_t(char *filename);
        sc_sparse_t(sc_mat_t<index_t, scalar_t, vector_t> *other);
        virtual ~sc_sparse_t();

        //vector multiplication:
        virtual vector_t * vect_mult2(vector_t *cand);
        virtual vector_t * left_mult2(vector_t *cor);

        //the above are good for the sparse calculator, but not for solvers
        virtual void vect_mult2(vector_t &cand, vector_t &result);
        virtual void left_mult2(vector_t &cor, vector_t &result);

        virtual void get_row2(index_t i, vector_t &row);
        virtual vector_t * get_row2(index_t i);

        full_matrix<index_t, real> *cmult2(vector_t &cand);
        full_matrix<index_t, real> *left_cmult2(vector_t &cor);
    };

    template <class index_t, class scalar_t, class vector_t>
    class sc_full_t:public sc_mat_t<index_t, scalar_t, vector_t>,
		public full_matrix<index_t, scalar_t> {
      public:
        sc_full_t(char *filename);
        sc_full_t(sc_mat_t<index_t, scalar_t, vector_t> *other);
        virtual ~sc_full_t();

        //vector multiplication:
        virtual vector_t * vect_mult2(vector_t *cand);
        virtual vector_t * left_mult2(vector_t *cor);

        //the above are good for the sparse calculator, but not for solvers
        virtual void vect_mult2(vector_t &cand, vector_t &result);
        virtual void left_mult2(vector_t &cor, vector_t &result);

        virtual void get_row2(index_t i, vector_t &row);
        virtual vector_t * get_row2(index_t i);

    };

    template <class index_t, class scalar_t, class vector_t>
    class sc_sparse_array_t:public sc_mat_t<index_t, scalar_t, vector_t>,
		public sparse_array<index_t, scalar_t, sparse_t> {
      public:
        sc_sparse_array_t(char *filename);
        sc_sparse_array_t(sc_mat_t<index_t, scalar_t> *other);
        virtual ~sc_sparse_array_t();

        //vector multiplication:
        virtual vector_t * vect_mult2(vector_t *cand);
        virtual vector_t * left_mult2(vector_t *cor);

        //the above are good for the sparse calculator, but not for solvers
        virtual void vect_mult2(vector_t &cand, vector_t &result);
        virtual void left_mult2(vector_t &cor, vector_t &result);

        virtual void get_row2(index_t i, vector_t &row);
        virtual vector_t * get_row2(index_t i);

    };


  } //end namespace libsparse
} //end namespace libpetey

#endif

