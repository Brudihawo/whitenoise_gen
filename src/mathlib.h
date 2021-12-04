#ifndef MATHLIB_H
#define MATHLIB_H

#define ERFINV_DEGREE 50
#define SQRT_PI_2 sqrt(M_PI) / 2
#define COS_RESULUTION 10000

#define NORM(x, min, max) (x - min) / (max - min)

typedef enum {
  UNIFORM = 0,
  GAUSS,
  INVERSE,
} distType; 

typedef struct {
  float mu;
  float sigma;
} gaussData;

typedef struct {
  float alpha;
  float fac;
} inverseData;

void gen_erfinv_coefs();
void gen_cos_lut();

float cosll(float x);

float clampf(float val, float max, float min);

float lerpf(float min, float max, float t);

float poly(float x, float* coefs, unsigned int degree);

float errfinv(float x, int degree);

float rand_gauss(float mu, float sigma);

float gauss(float x, gaussData *gdata);


#endif // MATHLIB_H header guard
