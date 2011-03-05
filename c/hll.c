
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "bingham/util.h"
#include "bingham/hll.h"





/*
 * Fills in a new HLL with a default prior.
 */
static void hll_default_prior(hll_t *hll)
{
  int dx = hll->dx;
  int n = hll->n;

  safe_malloc(hll->x0, dx, double);
  mean(hll->x0, hll->X, n, dx);  // x0 = mean(X)

  hll->S0 = new_matrix2(dx, dx);
  cov(hll->S0, hll->X, hll->x0, n, dx);

  hll->w0 = 2;

  /*
    x0 = zeros(1,3);
    S0 = mean(sum(X.^2,2))/3 * eye(3);
    w0 = 2;
  */
}


/*
 * Make a new HLL model from Q->X, with a default prior.
 */
void hll_new(hll_t *hll, double **Q, double **X, int n, int dq, int dx)
{
  hll->Q = matrix_clone(Q, n, dq);
  hll->X = X;
  hll->n = n;
  hll->dq = dq;
  hll->dx = dx;

  hll->r = .2;  //dbug: is there a more principled way of setting this?

  hll_default_prior(hll);
}


/*
 * Free an HLL model.
 */
void hll_free(hll_t *hll)
{
  free_matrix2(hll->Q);
  free_matrix2(hll->X);
  if (hll->x0)
    free(hll->x0);
  if (hll->S0)
    free(hll->S0);
}


/*
 * Sample n Gaussians (with means X and covariances S) from HLL at sample points Q.
 */
void hll_sample(double **X, double ***S, double **Q, hll_t *hll, int n)
{
  double r = hll->r;
  int nx = hll->dx;
  int nq = hll->dq;
  int i, j;

  double **WS = new_matrix2(nx, nx);

  for (i = 0; i < n; i++) {

    // compute weights
    double dq;
    double w[hll->n];
    for (j = 0; j < hll->n; j++) {
      dq = acos(fabs(dot(Q[i], hll->Q[j], nq)));
      w[j] = exp(-(dq/r)*(dq/r));
    }

    // threshold weights
    double wmax = max(w, hll->n);
    double wthresh = wmax/50;  //dbug: make this a parameter?
    for (j = 0; j < hll->n; j++)
      if (w[j] < wthresh)
	w[j] = 0;
    double wtot = hll->w0 + sum(w, hll->n);

    // compute posterior mean
    mult(X[i], hll->x0, hll->w0, nx);  // X[i] = w0*x0
    for (j = 0; j < hll->n; j++) {
      if (w[j] > 0) {
	double wx[nx];
	mult(wx, hll->X[j], w[j], nx);
	add(X[i], X[i], wx, nx);       // X[i] += wx
      }
    }
    mult(X[i], X[i], 1/wtot, nx);  // X[i] /= wtot

    // compute posterior covariance matrix
    mult(S[i][0], hll->S0[0], hll->w0, nx*nx);  // S[i] = w0*S0
    for (j = 0; j < hll->n; j++) {
      if (w[j] > 0) {
	double wdx[nx];
	sub(wdx, hll->X[j], X[i], nx);
	mult(wdx, wdx, w[j], nx);
	outer_prod(WS, wdx, wdx, nx, nx);    // WS = wdx'*wdx
	matrix_add(S[i], S[i], WS, nx, nx);  // S[i] += WS
      }
    }
    mult(S[i][0], S[i][0], 1/wtot, nx*nx);  // S[i] /= wtot
  }

  free_matrix2(WS);
}
