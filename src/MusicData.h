#pragma once
#include <juce_core/juce_core.h>

struct Note
{
    int pitch;
    double start;
    double duration;
    int velocity;
};

struct Sequence
{
    juce::Array<Note> notes;
};
