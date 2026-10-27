#pragma once

#include <array>
#include <span>

//==============================================================================
namespace ParameterSpecs
{
    enum class Kind { real, integer, choice };

    struct Spec
    {
        const char* id;
        const char* name;
        Kind kind;
        float min, max, defaultValue;
        float step = 0.0f;
        float skew = 1.0f;
        const char* unit = "";
        std::span<const char* const> choices = {};
    };

    inline constexpr const char* glideModes[] { "Time", "Rate" };

    inline constexpr Spec masterGain         { "masterGain", "Master Gain", Kind::real, -60.0f, 6.0f, 0.0f, 0.1f, 1.0f, "dB" };
    inline constexpr Spec oscOctave          { "oscOctave", "Octave", Kind::integer, -4.0f, 0.0f, -2.0f, 1.0f, 1.0f, "oct" };
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
    inline constexpr Spec glideMode          { "glideMode", "Glide Mode", Kind::choice, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, "", glideModes };
    inline constexpr Spec pitchBend          { "pitchBend", "Pitch Bend", Kind::real, -1.0f, 1.0f, 0.0f, 0.0001f };
    inline constexpr Spec pitchBendRange     { "pitchBendRange", "Bend Range", Kind::integer, 0.0f, 24.0f, 2.0f, 1.0f, 1.0f, "st" };

    // The host's parameter order
    inline constexpr std::array all {
        &masterGain, &oscOctave, &osc1Fine, &osc2Fine, &oscMix,
        &envAttack, &envDecay, &envSustain, &envRelease,
        &envAttackCurve, &envDecayCurve, &envReleaseCurve,
        &saturationDrive, &saturationDriveEnv,
        &glideTime, &glideMode,
        &pitchBend, &pitchBendRange
    };
}
