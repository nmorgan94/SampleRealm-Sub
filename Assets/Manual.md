# SampleRealm: SUB

A monophonic sub-bass synth with two sine oscillators. The most recent held note sounds.

## Oscillator

- OCTAVE shifts both oscillators down by 0 to 4 octaves.
- OSC 1 FINE and OSC 2 FINE tune each oscillator by up to 50 cents either way.
- OSC MIX crossfades between the two. Its value reads as the Osc 1 / Osc 2 split, like 70 / 30.

## Envelope

- ATTACK, DECAY and RELEASE run from 1 ms to 5 seconds. SUSTAIN is the level held while the note is down.
- The small knob beside Attack, Decay and Release bends that stage's curve. Double-click it to make it straight again.
- A note played while another is held changes pitch without restarting the envelope.

## Tone & Pitch

- DRIVE adds harmonics without making the sound louder.
- DRIVE ENV lets the envelope move the drive. Above 0 the drive falls away as the note decays, and below 0 it builds up. At Drive 0 the note stays clean.
- GLIDE slides between overlapping notes, up to 1 second. Notes played apart jump straight to pitch.
- RATE makes the glide time the time for an octave, so smaller steps slide faster. TIME makes every slide take the same time.
- The pitch wheel follows your MIDI pitch wheel, and you can drag it too. Drag the RANGE box to set the bend range, up to 24 semitones, and double-click it to reset.

## Master

The MASTER knob sets the output level, from -60 dB to +6 dB. The meter beside it shows the output peak,
and its light turns on when the output goes over 0 dB.

## Presets

- The arrows step through the presets, and the name opens the full list.
- A * after the name means the sound has changed since the preset was loaded. Moving the pitch wheel doesn't count.
- Save overwrites the current user preset. For a factory preset it asks for a name.
- The ... menu has Init Preset, Save As, Rename and Delete. In the plugin it also has Show Preset Folder.
- Factory presets can't be renamed, deleted or overwritten.
