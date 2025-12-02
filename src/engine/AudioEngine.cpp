#include "AudioEngine.h"
#include "../components/PluginWindow.h"

AudioEngine::AudioEngine(ProjectState& state) : projectState(state)
{
    mainProcessor = std::make_unique<juce::AudioProcessorGraph>();
    formatManager.registerBasicFormats();
    backgroundThread.startThread();
}

AudioEngine::~AudioEngine()
{
    mainProcessor = nullptr;
    backgroundThread.stopThread(1000);
}

void AudioEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
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
    // Capture Input for Recording
    if (isRecording && !trackRecorders.empty())
    {
        for (auto& pair : trackRecorders)
        {
            auto* writer = pair.second.get();
            if (writer)
            {
                writer->write(bufferToFill.buffer->getArrayOfReadPointers(), bufferToFill.numSamples);
            }
        }
    }

    bufferToFill.clearActiveBufferRegion();
    
    if (projectState.isPlaying)
    {
        double samplesPerBeat = (60.0 / projectState.tempo) * currentSampleRate;
        double startBeat = projectState.playheadBeat;
        double endBeat = startBeat + (bufferToFill.numSamples / samplesPerBeat);
        
        for (const auto& track : projectState.tracks)
        {
            if (track->mute) continue;
            
            // Apply Automation (Control Rate)
            for (const auto& curve : track->automationCurves)
            {
                if (!curve.active || curve.points.empty()) continue;
                
                // Simple linear interpolation
                float value = 0.0f;
                auto it = std::lower_bound(curve.points.begin(), curve.points.end(), startBeat, 
                    [](const AutomationPoint& p, double b) { return p.time < b; });
                
                if (it == curve.points.begin())
                {
                    value = it->value;
                }
                else if (it == curve.points.end())
                {
                    value = curve.points.back().value;
                }
                else
                {
                    auto& p2 = *it;
                    auto& p1 = *(it - 1);
                    double t = (startBeat - p1.time) / (p2.time - p1.time);
                    value = p1.value + (p2.value - p1.value) * (float)t;
                }
                
                if (curve.parameterID == "Volume") track->volume = value;
                else if (curve.parameterID == "Pan") track->pan = value;
            }
            
            juce::AudioBuffer<float> trackBuffer(2, bufferToFill.numSamples);
            trackBuffer.clear();
            juce::MidiBuffer trackMidi;
            
            // 1. Generate Audio/MIDI
            if (track->type == TrackType::Audio)
            {
                // Render Audio Clips
                for (const auto& clip : track->clips)
                {
                    if (clip.isMidi) continue;
                    
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
                            
                            for (int ch = 0; ch < std::min(trackBuffer.getNumChannels(), tempBuffer.getNumChannels()); ++ch)
                            {
                                trackBuffer.addFrom(ch, startSampleInBlock, tempBuffer, ch, 0, numSamplesToCopy, 1.0f);
                            }
                        }
                    }
                }
            }
            else if (track->type == TrackType::Midi)
            {
                // Collect MIDI Events
                for (const auto& clip : track->clips)
                {
                    if (!clip.isMidi) continue;
                    
                    auto& seq = clip.midiSequence;
                    for (int i = 0; i < seq.getNumEvents(); ++i)
                    {
                        auto* event = seq.getEventPointer(i);
                        double eventAbsBeat = clip.startBeat + event->message.getTimeStamp();
                        
                        if (eventAbsBeat >= startBeat && eventAbsBeat < endBeat)
                        {
                            int sampleOffset = (int)((eventAbsBeat - startBeat) * samplesPerBeat);
                            if (sampleOffset >= 0 && sampleOffset < bufferToFill.numSamples)
                            {
                                trackMidi.addEvent(event->message, sampleOffset);
                            }
                        }
                    }
                }
                
                // Process Instrument Plugin
                if (track->instrumentPlugin && track->instrumentPlugin->instance && !track->instrumentPlugin->bypassed)
                {
                    track->instrumentPlugin->instance->processBlock(trackBuffer, trackMidi);
                }
            }
            
            // 2. Process Inserts
            for (auto& slot : track->insertPlugins)
            {
                if (slot && slot->instance && !slot->bypassed)
                {
                    slot->instance->processBlock(trackBuffer, trackMidi);
                }
            }
            
            // 3. Mix to Main Buffer
            for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
            {
                bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, trackBuffer, ch, 0, trackBuffer.getNumSamples(), track->volume);
            }
        }
    }
    
    mainProcessor->processBlock(*bufferToFill.buffer, midiMessages);
}

void AudioEngine::startRecording()
{
    if (isRecording) return;
    
    recordingStartBeat = projectState.playheadBeat;
    trackRecorders.clear();
    recordingFiles.clear();
    
    auto docDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile("AiceCube").getChildFile("Recordings");
    docDir.createDirectory();
    
    for (auto& track : projectState.tracks)
    {
        if (track->type == TrackType::Audio && track->arm)
        {
            auto filename = track->name + "_" + juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S") + ".wav";
            auto file = docDir.getChildFile(filename);
            recordingFiles[track.get()] = file;
            
            if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream()))
            {
                juce::WavAudioFormat wavFormat;
                if (auto writer = wavFormat.createWriterFor(fileStream.get(), currentSampleRate, 2, 16, {}, 0))
                {
                    fileStream.release(); // Writer takes ownership
                    
                    trackRecorders[track.get()] = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
                        writer, backgroundThread, 32768);
                }
            }
        }
    }
    
    if (!trackRecorders.empty())
    {
        isRecording = true;
    }
}

void AudioEngine::stopRecording()
{
    if (!isRecording) return;
    
    isRecording = false;
    trackRecorders.clear(); // Flushes and closes writers
    
    // Create Clips
    double endBeat = projectState.playheadBeat;
    double length = endBeat - recordingStartBeat;
    
    if (length > 0)
    {
        for (auto& pair : recordingFiles)
        {
            Track* track = pair.first;
            juce::File file = pair.second;
            
            Clip newClip;
            newClip.name = file.getFileNameWithoutExtension();
            newClip.startBeat = recordingStartBeat;
            newClip.lengthBeats = length;
            newClip.isMidi = false;
            newClip.audioFile = file;
            newClip.trackIndex = 0;
            
            track->clips.push_back(newClip);
        }
    }
    
    recordingFiles.clear();
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

void AudioEngine::scanPlugins()
{
    pluginFormatManager.addFormat(new juce::VST3PluginFormat());
    
    juce::File appDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    knownPluginListFile = appDir.getChildFile("AiceCube").getChildFile("known_plugins.xml");
    
    if (knownPluginListFile.exists())
    {
        auto xml = juce::parseXML(knownPluginListFile);
        if (xml)
            knownPluginList.recreateFromXml(*xml);
    }
    
    juce::FileSearchPath searchPath;
    searchPath.add(juce::File("C:\\Program Files\\Common Files\\VST3"));
    
    juce::AudioPluginFormat* vst3Format = nullptr;
    for (int i = 0; i < pluginFormatManager.getNumFormats(); ++i)
    {
        if (pluginFormatManager.getFormat(i)->getName() == "VST3")
        {
            vst3Format = pluginFormatManager.getFormat(i);
            break;
        }
    }
    
    if (vst3Format)
    {
        pluginScanner.reset(new juce::PluginDirectoryScanner(knownPluginList, *vst3Format, searchPath, true, juce::File()));
        
        juce::String pluginName;
        while (pluginScanner->scanNextFile(true, pluginName))
        {
            // Scanning...
        }
        pluginScanner = nullptr;
    }
    
    auto xml = knownPluginList.createXml();
    xml->writeTo(knownPluginListFile);
}

std::shared_ptr<juce::AudioPluginInstance> AudioEngine::loadPlugin(const juce::PluginDescription& desc)
{
    juce::String error;
    auto instance = pluginFormatManager.createPluginInstance(desc, currentSampleRate, currentBlockSize, error);
    
    if (instance)
    {
        return std::shared_ptr<juce::AudioPluginInstance>(instance.release());
    }
    return nullptr;
}

void AudioEngine::addPluginToTrack(Track* track, int slotIndex, const juce::PluginDescription& desc)
{
    auto instance = loadPlugin(desc);
    if (instance)
    {
        if (track->insertPlugins.size() <= slotIndex)
            track->insertPlugins.resize(slotIndex + 1);
            
        if (!track->insertPlugins[slotIndex])
            track->insertPlugins[slotIndex] = std::make_shared<PluginSlot>();
            
        track->insertPlugins[slotIndex]->instance = instance;
        track->insertPlugins[slotIndex]->identifier = desc.fileOrIdentifier;
        
        updateGraph();
        showPluginWindow(instance.get());
    }
}

void AudioEngine::setInstrumentPlugin(Track* track, const juce::PluginDescription& desc)
{
    auto instance = loadPlugin(desc);
    if (instance)
    {
        if (!track->instrumentPlugin)
            track->instrumentPlugin = std::make_shared<PluginSlot>();
            
        track->instrumentPlugin->instance = instance;
        track->instrumentPlugin->identifier = desc.fileOrIdentifier;
        
        updateGraph();
        showPluginWindow(instance.get());
    }
}

void AudioEngine::showPluginWindow(juce::AudioPluginInstance* plugin)
{
    if (!plugin) return;
    
    if (pluginWindows.find(plugin) != pluginWindows.end() && pluginWindows[plugin] != nullptr)
    {
        pluginWindows[plugin]->toFront(true);
    }
    else
    {
        auto* editor = plugin->createEditor();
        if (editor)
        {
            auto* window = new PluginWindow(*plugin, editor);
            pluginWindows[plugin] = window;
        }
    }
}

void AudioEngine::closePluginWindow(juce::AudioPluginInstance* plugin)
{
    if (pluginWindows.find(plugin) != pluginWindows.end())
    {
        if (pluginWindows[plugin] != nullptr)
            delete pluginWindows[plugin];
        pluginWindows.erase(plugin);
    }
}
