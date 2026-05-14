#pragma once
#include <cmath>

class OnePoleFilter
{
public:
    void setSampleRate(double sr) { sampleRate = sr; }

    // R_normalized: 0 = fully open (18 kHz), 1 = fully closed (20 Hz)
    void setCutoff(float R_normalized)
    {
        // Exponential mapping across the audible range
        float fc = 20.0f * std::pow(900.0f, 1.0f - R_normalized);
        float w  = std::exp(-2.0f * 3.14159265f * fc / (float)sampleRate);
        a = w;
        b = 1.0f - w;
    }

    float process(float x)
    {
        y = b * x + a * y;
        return y;
    }

    void reset() { y = 0.0f; }

private:
    double sampleRate = 44100.0;
    float  a = 0.0f;
    float  b = 1.0f;
    float  y = 0.0f;
};
