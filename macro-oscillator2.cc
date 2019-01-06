#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "plaits/dsp/dsp.h"

#ifdef OSC_VA
#include "plaits/dsp/engine/virtual_analog_engine.h"
plaits::VirtualAnalogEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
#endif

#ifdef OSC_WSH
#include "plaits/dsp/engine/waveshaping_engine.h"
plaits::WaveshapingEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
#endif

#ifdef OSC_FM
#include "plaits/dsp/engine/fm_engine.h"
plaits::FMEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
#endif

#ifdef OSC_GRN
#include "plaits/dsp/engine/grain_engine.h"
plaits::GrainEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
#endif

#ifdef OSC_ADD
#include "plaits/dsp/engine/additive_engine.h"
plaits::AdditiveEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
#endif

#if defined(OSC_WTA) || defined(OSC_WTB) || defined(OSC_WTC) || defined(OSC_WTD) || defined(OSC_WTE) || defined(OSC_WTF)
#include "plaits/dsp/engine/wavetable_engine.h"
plaits::WavetableEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
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

int shape_param = 0, shift_shape_param = 0;

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

  float shape_lfo = q31_to_f32(params->shape_lfo);

  parameters.harmonics = clip01f(harmonics + 
    (shape_param == 0 ? (shape + shape_lfo) : 0.0f) +
    (shift_shape_param == 0 ? shiftshape : 0.0f));

  parameters.timbre = clip01f(timbre + 
    (shape_param == 1 ? (shape + shape_lfo) : 0.0f) +
    (shift_shape_param == 1 ? shiftshape : 0.0f));

  parameters.morph = clip01f(morph + 
    (shape_param == 2 ? (shape + shape_lfo) : 0.0f) +
    (shift_shape_param == 2 ? shiftshape : 0.0f));

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
    shape_param = value % 3;
    break;

  case k_osc_param_id6:
    shift_shape_param = value % 3;
    break;

  case k_osc_param_shape:
    shape = param_val_to_f32(value);
    break;

  case k_osc_param_shiftshape:
    shiftshape = param_val_to_f32(value);
    break;

  default:
    break;
  }
}
