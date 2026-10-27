// The page side of these names is webui/src/plugin/juceBackend.ts
#include "WebViewEditor.h"
#include "../shared/ParameterSpecs.h"
#include "../shared/UiInfoJson.h"
#include "../ui/CustomLookAndFeel.h"
#include <WebUIData.h>

#if JUCE_MAC
 #import <Foundation/Foundation.h>
#endif

namespace
{
    constexpr int editorWidth = 700;
    constexpr int editorHeight = 640;
    constexpr int levelEventsHz = 30;

    const char* mimeTypeFor (const juce::String& fileName)
    {
        static const std::unordered_map<juce::String, const char*> types {
            { "html", "text/html" },
            { "js",   "text/javascript" },
            { "css",  "text/css" },
            { "ttf",  "font/ttf" },
            { "json", "application/json" }
        };

        const auto it = types.find (fileName.fromLastOccurrenceOf (".", false, false).toLowerCase());
        return it != types.end() ? it->second : "application/octet-stream";
    }

    std::vector<std::byte> readAll (juce::InputStream& stream)
    {
        std::vector<std::byte> bytes (static_cast<size_t> (stream.getTotalLength()));
        stream.read (bytes.data(), static_cast<int> (bytes.size()));
        return bytes;
    }

    struct SinglePageBrowser final : juce::WebBrowserComponent
    {
        using WebBrowserComponent::WebBrowserComponent;

        bool pageAboutToLoad (const juce::String& newURL) override
        {
            return newURL == getResourceProviderRoot();
        }
    };

    // The Mac web view is white until the page draws, and JUCE has no option for it
    void hideMacWebViewBackground ([[maybe_unused]] juce::WebBrowserComponent& browser)
    {
       #if JUCE_MAC
        for (auto* child : browser.getChildren())
            if (auto* host = dynamic_cast<juce::NSViewComponent*> (child))
                [(id) host->getView() setValue: @NO forKey: @"drawsBackground"];
       #endif
    }
}

//==============================================================================
WebViewEditor::WebViewEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      uiZipStream (WebUIData::SubWebUI_zip, static_cast<size_t> (WebUIData::SubWebUI_zipSize), false)
{
    for (const auto* spec : ParameterSpecs::all)
        relays.push_back (std::make_unique<juce::WebSliderRelay> (spec->id));

    browser = std::make_unique<SinglePageBrowser> (createOptions());
    hideMacWebViewBackground (*browser);

    auto& apvts = processorRef.getAPVTS();

    for (size_t i = 0; i < relays.size(); ++i)
        attachments.push_back (std::make_unique<juce::WebSliderParameterAttachment> (
            *apvts.getParameter (ParameterSpecs::all[i]->id), *relays[i], nullptr));

    addAndMakeVisible (*browser);
    browser->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setSize (editorWidth, editorHeight);
    startTimerHz (levelEventsHz);
}

juce::WebBrowserComponent::Options WebViewEditor::createOptions()
{
    auto options = juce::WebBrowserComponent::Options{}
                       .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
                       .withWinWebView2Options (juce::WebBrowserComponent::Options::WinWebView2{}
                                                    .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory))
                                                    .withBackgroundColour (CustomLookAndFeel::background))
                       .withNativeIntegrationEnabled()
                       .withResourceProvider ([this] (const auto& url) { return getResource (url); });

    for (auto& relay : relays)
        options = options.withOptionsFrom (*relay);

    auto& presets = processorRef.getPresetManager();

    return options
        .withInitialisationData ("uiInfo", juce::String (uiInfoJson (JucePlugin_VersionString)))
        .withNativeFunction ("presetList", [&presets] (const auto&, auto complete)
        {
            auto result = std::make_unique<juce::DynamicObject>();
            result->setProperty ("names", presets.getAllPresetNames());
            result->setProperty ("current", presets.getCurrentPresetName());
            complete (juce::var (result.release()));
        })
        .withNativeFunction ("presetLoad", [&presets] (const auto& args, auto complete)
        {
            complete (presets.loadPreset (args[0].toString()));
        })
        .withNativeFunction ("presetSave", [&presets] (const auto& args, auto complete)
        {
            complete (presets.savePreset (args[0].toString()));
        })
        .withNativeFunction ("presetDelete", [&presets] (const auto& args, auto complete)
        {
            complete (presets.deletePreset (args[0].toString()));
        });
}

std::optional<juce::WebBrowserComponent::Resource> WebViewEditor::getResource (const juce::String& url)
{
    const auto path = url == "/" ? juce::String ("index.html") : url.fromFirstOccurrenceOf ("/", false, false);

    if (auto* entry = uiZip.getEntry (path))
    {
        const std::unique_ptr<juce::InputStream> stream (uiZip.createStreamForEntry (*entry));

        if (stream != nullptr)
            return juce::WebBrowserComponent::Resource { readAll (*stream), mimeTypeFor (path) };
    }

    return std::nullopt;
}

void WebViewEditor::timerCallback()
{
    const auto peak = processorRef.getOutputLevel();
    const auto clipping = processorRef.isOutputClipping();

    // Silence only needs sending once
    if (peak == 0.0f && lastPeak == 0.0f && clipping == lastClipping)
        return;

    lastPeak = peak;
    lastClipping = clipping;

    auto level = std::make_unique<juce::DynamicObject>();
    level->setProperty ("peak", peak);
    level->setProperty ("clipping", clipping);
    browser->emitEventIfBrowserIsVisible ("level", juce::var (level.release()));
}

void WebViewEditor::paint (juce::Graphics& g)
{
    g.fillAll (CustomLookAndFeel::background);
}

void WebViewEditor::resized()
{
    browser->setBounds (getLocalBounds());
}
