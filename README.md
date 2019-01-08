Macro Oscillator 2 for Prologue
===============================

This is a port of some of Mutable Instruments Plaits oscillators to the Korg Prologue MultiEngine.

Oscillators
-----
| Name | Oscillator | `Shape` operation | `Shift-shape` operation |
|--|--|--|--|
| `va` | Pair of classic waveforms | Shape | Pulse width |
| `wsh` | Waveshaping oscillator | Amount | Waveform |
| `fm` | Two operator FM | Modulation index | Frequency ratio |
| `grn` | Granular formant oscillator | Frequency ratio | Formant frequency |
| `add` | Harmonic oscillator |Index of prominent harmonic  | Bump shape |
| `wta`-`wtf`* | Wavetable oscillator | Row index | Column index |

\* Due to the 32k size constraint in the MultiEngine the Wavetable oscillator is split into 6 oscillators with 4 'rows' each.

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
| 3          | `Parameter 2` |       |


For more information please read the excellent [Mutable Instruments Plaits documentation](https://mutable-instruments.net/modules/plaits/manual/).


Issues
----
* The Prologue Librarian tends to timeout when transferring the user oscillator, however the transfer is still complete. Try adding the user oscillator one at a time and _Send All_ / _Receive All_ for each oscillator.

Building
-------
* Checkout the repo (including subrepos)
* Follow the toolchain installation instructions in the `logue-sdk`
* Build with `make`

(only tested on MacOSX)

