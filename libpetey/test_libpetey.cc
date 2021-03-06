#include <stdio.h>

#include "erfinv.h"
#include "peteys_tmpl_lib.h"
#include "kselect.h"
#include "randomize.h"
#include "bit_array.h"
#include "solve_lode.h"
#include "rk_dumb_ts.h"
#include "intersection.h"

using namespace libpetey;

int main (int argc, char *argv[]) {
  FILE *logfs=stdout;
  //test kselect classes (all four of them):
  const int nk=5;
  int k[nk]={1, 2, 5, 10, 20};
  const int nn=7;
  int n[nn]={1, 2, 5, 10, 20, 50, 100};
  kiselect_base<float> *kisel;
  int err;
  int exit_code=0;

  printf("Inverse error function:\n");
  test_erfinv(-6, 6., 40, 0);
  printf("Inverse complementary error function:\n");
  test_erfinv(-6, 28., 40, 1);

  ran_init();

  fprintf(logfs, "Testing k-select object\n");

  for (int i=0; i<nk; i++) {
    for (int j=0; j<nn; j++) {
      if (k[i]>n[j]) continue;
      kisel=new kiselect_naive<float>(k[i]);
      err=kisel->test(n[j]);
      if (err!=0) {
        fprintf(stderr, "kiselect_naive test failed for n=%d, k=%d\n", n[j], k[i]);
	exit_code=1;
      }
      delete kisel;
      kisel=new kiselect_tree<float>(k[i]);
      err=kisel->test(n[j]);
      if (err!=0) {
        fprintf(stderr, "kiselect_tree test failed for n=%d, k=%d\n", n[j], k[i]);
	exit_code=1;
      }
      delete kisel;
      kisel=new kiselect_heap<float>(k[i]);
      err=kisel->test(n[j]);
      if (err!=0) {
        fprintf(stderr, "kiselect_heap test failed for n=%d, k=%d\n", n[j], k[i]);
	exit_code=1;
      }
      delete kisel;
      kisel=new kiselect_quick<float>(k[i]);
      err=kisel->test(n[j]);
      if (err!=0) {
        fprintf(stderr, "kiselect_quick test failed for n=%d, k=%d\n", n[j], k[i]);
	exit_code=1;
      }
      delete kisel;
    }
  }

  //test R-K integrator:
  fprintf(logfs, "Testing R-K integrator\n");
  double h=0.01;
  int nterm=5;
  double t1=0.;
  double t2=1.;
  err=test_rk_ts<double>(nterm, t1, t2, h);
  if (err!=0) {
    fprintf(stderr, "Runge-Kutta integrator failed for n=%d, h=%g; integration limits: [%g, %g]\n", nterm, h);
    exit_code=1;
  }

  h=0.1;
  nterm=8;
  t1=0.;
  t2=100.;
  err=test_rk_ts<double>(nterm, t1, t2, h);
  if (err!=0) {
    fprintf(stderr, "Runge-Kutta integrator failed for n=%d, h=%g; integration limits: [%g, %g]\n", nterm, h);
    exit_code=1;
  }

  err=test_lode(5, 1e-10);
  //err=test_lode(5, 1e-15);
  /*
  err=test_lode(5, 1e-10);
  err=test_lode(100, 1e-10);
  err=test_lode(10000, 1e-10);
  err=test_lode(1000000, 1e-10);
  */

  err=test_rk_ts2<double>(5, 0.001, 100);
  err=test_rk_ts2<double>(5, 0.01, 1000);

  fprintf(logfs, "Testing bit array\n");
  err=test_bit_array(1, 10);
  if (err!=0) exit_code=err;
  err=test_bit_array(10, 10);
  if (err!=0) exit_code=err;
  err=test_bit_array(100, 10);
  if (err!=0) exit_code=err;

  fprintf(logfs, "Testing unify_vectors command:\n");
  err=test_unify_vectors<float>(100, 3, 3, 0.1);
  if (err!=0) exit_code=err;

  ran_end();

  return exit_code;

}

