#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class SimpleSynth
{
public:
    SimpleSynth()
    {
        // Add some voices
        for (int i = 0; i < 8; ++i)
            synth.addVoice(new SineWaveVoice());

        // Add a sound
        synth.addSound(new SineWaveSound());
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
    }

    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        synth.renderNextBlock(buffer, juce::MidiBuffer(), startSample, numSamples);
    }
    
    void noteOn(int note, float velocity)
    {
        synth.noteOn(1, note, velocity);
    }
    
    void noteOff(int note)
    {
        synth.noteOff(1, note, 0.0f, true);
    }
    
    void allNotesOff()
    {
        synth.allNotesOff(1, false);
    }

private:
    // Basic Sine Wave Sound
    struct SineWaveSound : public juce::SynthesiserSound
    {
        bool appliesToNote(int) override { return true; }
        bool appliesToChannel(int) override { return true; }
    };

    // Basic Sine Wave Voice
    struct SineWaveVoice : public juce::SynthesiserVoice
    {
        bool canPlaySound(juce::SynthesiserSound* sound) override
        {
            return dynamic_cast<SineWaveSound*>(sound) != nullptr;
        }

        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
        {
            currentAngle = 0.0;
            level = velocity * 0.15;
            tailOff = 0.0;

            auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
            auto cyclesPerSample = cyclesPerSecond / getSampleRate();

            angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
        }

        void stopNote(float, bool allowTailOff) override
        {
            if (allowTailOff)
            {
                if (tailOff == 0.0)
                    tailOff = 1.0;
            }
            else
            {
                clearCurrentNote();
                angleDelta = 0.0;
            }
        }

        void pitchWheelMoved(int) override {}
        void controllerMoved(int, int) override {}

        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
        {
            if (angleDelta != 0.0)
            {
                if (tailOff > 0.0) // Simple release
                {
                    while (--numSamples >= 0)
                    {
                        auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample(i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;

                        tailOff *= 0.99;

                        if (tailOff <= 0.005)
                        {
                            clearCurrentNote();
                            angleDelta = 0.0;
                            break;
                        }
                    }
                }
                else
                {
                    while (--numSamples >= 0)
                    {
                        auto currentSample = (float)(std::sin(currentAngle) * level);

                        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample(i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;
                    }
                }
            }
        }

        double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
    };

    juce::Synthesiser synth;
};
