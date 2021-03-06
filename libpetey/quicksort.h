#ifndef QUICKSORT_H_INCLUDED
#define QUICKSORT_H_INCLUDED

namespace libpetey {

  template<class type>
  void quicksort(type *data, long n);

  template<class type>
  void quicksort(type *data, long *ind, long n);

  template<class type>
  void kleast_quick(type *array, long n, long k, long left, long right);

  template<class type>
  void kleast_quick(type *data, long n, long k);

  template<class type>
  void kleast_quick(type *data, long n, long k, type *result);

  template<class type>
  void kleast_quick(type *data, long n, long k, long *ind, long left, long right);

  template<class type>
  void kleast_quick(type *data, long n, long k, long *ind);

  template<class type>
  void kleast_quick(type *data, long n, long k, type *result, long *ind);

}

#endif
