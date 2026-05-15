#include "PluginProcessor.h"
#include "PluginEditor.h"

LopasGateProcessor::LopasGateProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input",   juce::AudioChannelSet::mono(), true)
                         .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout LopasGateProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "mode", "Mode",
        juce::StringArray { "LP", "VCA", "Combo" }, 2));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float>(0.05f, 3.0f, 0.001f, 0.4f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "vacSpeed", "Vactrol Speed",
        juce::StringArray { "Slow", "Med", "Fast" }, 1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "level", "Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    return layout;
}

void LopasGateProcessor::prepareToPlay(double sampleRate, int)
{
    envelope.setSampleRate(sampleRate);
    filter.setSampleRate(sampleRate);
    vactrol.reset();
    filter.reset();
    envelope.reset();
    currentR       = 1.0f;
    feedbackSample = 0.0f;
    ctrlRateCounter = 0;

    // Apply current parameter values
    auto decayParam    = apvts.getRawParameterValue("decay");
    auto vacSpeedParam = apvts.getRawParameterValue("vacSpeed");

    envelope.setDecaySeconds(decayParam->load());
    vactrol.setSpeed(static_cast<VactrolModel::Speed>((int)vacSpeedParam->load()));
    filter.setCutoff(1.0f); // start fully closed
}

void LopasGateProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ScopedNoDenormals noDenormals;

    auto* modeParam      = apvts.getRawParameterValue("mode");
    auto* decayParam     = apvts.getRawParameterValue("decay");
    auto* vacSpeedParam  = apvts.getRawParameterValue("vacSpeed");
    auto* resonanceParam = apvts.getRawParameterValue("resonance");
    auto* levelParam     = apvts.getRawParameterValue("level");

    const int mode      = (int)modeParam->load();
    const float res     = resonanceParam->load();
    const float level   = levelParam->load();

    envelope.setDecaySeconds(decayParam->load());
    vactrol.setSpeed(static_cast<VactrolModel::Speed>((int)vacSpeedParam->load()));

    // Consume strike request from UI
    if (strikeRequested.exchange(false))
        envelope.trigger();

    // Build a lookup of MIDI events by sample position
    auto midiIt  = midiBuffer.begin();
    auto midiEnd = midiBuffer.end();

    auto* channelData = buffer.getWritePointer(0);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // Consume MIDI note-ons at this sample
        while (midiIt != midiEnd && (*midiIt).samplePosition <= i)
        {
            auto msg = (*midiIt).getMessage();
            if (msg.isNoteOn())
                envelope.trigger();
            ++midiIt;
        }

        // Envelope → CV
        float cv = envelope.process();

        // Vactrol runs at audio rate — coefficients are tuned for per-sample stepping
        currentR = vactrol.process(1.0f - cv); // cv=1 → targetR=0 (open)

        // setCutoff has exp/pow; only call when currentR ticks forward at control rate
        if (++ctrlRateCounter >= kCtrlInterval)
        {
            ctrlRateCounter = 0;
            filter.setCutoff(currentR);
        }

        float in = channelData[i];

        // Resonance: feedback path around filter input
        // maxFeedback < 1 keeps stability; 0.9 allows noticeable resonance peak
        const float maxFeedback = 0.9f;
        float filterIn = in + feedbackSample * res * maxFeedback;

        float out = filterIn;

        if (mode == 0 || mode == 2) // LP or Combo
        {
            out = filter.process(filterIn);
            feedbackSample = out;
        }
        else
        {
            // VCA mode: bypass filter but still process it to track state cleanly
            filter.process(filterIn);
            feedbackSample = 0.0f;
            out = in;
        }

        // VCA gain: gamma=1.5 curve
        if (mode == 1 || mode == 2) // VCA or Combo
        {
            float gain = std::pow(1.0f - currentR, 1.5f);
            out *= gain;
        }

        channelData[i] = out * level;
    }
}

void LopasGateProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void LopasGateProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LopasGateProcessor();
}
