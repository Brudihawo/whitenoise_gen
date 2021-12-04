#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "stdio.h"

#include "mathlib.h"

static float erfinv_coefs[ERFINV_DEGREE] = { 0.0 };
float cos_lut[COS_RESULUTION];

float cosll(float x) {
  int rem = x / (2 * M_PI) * COS_RESULUTION;
  if (rem < 0) rem = -rem;
  return cos_lut[rem % COS_RESULUTION];
}

void gen_cos_lut() {
  for (long i = 0; i < COS_RESULUTION; i++) {
    cos_lut[i] = cosf(2 * M_PI / COS_RESULUTION * i);
  }
}

float clampf(float val, float max, float min) {
  if (val > max) return max;
  if (val < min) return min;
  return val;
}

void gen_erfinv_coefs() {
  erfinv_coefs[0] = 1.0;

  for (int k = 1; k < ERFINV_DEGREE; k++) {
    erfinv_coefs[k] = 0.0;
    for (int m = 0; m < k; m++) {
      erfinv_coefs[k] += (erfinv_coefs[m] * erfinv_coefs[k - 1 - m])
                         / ((m + 1) * (2 * m + 1));
    }
  }
  for (int k = 1; k < ERFINV_DEGREE; k++) {
    erfinv_coefs[k] *= powf(SQRT_PI_2, 2 * k + 1) / (2 * k + 1);
  }
}

float poly(float x, float* coefs, unsigned int degree) {
  if (degree == 0) {
    return coefs[0];
  } else if (degree == 1) {
    return coefs[0] + x * coefs[1];
  }

  float ret = coefs[0] + coefs[1] * x;
  for (int i = 0; i <= degree; i++) {
    ret += coefs[i] * powf(x, i);
  }
  return ret;
}

float gauss(float x, gaussData *gdata) {
  return 1 / (gdata->sigma * sqrtf(2 * M_PI))  
          * expf(-0.5 * powf((x - gdata->mu) / gdata->sigma, 2));
}

float errfinv(float x, int degree) {
  float ret = 0.0;
  for (int k = 0; k < degree; k++) {
    ret += erfinv_coefs[k] * powf(x, 2 * k + 1);
  }
  return ret;
}

static inline float frand(float min, float max) {
  assert(max > min && "Minimum needs to be smaller than maximum!");
  return min + (float) rand() / (float)RAND_MAX * (max - min);
}

float rand_gauss(float mu, float sigma) {
  float p = frand(0, 1);
  return M_SQRT2 * sigma * errfinv(2 * p - 1, ERFINV_DEGREE) + mu;
}

float lerpf(float min, float max, float t) {
  return min + (max - min) * t;
}
