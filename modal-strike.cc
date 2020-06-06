#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/limiter.h"
#include "stmlib/utils/dsp.h"
#include "elements/dsp/part.h"
#include "elements/resources.h"

using namespace elements;

inline float get_shape();
inline float get_shift_shape();
inline float get_strength();
inline float get_damping();
inline float get_timbre();
inline float get_brightness();
inline float get_mallet();

Exciter strike_;
Resonator resonator_;

#if defined(USE_LIMITER)
stmlib::Limiter limiter_;
#endif

bool previous_gate_ = false;
float exciter_level_ = 0.f;
float strength_ = 0.f;
float envelope_value_ = 0.f;

float strike_buffer_[kMaxBlockSize];

float bow_strength_buffer_[kMaxBlockSize];

float raw[kMaxBlockSize];
float center[kMaxBlockSize+2] = {.0f};

Patch patch_ = {
  .exciter_envelope_shape = 1.0f,
  .exciter_bow_level = 0.0f,
  .exciter_bow_timbre = 0.5f,
  .exciter_blow_level = 0.0f,
  .exciter_blow_meta = 0.5f,
  .exciter_blow_timbre = 0.5f,
  .exciter_strike_level = 0.8f,
  .exciter_strike_meta = 0.5f,
  .exciter_strike_timbre = 0.5f,
  .exciter_signature = 0.0f,
  .resonator_geometry = 0.2f,
  .resonator_brightness = 0.5f,
  .resonator_damping = 0.25f,
  .resonator_position = 0.3f,
  .resonator_modulation_frequency = 0.5f / kSampleRate,
  .resonator_modulation_offset = 0.1f,
  .reverb_diffusion = 0.625f,
  .reverb_lp = 0.7f,
  .space = 0.5f
};

PerformanceState performance_state_ = {
  .gate = false,
  .note = 69.0f,
  .modulation = .0f,
  .strength = 1.0f
};

float shape_lfo;

inline uint8_t GetGateFlags(bool gate_in) {
  uint8_t flags = 0;
  if (gate_in) {
    if (!previous_gate_) {
      flags |= EXCITER_FLAG_RISING_EDGE;
    }
    flags |= EXCITER_FLAG_GATE;
  } else if (previous_gate_) {
    flags = EXCITER_FLAG_FALLING_EDGE;
  }
  previous_gate_ = gate_in;
  return flags;
}

void Seed(uint32_t* seed, size_t size) {
  // Scramble all bits from the serial number.
  uint32_t signature = 0xf0cacc1a;
  for (size_t i = 0; i < size; ++i) {
    signature ^= seed[i];
    signature = signature * 1664525L + 1013904223L;
  }
  float x;

  x = static_cast<float>(signature & 7) / 8.0f;
  signature >>= 3;
  patch_.resonator_modulation_frequency = (0.4f + 0.8f * x) / elements::kSampleRate;
  
  x = static_cast<float>(signature & 7) / 8.0f;
  signature >>= 3;
  patch_.resonator_modulation_offset = 0.05f + 0.1f * x;

  x = static_cast<float>(signature & 7) / 8.0f;
  signature >>= 3;
  patch_.reverb_diffusion = 0.55f + 0.15f * x;

  x = static_cast<float>(signature & 7) / 8.0f;
  signature >>= 3;
  patch_.reverb_lp = 0.7f + 0.2f * x;

  x = static_cast<float>(signature & 7) / 8.0f;
  signature >>= 3;
  patch_.exciter_signature = x;
}

void OSC_INIT(uint32_t platform, uint32_t api)
{
  uint32_t random = 0x82eef2a3;
  stmlib::Random::Seed(random);
  Seed(&random, 1);
  strike_.Init();
  resonator_.Init();
  
#if defined(USE_LIMITER)
  limiter_.Init();
#endif
}

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 48000 Hz

* 0 Hz - 11000 Hz
  gain = 1
  actual ripple = 22.664844906794034 dB

* 12000 Hz - 24000 Hz
  gain = 0
  actual attenuation = -28.499686493575386 dB
*/

static const float ipf[] = { 0.10639816444506338f, 0.26598957651736876f, 0.3989644179169387f };
#define lp_even(a,b,c) f32_to_q31(stmlib::SoftLimit((ipf[1] * a) + (ipf[2] * b) + (ipf[0] * c)))
#define lp_odd(a,b,c)  f32_to_q31(stmlib::SoftLimit((ipf[0] * a) + (ipf[2] * b) + (ipf[1] * c)))

__fast_inline void cycle(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
  shape_lfo = q31_to_f32(params->shape_lfo);

  performance_state_.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
  int32_t pitch = static_cast<int32_t>((performance_state_.note + 41.0f) * 256.0f);
  if (pitch < 0) {
    pitch = 0;
  } else if (pitch >= 65535) {
    pitch = 65535;
  }
  float frequency = lut_midi_to_f_high[pitch >> 8] * lut_midi_to_f_low[pitch & 0xff];

  //patch_.exciter_envelope_shape = 1.0f;
  patch_.exciter_strike_level = get_strength();
  patch_.exciter_strike_meta = get_mallet();
  patch_.exciter_strike_timbre = get_timbre();
  //patch_.exciter_signature = 0.0f;
  patch_.resonator_damping = get_damping();
  patch_.resonator_brightness = get_brightness();
  patch_.resonator_geometry = get_shift_shape();
  patch_.resonator_position = get_shape();
  
  uint8_t flags = GetGateFlags(performance_state_.gate);

  float strike_meta = patch_.exciter_strike_meta;
  strike_.set_meta(
      strike_meta <= 0.4f ? strike_meta * 0.625f : strike_meta * 1.25f - 0.25f,
      EXCITER_MODEL_MALLET,
      EXCITER_MODEL_PARTICLES);
  strike_.set_timbre(patch_.exciter_strike_timbre);
  strike_.set_signature(patch_.exciter_signature);
  strike_.Process(flags, strike_buffer_, kMaxBlockSize);

  // The Strike exciter is implemented in such a way that raising the level
  // beyond a certain point doesn't change the exciter amplitude, but instead,
  // increasingly mixes the raw exciter signal into the resonator output.
  float strike_level, strike_bleed;
  strike_level = patch_.exciter_strike_level * 1.25f;
  strike_bleed = strike_level > 1.0f ? (strike_level - 1.0f) * 2.0f : 0.0f;
  strike_level = strike_level < 1.0f ? strike_level : 1.0f;

#if defined(USE_LIMITER)
  strike_level *= 1.5f;
#endif

  // Sum all sources of excitation.
  for (size_t i = 0; i < kMaxBlockSize; ++i) {
    raw[i] = strike_buffer_[i] * strike_level;
  }

    // Some exciters can cause palm mutes on release.
  float damping = patch_.resonator_damping;
  damping -= strike_.damping() * strike_level * 0.125f;
  damping -= (1.0f - bow_strength_buffer_[0]) * \
      patch_.exciter_bow_level * 0.0625f;
  
  if (damping <= 0.0f) {
    damping = 0.0f;
  }

  // Configure resonator.
  resonator_.set_frequency(frequency);
  resonator_.set_geometry(patch_.resonator_geometry);
  resonator_.set_brightness(patch_.resonator_brightness);
  resonator_.set_position(patch_.resonator_position);
  resonator_.set_damping(damping);
  resonator_.set_modulation_frequency(patch_.resonator_modulation_frequency);
  resonator_.set_modulation_offset(patch_.resonator_modulation_offset);

  // Process through resonator.
  resonator_.Process(bow_strength_buffer_, raw, center+2, NULL, kMaxBlockSize);

  for (size_t i=0; i<kMaxBlockSize; ++i) {
    center[i+2] = (center[i+2] + (strike_bleed * strike_buffer_[i]));
  }

#if defined(USE_LIMITER)
  limiter_.Process(2.f, center+2, kMaxBlockSize);
#endif

  for (size_t i=0; i<kMaxBlockSize; ++i) {
    yn[i*2] = lp_even(center[i], center[i+1], center[i+2]);
    yn[i*2+1] = lp_odd(center[i], center[i+1], center[i+2]);
  }
  center[0] = center[kMaxBlockSize];
  center[1] = center[kMaxBlockSize+1];
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
    int32_t frames_to_go = frames;
    while(frames_to_go > 0) {
        cycle(params, yn, (frames_to_go > kMaxBlockSize) ? \
              kMaxBlockSize : frames_to_go);
        frames_to_go -= 2 * kMaxBlockSize;
        yn += 2 * kMaxBlockSize;
    }
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  performance_state_.gate = true;
}
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  performance_state_.gate = false;
}

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0;

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

inline float get_shape() {
  return clip01f(shape + (p_values[k_user_osc_param_id6] == 0 ? shape_lfo : 0.0f));
}
inline float get_shift_shape() {
  return clip01f(shiftshape + (p_values[k_user_osc_param_id6] == 1 ? shape_lfo : 0.0f));
}
inline float get_strength() {
  return clip01f((p_values[k_user_osc_param_id1] * 0.01f) + (p_values[k_user_osc_param_id6] == 2 ? shape_lfo : 0.0f));
}
inline float get_mallet() {
  return clip01f((p_values[k_user_osc_param_id2] * 0.01f) + (p_values[k_user_osc_param_id6] == 3 ? shape_lfo : 0.0f));
}
inline float get_timbre() {
  return clip01f((p_values[k_user_osc_param_id3] * 0.01f) + (p_values[k_user_osc_param_id6] == 4 ? shape_lfo : 0.0f));
}
inline float get_damping() {
  return clip01f((p_values[k_user_osc_param_id4] * 0.01f) + (p_values[k_user_osc_param_id6] == 5 ? shape_lfo : 0.0f));
}
inline float get_brightness() {
  return clip01f((p_values[k_user_osc_param_id5] * 0.01f) + (p_values[k_user_osc_param_id6] == 6 ? shape_lfo : 0.0f));
}
