Eurorack Oscillators for Korg prologue, minilogue xd and Nu:tekt NTS-1
=================================

Ports of some of Mutable Instruments (tm) oscillators to the Korg "logue" multi-engine.

See [releases](https://github.com/peterall/eurorack-prologue/releases) for latest binaries.


Macro Oscillator 2 (based on Plaits)
====
-----
| Name | Oscillator | `Shape` operation | `Shift-shape` operation |
|--|--|--|--|
| `mo2_va` | Pair of classic waveforms | Shape | Pulse width |
| `mo2_wsh` | Waveshaping oscillator | Amount | Waveform |
| `mo2_fm` | Two operator FM | Modulation index | Frequency ratio |
| `mo2_grn` | Granular formant oscillator | Frequency ratio | Formant frequency |
| `mo2_add` | Harmonic oscillator |Index of prominent harmonic  | Bump shape |
| `mo2_wta`* | Wavetable: Additive (2x sine, quadratic, comb) | Row index | Column index |
| `mo2_wtb`* | Wavetable: Additive (pair, triangle stack, 2x drawbar) | Row index | Column index |
| `mo2_wtc`* | Wavetable: Formantish (trisaw, sawtri, burst, bandpass formant) | Row index | Column index |
| `mo2_wtd`* | Wavetable: Formantish (formant, digi_formant, pulse, sine power) | Row index | Column index |
| `mo2_wte`* | Wavetable: Braids (male, choir, digi, drone) | Row index | Column index |
| `mo2_wtf`* | Wavetable: Braids (metal, fant, 2x unknown) | Row index | Column index |
| `mo2_string` | Inharmonic string model | Decay | Brightness |

\* Due to the 32k size constraint in the multi-engine the Wavetable oscillator is split into 6 oscillators of 8 rows (scannable by Shape) by 4 columns (scannable by Shift-shape)

Parameters
----
In the Multi-engine menu you can find additional parameters for the oscillators.

`Parameter 1` is oscillator specific and controls whichever parameter is not mapped to `Shape` or `Shift-shape`.

`Parameter 2` sets the mix between the oscillator `out` and `aux`.

`LFO Target` sets the Shape LFO target, see list below.

`LFO2 Rate` is rate of LFO2.

`LFO2 Int` is intensity (depth) of LFO2.

`LFO2 Target` sets the target for LFO2 according to the list below.

LFO2
---

The oscillator has a built-in additional cosine key-synced LFO which can module an internal parameter (set with the `LFO2 Target` parameter in the oscillator menu):

| LFO Target | Parameter     | Notes |
|------------|---------------|-------|
| 1          | `Shape`       |       |
| 2          | `Shift-shape` |       |
| 3          | `Parameter 1` | Not implemented for Wavetable oscillator |
| 4          | `Parameter 2` |       |
| 5          | `Pitch` |       |
| 6          | _reserved_ (Amplitude?) |       |
| 7          | `LFO2 Rate` |       |
| 8          | `LFO2 Int` |       |


For more information please read the excellent [Mutable Instruments Plaits documentation](https://mutable-instruments.net/modules/plaits/manual/).

Tips
---
Many parameters 'neutral' settings are in center position, such as `va` Detune or `fm` Feedback, however the prologue defaults all parameters to the lowest value, hence get used to going into the menus and set the first parameter to 50% when instantiating the oscillator.

Modal Resonator (based on Elements)
====
*Physical modeling synthesis*

| Name | Oscillator |
|--|--|
| `mod s` | Strike exciter with modal resonator |

Parameters
----

| Parameter               | Parameter             | LFO Target | Notes |
|-------------------------|-----------------------|------------|-------|
| `Shape` knob            | Resonator position    | 1 | Position where the mallet strikes, has a comb-filtering effect.  |
| `Shift` + `Shape` knob  | Resonator geometry    | 2 | Geometry and stiffness of resonator. Set to 25-30% for a nice tuned sound. |
| `Strength` menu         | Strike strength       | 3 | Mallet strength, high values causes the strike to bleed into the resonator output. |
| `Mallet` menu               | Strike mallet         | 4 | Type of mallet, over 70% is bouncing particles. |
| `Timbre` menu                | Strike timbre         | 5 | Brightness/speed of the excitation. |
| `Damping` menu              | Resonator damping     | 6 | The rate of energy dissipation in the resonator. High values cause long release effect. |
| `Brightness` menu           | Resonator brightness  | 7 | Muting of high frequencies |
| `LFO Target` menu           | multi-engine `Shape` LFO target |  | Sets which parameter is modulated by the `Shape` LFO (see LFO Target column)      |


For more information please read the excellent [Mutable Instruments Elements documentation](https://mutable-instruments.net/modules/elements/manual/).

Limitations
-----
Due to compute and memory (32K!) limitations in the prologue multi-engine quite a few short-cuts had to be taken:

* Only the Strike exciter is used
* Sample-player and Granular sample-player mallet-modes did not fit in memory
* Resonator filter bank is reduced to 24+2 filters from 52+8
* Resonator filters are recomputed one per block instead all-ish every block
* Samplerate is 24KHz vs 32KHz in Elements

*Sounds pretty great IMO but go buy Elements for the real experience!*

Tips
---
*When you first select the oscillator it will make no sound, all parameters are at 0%!* Increase the `Strength` and `Damping` parameters until you start hearing something.

Try a nice pluck:

| Parameter           |  Value |
|---------------------|--------|
| `Shape`             | 50%    |
| `Shift` + `Shape`   | 30%    |
| `Strength`          | 90%    |
| `Mallet`            | 45%    |
| `Timbre`            | 45%    |
| `Damping`           | 70%    |
| `Brightness`        | 45%    |

Issues
====

* The prologue Sound Librarian tends to timeout when transferring the user oscillator, however the transfer is still complete. Try adding the user oscillator one at a time and _Send All_ / _Receive All_ for each oscillator.

* There's been [many reports](https://github.com/peterall/eurorack-prologue/issues/2) that the Modal Resonator oscillator doesn't produce any sound. I've included a few versions which lower CPU usage which may yield better results. On my prologue there's been cases where I've had issues after a factory-reset where the oscillator wouldn't produce sound. Installing _sequentially in the same oscillator slot_ the lightest CPU version `osc_modal_strike_16_nolimit` (16 filters and removed limiter), followed by `osc_modal_strike_24_nolimit` followed by `osc_modal_strike` resolved the issue for me. _Your milage may vary_.

* When first selecting the oscillator in the multi-engine, all values default to their minimum values, however the display seems to default to 0. For bipolar values it means the display might still show 0% while internally in the oscillator the value is -100%.

Building
====

* Checkout the repo (including subrepos)
* Follow the toolchain installation instructions in the `logue-sdk`
* Make sure you have the `jq` tool installed (`brew install jq`)
* Build with `make`

(only tested on MacOSX)

Acknowledgements
====
*All credit to Emilie Gillet for her amazing modules!*
