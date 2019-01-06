#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "plaits/dsp/dsp.h"

#ifdef OSC_VA
#include "plaits/dsp/engine/virtual_analog_engine.h"
plaits::VirtualAnalogEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
#endif

#ifdef OSC_WS
#include "plaits/dsp/engine/waveshaping_engine.h"
plaits::WaveshapingEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
#endif

#ifdef OSC_FM
#include "plaits/dsp/engine/fm_engine.h"
plaits::FMEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
#endif

#ifdef OSC_GR
#include "plaits/dsp/engine/grain_engine.h"
plaits::GrainEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
#endif

#ifdef OSC_AD
#include "plaits/dsp/engine/additive_engine.h"
plaits::AdditiveEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
#endif

plaits::EngineParameters parameters = {
  .trigger = plaits::TRIGGER_UNPATCHED, 
  .note = 0,
  .timbre = 0,
  .morph = 0,
  .harmonics = 0,
  .accent = 0
};

float harmonics = 0, 
  timbre = 0, 
  morph = 0, 
  auxmix = 0,
  shape = 0,
  shiftshape = 0;

int param_mode = 0;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  static uint8_t engine_buffer[plaits::kMaxBlockSize*sizeof(float)];
  stmlib::BufferAllocator allocator;
  allocator.Init(engine_buffer, sizeof(engine_buffer));
  engine.Init(&allocator);
}

void OSC_CYCLE(const user_osc_param_t *const params,
               int32_t *yn,
               const uint32_t frames)
{
  static float out[plaits::kMaxBlockSize], aux[plaits::kMaxBlockSize];
  static bool enveloped;

  parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);

  float shape_lfo = clip01f(shape + q31_to_f32(params->shape_lfo));

  switch(param_mode) {
  case 0:
    parameters.timbre = shape_lfo;
    parameters.morph = shiftshape;
    parameters.harmonics = harmonics;
    break;
  case 1:
    parameters.timbre = shape_lfo;
    parameters.morph = morph;
    parameters.harmonics = shiftshape;
    break;
  case 2:
    parameters.timbre = shiftshape;
    parameters.morph = shape_lfo;
    parameters.harmonics = harmonics;
    break;
  case 3:
    parameters.timbre = timbre;
    parameters.morph = shape_lfo;
    parameters.harmonics = shiftshape;
    break;
  case 4:
    parameters.timbre = shiftshape;
    parameters.morph = morph;
    parameters.harmonics = shape_lfo;
    break;
  case 5:
    parameters.timbre = timbre;
    parameters.morph = shiftshape;
    parameters.harmonics = shape_lfo;
    break;
  }

  engine.Render(parameters, out, aux, (size_t)frames, &enveloped);

  for(size_t i=0;i<frames;i++) {
    float o = out[i] * out_gain, a = aux[i] * aux_gain;
    yn[i] = f32_to_q31(stmlib::Crossfade(o, a, auxmix));
  }
}

void OSC_NOTEON(const user_osc_param_t *const params)
{
  (void)params;
}

void OSC_NOTEOFF(const user_osc_param_t *const params)
{
  (void)params;
}

inline float percentage_to_f32(int16_t value) {
  return clip01f(value * 0.01f);
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  switch (index)
  {
  case k_osc_param_id1:
    harmonics = percentage_to_f32(value);
    break;

  case k_osc_param_id2:
    timbre = percentage_to_f32(value);
    break;

  case k_osc_param_id3:
    morph = percentage_to_f32(value);
    break;

  case k_osc_param_id4:
    auxmix = percentage_to_f32(value);
    break;

  case k_osc_param_id5:
    param_mode = value % 6;
    break;

  case k_osc_param_id6:
    break;

  case k_osc_param_shape:
    shape = clip01f(param_val_to_f32(value));
    break;

  case k_osc_param_shiftshape:
    shiftshape = clip01f(param_val_to_f32(value));
    break;

  default:
    break;
  }
}
