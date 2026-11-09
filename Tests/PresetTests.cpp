#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestHelpers.h"
#include "ui/PresetActions.h"

using namespace TestHelpers;
using Catch::Matchers::WithinAbs;

namespace
{
    // As Assets/Presets would hold them, setting only the master gain
    juce::MemoryBlock factoryPreset (const juce::Identifier& stateType, const juce::String& name,
                                     const juce::String& category, float gain)
    {
        juce::XmlElement preset ("PRESET");
        preset.setAttribute ("name", name);
        preset.setAttribute ("category", category);

        auto* param = preset.createNewChildElement (stateType.toString())->createNewChildElement ("PARAM");
        param->setAttribute ("id", Parameters::masterGainId.getParamID());
        param->setAttribute ("value", gain);

        const auto text = preset.toString();
        return { text.toRawUTF8(), text.getNumBytesAsUTF8() };
    }

    // The plugin's config with test factory presets, and user presets in a folder of their own
    struct PresetFixture : ProcessorFixture
    {
        ~PresetFixture()   { userDirectory.deleteRecursively(); }

        juce::File userDirectory = juce::File::createTempFile ("presets");

        PresetManager presets { processor.getAPVTS(), [this]
        {
            const auto type = processor.getAPVTS().state.getType();

            auto config = AudioPluginAudioProcessor::createPresetConfig();
            config.userDirectory = userDirectory;
            config.factoryPresets = { factoryPreset (type, "Cut -24", "Cut", -24.0f),
                                      factoryPreset (type, "Boost +6", "Boost", 6.0f),
                                      factoryPreset (type, "Cut -6", "Cut", -6.0f),
                                      factoryPreset (type, "Boost +3", "Boost", 3.0f),
                                      factoryPreset (type, "Cut -12", "Cut", -12.0f) };
            return config;
        }() };

        float gain()   { return getParameter (processor, Parameters::masterGainId); }
    };

    juce::StringArray namesOf (const juce::Array<PresetManager::Preset>& list)
    {
        juce::StringArray names;

        for (const auto& preset : list)
            names.add (preset.name);

        return names;
    }

    const juce::StringArray factoryNames { "Boost +3", "Boost +6", "Cut -6", "Cut -12", "Cut -24" };
}

//==============================================================================
TEST_CASE_METHOD (PresetFixture, "Factory presets are listed by category, then name", "[presets]")
{
    CHECK (namesOf (presets.getFactoryPresets()) == factoryNames);
    CHECK (namesOf (presets.getPresets()) == factoryNames);

    for (const auto& preset : presets.getFactoryPresets())
    {
        CAPTURE (preset.name);
        CHECK (preset.isFactory);
        CHECK (preset.name.startsWith (preset.category));
    }
}

TEST_CASE_METHOD (PresetFixture, "A loaded preset sets its values and shows its name", "[presets]")
{
    CHECK (presets.getCurrent().name == "Init");

    REQUIRE (presets.loadPreset (presets.find ("Cut -12", true).value()));

    CHECK_THAT (gain(), WithinAbs (-12.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Cut -12");
    CHECK (presets.getCurrent().category == "Cut");
    CHECK (presets.getCurrent().isFactory);
    CHECK_FALSE (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Moving a parameter marks the preset modified until the next load", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));

    setParameter (processor, Parameters::masterGainId, -1.0f);
    CHECK (presets.isModified());

    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));
    CHECK_FALSE (presets.isModified());
    CHECK_THAT (gain(), WithinAbs (3.0, 1.0e-4));
}

TEST_CASE_METHOD (PresetFixture, "Pitch bend doesn't mark the preset modified, but its range does", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));

    setParameter (processor, Parameters::pitchBendId, 0.5f);
    CHECK_FALSE (presets.isModified());

    setParameter (processor, Parameters::pitchBendRangeId, 12.0f);
    CHECK (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Changing state outside the parameters marks the preset modified", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +3", true).value()));

    processor.updateHostDisplay (juce::AudioProcessor::ChangeDetails{}.withProgramChanged (true));
    CHECK_FALSE (presets.isModified());

    processor.updateHostDisplay (juce::AudioProcessor::ChangeDetails{}.withNonParameterStateChanged (true));
    CHECK (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Init restores the defaults", "[presets]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +6", true).value()));
    setParameter (processor, Parameters::masterGainId, 4.0f);
    setParameter (processor, Parameters::envAttackId, 1.5f);

    presets.loadInit();

    CHECK_THAT (gain(), WithinAbs (ParameterSpecs::masterGain.defaultValue, 1.0e-4));
    CHECK_THAT (getParameter (processor, Parameters::envAttackId), WithinAbs (ParameterSpecs::envAttack.defaultValue, 1.0e-4));
    CHECK (presets.getCurrent().name == "Init");
    CHECK_FALSE (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "Next and previous wrap, and start from either end after Init", "[presets]")
{
    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);

    REQUIRE (presets.loadPrevious());
    CHECK (presets.getCurrent().name == factoryNames[factoryNames.size() - 1]);

    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);

    presets.loadInit();
    REQUIRE (presets.loadPrevious());
    CHECK (presets.getCurrent().name == factoryNames[factoryNames.size() - 1]);
    CHECK_THAT (gain(), WithinAbs (-24.0, 1.0e-4));
}

TEST_CASE_METHOD (PresetFixture, "User presets save, rename and delete in their folder", "[presets]")
{
    setParameter (processor, Parameters::masterGainId, -3.0f);

    REQUIRE (presets.saveUserPreset ("Mine", "Cut").wasOk());
    const auto mine = userDirectory.getChildFile ("Mine.sub");
    CHECK (mine.existsAsFile());
    CHECK (presets.getCurrent().name == "Mine");
    CHECK_FALSE (presets.getCurrent().isFactory);
    CHECK_FALSE (presets.isModified());

    // Listed after the factory presets
    REQUIRE (presets.getPresets().getLast().file == mine);

    presets.loadInit();
    REQUIRE (presets.loadPreset (presets.getPresets().getLast()));
    CHECK_THAT (gain(), WithinAbs (-3.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Mine");

    REQUIRE (presets.renameUserPreset (presets.getPresets().getLast(), "Yours").wasOk());
    const auto yours = userDirectory.getChildFile ("Yours.sub");
    CHECK_FALSE (mine.exists());
    CHECK (yours.existsAsFile());
    CHECK (presets.getCurrent().name == "Yours");
    CHECK (namesOf (presets.getUserPresets()) == juce::StringArray { "Yours" });

    REQUIRE (presets.deleteUserPreset (presets.getPresets().getLast()).wasOk());
    CHECK_FALSE (yours.exists());
    CHECK (presets.getUserPresets().isEmpty());
    CHECK (presets.getCurrent().name == "Yours");
    CHECK (presets.isModified());
}

TEST_CASE_METHOD (PresetFixture, "User presets can't take a factory name", "[presets]")
{
    CHECK (presets.saveUserPreset ("boost +3", {}).failed());
    CHECK (presets.saveUserPreset ("   ", {}).failed());
    CHECK_FALSE (userDirectory.getChildFile ("boost +3.sub").exists());

    REQUIRE (presets.saveUserPreset ("Mine", {}).wasOk());
    CHECK (presets.renameUserPreset (presets.getUserPresets().getFirst(), "Cut -6").failed());
    CHECK (userDirectory.getChildFile ("Mine.sub").existsAsFile());
}

TEST_CASE_METHOD (PresetFixture, "Next and previous skip a user preset deleted outside the plugin", "[presets]")
{
    REQUIRE (presets.saveUserPreset ("A", {}).wasOk());
    REQUIRE (presets.saveUserPreset ("B", {}).wasOk());
    REQUIRE (presets.loadPreset (presets.find ("A", false).value()));

    REQUIRE (userDirectory.getChildFile ("B.sub").deleteFile());

    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);
}

TEST_CASE_METHOD (PresetFixture, "Next and previous step past a preset that won't load", "[presets]")
{
    REQUIRE (presets.saveUserPreset ("A", {}).wasOk());
    REQUIRE (userDirectory.getChildFile ("Broken.sub").replaceWithText ("not a preset"));

    REQUIRE (presets.loadNext());
    CHECK (presets.getCurrent().name == factoryNames[0]);

    REQUIRE (presets.loadPrevious());
    CHECK (presets.getCurrent().name == "A");
}

TEST_CASE_METHOD (PresetFixture, "Choosing a preset that won't load says so, and keeps the current one", "[presets][ui]")
{
    OverlayFixture overlay;
    PromptOverlay prompt;
    overlay.parent.addChildComponent (prompt);
    PresetActions actions { presets, prompt };

    REQUIRE (presets.saveUserPreset ("A", {}).wasOk());
    REQUIRE (userDirectory.getChildFile ("Broken.sub").replaceWithText ("not a preset"));
    presets.refreshUserPresets();

    actions.load (presets.find ("A", false).value());
    CHECK_FALSE (prompt.isVisible());

    actions.load (presets.find ("Broken", false).value());
    CHECK (prompt.isVisible());
    CHECK (presets.getCurrent().name == "A");
}

TEST_CASE_METHOD (PresetFixture, "Rescanning moves the change counter only when the folder changed", "[presets]")
{
    presets.refreshUserPresets();
    const auto before = presets.getChangeCounter();

    presets.refreshUserPresets();
    CHECK (presets.getChangeCounter() == before);

    REQUIRE (userDirectory.createDirectory().wasOk());
    REQUIRE (userDirectory.getChildFile ("Mine.sub").replaceWithText ("<PRESET/>"));
    presets.refreshUserPresets();
    CHECK (presets.getChangeCounter() != before);
}

TEST_CASE_METHOD (PresetFixture, "Presets are found by name and kind", "[presets]")
{
    REQUIRE (presets.saveUserPreset ("Mine", {}).wasOk());

    CHECK (presets.find ("Cut -6", true).has_value());
    CHECK (presets.find ("Mine", false)->file == userDirectory.getChildFile ("Mine.sub"));
    CHECK_FALSE (presets.find ("Cut -6", false).has_value());
    CHECK_FALSE (presets.find ("Mine", true).has_value());
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

TEST_CASE_METHOD (PresetFixture, "State from before presets existed shows Init, modified", "[presets][state]")
{
    REQUIRE (presets.loadPreset (presets.find ("Boost +6", true).value()));

    juce::ValueTree state (processor.getAPVTS().state.getType());
    state.appendChild ({ "PARAM", { { "id", Parameters::masterGainId.getParamID() }, { "value", -20.0f } } }, nullptr);
    presets.restoreState (state);

    CHECK_THAT (gain(), WithinAbs (-20.0, 1.0e-4));
    CHECK (presets.getCurrent().name == "Init");
    CHECK (presets.isModified());
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
