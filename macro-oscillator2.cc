#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/utils/random.h"
#include "plaits/dsp/dsp.h"
#include "plaits/dsp/engine/engine.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, lfo2 = 0, mix = 0;
bool gate = false, previous_gate = false;

plaits::EngineParameters parameters = {
    .trigger = plaits::TRIGGER_UNPATCHED, 
    .note = 0,
    .timbre = 0,
    .morph = 0,
    .harmonics = 0,
    .accent = 0
};

stmlib::CosineOscillator lfo;

enum LfoTarget {
  LfoTargetShape,
  LfoTargetShiftShape,
  LfoTargetParam1,
  LfoTargetParam2,
  LfoTargetPitch,
  LfoTargetAmplitude,
  LfoTargetLfo2Frequency,
  LfoTargetLfo2Depth
};

inline float get_lfo_value(enum LfoTarget target) {
  return (p_values[k_user_osc_param_id3] == target ? shape_lfo : 0.0f) +
    (p_values[k_user_osc_param_id6] == target ? lfo2 : 0.0f);
}

inline float get_shape() {
  return clip01f(shape + get_lfo_value(LfoTargetShape));
}
inline float get_shift_shape() {
  return clip01f(shiftshape + get_lfo_value(LfoTargetShiftShape));
}
inline float get_param_id1() {
  return clip01f((p_values[k_user_osc_param_id1] * 0.005f) + get_lfo_value(LfoTargetParam1));
}
inline float get_param_id2() {
  return clip01f((p_values[k_user_osc_param_id2] * 0.01f) + get_lfo_value(LfoTargetParam2));
}
inline float get_param_lfo2_frequency() {
  return clip01f((p_values[k_user_osc_param_id4] * 0.01f) + get_lfo_value(LfoTargetLfo2Frequency));
}
inline float get_param_lfo2_depth() {
  return clip01f((p_values[k_user_osc_param_id5] * 0.01f) + get_lfo_value(LfoTargetLfo2Depth));
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
  parameters.harmonics = p_values[k_user_osc_param_id1] == 0 ? 0.f : 1.f;
  parameters.timbre = get_shape();
  parameters.morph = get_shift_shape();
  mix = get_param_id2();
}
#endif

#if defined(OSC_STRING)
#define USE_LIMITER
//float out_gain = 0.5f, aux_gain = 0.5f;
#include "plaits/dsp/engine/string_engine.h"
plaits::StringEngine engine;
void update_parameters() {
  parameters.harmonics = get_param_id1();
  parameters.timbre = get_shift_shape();
  parameters.morph = get_shape();
  mix = get_param_id2();
}
#endif

#if defined(OSC_MODAL)
#define USE_LIMITER
#include "plaits/dsp/engine/modal_engine.h"
plaits::ModalEngine engine;
void update_parameters() {
  parameters.harmonics = get_param_id1();
  parameters.timbre = get_shift_shape();
  parameters.morph = get_shape();
  mix = get_param_id2();
}
#endif

#if defined(USE_LIMITER)
#include "stmlib/dsp/limiter.h"
stmlib::Limiter limiter_;
#endif

void OSC_INIT(uint32_t platform, uint32_t api)
{
#if defined(OSC_STRING)
  stmlib::Random::Seed(0x82eef2a3);
  static uint8_t engine_buffer[4096*sizeof(float)] = {0};
#else 
 #if defined(OSC_MODAL)
  stmlib::Random::Seed(0x82eef2a3);
  static uint8_t engine_buffer[plaits::kMaxBlockSize*sizeof(float)] = {0};
 #else
  static uint8_t engine_buffer[plaits::kMaxBlockSize*sizeof(float)] = {0};
 #endif
#endif

#if defined(USE_LIMITER)
  limiter_.Init();
#endif

  stmlib::BufferAllocator allocator;
  allocator.Init(engine_buffer, sizeof(engine_buffer));
  engine.Init(&allocator);
  lfo.InitApproximate(0);
  lfo.Start();

  p_values[0] = 100;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  gate = true;
  lfo.Start();
}
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  gate = false;
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
  static float out[plaits::kMaxBlockSize], aux[plaits::kMaxBlockSize];
  static bool enveloped;

  shape_lfo = q31_to_f32(params->shape_lfo);
  lfo.InitApproximate(get_param_lfo2_frequency() / 600.f);
  lfo2 = (lfo.Next() - 0.5f) * 2.0f * get_param_lfo2_depth();
 
  parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
  parameters.note += (get_lfo_value(LfoTargetPitch) * 0.5);

  if(gate && !previous_gate) {
    parameters.trigger = plaits::TRIGGER_RISING_EDGE;
  } else {
    parameters.trigger = plaits::TRIGGER_LOW;
  }
  previous_gate = gate;

  update_parameters();

  engine.Render(parameters, out, aux, plaits::kMaxBlockSize, &enveloped);

#if defined(USE_LIMITER)
  limiter_.Process(1.0 - mix, out, plaits::kMaxBlockSize);
  for(size_t i=0;i<plaits::kMaxBlockSize;i++) {
    yn[i] = f32_to_q31(out[i]);
  }
#else
  for(size_t i=0;i<plaits::kMaxBlockSize;i++) {
    float o = out[i] * out_gain, a = aux[i] * aux_gain;
    yn[i] = f32_to_q31(stmlib::Crossfade(o, a, mix));
  }
#endif
}

inline float percentage_to_f32(int16_t value) {
  return clip01f(value * 0.01f);
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  switch (index)
  {
  case k_user_osc_param_id1:
  case k_user_osc_param_id2:
  case k_user_osc_param_id3:
  case k_user_osc_param_id4:
  case k_user_osc_param_id5:
  case k_user_osc_param_id6:
    p_values[index] = value;
    break;

  case k_user_osc_param_shape:
    shape = param_val_to_f32(value);
    break;

  case k_user_osc_param_shiftshape:
    shiftshape = param_val_to_f32(value);
    break;

  default:
    break;
  }
}
