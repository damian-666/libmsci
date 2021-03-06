#ifndef SPARSE_ELEMENT_H
#define SPARSE_ELEMENT_H

#include "sparse.h"

namespace libpetey {
  namespace libsparse {

    template <class index_t, class data_t>
    class sparse_el {
      friend class sparse<index_t, data_t>;
      //friend void conj_grad_norm(sparse_matrix *, data_t *, data_t *, data_t, long);

      protected:
        index_t i;
        index_t j;
        data_t value;
      public:
        sparse_el();
        sparse_el(index_t iin, index_t jin, data_t valin);
        sparse_el(const sparse_el &el);
        sparse_el operator = (const sparse_el &el);
        int operator == (const sparse_el &el);
        int operator != (const sparse_el &el);
        int operator < (const sparse_el &el);
        int operator > (const sparse_el &el);
        int operator <= (const sparse_el &el);
        int operator >= (const sparse_el &el);
    };

    template <class index_t, class data_t>
    inline sparse_el<index_t, data_t>::sparse_el() {
      i=-1;
      j=-1;
      value=0;
    }

    template <class index_t, class data_t>
    inline sparse_el<index_t, data_t>::sparse_el(index_t iin, index_t jin, data_t valin) {
      i=iin;
      j=jin;
      value=valin;
    }

    template <class index_t, class data_t>
    inline sparse_el<index_t, data_t>::sparse_el(const sparse_el &el) {
      i=el.i;
      j=el.j;
      value=el.value;
    }

    template <class index_t, class data_t>
    inline sparse_el<index_t, data_t> sparse_el<index_t, data_t>::operator =
		(const sparse_el &el) {
      i=el.i;
      j=el.j;
      value=el.value;

      return *this;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator == (const sparse_el &el) {
      if (i==el.i && j==el.j) return 1;
      return 0;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator != (const sparse_el &el) {
      if (i!=el.i || j!=el.j) return 1;
      return 0;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator < (const sparse_el &el) {
      if (i<el.i) return 1;
      if (i==el.i && j<el.j) return 1;
      return 0;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator > (const sparse_el &el) {
      if (i>el.i) return 1;
      if (i==el.i && j>el.j) return 1;
      return 0;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator <= (const sparse_el &el) {
      if (i<el.i) return 1;
      if (i==el.i && j<=el.j) return 1;
      return 0;
    }

    template <class index_t, class data_t>
    inline int sparse_el<index_t, data_t>::operator >= (const sparse_el &el) {
      if (i>el.i) return 1;
      if (i==el.i && j>=el.j) return 1;
      return 0;
    }

  } //end namespace libsparse
} //end namespace libpetey

#endif

