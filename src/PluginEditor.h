#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LopasGateEditor : public juce::AudioProcessorEditor
{
public:
    explicit LopasGateEditor(LopasGateProcessor&);
    ~LopasGateEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LopasGateProcessor& proc;

    // Mode buttons (radio group)
    juce::TextButton lpBtn   { "LP" };
    juce::TextButton vcaBtn  { "VCA" };
    juce::TextButton comboBtn{ "COMBO" };

    juce::TextButton strikeBtn { "STRIKE" };

    // Knobs
    juce::Slider decayKnob    { juce::Slider::Rotary, juce::Slider::TextBoxBelow };
    juce::Slider speedKnob    { juce::Slider::Rotary, juce::Slider::TextBoxBelow };
    juce::Slider resonanceKnob{ juce::Slider::Rotary, juce::Slider::TextBoxBelow };
    juce::Slider levelKnob    { juce::Slider::Rotary, juce::Slider::TextBoxBelow };

    juce::Label decayLabel    { {}, "DECAY" };
    juce::Label speedLabel    { {}, "VAC SPEED" };
    juce::Label resonanceLabel{ {}, "RESONANCE" };
    juce::Label levelLabel    { {}, "LEVEL" };

    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach;

    // Hidden combo for mode parameter binding
    juce::ComboBox modeCombo;

    void updateModeButtons(int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LopasGateEditor)
};
