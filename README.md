Eurorack Oscillators for Korg prologue
=================================

Ports of some of Mutable Instruments (tm) oscillators to the Korg prologue multi-engine.

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
| `mo2_wta`-`mo2_wtf`* | Wavetable oscillator | Row index | Column index |

\* Due to the 32k size constraint in the multi-engine the Wavetable oscillator is split into 6 oscillators with 4 'rows' each.

Parameters
----
In the Multi-engine menu you can find additional parameters for the oscillators.

`Parameter 1` is oscillator specific and controls whichever parameter is not mapped to `Shape` or `Shift-shape`

`Parameter 2` sets the mix between the oscillator `out` and `aux`.

`Parameter 3` sets the Shape LFO target according to the table below:

| LFO Target | Parameter     | Notes |
|------------|---------------|-------|
| 1          | `Shape`       |       |
| 2          | `Shift-shape` |       |
| 3          | `Parameter 1` | Not implemented for Wavetable oscillator |
| 4          | `Parameter 2` |       |


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

Common
====

Issues
----
* The prologue Sound Librarian tends to timeout when transferring the user oscillator, however the transfer is still complete. Try adding the user oscillator one at a time and _Send All_ / _Receive All_ for each oscillator.

Building
-------
* Checkout the repo (including subrepos)
* Follow the toolchain installation instructions in the `logue-sdk`
* Build with `make`

(only tested on MacOSX)

Acknowledgements
-------
*All credit to Emilie Gillet for her amazing modules!*
