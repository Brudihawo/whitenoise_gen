#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#include <SDL2/SDL.h>

#include "mathlib.h"
#include "sound.h"

#define DEVICE_FREQ 41000
#define N_FREQS 2000

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define BACKGROUND 0x352f2AFF
#define FOREGROUND 0xF4F0EDFF
#define ACCENT     0xE49B5DFF
#define BORDER 128
#define WINDOW_WIDTH (SCREEN_WIDTH - 4 * BORDER) / 2
#define WINDOW_HEIGHT (SCREEN_HEIGHT - 4 * BORDER) / 2
#define SLIDER_HEIGHT 10

#define PLOT_RES 128

typedef enum {
  SLIDER_VOLUME = 0,
  SLIDER_ALPHA,
  SLIDER_FAC,
} sliderID;

#define HEXCOLOR(code) \
  ((code) >> (3 * 8)) & 0xFF, \
  ((code) >> (2 * 8)) & 0xFF, \
  ((code) >> (1 * 8)) & 0xFF, \
  ((code) >> (0 * 8)) & 0xFF


struct applicationState {
  int mx, my;
  int active;
};

struct applicationState appstate;

long cur = 100;
long count() {
  return cur++;
}
void count_set(long count) {
  cur = count;
}


static void white_noise_callback(void* userdata, Uint8 *stream, int len) {
  assert(len % 4 == 0); // we use the stream as Uint16
  waveSpec *wspec = (waveSpec*) userdata;
  float *c_stream = (float*) stream;
  for (int i = 0; i < len / 4; i++) {
    c_stream[i] = soundwave_at(wspec, 1.0 / DEVICE_FREQ * count());
  }
}

void test_signal(waveSpec* wspec, int n_ticks) {
  float* vals = malloc(n_ticks * sizeof(float));

  FILE* f = fopen("./data/wave.csv", "w");

  long before = clock();
  for (int i = 0; i < n_ticks; i++) {
    float cur_time = 1.0 / DEVICE_FREQ * i;
    vals[i] = soundwave_at(wspec, cur_time);
  }
  long after = clock();
  float time_ms = (float)(after - before) / CLOCKS_PER_SEC / n_ticks * 1000;

  for (int i = 0; i < n_ticks; i++) {
    float cur_time = 1.0 / DEVICE_FREQ * i + 1.0;
    fprintf(f, "%E, %E\n", cur_time, vals[i]);
  }

  fprintf(stderr, "Took %f ms per it.\nAcceptable is %fms (* %f).\n",
          time_ms, 1.0 / DEVICE_FREQ * 1000,
          1.0 / DEVICE_FREQ * 1000 / time_ms);

  free(vals);
  fclose(f);

  FILE* f2 = fopen("./data/frequencies.csv", "w");
  dump_wave_spec(f2, wspec);
  fclose(f2);
}

void draw_graph(SDL_Renderer* renderer, float* val_list, float min, float max,
                int n_pts, int x, int y, int w, int h) {
  int dx = w / n_pts;
  assert(dx > 2); // small dx are invalid

  for (int i = 1; i < n_pts; i++) {
    SDL_RenderDrawLine(renderer, x + i * dx,
                                 y + h * (1 - (int) NORM(val_list[i - 1], min, max)),
                                 x + (i + 1) * dx,
                                 y + h * (1 - (int) NORM(val_list[i], min, max)));
  }
}

void draw_freqs(SDL_Renderer* renderer, waveSpec* wspec,
                int x, int y, int w, int h, float* pointbuf, int n_pts) {


  memset(pointbuf, 0.0, n_pts * sizeof(float));

  for (int i = 0; i < wspec->n_freqs; i++) {
    int idx = NORM(wspec->frequencies[i], FREQ_MIN, FREQ_MAX) * (float)n_pts;

    pointbuf[idx] += wspec->intensities[i];
  }

  float max = FLT_MIN;
  float min = FLT_MAX;
  for (int idx = 0; idx < n_pts; idx++) {
    if (pointbuf[idx] < min) min = pointbuf[idx];
    if (pointbuf[idx] > max) max = pointbuf[idx];
  }

  draw_graph(renderer, pointbuf, min, max, n_pts, x, y, w, h);
}

void draw_wave(SDL_Renderer *renderer, waveSpec *wspec,
               int x, int y, int w, int h, float* pointbuf, int n_pts) {

  float max = FLT_MIN;
  float min = FLT_MAX;

  for (int i = 0; i < n_pts; i++) {
    pointbuf[i] = soundwave_at(wspec, (float) i / DEVICE_FREQ);
    if (pointbuf[i] < min) min = pointbuf[i];
    if (pointbuf[i] > max) max = pointbuf[i];
  }
  draw_graph(renderer, pointbuf, min, max, n_pts, x, y, w, h);
}

// TODO: Sliders for changing parameters
void draw_grid(SDL_Renderer* renderer, int nx, int ny, int w, int h) {
  for (int i = 1; i < nx; i++) {
    SDL_RenderDrawLine(renderer, w / nx * i, 0, w / nx * i, h);
  }
  for (int j = 1; j < ny; j++) {
    SDL_RenderDrawLine(renderer, 0, h / ny * j, w, h / ny * j);
  }
}


bool slider(int id, SDL_Renderer* renderer, float* val, float min, float max,
            int x, int y, int w, int h) {
  bool changed = false;
  const SDL_Rect slider_area = {
    .x = x,
    .y = y,
    .w = w,
    .h = h
  };

  SDL_PumpEvents();
  Uint32 buttons = SDL_GetMouseState(&appstate.mx, &appstate.my);
  SDL_Rect slider_pt;
  if (appstate.active == id) {
    if (appstate.mx > x + w) appstate.mx = x + w;
    if (appstate.mx < x) appstate.mx = x;

    slider_pt.x = appstate.mx;
    slider_pt.y = y;
    slider_pt.w = SLIDER_HEIGHT;
    slider_pt.h = 4 * SLIDER_HEIGHT;
    *val = lerpf(min, max, NORM((float)appstate.mx, x, x + w));
    changed = true;
  } else {
    slider_pt.x = NORM(*val, min, max) * w + x;
    slider_pt.y = y;
    slider_pt.w = SLIDER_HEIGHT;
    slider_pt.h = 4 * SLIDER_HEIGHT;
    changed = false;
  }

  if ((buttons & SDL_BUTTON_LMASK) != 0 
       && appstate.mx > slider_pt.x 
       && appstate.mx < slider_pt.x + slider_pt.w
       && appstate.my > slider_pt.y 
       && appstate.my < slider_pt.y + slider_pt.h) {
    appstate.active = id;
  }
  if ((buttons & SDL_BUTTON_LMASK) == 0) appstate.active = -1;

  SDL_RenderFillRect(renderer, &slider_pt);
  SDL_RenderFillRect(renderer, &slider_area);

  return changed;
}

int main(void) {
  gen_erfinv_coefs();
  gen_cos_lut();
  srand(69);

  inverseData i_data = {
    .alpha = 0.1,
    .fac = 1,
  };

  waveSpec wspec = new_wave_spec(N_FREQS, INVERSE, 1.0, &i_data);
  test_signal(&wspec, DEVICE_FREQ);
  // SDL Stuff
  SDL_Window *window = NULL;
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Could not initialize SDL2: %s\n", SDL_GetError());
  }

  SDL_AudioDeviceID dev;
  SDL_AudioSpec have;
  SDL_AudioSpec want = {
    .freq = DEVICE_FREQ,
    .format = AUDIO_F32SYS,
    .channels = 1,
    .samples = 2048,
    .callback = white_noise_callback,
    .userdata = &wspec,
  };

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (dev == 0) {
    fprintf(stderr, "Failed to open audio: %s", SDL_GetError());
  } else {
    printf("Frequency: %d\n", have.freq);
    SDL_PauseAudioDevice(dev, 0);
  }

  window = SDL_CreateWindow("White Noise Generator",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);

  if (window == NULL) {
    fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  float* pointbuf = malloc(PLOT_RES * sizeof(float));

  bool quit = false;
  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          quit = true;
          break;
      }
    }
    SDL_SetRenderDrawColor(renderer, HEXCOLOR(BACKGROUND));
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, HEXCOLOR(FOREGROUND));
    draw_grid(renderer, 2, 2, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetRenderDrawColor(renderer, HEXCOLOR(ACCENT));

    // ROW 1
    draw_freqs(renderer, &wspec, BORDER,
                                 BORDER,
                                 WINDOW_WIDTH,
                                 WINDOW_HEIGHT,
                                 pointbuf, PLOT_RES);

    draw_wave(renderer, &wspec, BORDER,
                                3 * BORDER + WINDOW_HEIGHT,
                                WINDOW_WIDTH,
                                WINDOW_HEIGHT,
                                pointbuf, PLOT_RES);


    slider(SLIDER_VOLUME, renderer, &wspec.loudness, 0, 1,
           3 * BORDER + WINDOW_WIDTH,
           BORDER,
           WINDOW_WIDTH, SLIDER_HEIGHT);

    bool changed;
    changed = slider(SLIDER_ALPHA, renderer, &i_data.alpha, -2, 2,
                   i  3 * BORDER + WINDOW_WIDTH,
                     2 * BORDER,
                     WINDOW_WIDTH, SLIDER_HEIGHT);

    changed |= slider(SLIDER_FAC, renderer, &i_data.fac, 0, 10,
                      3 * BORDER + WINDOW_WIDTH,
                      3 * BORDER,
                      WINDOW_WIDTH, SLIDER_HEIGHT);
    if (changed) {
      update_wave_spec_intensities(&wspec, &i_data);
      printf("New Configuration:\nAlpha: %E, Fac: %E\n", i_data.alpha, i_data.fac);
    }

    SDL_RenderPresent(renderer);
  }

  SDL_CloseAudioDevice(dev);
  SDL_DestroyWindow(window);
  wspec_free(wspec);
  SDL_Quit();
  return 0;
}
