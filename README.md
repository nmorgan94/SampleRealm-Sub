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
- A C++23-capable compiler:
  - **macOS**: Xcode or its Command Line Tools (the `xcode` preset needs the full Xcode app)
  - **Windows**: Visual Studio 2022 or later with the "Desktop development with C++" workload
- Git (for downloading JUCE)

macOS builds VST3, AU and Standalone. Windows builds VST3 and Standalone, as AU is macOS-only.

## Commands

Every command is the same on macOS and Windows, except the rows marked **macOS** or **Windows**.
Each workflow configures, builds and, for tests, runs in one go.

| Task | Command |
|---|---|
| Build the plugin (Debug) | `cmake --workflow --preset debug` |
| Build the plugin (Release) | `cmake --workflow --preset release` |
| Run the unit tests | `cmake --workflow --preset unit` |
| Run the unit tests against a Release build | `cmake --workflow --preset unit-release` |
| Run the unit tests and pluginval | `cmake --workflow --preset test` |
| Run all tests against a Release build | `cmake --workflow --preset test-release` |
| Launch the Standalone app (Debug) | `cmake --build --preset run` |
| Install the plugin for your DAW: **macOS** | Automatic on every build |
| Install the plugin for your DAW: **Windows** | `cmake --build --preset install-windows`, from an administrator prompt |
| Open in an IDE for debugging: **macOS** | `cmake --preset xcode`, then open `build-xcode/SubBass.xcodeproj` |
| Open in an IDE for debugging: **Windows** | Open `build-debug\SubBass.sln` in Visual Studio after a Debug workflow |

`run` and `install-windows` use the Debug build, so run `cmake --workflow --preset debug` once first.

The individual steps still work on their own, for example to pass a `-D` option:

```bash
cmake --preset debug -DPLUGINVAL_STRICTNESS=5   # configure
cmake --build --preset test                     # build
ctest --preset debug                            # test
```


## Unit Tests

Unit tests live in `Tests/` and use [Catch2](https://github.com/catchorg/Catch2). They build into a
`SubBass_Tests` executable that links the plugin's code and drives the processor directly, checking
parameter ranges and defaults, bus layouts, state save/restore, and that the output stays finite
across block sizes and parameter settings. Run them with the `unit`, `test` or `test-release`
workflow (see [Commands](#commands)).

Like pluginval, the tests are not part of a plain build: the `debug` workflow builds only the plugin.

Add a `.cpp` file to `Tests/` and reconfigure to pick it up. `Tests/TestHelpers.h` has helpers for
setting parameters and processing buffers. Set `-DENABLE_UNIT_TESTS=OFF` to skip them entirely.

## Validating the Plugin

[pluginval](https://github.com/Tracktion/pluginval) loads the built plugin as a host would and
tests it for stability. It is built from source on demand, so there is nothing to install.

```bash
cmake --workflow --preset test           # unit tests and pluginval, Debug
cmake --workflow --preset test-release   # unit tests and pluginval, Release
```

The `validate` target runs pluginval on its own, skipping the unit tests:

```bash
cmake --build --preset debug --target validate     # Debug
cmake --build --preset release --target validate   # Release: validate this before shipping
```

Logs land in `build-debug/pluginval-logs/`, or `build-release/pluginval-logs/` for a release run.
Strictness defaults to 10; use `-DPLUGINVAL_STRICTNESS=5` (range 1-10) for a faster run, or
`-DENABLE_PLUGINVAL=OFF` to skip pluginval entirely.

Only VST3 and AU are validated; the standalone is skipped. The AU test validates the installed
component in `~/Library/Audio/Plug-Ins/Components`, since macOS resolves Audio Units through its
registry rather than by path - so it needs `PLUGIN_COPY_AFTER_BUILD` left on. Steinberg's VST3
conformance validator is off by default, as it pulls ~300MB of SDK for one extra test; enable it
with `-DPLUGINVAL_VST3_VALIDATOR=ON`.

## CI

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) builds the Release plugin on macOS and Windows
and runs the unit tests. It only runs when started by hand: **Actions → CI → Run workflow** on
GitHub, or `gh workflow run ci.yml`. Tick **Also run pluginval** (or pass `-f pluginval=true`) to
validate the plugin too, which takes longer.

Each run's page lists the plugins under **Artifacts** for 30 days: `SubBass-macOS.tar.gz` (VST3 and
AU) and `SubBass-Windows.tar.gz` (VST3). They are unsigned.


## Debugging in Xcode

Generate and open the project with the `xcode` preset (see [Commands](#commands)), then:

1. Select your plugin target from the scheme dropdown
2. Go to **Product → Scheme → Edit Scheme**
3. Click **Run** on the left sidebar
4. Under **Executable**, choose **Other** and pick a host such as AudioPluginHost
5. Press **Cmd+R** to build and run, then load SubBass in the host
