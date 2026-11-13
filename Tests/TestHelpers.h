#pragma once

#include <PluginTestHelpers.h>

#include "PluginEditor.h"

//==============================================================================
namespace TestHelpers
{
    using ProcessorFixture = PreparedProcessor<AudioPluginAudioProcessor>;

    // The current parameters, restored as a session saved while a user preset showed,
    // with the properties PresetManager::addPresetInfoTo writes
    inline void restoreSessionShowing (AudioPluginAudioProcessor& processor, const juce::String& presetName, bool modified)
    {
        auto state = processor.getAPVTS().copyState();
        state.setProperty ("presetName", presetName, nullptr);
        state.setProperty ("presetCategory", "", nullptr);
        state.setProperty ("presetIsFactory", false, nullptr);
        state.setProperty ("presetModified", modified, nullptr);
        processor.getPresetManager().restoreState (state);
    }
}
