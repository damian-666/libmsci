#ifndef SPARSE_ARRAY_H
#define SPARSE_ARRAY_H

#include <assert.h>

#include "petey_pointer.h"
#include "full_matrix.h"
#include "sparse.h"

namespace libpetey {
  namespace libsparse {

    //sparse array class
    template <class index_t, class real, class sparse_t >
    class sparse_array:public matrix_base<index_t, real> {

      private:
        void copy(sparse_array<index_t, real, sparse_t> & other, 
			long l1=-1, long l2=-1);
        void convert(sparse_t &other);
        void convert(full_matrix<index_t, real> &other);

      protected:
        petey_pointer<sparse_t> *sparse_a;
        long nsparse;
        index_t m;

      public:
        sparse_array();
        sparse_array(sparse_t *s, long n);
        sparse_array(sparse_t **s, long n);

        sparse_array(matrix_base<index_t, real> *other);
        sparse_array(sparse_t &other);
        sparse_array(sparse_array<index_t, real, sparse_t> & other);
        sparse_array(sparse_array<index_t, real, sparse_t> & other, long l1, long l2);
        sparse_array(full_matrix<index_t, real> & other);

        sparse_array(char *file);
        virtual ~sparse_array();
        void clean();
        operator real ** ();

        sparse_array(const sparse_array &s); 
        virtual matrix_base<index_t, real> & operator = (sparse_array &s);
        virtual matrix_base<index_t, real> & operator = (sparse_t &s);
        virtual matrix_base<index_t, real> & operator = (full_matrix<index_t, real> &s);

        //sparse_array<index_t, real, sparse_t> & operator = (matrix_base<index_t, real> &s);

        sparse<index_t, real> * get_el(long ind);		//NOT A COPY

        void reverse();
        virtual void transpose();
        //sparse_array *transpose();

        virtual void vect_mult(real *cand, real *result);
        virtual void left_mult(real *cor, real *result);

        full_matrix<index_t, real> * cmult(real *cand);
        full_matrix<index_t, real> * left_cmult(real *cor);

        void mat_mult(sparse_t *cand, sparse_t *result);
        void mat_mult(real **cand, real **result);
        void mat_mult(sparse_array<index_t, real, sparse_t> &cand,
		    sparse_array<index_t, real, sparse_t> &result);

        //just like with sparse matrices, multiplying by the transpose is 
        //more efficient, except we get back the transpose ):
        void mat_mult_tt(real **cand, real **result);

        void left_m_mult(real **cor, real **result);

        //performs the transform: m*A+b*I on each matrix...
        void asl_transform(real m, real b);

        virtual real * vect_mult(real *cand);

        sparse_array * mat_mult(sparse_t &cand);

        //access elements:
        virtual real operator () (index_t i, index_t j);
        virtual long cel (real val, index_t i, index_t j);

        //access rows:
        virtual real *operator ( ) (index_t i);
        virtual void get_row(index_t i, real *row);

        index_t dimension() const;
        virtual void dimensions(index_t &i, index_t &j) const;
        long nel();

        size_t read(FILE *fs, long n);
        virtual size_t read(FILE *fs);
        virtual size_t write(FILE *fs);
        virtual void print(FILE *fs);
        virtual int scan(FILE *fs);

        //"canonical" matrix multiply and add methods:
        virtual matrix_base<index_t, real> * mat_mult(matrix_base<index_t, real> *cand);

        virtual matrix_base<index_t, real> * add(matrix_base<index_t, real> *b);

        virtual void scal_mult(real cand);
        virtual real *left_mult(real *cor);

        virtual matrix_base<index_t, real> *clone();

        virtual real norm();
/*
        virtual operator sparse<index_t, real>& ();
        virtual operator sparse_array<index_t, real, sparse_t>& ();
        virtual operator full_matrix<index_t, real>& ();
*/

    };


    template <class index_t, class real, class sparse_t>
    inline index_t sparse_array<index_t, real, sparse_t>::dimension() const {
      return m;
    }

    template <class index_t, class real, class sparse_t>
    inline long sparse_array<index_t, real, sparse_t>::nel() {
      return nsparse;
    }

    //template <class index_t, class real>
    //class sparse_arr:public sparse_array<index_t, real, sparse<index_t, real> > {};
  } //end namespace libsparse
} //end namesapce libpetey

#endif

