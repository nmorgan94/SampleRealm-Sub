#pragma once

#include <samplerealm/ParameterSpec.h>

//==============================================================================
namespace ParameterSpecs
{
    inline constexpr const char* glideModes[] { "Time", "Rate" };

    inline constexpr Spec masterGain         { "masterGain", "Master Gain", Kind::real, -60.0f, 6.0f, 0.0f, 0.1f, 1.0f, "dB" };
    inline constexpr Spec oscOctave          = integer ("oscOctave", "Octave", -4, 0, -2, "oct");
    inline constexpr Spec osc1Fine           { "osc1Fine", "Osc 1 Fine", Kind::real, -50.0f, 50.0f, 0.0f, 0.1f, 1.0f, "ct" };
    inline constexpr Spec osc2Fine           { "osc2Fine", "Osc 2 Fine", Kind::real, -50.0f, 50.0f, 0.0f, 0.1f, 1.0f, "ct" };
    inline constexpr Spec oscMix             { "oscMix", "Osc Mix", Kind::real, 0.0f, 1.0f, 0.5f, 0.001f };
    inline constexpr Spec envAttack          { "envAttack", "Attack", Kind::real, 0.001f, 5.0f, 0.005f, 0.001f, 0.3f, "s" };
    inline constexpr Spec envDecay           { "envDecay", "Decay", Kind::real, 0.001f, 5.0f, 0.1f, 0.001f, 0.3f, "s" };
    inline constexpr Spec envSustain         { "envSustain", "Sustain", Kind::real, 0.0f, 1.0f, 0.8f, 0.001f };
    inline constexpr Spec envRelease         { "envRelease", "Release", Kind::real, 0.001f, 5.0f, 0.2f, 0.001f, 0.3f, "s" };
    inline constexpr Spec envAttackCurve     { "envAttackCurve", "Attack Curve", Kind::real, -1.0f, 1.0f, 0.0f, 0.001f };
    inline constexpr Spec envDecayCurve      { "envDecayCurve", "Decay Curve", Kind::real, -1.0f, 1.0f, 0.0f, 0.001f };
    inline constexpr Spec envReleaseCurve    { "envReleaseCurve", "Release Curve", Kind::real, -1.0f, 1.0f, 0.0f, 0.001f };
    inline constexpr Spec saturationDrive    { "saturationDrive", "Drive", Kind::real, 0.0f, 1.0f, 0.2f, 0.001f };
    inline constexpr Spec saturationDriveEnv { "saturationDriveEnv", "Drive Env", Kind::real, -1.0f, 1.0f, 0.0f, 0.001f };
    inline constexpr Spec glideTime          { "glideTime", "Glide", Kind::real, 0.0f, 1.0f, 0.0f, 0.001f, 0.3f, "s" };
    inline constexpr Spec glideMode          = choice ("glideMode", "Glide Mode", glideModes, 1);
    inline constexpr Spec pitchBend          = performanceControl ({ "pitchBend", "Pitch Bend", Kind::real, -1.0f, 1.0f, 0.0f, 0.0001f });
    inline constexpr Spec pitchBendRange     = integer ("pitchBendRange", "Bend Range", 0, 24, 2, "st");

    // The host's parameter order
    inline constexpr std::array all {
        &masterGain, &oscOctave, &osc1Fine, &osc2Fine, &oscMix,
        &envAttack, &envDecay, &envSustain, &envRelease,
        &envAttackCurve, &envDecayCurve, &envReleaseCurve,
        &saturationDrive, &saturationDriveEnv,
        &glideTime, &glideMode,
        &pitchBend, &pitchBendRange
    };

    static_assert (areValid (all));
}
