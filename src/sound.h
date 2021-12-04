#ifndef SOUND_H
#define SOUND_H

#include "mathlib.h"
#include <stdio.h>

#define FREQ_MIN 2.0
#define FREQ_MAX 20500.0

typedef struct {
  unsigned int n_freqs;
  float loudness;
  float* frequencies;
  float* intensities;
  distType dtype;
} waveSpec;

float soundwave_at(waveSpec *wspec, float time);
float soundwave_at2(waveSpec *wspec, float time);

waveSpec new_wave_spec(unsigned int n_frequencies, distType dtype, float loudness,
                       void *userdata);

void update_wave_spec_intensities(waveSpec* wspec, void *userdata);

void wspec_free(waveSpec wspec);

void dump_wave_spec(FILE* f, waveSpec *wspec);

#endif // SOUND_H header guard
