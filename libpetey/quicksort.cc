#include <stdio.h>

namespace libpetey {

template<class type>
long qslt_partition(type *array, long left, long right, long pivotIndex) {
     type pivotValue;
     type swap;
     long storeIndex;

     pivotValue = array[pivotIndex];
     swap=array[pivotIndex];
     array[pivotIndex]=array[right]; // Move pivot to end
     array[right]=swap;

     storeIndex = left;
     for (long i=left; i<right; i++) { // left = i < right  
         if (array[i] <= pivotValue) { 
             //swap array[i] and array[storeIndex]
             swap=array[storeIndex];
             array[storeIndex]=array[i]; // Move pivot to end
             array[i]=swap;
             storeIndex = storeIndex + 1;
	 }
     }
     //swap array[storeIndex] and array[right] // Move pivot to its final place
     swap=array[storeIndex];
     array[storeIndex]=array[right]; // Move pivot to its final place
     array[right]=swap;

     return storeIndex;
}

template<class type>
long qslt_partition(type *array, long *ind, long left, long right, long pivotIndex) {
     type pivotValue;
     long swap;
     long storeIndex;

     pivotValue = array[ind[pivotIndex]];
     swap=ind[pivotIndex];
     ind[pivotIndex]=ind[right]; // Move pivot to end
     ind[right]=swap;

     storeIndex = left;
     for (long i=left; i<right; i++) { // left = i < right  
         if (array[ind[i]] <= pivotValue) { 
             //swap array[i] and array[storeIndex]
             swap=ind[storeIndex];
             ind[storeIndex]=ind[i]; // Move pivot to end
             ind[i]=swap;
             storeIndex = storeIndex + 1;
	 }
     }
     //swap array[storeIndex] and array[right] // Move pivot to its final place
     swap=ind[storeIndex];
     ind[storeIndex]=ind[right]; // Move pivot to its final place
     ind[right]=swap;

     return storeIndex;
}

template<class type>
long qsgt_partition(type *array, long left, long right, long pivotIndex) {
     type pivotValue;
     type swap;
     long storeIndex;

     pivotValue = array[pivotIndex];
     swap=array[pivotIndex];
     array[pivotIndex]=array[right]; // Move pivot to end
     array[right]=swap;

     storeIndex = left;
     for (long i=left; i<right; i++) { // left = i < right  
         if (array[i] >= pivotValue) { 
             //swap array[i] and array[storeIndex]
             swap=array[storeIndex];
             array[storeIndex]=array[i]; // Move pivot to end
             array[i]=swap;
             storeIndex = storeIndex + 1;
	 }
     }
     //swap array[storeIndex] and array[right] // Move pivot to its final place
     swap=array[storeIndex];
     array[storeIndex]=array[right]; // Move pivot to its final place
     array[right]=swap;

     return storeIndex;
}

template<class type>
long qsgt_partition(type *array, long *ind, long left, long right, long pivotIndex) {
     type pivotValue;
     long swap;
     long storeIndex;

     pivotValue = array[ind[pivotIndex]];
     swap=ind[pivotIndex];
     ind[pivotIndex]=ind[right]; // Move pivot to end
     ind[right]=swap;

     storeIndex = left;
     for (long i=left; i<right; i++) { // left = i < right  
         if (array[ind[i]] >= pivotValue) { 
             //swap array[i] and array[storeIndex]
             swap=ind[storeIndex];
             ind[storeIndex]=ind[i]; // Move pivot to end
             ind[i]=swap;
             storeIndex = storeIndex + 1;
	 }
     }
     //swap array[storeIndex] and array[right] // Move pivot to its final place
     swap=ind[storeIndex];
     ind[storeIndex]=ind[right]; // Move pivot to its final place
     ind[right]=swap;

     return storeIndex;
}

//cribbed straight from Wikipedia:
template<class type>
void quicksort(type *array, long n, long left, long right) {
    long pivotIndex;
    long pivotNewIndex;

    //for (long i=0; i<n; i++) printf("%g ", array[i]);
    //printf("\n");

    if (right > left) {
         pivotIndex = (left+right)/2;
         pivotNewIndex = qslt_partition(array, left, right, pivotIndex);
         quicksort(array, n, left, pivotNewIndex - 1);
         quicksort(array, n, pivotNewIndex + 1, right);
    }
}

//cribbed straight from Wikipedia:
template<class type>
void quicksort(type *array, long *ind, long n, long left, long right) {
    long pivotIndex;
    long pivotNewIndex;

    //for (long i=0; i<n; i++) printf("%g ", array[i]);
    //printf("\n");

    if (right > left) {
         pivotIndex = (left+right)/2;
         pivotNewIndex = qslt_partition(array, ind, left, right, pivotIndex);
         quicksort(array, ind, n, left, pivotNewIndex - 1);
         quicksort(array, ind, n, pivotNewIndex + 1, right);
    }
}

template<class type>
void quicksort(type *data, long n) {
	quicksort(data, n, 0, n-1);
}

template<class type>
void quicksort(type *data, long *ind, long n) {
        for (long i=0; i<n; i++) ind[i]=i;
	quicksort(data, ind, n, 0, n-1);
}

template void quicksort<float>(float *data, long n);
template void quicksort<double>(double *data, long n);
template void quicksort<float>(float *data, long *ind, long n);
template void quicksort<double>(double *data, long *ind, long n);

template<class type>
void kleast_quick(type *array, long n, long k, long left, long right) {
    long pivotIndex;
    long pivotNewIndex;

    //for (long i=0; i<n; i++) printf("%g ", array[i]);
    //printf("\n");

    if (right > left) {
         pivotIndex = (left+right)/2;
         pivotNewIndex = qslt_partition(array, left, right, pivotIndex);
         if (pivotNewIndex >= k) kleast_quick(array, n, k, left, pivotNewIndex - 1);
         //kleast(array, n, k, left, pivotNewIndex - 1);
         if (pivotNewIndex+1 < k) kleast_quick(array, n, k, pivotNewIndex + 1, right);
    }
}

template<class type>
void kleast_quick(type *data, long n, long k) {
	kleast_quick(data, n, k, 0, n-1);
}

template<class type>
void kleast_quick(type *data, long n, long k, type *result) {
	kleast_quick(data, n, k, 0, n-1);
        for (long i=0; i<k; i++) result[i]=data[i];
}

template void kleast_quick<float>(float *, long, long);
template void kleast_quick<double>(double *, long, long);
template void kleast_quick<float>(float *, long, long, float *);
template void kleast_quick<double>(double *, long, long, double *);

template<class type>
void kleast_quick(type *array, long n, long k, long *ind, long left, long right) {
    long pivotIndex;
    long pivotNewIndex;

    //for (long i=0; i<n; i++) printf("%g ", array[i]);
    //printf("\n");


    if (right > left) {
         pivotIndex = (left+right)/2;
         pivotNewIndex = qslt_partition(array, ind, left, right, pivotIndex);
         if (pivotNewIndex >= k) kleast_quick(array, n, k, ind, left, pivotNewIndex - 1);
         //kleast(array, n, k, left, pivotNewIndex - 1);
         if (pivotNewIndex+1 < k) kleast_quick(array, n, k, ind, pivotNewIndex + 1, right);
    }
}

template<class type>
void kleast_quick(type *data, long n, long k, long *ind) {
    long *ind2;
    ind2=new long[n];
    for (long i=0; i<n; i++) ind2[i]=i;
    kleast_quick(data, n, k, ind2, 0, n-1);
    for (long i=0; i<k; i++) {
      ind[i]=ind2[i];
    }
    delete [] ind2;
}

//all this bullshit to make it compatible with an interface that wasn't very good
//to begin with....
template<class type>
void kleast_quick(type *data, long n, long k, type *result, long *ind) {
    long *ind2;
    ind2=new long[n];
    for (long i=0; i<n; i++) ind2[i]=i;
    kleast_quick(data, n, k, ind2, 0, n-1);
    for (long i=0; i<k; i++) {
      result[i]=data[ind2[i]];
      ind[i]=ind2[i];
    }
    delete [] ind2;
}

template void kleast_quick<float>(float *, long, long, float *, long *);
template void kleast_quick<double>(double *, long, long, double *, long *);

} //end namespace libpetey

