namespace libpetey {

template <class ind_type, class dep_type>
dep_type **rk_dumb(ind_type t0, dep_type *xinit, long nx, ind_type dt, long nt,
		 void (* derivs) (ind_type, dep_type *, dep_type *, long) ) {
  dep_type **xvec;
  ind_type t, tph;
  dep_type dxall[nx], dx[nx];
  dep_type xint[nx];
  long i, j;

  xvec=new dep_type * [nt+1];

  xvec[0]=new dep_type [nx];
  for (j=0;j<nx;j++) xvec[0][j]=xinit[j];

  t=t0;
  for (i=1;i<=nt;i++) {
    //allocate space for the next vector of dep. values:
    xvec[i]=new dep_type[nx];

    //take the first set of derivatives:
    (* derivs) (t, xvec[i-1], dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt/2;	//calculate intermediate dep. values
      dxall[j]=dx[j]/6;				//start calculating the total deriv
    }

    //take the second set of derivatives:
    tph=dt/2.;
    tph=t+tph;
    (* derivs) (tph, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt/2;
      dxall[j]=dxall[j]+dx[j]/3;
    }

    //take the third set of derivatives:
    (* derivs) (tph, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt;
      dxall[j]=dxall[j]+dx[j]/3;
    }

    //take the fourth set of derivatives:
    t=t+dt;
    (* derivs) (t, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xvec[i][j]=xvec[i-1][j]+(dxall[j]+dx[j]/6)*dt;	//calculate next vector
    }
  }

  return xvec;
}

template <class ind_type, class dep_type>
void rk_dumb(ind_type t0, dep_type *xinit, long nx, ind_type dt, long nt,
		dep_type ** xvec,
		 void (* derivs) (ind_type, dep_type *, dep_type *, long) ) {
  ind_type t, tph;
  dep_type dxall[nx], dx[nx];
  dep_type xint[nx];
  long i, j;

  for (j=0;j<nx;j++) xvec[0][j]=xinit[j];

  t=t0;
  for (i=1;i<=nt;i++) {
    //take the first set of derivatives:
    (* derivs) (t, xvec[i-1], dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt/2;	//calculate intermediate dep. values
      dxall[j]=dx[j]/6;				//start calculating the total deriv
    }

    //take the second set of derivatives:
    tph=dt/2.;
    tph=t+tph;
    (* derivs) (tph, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt/2;
      dxall[j]=dxall[j]+dx[j]/3;
    }

    //take the third set of derivatives:
    (* derivs) (tph, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xint[j]=xvec[i-1][j]+dx[j]*dt;
      dxall[j]=dxall[j]+dx[j]/3;
    }

    //take the fourth set of derivatives:
    t=t+dt;
    (* derivs) (t, xint, dx, nx);
    for (j=0;j<nx;j++) {
      xvec[i][j]=xvec[i-1][j]+(dxall[j]+dx[j]/6)*dt;	//calculate next vector
    }
  }

}

} //end namespace libpetey

