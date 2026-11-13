#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

// PresetManager's own behaviour is tested in SampleRealm-Modules; these are Sub's presets and its pitch bend

namespace
{
    // As Assets/Presets would hold it, setting only the master gain
    juce::MemoryBlock factoryPreset (const juce::Identifier& stateType, const juce::String& name, float gain)
    {
        juce::XmlElement preset ("PRESET");
        preset.setAttribute ("name", name);
        preset.setAttribute ("category", "");

        auto* param = preset.createNewChildElement (stateType.toString())->createNewChildElement ("PARAM");
        param->setAttribute ("id", Parameters::masterGainId.getParamID());
        param->setAttribute ("value", gain);

        const auto text = preset.toString();
        return { text.toRawUTF8(), text.getNumBytesAsUTF8() };
    }

    // The plugin's config with a test factory preset, and user presets in a folder of their own
    struct PresetFixture : ProcessorFixture
    {
        ~PresetFixture()   { userDirectory.deleteRecursively(); }

        juce::File userDirectory = juce::File::createTempFile ("presets");

        PresetManager presets { processor.getAPVTS(), [this]
        {
            auto config = AudioPluginAudioProcessor::createPresetConfig();
            config.userDirectory = userDirectory;
            config.factoryPresets = { factoryPreset (processor.getAPVTS().state.getType(), "Boost +3", 3.0f) };
            return config;
        }() };
    };
}

//==============================================================================
TEST_CASE_METHOD (PresetFixture, "Pitch bend doesn't mark the preset modified, but its range does", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));

    setParameter (processor, Parameters::pitchBendId, 0.5f);
    CHECK_FALSE (presets.isModified());

    setParameter (processor, Parameters::pitchBendRangeId, 12.0f);
    CHECK (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "A saved preset leaves out pitch bend, so loading it centres the bend", "[presets]")
{
    setParameter (processor, Parameters::pitchBendId, 0.5f);
    REQUIRE (presets.saveUserPreset ("Bent", {}).wasOk());

    const auto xml = juce::XmlDocument::parse (userDirectory.getChildFile ("Bent.sub"));
    REQUIRE (xml != nullptr);
    const auto* state = xml->getChildByName (processor.getAPVTS().state.getType());
    REQUIRE (state != nullptr);
    CHECK (state->getChildByAttribute ("id", Parameters::pitchBendId.getParamID()) == nullptr);
    CHECK (state->getChildByAttribute ("id", Parameters::pitchBendRangeId.getParamID()) != nullptr);

    REQUIRE (presets.loadPreset (presets.find ("Bent", false).value()));
    CHECK_THAT (getParameter (processor, Parameters::pitchBendId), WithinAbs (0.0, 1.0e-4));
}

TEST_CASE_METHOD (ProcessorFixture, "Every shipped factory preset sets every parameter but the performance controls in range, and loads unmodified", "[presets]")
{
    auto& presets = processor.getPresetManager();
    REQUIRE_FALSE (presets.getFactoryPresets().isEmpty());

    for (const auto& data : AudioPluginAudioProcessor::createPresetConfig().factoryPresets)
    {
        const auto xml = juce::parseXML (data.toString());
        REQUIRE (xml != nullptr);
        const auto* state = xml->getChildByName (processor.getAPVTS().state.getType());
        REQUIRE (state != nullptr);

        for (const auto* spec : ParameterSpecs::all)
        {
            INFO (xml->getStringAttribute ("name") << ": " << spec->id);
            const auto* param = state->getChildByAttribute ("id", spec->id);

            if (spec->isPerformanceControl)
            {
                CHECK (param == nullptr);
                continue;
            }

            REQUIRE (param != nullptr);
            const auto value = param->getStringAttribute ("value").getFloatValue();
            CHECK (value >= spec->min);
            CHECK (value <= spec->max);
        }
    }

    for (const auto& preset : presets.getFactoryPresets())
    {
        INFO (preset.name);
        CHECK (presets.loadPreset (preset));
        CHECK (presets.getCurrent().name == preset.name);
        CHECK_FALSE (presets.isModified());
    }
}
