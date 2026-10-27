#pragma once

#include <algorithm>
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

    constexpr Spec integer (const char* id, const char* name, int min, int max, int defaultValue, const char* unit = "")
    {
        return { id, name, Kind::integer, static_cast<float> (min), static_cast<float> (max),
                 static_cast<float> (defaultValue), 1.0f, 1.0f, unit };
    }

    constexpr Spec choice (const char* id, const char* name, std::span<const char* const> choices, int defaultValue)
    {
        return { id, name, Kind::choice, 0.0f, static_cast<float> (choices.size() - 1),
                 static_cast<float> (defaultValue), 1.0f, 1.0f, "", choices };
    }

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
    inline constexpr Spec pitchBend          { "pitchBend", "Pitch Bend", Kind::real, -1.0f, 1.0f, 0.0f, 0.0001f };
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

    static_assert (std::ranges::all_of (all, [] (const Spec* spec)
    {
        if (spec->kind == Kind::real)
            return true;

        return spec->step == 1.0f
            && (spec->kind == Kind::integer
                || (spec->min == 0.0f && static_cast<size_t> (spec->max) == spec->choices.size() - 1));
    }), "Integer and choice specs need step 1, and a choice spec's range is 0 to its last choice");
}
