#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "mathlib.h"

#include "sound.h"

waveSpec new_wave_spec(unsigned int n_frequencies, distType dtype, float loudness, void*userdata) {
  waveSpec wspec = {
    .n_freqs = n_frequencies,
    .loudness = loudness,
    .dtype = dtype,
    .frequencies = malloc(n_frequencies * sizeof(float)),
    .intensities = malloc(n_frequencies * sizeof(float)),
  };

  float span_f = FREQ_MAX - FREQ_MIN;
  for (int i = 0; i < n_frequencies; i++) {
    // Sample frequencies from linear function with gauss distribution
    float cur_val = FREQ_MIN + span_f / (float)n_frequencies * i;
    wspec.frequencies[i] = rand_gauss(cur_val, cur_val * 0.2);
  }

  update_wave_spec_intensities(&wspec, userdata);
  return wspec;
}

void update_wave_spec_intensities(waveSpec* wspec, void *userdata) {
  float mean_i = 0;

  for (int i = 0; i < wspec->n_freqs; i++) {
    switch (wspec->dtype) {
      case UNIFORM:
        wspec->intensities[i] = 1.0;
        break;
      case GAUSS:
        wspec->intensities[i] = gauss(wspec->frequencies[i], (gaussData*) userdata);
        break;
      case INVERSE: {
        inverseData *id = (inverseData*) userdata;
        wspec->intensities[i] = powf(1.0 / (wspec->frequencies[i] + id->fac), id->alpha);
        } break;
    }
    mean_i += wspec->intensities[i];
  }

  mean_i /= wspec->n_freqs;
  for (int i = 0; i < wspec->n_freqs; i++) {
    wspec->intensities[i] /= mean_i;
  }
}

void wspec_free(waveSpec wspec) {
  free(wspec.frequencies);
  free(wspec.intensities);
}


float soundwave_at(waveSpec *wspec, float time) {
  float ret = 0.0;

  for (int i = 0; i < wspec->n_freqs; i++) {
    float tmp = wspec->intensities[i]
                  * cosll(2.0f * M_PI * wspec->frequencies[i] * time);
    if (i % 2 == 0) {
      ret += tmp;
    } else {
      ret -= tmp;
    }
  }

  return ret * wspec->loudness;
}

float soundwave_at2(waveSpec *wspec, float time) {
  float ret = 0.0;

  for (int i = 0; i < wspec->n_freqs; i++) {
    float tmp = wspec->intensities[i]
                  * cosf(2.0f * M_PI * wspec->frequencies[i] * time);
    if (i % 2 == 0) {
      ret += tmp;
    } else {
      ret -= tmp;
    }
  }

  return ret * wspec->loudness;
}

void dump_wave_spec(FILE* f, waveSpec *wspec) {
  for (int i = 0; i < wspec->n_freqs; i++) {
    fprintf(f,"%E, %E\n", wspec->frequencies[i],
                          wspec->intensities[i]);
  }
}

