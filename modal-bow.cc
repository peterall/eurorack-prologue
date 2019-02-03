#include "userosc.h"
#include "stmlib/dsp/dsp.h"
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
inline float get_envelope();

MultistageEnvelope envelope_;
Exciter bow_;
//Diffuser diffuser_;
Resonator resonator_;
//stmlib::DCBlocker dc_blocker_;

//float diffuser_buffer_[1024];

bool previous_gate_ = false;
float exciter_level_ = 0.f;
float strength_ = 0.f;
float envelope_value_ = 0.f;

float bow_buffer_[kMaxBlockSize];
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

void Seed() {
  uint32_t signature = stmlib::Random::GetWord();
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
  stmlib::Random::Seed(_osc_mcu_hash());
  Seed();
  envelope_.Init();
  bow_.Init();
  //diffuser_.Init(diffuser_buffer_);

  bow_.set_model(EXCITER_MODEL_FLOW);
  bow_.set_parameter(0.7f);
  bow_.set_timbre(0.5f);

  envelope_.set_adsr(0.5f, 0.5f, 0.5f, 0.5f);

  resonator_.Init();

//  dc_blocker_.Init(1.0f - 10.0f / kSampleRate);
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

uint32_t clock_divider = 0;

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
  static uint8_t flags;
  static float frequency;

  if((clock_divider & 1) == 0) {
    shape_lfo = q31_to_f32(params->shape_lfo);

    performance_state_.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
    int32_t pitch = static_cast<int32_t>((performance_state_.note + 41.0f) * 256.0f);
    if (pitch < 0) {
      pitch = 0;
    } else if (pitch >= 65535) {
      pitch = 65535;
    }
    frequency = lut_midi_to_f_high[pitch >> 8] * lut_midi_to_f_low[pitch & 0xff];
    flags = GetGateFlags(performance_state_.gate);

    //patch_.exciter_envelope_shape = 1.0f;
    patch_.exciter_bow_level = get_strength();
    patch_.exciter_envelope_shape = get_envelope();
    patch_.exciter_bow_timbre = get_timbre();
    //patch_.exciter_signature = 0.0f;
    patch_.resonator_damping = get_damping();
    patch_.resonator_brightness = get_brightness();
    patch_.resonator_geometry = get_shift_shape();
    patch_.resonator_position = get_shape();
    
    // Compute the envelope.
    float envelope_gain = 1.0f;
    if (patch_.exciter_envelope_shape < 0.4f) {
      float a = patch_.exciter_envelope_shape * 0.75f + 0.15f;
      float dr = a * 1.8f;
      envelope_.set_adsr(a, dr, 0.0f, dr);
      envelope_gain = 5.0f - patch_.exciter_envelope_shape * 10.0f;
    } else if (patch_.exciter_envelope_shape < 0.6f) {
      float s = (patch_.exciter_envelope_shape - 0.4f) * 5.0f;
      envelope_.set_adsr(0.45f, 0.81f, s, 0.81f);
    } else {
      float a = (1.0f - patch_.exciter_envelope_shape) * 0.75f + 0.15f;
      float dr = a * 1.8f;
      envelope_.set_adsr(a, dr, 1.0f, dr);
    }
    float envelope_value = envelope_.Process(flags) * envelope_gain;
    float envelope_increment = (envelope_value - envelope_value_) / kMaxBlockSize;
    
  // Configure and evaluate exciters.
    float brightness_factor = 0.4f + 0.6f * patch_.resonator_brightness;
    bow_.set_timbre(patch_.exciter_bow_timbre * brightness_factor);
    
  }
  bow_.Process(flags, bow_buffer_, kMaxBlockSize);


  // The strength parameter is very sensitive to zipper noise.
 // float strength = patch_.exciter_bow_level * 256.0f;
 // float strength_increment = (strength - strength_) / kMaxBlockSize;
  
  // Sum all sources of excitation.
  /*
  for (size_t i = 0; i < kMaxBlockSize; ++i) {
    strength_ += strength_increment;
    envelope_value_ += envelope_increment;
    float e = envelope_value_;
    float strength_lut = strength_;
    MAKE_INTEGRAL_FRACTIONAL(strength_lut);
    float accent = lut_accent_gain_coarse[strength_lut_integral] *
       lut_accent_gain_fine[
           static_cast<int32_t>(256.0f * strength_lut_fractional)];

    bow_strength_buffer_[i] = e * patch_.exciter_bow_level;

    raw[i] = bow_buffer_[i] * bow_strength_buffer_[i] * 0.125f * accent;
 }
*/
  
    // Some exciters can cause palm mutes on release.
  if((clock_divider & 1) == 0) {
    float damping = patch_.resonator_damping;
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
  }
  // Process through resonator.
  resonator_.Process(bow_strength_buffer_, bow_buffer_, center+2, NULL, kMaxBlockSize);

  for (size_t i=0; i<kMaxBlockSize; ++i) {
    yn[i*2] = lp_even(center[i], center[i+1], center[i+2]);
    yn[i*2+1] = lp_odd(center[i], center[i+1], center[i+2]);
  }
  center[0] = center[kMaxBlockSize];
  center[1] = center[kMaxBlockSize+1];

  clock_divider++;
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

inline float get_shape() {
  return clip01f(shape + (p_values[k_osc_param_id6] == 0 ? shape_lfo : 0.0f));
}
inline float get_shift_shape() {
  return clip01f(shiftshape + (p_values[k_osc_param_id6] == 1 ? shape_lfo : 0.0f));
}
inline float get_strength() {
  return clip01f((p_values[k_osc_param_id1] * 0.01f) + (p_values[k_osc_param_id6] == 2 ? shape_lfo : 0.0f));
}
inline float get_envelope() {
  return clip01f((p_values[k_osc_param_id2] * 0.01f) + (p_values[k_osc_param_id6] == 3 ? shape_lfo : 0.0f));
}
inline float get_timbre() {
  return clip01f((p_values[k_osc_param_id3] * 0.01f) + (p_values[k_osc_param_id6] == 4 ? shape_lfo : 0.0f));
}
inline float get_damping() {
  return clip01f((p_values[k_osc_param_id4] * 0.01f) + (p_values[k_osc_param_id6] == 5 ? shape_lfo : 0.0f));
}
inline float get_brightness() {
  return clip01f((p_values[k_osc_param_id5] * 0.01f) + (p_values[k_osc_param_id6] == 6 ? shape_lfo : 0.0f));
}
