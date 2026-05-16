#pragma once
#include <JuceHeader.h>
#include "VactrolModel.h"
#include "OnePoleFilter.h"
#include "EnvelopeGenerator.h"

class LopasGateProcessor : public juce::AudioProcessor
{
public:
    LopasGateProcessor();
    ~LopasGateProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Lopas Gate"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<bool> strikeRequested        { false };
    std::atomic<bool> strikeReleaseRequested { false };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static constexpr int kMaxChannels  = 2;
    static constexpr int kCtrlInterval = 44; // ~1kHz at 44.1kHz

    EnvelopeGenerator envelope;
    VactrolModel      vactrol;
    OnePoleFilter     filter[kMaxChannels];

    float currentR             = 1.0f;
    float feedbackSample[kMaxChannels] = {};
    bool  lastGateParam        = false;

    int ctrlRateCounter  = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LopasGateProcessor)
};
