# Lopas Gate

A Buchla 292-style Low-Pass Gate VST/Standalone plugin built with JUCE 7.

The defining character of West Coast synthesis — a single circuit controlling both filter cutoff and amplitude from the same control signal, shaped by a vactrol's organic memory effect. Based on the Parker & D'Angelo DAFx 2013 vactrol model.

---

## What it does

Three modes, one CV source:

- **LP** — 1-pole (6 dB/oct) low-pass filter only
- **VCA** — voltage-controlled amplitude only
- **Combo** — filter cutoff and amplitude coupled to the same signal; the acoustic instrument decay mode

The vactrol emulation is what makes it sound right. The LDR resistance doesn't track the CV instantly — it has a slow, asymmetric attack/decay with memory of prior triggers. This produces the characteristic bloom-and-natural-decay that a conventional filter+VCA chain can't replicate without careful programming.

---

## Parameters

| Parameter | Range | Notes |
|-----------|-------|-------|
| Mode | LP / VCA / Combo | Three-way switch |
| Strike | Button | Hold to sustain gate open; release starts decay |
| Decay | 50ms – 3s | Envelope decay time after gate closes |
| Vac Speed | Slow / Med / Fast | Vactrol attack+decay coefficients |
| Resonance | 0 – 100% | Feedback resonance; 0 = 292c character, higher = 292h |
| Level | 0 – 100% | Output gain |

**Vactrol speed presets:**
- **Slow** — NSL-7053 character; very organic, longest bloom
- **Med** — VTL5C3 character; original Buchla 292
- **Fast** — solid-state character; snappier, more consistent

---

## Signal path

```
[MIDI note-on / Strike button]
         ↓
[Envelope Generator]   1ms attack → sustain → exponential decay on release
         ↓
[Vactrol Model]        asymmetric IIR at audio rate — attack << decay
         ↓ R_normalized
         ├──────────────────────┐
[1-pole LP filter]          [VCA gain]
         └──────────────────────┘
                   ↓
              [Audio Out]
```

Gate open: note-on or Strike held — envelope attacks and holds at peak.
Gate close: note-off or Strike released — exponential decay begins.

---

## MIDI control

The plugin accepts MIDI and responds to note-on/note-off with sample-accurate timing. MIDI is the intended way to drive the gate from a step sequencer.

Because the plugin is an audio effect (not an instrument), DAWs don't route MIDI to it automatically. In **Ableton Live**:

1. Place Lopas Gate on an audio track as an effect.
2. Create a separate MIDI track for your step sequencer.
3. On the MIDI track, set **MIDI To** → select the audio track → select **Lopas Gate** from the second dropdown.

Note duration controls gate duration: a short step gives a short gate; a held note sustains the gate open until note-off.

---

## Building

Requires CMake 3.22+ and a C++17 compiler. JUCE is fetched automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)
```

Artifacts:
- `build/LopasGate_artefacts/Release/Standalone/Lopas Gate`
- `build/LopasGate_artefacts/Release/VST3/Lopas Gate.vst3/`

---

## Reference

- Parker & D'Angelo, "A Digital Model of the Buchla Lowpass-Gate," DAFx 2013
- Buchla 292h / 292t (Tiptop) hardware reference
