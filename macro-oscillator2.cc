#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "plaits/dsp/dsp.h"
#include "plaits/dsp/engine/engine.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, mix = 0;
plaits::EngineParameters parameters = {
    .trigger = plaits::TRIGGER_UNPATCHED, 
    .note = 0,
    .timbre = 0,
    .morph = 0,
    .harmonics = 0,
    .accent = 0
  };

#ifdef OSC_VA
#include "plaits/dsp/engine/virtual_analog_engine.h"
plaits::VirtualAnalogEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
void update_parameters() {
  parameters.harmonics = clip01f(p_values[k_osc_param_id1] * 0.01f);
  parameters.timbre = clip01f(shiftshape);
  parameters.morph = clip01f(shape + shape_lfo);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif

#ifdef OSC_WSH
#include "plaits/dsp/engine/waveshaping_engine.h"
plaits::WaveshapingEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = clip01f(shiftshape);
  parameters.timbre = clip01f(shape + shape_lfo);
  parameters.morph = clip01f(p_values[k_osc_param_id1] * 0.01f);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif

#ifdef OSC_FM
#include "plaits/dsp/engine/fm_engine.h"
plaits::FMEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = clip01f(shiftshape);
  parameters.timbre = clip01f(shape + shape_lfo);
  parameters.morph = clip01f(p_values[k_osc_param_id1] * 0.01f);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif

#ifdef OSC_GRN
#include "plaits/dsp/engine/grain_engine.h"
plaits::GrainEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = clip01f(shape + shape_lfo);
  parameters.timbre = clip01f(shiftshape);
  parameters.morph = clip01f(p_values[k_osc_param_id1] * 0.01f);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif

#ifdef OSC_ADD
#include "plaits/dsp/engine/additive_engine.h"
plaits::AdditiveEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
void update_parameters() {
  parameters.harmonics = clip01f(p_values[k_osc_param_id1] * 0.01f);
  parameters.timbre = clip01f(shape + shape_lfo);
  parameters.morph = clip01f(shiftshape);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif

#if defined(OSC_WTA) || defined(OSC_WTB) || defined(OSC_WTC) || defined(OSC_WTD) || defined(OSC_WTE) || defined(OSC_WTF)
#include "plaits/dsp/engine/wavetable_engine.h"
plaits::WavetableEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = p_values[k_osc_param_id1] == 0 ? (0.5f - 0.0625f) : (0.5f + 0.0625f);
  parameters.timbre = clip01f(shape + shape_lfo);
  parameters.morph = clip01f(shiftshape);
  mix = clip01f(p_values[k_osc_param_id2] * 0.01f);
}
#endif


void OSC_INIT(uint32_t platform, uint32_t api)
{
  static uint8_t engine_buffer[plaits::kMaxBlockSize*sizeof(float)];
  stmlib::BufferAllocator allocator;
  allocator.Init(engine_buffer, sizeof(engine_buffer));
  engine.Init(&allocator);
}

/*
template<warps::XmodAlgorithm alg>
void xmod(float *out, float *aux, int32_t *yn, const size_t frames) {
    for(size_t i=0;i<frames;i++) {
    float o = out[i] * out_gain, a = aux[i] * aux_gain;
    float s = warps::Modulator::Xmod<alg>(o, a, mix);
    yn[i] = f32_to_q31(s);
  }
}


typedef void (*MixFn)(float *out, float *aux, int32_t *yn, const size_t frames);

MixFn mixers[6] = {
  &xmod<warps::ALGORITHM_XFADE>,
  &xmod<warps::ALGORITHM_ANALOG_RING_MODULATION>,
  &xmod<warps::ALGORITHM_DIGITAL_RING_MODULATION>,
  &xmod<warps::ALGORITHM_XOR>,
  &xmod<warps::ALGORITHM_COMPARATOR>,
  &xmod<warps::ALGORITHM_NOP>
};
*/
void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
  static float out[plaits::kMaxBlockSize], aux[plaits::kMaxBlockSize];
  static bool enveloped;

  shape_lfo = q31_to_f32(params->shape_lfo);
  parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);

  update_parameters();

  engine.Render(parameters, out, aux, (size_t)frames, &enveloped);

//  mixers[p_values[k_osc_param_id3] % 6](out, aux, yn, frames);

  for(size_t i=0;i<frames;i++) {
    float o = out[i] * out_gain, a = aux[i] * aux_gain;
    yn[i] = f32_to_q31(stmlib::Crossfade(o, a, mix));
  }
}

inline float percentage_to_f32(int16_t value) {
  return clip01f(value * 0.01f);
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  switch (index)
  {
  case k_osc_param_id1:
  case k_osc_param_id2:
  case k_osc_param_id3:
  case k_osc_param_id4:
  case k_osc_param_id5:
  case k_osc_param_id6:
    p_values[index] = value;
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
