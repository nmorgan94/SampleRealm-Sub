#include "Parameters.h"

//==============================================================================
juce::AudioParameterFloatAttributes Parameters::withValueText (const ParameterSpecs::Spec& spec, juce::AudioParameterFloatAttributes attributes)
{
    if (&spec != &ParameterSpecs::oscMix)
        return attributes;

    // e.g. "70 / 30"
    return attributes
        .withStringFromValueFunction ([] (float mix, int)
        {
            const auto osc2Percent = juce::roundToInt (mix * 100.0f);
            return juce::String (100 - osc2Percent) + " / " + juce::String (osc2Percent);
        })
        .withValueFromStringFunction ([] (const juce::String& text)
        {
            return text.fromLastOccurrenceOf ("/", false, false).getFloatValue() / 100.0f;
        });
}
