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
| Gate | Off / On | Automatable boolean — maps directly to MIDI in Ableton (see below) |
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
[MIDI note-on / Gate param / Strike button]
         ↓
[Envelope Generator]   1ms attack / hold (sustain) / exponential decay
         ↓
[Vactrol Model]        asymmetric IIR at audio rate — attack << decay
         ↓ R_normalized
         ├──────────────────────┐
[1-pole LP filter]          [VCA gain]
         └──────────────────────┘
                   ↓
              [Audio Out]
```

Gate signal flow: note-on / Gate → 1 opens the gate (attack → sustain); note-off / Gate → 0 starts the decay.

---

## Triggering the gate

Three ways to open and close the gate — all work simultaneously:

| Method | How |
|--------|-----|
| **MIDI notes** | Route a MIDI track to the audio track containing Lopas Gate via Ableton's **MIDI To** dropdown. Note-on opens, note-off closes. |
| **Gate parameter** | In Ableton, press **Ctrl+M**, click the Gate button in the plugin, then press a key on your controller or step sequencer to map it. Hold = open, release = decay. This is the easiest path for step sequencers. |
| **Strike button** | Hold the button in the plugin UI. Release starts decay. |

**Recommended setup for a step sequencer driving an instrument:** put the instrument and Lopas Gate on the same audio track; put the step sequencer on its own MIDI track; MIDI-map the step sequencer's notes to the Gate parameter.

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
