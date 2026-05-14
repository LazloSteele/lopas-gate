#include "PluginEditor.h"

static const juce::Colour kBg       { 0xFF121417 };
static const juce::Colour kPanel    { 0xFF1E2228 };
static const juce::Colour kAccent   { 0xFFFF9F1C };
static const juce::Colour kText     { 0xFFDDDDDD };
static const juce::Colour kSubtext  { 0xFF888888 };
static const juce::Colour kBtnActive{ 0xFF3A4652 };

LopasGateEditor::LopasGateEditor(LopasGateProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(420, 240);

    // Hidden combo box for mode APVTS binding
    modeCombo.addItem("LP",    1);
    modeCombo.addItem("VCA",   2);
    modeCombo.addItem("COMBO", 3);
    modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.apvts, "mode", modeCombo);

    // Mode buttons
    auto setupModeBtn = [&](juce::TextButton& btn, int idx)
    {
        addAndMakeVisible(btn);
        btn.setClickingTogglesState(false);
        btn.onClick = [this, idx, &btn]
        {
            modeCombo.setSelectedItemIndex(idx, juce::sendNotification);
            updateModeButtons(idx);
        };
    };
    setupModeBtn(lpBtn,    0);
    setupModeBtn(vcaBtn,   1);
    setupModeBtn(comboBtn, 2);

    // Keep buttons in sync when param changes externally
    modeCombo.onChange = [this]
    {
        updateModeButtons(modeCombo.getSelectedItemIndex());
    };
    updateModeButtons(modeCombo.getSelectedItemIndex());

    // Strike button
    addAndMakeVisible(strikeBtn);
    strikeBtn.onClick = [this] { proc.strikeRequested.store(true); };

    // Knobs
    auto setupKnob = [&](juce::Slider& knob, juce::Label& label, const juce::String& paramId)
    {
        addAndMakeVisible(knob);
        addAndMakeVisible(label);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(10.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, kSubtext);
        knob.setColour(juce::Slider::thumbColourId,              kAccent);
        knob.setColour(juce::Slider::trackColourId,              kAccent);
        knob.setColour(juce::Slider::backgroundColourId,         kBtnActive);
        knob.setColour(juce::Slider::textBoxTextColourId,        kText);
        knob.setColour(juce::Slider::textBoxBackgroundColourId,  kBg);
        knob.setColour(juce::Slider::textBoxOutlineColourId,     juce::Colours::transparentBlack);
        return std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            proc.apvts, paramId, knob);
    };

    decayAttach     = setupKnob(decayKnob,     decayLabel,     "decay");
    resonanceAttach = setupKnob(resonanceKnob, resonanceLabel, "resonance");
    levelAttach     = setupKnob(levelKnob,     levelLabel,     "level");

    // Speed knob: snaps to 0/1/2 for Slow/Med/Fast
    speedAttach = setupKnob(speedKnob, speedLabel, "vacSpeed");
    speedKnob.setRange(0.0, 2.0, 1.0);
    speedKnob.textFromValueFunction = [](double v) -> juce::String
    {
        int i = juce::roundToInt(v);
        return i == 0 ? "Slow" : (i == 1 ? "Med" : "Fast");
    };
    speedKnob.valueFromTextFunction = [](const juce::String& t) -> double
    {
        if (t.equalsIgnoreCase("Slow")) return 0.0;
        if (t.equalsIgnoreCase("Fast")) return 2.0;
        return 1.0;
    };
    speedKnob.updateText();
}

void LopasGateEditor::updateModeButtons(int mode)
{
    auto highlight = [&](juce::TextButton& btn, bool active)
    {
        btn.setColour(juce::TextButton::buttonColourId,
                      active ? kAccent : kBtnActive);
        btn.setColour(juce::TextButton::textColourOnId,  active ? kBg    : kText);
        btn.setColour(juce::TextButton::textColourOffId, active ? kBg    : kText);
    };
    highlight(lpBtn,    mode == 0);
    highlight(vcaBtn,   mode == 1);
    highlight(comboBtn, mode == 2);
}

void LopasGateEditor::paint(juce::Graphics& g)
{
    g.fillAll(kBg);

    // Title
    g.setColour(kAccent);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("LOPAS GATE", getLocalBounds().removeFromTop(30), juce::Justification::centred);

    // Subtle separator
    g.setColour(kBtnActive);
    g.drawHorizontalLine(30, 10.0f, (float)getWidth() - 10.0f);
}

void LopasGateEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(22); // title

    // Top row: mode buttons + strike
    auto topRow = area.removeFromTop(36);
    topRow.removeFromTop(4);
    lpBtn   .setBounds(topRow.removeFromLeft(60).reduced(2));
    vcaBtn  .setBounds(topRow.removeFromLeft(60).reduced(2));
    comboBtn.setBounds(topRow.removeFromLeft(72).reduced(2));
    topRow.removeFromLeft(8);
    strikeBtn.setBounds(topRow.removeFromLeft(72).reduced(2));

    area.removeFromTop(8);

    // Knob row
    auto knobRow = area;
    const int knobW = knobRow.getWidth() / 4;

    auto placeKnob = [&](juce::Slider& knob, juce::Label& label)
    {
        auto cell = knobRow.removeFromLeft(knobW);
        label.setBounds(cell.removeFromBottom(16));
        knob.setBounds(cell);
    };

    placeKnob(decayKnob,     decayLabel);
    placeKnob(speedKnob,     speedLabel);
    placeKnob(resonanceKnob, resonanceLabel);
    placeKnob(levelKnob,     levelLabel);
}

juce::AudioProcessorEditor* LopasGateProcessor::createEditor()
{
    return new LopasGateEditor(*this);
}
