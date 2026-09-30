# SampleRealm: Sub-bass

A monophonic sub-bass synthesizer

## Features

### Oscillators
- 2 sine oscillators with independent fine tuning
- Shared octave offset, −4 to 0 octaves
- Fine pitch, ±50 cents per oscillator
- Crossfader between the two fine knobs, reading as an Osc 1 / Osc 2 percentage split (e.g. `70 / 30`)

### Voice
- Monophonic with a held-note stack - the most recent note sounds
- Legato retriggering: the envelope restarts only on the first note of a phrase, so overlapping notes change pitch without a new attack
- Sample-accurate note timing, MIDI events split within the block

### Glide
- Slides only between overlapping notes; releasing between notes jumps straight to pitch
- Glide time 0 to 1 second, skewed for finer control at short times
- Mode switch under the knob:
  - **Rate** (default) - the time is for a one-octave slide, so smaller intervals slide proportionally faster
  - **Time** - every slide takes the set time, whatever the interval

### Pitch Wheel
- Follows MIDI pitch bend, and the on-screen wheel moves with it
- Drag the wheel to bend; it springs back to centre on release
- Bend range 0 to 24 semitones (default ±2), set by dragging the **RANGE** box; double-click to reset
- Bend is a parameter, so it can be automated and is saved with presets

### Envelope
- ADSR with attack, decay, and release from 1 ms to 5 seconds
- Skewed ranges for finer control at short times
- Attack, decay, and release each have a curve knob, −1 to +1; double-click to reset to linear

### Saturation
- tanh soft-clip drive, applied pre-envelope
- Gain-normalized, so raising drive adds harmonics without raising level

#### Drive Env

Drive Env scales Drive rather than offsetting it, which keeps Drive a ceiling -
at Drive 0 the note stays clean whatever the amount is set to:

| Amount | As the note decays  |
| ------ | ------------------  | 
| `+1`   | falls to clean      |
| `0`    | full Drive          |
| `−1`   | rises to full Drive |

Drive Env follows the envelope's curves, not a straight version of them.

### Output
- Master gain, −60 to +6 dB


## Build Requirements

- CMake 3.25+
- A C++23-capable compiler
- Git
- macOS development environment for AU/Standalone/VST3 builds

## Building

**Debug Build:**
```bash
cmake --preset debug
cmake --build --preset debug
```

**Release Build:**
```bash
cmake --preset release
cmake --build --preset release
```

Run the standalone:

```bash
open build-debug/SubBass_artefacts/Debug/Standalone/SubBass.app
```


## Validating the Plugin

[pluginval](https://github.com/Tracktion/pluginval) loads the built plugin as a host would and
tests it for stability. It is built from source on demand, so there is nothing to install.

**Debug:**
```bash
cmake --build --preset debug --target validate     # builds, then validates
ctest --preset debug                               # validates an existing build
```

**Release:**
```bash
cmake --build --preset release --target validate
ctest --preset release
```

Logs land in `build-debug/pluginval-logs/`, or `build-release/pluginval-logs/` for a release run.
Strictness defaults to 10; use `-DPLUGINVAL_STRICTNESS=5` (range 1-10) for a faster run, or
`-DENABLE_PLUGINVAL=OFF` to skip pluginval entirely.

Only VST3 and AU are validated; the standalone is skipped. The AU test validates the installed
component in `~/Library/Audio/Plug-Ins/Components`, since macOS resolves Audio Units through its
registry rather than by path - so it needs `COPY_PLUGIN_AFTER_BUILD` left on. Steinberg's VST3
conformance validator is off by default, as it pulls ~300MB of SDK for one extra test; enable it
with `-DPLUGINVAL_VST3_VALIDATOR=ON`.


## Debugging in Xcode

### 1. Generate Xcode Project

```bash
cmake -B build-xcode -G Xcode
open build-xcode/SubBass.xcodeproj
```

### 2. Configure Debugging

1. Select your plugin target from the scheme dropdown
2. Go to **Product → Scheme → Edit Scheme**
3. Click **Run** on the left sidebar
4. Under **Executable**, choose **Other** and navigate to executable.

### 3. Build and Run

1. Press **Cmd+B** to build the plugin
2. Press **Cmd+R** to run with AudioPluginHost
3. Load your plugin in AudioPluginHost
