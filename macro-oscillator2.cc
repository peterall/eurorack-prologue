#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "plaits/dsp/dsp.h"
#include "plaits/dsp/engine/engine.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, mix = 0;
bool rising_edge = false;

plaits::EngineParameters parameters = {
    .trigger = plaits::TRIGGER_UNPATCHED, 
    .note = 0,
    .timbre = 0,
    .morph = 0,
    .harmonics = 0,
    .accent = 0
};

inline float get_shape() {
  return clip01f(shape + (p_values[k_osc_param_id3] == 0 ? shape_lfo : 0.0f));
}
inline float get_shift_shape() {
  return clip01f(shiftshape + (p_values[k_osc_param_id3] == 1 ? shape_lfo : 0.0f));
}
inline float get_param_id1() {
  return clip01f((p_values[k_osc_param_id1] * 0.01f) + (p_values[k_osc_param_id3] == 2 ? shape_lfo : 0.0f));
}
inline float get_param_id2() {
  return clip01f((p_values[k_osc_param_id2] * 0.01f) + (p_values[k_osc_param_id3] == 3 ? shape_lfo : 0.0f));
}

#ifdef OSC_VA
#include "plaits/dsp/engine/virtual_analog_engine.h"
plaits::VirtualAnalogEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
void update_parameters() {
  parameters.harmonics = get_param_id1();
  parameters.timbre = get_shift_shape();
  parameters.morph = get_shape();
  mix = get_param_id2();
}
#endif

#ifdef OSC_WSH
#include "plaits/dsp/engine/waveshaping_engine.h"
plaits::WaveshapingEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = get_shift_shape();
  parameters.timbre = get_shape();
  parameters.morph = get_param_id1();
  mix = get_param_id2();
}
#endif

#ifdef OSC_FM
#include "plaits/dsp/engine/fm_engine.h"
plaits::FMEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = get_shift_shape();
  parameters.timbre = get_shape();
  parameters.morph = get_param_id1();
  mix = get_param_id2();
}
#endif

#ifdef OSC_GRN
#include "plaits/dsp/engine/grain_engine.h"
plaits::GrainEngine engine;
float out_gain = 0.7f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = get_shape();
  parameters.timbre = get_shift_shape();
  parameters.morph = get_param_id1();
  mix = get_param_id2();
}
#endif

#ifdef OSC_ADD
#include "plaits/dsp/engine/additive_engine.h"
plaits::AdditiveEngine engine;
float out_gain = 0.8f, aux_gain = 0.8f;
void update_parameters() {
  parameters.harmonics = get_param_id1();
  parameters.timbre = get_shape();
  parameters.morph = get_shift_shape();
  mix = get_param_id2();
}
#endif

#if defined(OSC_WTA) || defined(OSC_WTB) || defined(OSC_WTC) || defined(OSC_WTD) || defined(OSC_WTE) || defined(OSC_WTF)
#include "plaits/dsp/engine/wavetable_engine.h"
plaits::WavetableEngine engine;
float out_gain = 0.6f, aux_gain = 0.6f;
void update_parameters() {
  parameters.harmonics = p_values[k_osc_param_id1] == 0 ? (0.5f - 0.0625f) : (0.5f + 0.0625f);
  parameters.timbre = get_shape();
  parameters.morph = get_shift_shape();
  mix = get_param_id2();
}
#endif

#if defined(OSC_SPC)
#include "plaits/dsp/engine/speech_engine.h"
plaits::SpeechEngine engine;
float out_gain = 0.7f, aux_gain = 0.8f;
void update_parameters() {
  parameters.harmonics = get_shape();
  parameters.timbre = get_shift_shape();
  parameters.morph = get_param_id1();
  mix = get_param_id2();
}
#endif

void OSC_INIT(uint32_t platform, uint32_t api)
{
  stmlib::BufferAllocator allocator;

#ifdef OSC_SPC
  allocator.Init((void*)0x0800f100, 0xff00);
#else
  static uint8_t engine_buffer[plaits::kMaxBlockSize*sizeof(float)*2];
  allocator.Init(engine_buffer, sizeof(engine_buffer));
#endif
  engine.Init(&allocator);

#if defined(OSC_SPC)
  engine.set_prosody_amount(0.f);
  engine.set_speed(0.f);
#endif
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  rising_edge = true;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
  static float out[plaits::kMaxBlockSize], aux[plaits::kMaxBlockSize];
  static bool enveloped;

  shape_lfo = q31_to_f32(params->shape_lfo);
  parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
  parameters.trigger = rising_edge ? plaits::TRIGGER_RISING_EDGE : plaits::TRIGGER_LOW;
  rising_edge = false;

  update_parameters();

  engine.Render(parameters, out, aux, (size_t)frames, &enveloped);

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
