Macro Oscillator 2 for Prologue
===============================

This is a port of some of Mutable Instruments Plaits oscillators to the Korg Prologue MultiEngine.

Oscillators
-----
| Name | Oscillator | 
|--|--|--|
| `va` | Pair of classic waveforms |
| `wsh` | Waveshaping oscillator | 
| `fm` | Two operator FM |
| `grn` | Granular formant oscillator |
| `add` | Harmonic oscillator |
| `wta`-`wtf`* | Wavetable oscillator | 

\* Due to the 32k size constraint in the MultiEngine the Wavetable oscillator is split into 6 oscillators with 4 'rows' each.

Common parameters
----
In the MultiEngine options if the Prologue you can find `Harmonics`, `Timbre` and `Morph` paramters for the oscillator (0-100%). These will do different things depending on the oscillator and is described in the [Mutable Instruments Plaits documentation](https://mutable-instruments.net/modules/plaits/manual/)

Plaits offer both `out` and `aux` outputs, and the `Out/Aux` parameter sets the mix between them.

Finally, `Shape Prm` sets which parameter the `Shape` knob and Shape LFO controls, and `ShiftS Prm` sets which parameters is controlled when pressing `Shift` and turning the `Shape` knob. 

| Value | Parameter |
|--|--|
| 1| Harmonics |
| 2| Timbre |
| 3| Morph |

Issues
----
* Oscillators can sometime use more CPU than available causing the voice to hang forcing a reboot of the Prologue
* The Prologue Librarian tends to timeout when transferring the user oscillator, however typically the transfer is still complete. Try adding the user oscillator one at a time and transfer 'Send All' after each.

Building
-------
* Checkout the repo (including subrepos)
* Follow the toolchain installation instructions in the `logue-sdk`
* Build with `make`

(only tested on MacOSX)

