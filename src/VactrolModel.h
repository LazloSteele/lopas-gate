#pragma once

class VactrolModel
{
public:
    enum class Speed { Slow, Med, Fast };

    void setSpeed(Speed s)
    {
        switch (s)
        {
            case Speed::Slow: attackCoeff = 0.001f; decayCoeff = 0.005f; break;
            case Speed::Med:  attackCoeff = 0.003f; decayCoeff = 0.01f;  break;
            case Speed::Fast: attackCoeff = 0.01f;  decayCoeff = 0.03f;  break;
        }
    }

    // targetR: 0 = fully open (CV at peak), 1 = fully closed (CV at zero)
    // Returns R_normalized in [0, 1]
    float process(float targetR)
    {
        float coeff = (targetR < R) ? attackCoeff : decayCoeff;
        R += (targetR - R) * coeff;
        return R;
    }

    void reset() { R = 1.0f; }

private:
    float R           = 1.0f;
    float attackCoeff = 0.003f;
    float decayCoeff  = 0.01f;
};
