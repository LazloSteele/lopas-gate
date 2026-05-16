#include "PluginProcessor.h"
#include "PluginEditor.h"

LopasGateProcessor::LopasGateProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input",   juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

bool LopasGateProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
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

    layout.add(std::make_unique<juce::AudioParameterBool>(
        "gate", "Gate", false));

    return layout;
}

void LopasGateProcessor::prepareToPlay(double sampleRate, int)
{
    envelope.setSampleRate(sampleRate);
    vactrol.reset();
    envelope.reset();
    currentR        = 1.0f;
    ctrlRateCounter = 0;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        filter[ch].setSampleRate(sampleRate);
        filter[ch].reset();
        filter[ch].setCutoff(1.0f);
        feedbackSample[ch] = 0.0f;
    }

    auto decayParam    = apvts.getRawParameterValue("decay");
    auto vacSpeedParam = apvts.getRawParameterValue("vacSpeed");

    envelope.setDecaySeconds(decayParam->load());
    vactrol.setSpeed(static_cast<VactrolModel::Speed>((int)vacSpeedParam->load()));
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

    // Gate parameter — automatable / MIDI-mappable in the host
    bool gateOpen = apvts.getRawParameterValue("gate")->load() > 0.5f;
    if (gateOpen && !lastGateParam)
        envelope.trigger();
    else if (!gateOpen && lastGateParam)
        envelope.release();
    lastGateParam = gateOpen;

    // Consume strike requests from UI
    if (strikeRequested.exchange(false))
        envelope.trigger();
    if (strikeReleaseRequested.exchange(false))
        envelope.release();

    // Build a lookup of MIDI events by sample position
    auto midiIt  = midiBuffer.begin();
    auto midiEnd = midiBuffer.end();

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);

    float* channelPtrs[kMaxChannels];
    for (int ch = 0; ch < numChannels; ++ch)
        channelPtrs[ch] = buffer.getWritePointer(ch);

    for (int i = 0; i < numSamples; ++i)
    {
        // Consume MIDI note-ons at this sample
        while (midiIt != midiEnd && (*midiIt).samplePosition <= i)
        {
            auto msg = (*midiIt).getMessage();
            if (msg.isNoteOn())
                envelope.trigger();
            else if (msg.isNoteOff())
                envelope.release();
            ++midiIt;
        }

        // Envelope → CV
        float cv = envelope.process();

        // Vactrol runs at audio rate — coefficients are tuned for per-sample stepping
        currentR = vactrol.process(1.0f - cv); // cv=1 → targetR=0 (open)

        // setCutoff has exp/pow; only call at control rate
        if (++ctrlRateCounter >= kCtrlInterval)
        {
            ctrlRateCounter = 0;
            for (int ch = 0; ch < numChannels; ++ch)
                filter[ch].setCutoff(currentR);
        }

        // VCA gain is the same for all channels
        const float maxFeedback = 0.9f;
        const float gain = (mode == 1 || mode == 2)
                               ? std::pow(1.0f - currentR, 1.5f)
                               : 1.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float in      = channelPtrs[ch][i];
            float filterIn = in + feedbackSample[ch] * res * maxFeedback;
            float out      = filterIn;

            if (mode == 0 || mode == 2) // LP or Combo
            {
                out = filter[ch].process(filterIn);
                feedbackSample[ch] = out;
            }
            else // VCA only
            {
                filter[ch].process(filterIn);
                feedbackSample[ch] = 0.0f;
                out = in;
            }

            channelPtrs[ch][i] = out * gain * level;
        }
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
