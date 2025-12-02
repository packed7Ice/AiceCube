#include "AudioEngine.h"

AudioEngine::AudioEngine(AppState& state) : appState(state)
{
    mainProcessor = std::make_unique<juce::AudioProcessorGraph>();
}

AudioEngine::~AudioEngine()
{
    mainProcessor = nullptr;
}

void AudioEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    formatManager.registerBasicFormats();
    
    mainProcessor->setPlayConfigDetails(2, 2, sampleRate, samplesPerBlock);
    mainProcessor->prepareToPlay(sampleRate, samplesPerBlock);
    
    updateGraph();
}

void AudioEngine::releaseResources()
{
    mainProcessor->releaseResources();
    fileReaders.clear();
}

void AudioEngine::processAudio(const juce::AudioSourceChannelInfo& bufferToFill, juce::MidiBuffer& midiMessages)
{
    bufferToFill.clearActiveBufferRegion();
    
    if (appState.isPlaying)
    {
        double samplesPerBeat = (60.0 / appState.tempoBpm) * currentSampleRate;
        double startBeat = appState.playheadBeat;
        double endBeat = startBeat + (bufferToFill.numSamples / samplesPerBeat);
        
        for (const auto& track : appState.tracks)
        {
            if (track.type != TrackType::Audio || track.isMuted) continue;
            
            for (const auto& clip : track.clips)
            {
                if (clip.type != ClipType::Audio) continue;
                
                if (clip.getEndBeat() > startBeat && clip.startBeat < endBeat)
                {
                    double clipStartInBlockBeats = clip.startBeat - startBeat;
                    int startSampleInBlock = 0;
                    int numSamplesToCopy = bufferToFill.numSamples;
                    int fileReadStartSample = 0;
                    
                    if (clipStartInBlockBeats > 0)
                    {
                        startSampleInBlock = (int)(clipStartInBlockBeats * samplesPerBeat);
                        numSamplesToCopy -= startSampleInBlock;
                    }
                    else
                    {
                        fileReadStartSample = (int)((startBeat - clip.startBeat) * samplesPerBeat);
                    }
                    
                    double clipEndInBlockBeats = clip.getEndBeat() - startBeat;
                    int endSampleInBlock = (int)(clipEndInBlockBeats * samplesPerBeat);
                    if (endSampleInBlock < bufferToFill.numSamples)
                    {
                        numSamplesToCopy = std::min(numSamplesToCopy, endSampleInBlock - startSampleInBlock);
                    }
                    
                    if (numSamplesToCopy <= 0) continue;
                    
                    auto* reader = getReaderFor(clip.audioFile);
                    if (reader)
                    {
                        juce::AudioBuffer<float> tempBuffer(2, numSamplesToCopy);
                        reader->read(&tempBuffer, 0, numSamplesToCopy, fileReadStartSample, true, true);
                        
                        for (int ch = 0; ch < std::min(bufferToFill.buffer->getNumChannels(), tempBuffer.getNumChannels()); ++ch)
                        {
                            bufferToFill.buffer->addFrom(ch, bufferToFill.startSample + startSampleInBlock, tempBuffer, ch, 0, numSamplesToCopy, track.volume);
                        }
                    }
                }
            }
        }
    }
    
    mainProcessor->processBlock(*bufferToFill.buffer, midiMessages);
}

juce::AudioFormatReader* AudioEngine::getReaderFor(const juce::File& file)
{
    if (file == juce::File()) return nullptr;
    
    auto path = file.getFullPathName();
    if (fileReaders.find(path) == fileReaders.end())
    {
        auto* reader = formatManager.createReaderFor(file);
        if (reader)
        {
            fileReaders[path].reset(reader);
        }
        else
        {
            return nullptr;
        }
    }
    return fileReaders[path].get();
}

void AudioEngine::updateGraph()
{
    mainProcessor->clear();
    
    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
    
    audioInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
    midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
    midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));
}
