#pragma once
#include <cmath>

class EnvelopeGenerator
{
public:
    void setSampleRate(double sr)
    {
        sampleRate = sr;
        // Recompute attack length from the stored decay time
        attackSamplesTotal = (int)(0.001 * sampleRate);
    }

    void setDecaySeconds(float d)
    {
        // Exponential decay coefficient: reaches ~0 in d seconds
        decayCoeff = std::exp(-1.0f / (d * (float)sampleRate));
    }

    void trigger()
    {
        phase             = Phase::Attack;
        attackSamplesLeft = attackSamplesTotal;
        value             = 0.0f;
    }

    void release()
    {
        if (phase == Phase::Attack || phase == Phase::Sustain)
            phase = Phase::Decay;
    }

    float process()
    {
        switch (phase)
        {
            case Phase::Idle:
                return 0.0f;

            case Phase::Attack:
                value += 1.0f / (float)attackSamplesTotal;
                if (--attackSamplesLeft <= 0)
                {
                    value = 1.0f;
                    phase = Phase::Sustain;
                }
                return value;

            case Phase::Sustain:
                return 1.0f;

            case Phase::Decay:
                value *= decayCoeff;
                if (value < 0.0001f)
                {
                    value = 0.0f;
                    phase = Phase::Idle;
                }
                return value;
        }
        return 0.0f;
    }

    bool isActive() const { return phase != Phase::Idle; }

    void reset()
    {
        phase = Phase::Idle;
        value = 0.0f;
    }

private:
    enum class Phase { Idle, Attack, Sustain, Decay };

    double sampleRate        = 44100.0;
    Phase  phase             = Phase::Idle;
    float  value             = 0.0f;
    int    attackSamplesTotal = 44;
    int    attackSamplesLeft  = 44;
    float  decayCoeff        = 0.9999f;
};
