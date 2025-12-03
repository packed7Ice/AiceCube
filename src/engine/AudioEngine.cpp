#include "AudioEngine.h"
#include "../components/PluginWindow.h"

AudioEngine::AudioEngine(ProjectState& state) : projectState(state)
{
    mainProcessor = std::make_unique<juce::AudioProcessorGraph>();
    formatManager.registerBasicFormats();
    pluginFormatManager.addFormat(new juce::VST3PluginFormat());
    backgroundThread.startThread();
    loadPluginSearchPaths();
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
    
    // Prepare all track plugins
    for (const auto& track : projectState.tracks)
    {
        if (track->instrumentPlugin && track->instrumentPlugin->instance)
            track->instrumentPlugin->instance->prepareToPlay(sampleRate, samplesPerBlock);
            
        for (const auto& slot : track->insertPlugins)
            if (slot && slot->instance)
                slot->instance->prepareToPlay(sampleRate, samplesPerBlock);
    }
}



void AudioEngine::releaseResources()
{
    mainProcessor->releaseResources();
    fileReaders.clear();
}

void AudioEngine::processAudio(const juce::AudioSourceChannelInfo& bufferToFill, juce::MidiBuffer& midiMessages)
{
    const juce::ScopedLock sl(processLock);

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
    
    double samplesPerBeat = (60.0 / projectState.tempo) * currentSampleRate;
    
    if (projectState.isPlaying)
    {
        int samplesRemaining = bufferToFill.numSamples;
        int currentSampleOffset = 0;
        
        int loopCount = 0;
        
        while (samplesRemaining > 0 && loopCount++ < 5)
        {
            double currentStartBeat = projectState.playheadBeat;
            int samplesToProcess = samplesRemaining;
            
            if (projectState.isLooping)
            {
                if (currentStartBeat >= projectState.loopEnd)
                {
                    projectState.playheadBeat = projectState.loopStart;
                    currentStartBeat = projectState.loopStart;
                }
                
                if (currentStartBeat < projectState.loopEnd)
                {
                    double beatsToLoopEnd = projectState.loopEnd - currentStartBeat;
                    int samplesToLoopEnd = (int)(beatsToLoopEnd * samplesPerBeat);
                    
                    if (samplesToLoopEnd < samplesRemaining)
                    {
                        samplesToProcess = samplesToLoopEnd;
                    }
                }
            }
            
            if (samplesToProcess > 0)
            {
                juce::AudioSourceChannelInfo segmentInfo(bufferToFill.buffer, 
                                                         bufferToFill.startSample + currentSampleOffset, 
                                                         samplesToProcess);
                
                renderSegment(segmentInfo, midiMessages, currentSampleOffset, currentStartBeat, samplesPerBeat, true);
                
                // Play Metronome for this segment (After rendering tracks)
                playMetronome(segmentInfo, currentStartBeat, samplesPerBeat);
                
                currentSampleOffset += samplesToProcess;
                samplesRemaining -= samplesToProcess;
                projectState.playheadBeat += samplesToProcess / samplesPerBeat;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        juce::MidiBuffer emptyMidi;
        renderSegment(bufferToFill, emptyMidi, 0, projectState.playheadBeat, samplesPerBeat, false);
    }
    
    // mainProcessor->processBlock(*bufferToFill.buffer, midiMessages);
}

void AudioEngine::renderSegment(const juce::AudioSourceChannelInfo& bufferToFill, juce::MidiBuffer& midiMessages, int globalSampleOffset, double startBeat, double samplesPerBeat, bool generateMidi)
{
    double endBeat = startBeat + (bufferToFill.numSamples / samplesPerBeat);
    
    // 0. Prepare Bus Buffers
    for (const auto& track : projectState.tracks)
    {
        if (track->type == TrackType::Bus)
        {
            auto& busBuf = busBuffers[track->id];
            busBuf.setSize(2, bufferToFill.numSamples, false, false, true);
            busBuf.clear();
        }
    }
    
    // 1. Process Audio/Midi Tracks
    for (const auto& track : projectState.tracks)
    {
        if (track->mute) continue;
        if (track->type == TrackType::Bus) continue;
        
        // Apply Automation
        for (const auto& curve : track->automationCurves)
        {
            if (!curve.active || curve.points.empty()) continue;
            
            float value = 0.0f;
            auto it = std::lower_bound(curve.points.begin(), curve.points.end(), startBeat, 
                [](const AutomationPoint& p, double b) { return p.time < b; });
            
            if (it == curve.points.begin()) value = it->value;
            else if (it == curve.points.end()) value = curve.points.back().value;
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
        
        // Determine required channels based on plugin requirements
        int maxChannels = 2;
        if (track->type == TrackType::Midi && track->instrumentPlugin && track->instrumentPlugin->instance)
        {
            auto* instance = track->instrumentPlugin->instance.get();
            int ins = instance->getTotalNumInputChannels();
            int outs = instance->getTotalNumOutputChannels();
            maxChannels = std::max(ins, outs);
            if (maxChannels < 2) maxChannels = 2; // Ensure at least stereo for safety
        }
        
        // Allocate buffer with sufficient channels for the plugin
        juce::AudioBuffer<float> trackBuffer(maxChannels, bufferToFill.numSamples);
        trackBuffer.clear();
        juce::MidiBuffer trackMidi;
        
        // Generate Audio/MIDI
        if (track->type == TrackType::Audio)
        {
            if (generateMidi)
            {
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
                            juce::AudioBuffer<float> clipBuffer(2, numSamplesToCopy);
                            reader->read(&clipBuffer, 0, numSamplesToCopy, fileReadStartSample, true, true);
                            
                            for (int ch = 0; ch < std::min(trackBuffer.getNumChannels(), clipBuffer.getNumChannels()); ++ch)
                            {
                                trackBuffer.addFrom(ch, startSampleInBlock, clipBuffer, ch, 0, numSamplesToCopy, 1.0f);
                            }
                        }
                    }
                }
            }
        }
        else if (track->type == TrackType::Midi)
        {
            if (generateMidi)
            {
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
                                midiMessages.addEvent(event->message, globalSampleOffset + sampleOffset);
                            }
                        }
                    }
                }
            }
            
            if (track->instrumentPlugin && track->instrumentPlugin->instance && !track->instrumentPlugin->bypassed)
            {
                track->instrumentPlugin->instance->processBlock(trackBuffer, trackMidi);
            }
        }
        
        // Process Inserts
        for (auto& slot : track->insertPlugins)
        {
            if (slot && slot->instance && !slot->bypassed)
            {
                // Ensure buffer has enough channels for insert too?
                // Ideally we should check all plugins in chain and max out channels.
                // For now, assume inserts work with what instrument provided or stereo.
                slot->instance->processBlock(trackBuffer, trackMidi);
            }
        }
        
        // Process Sends
        for (const auto& send : track->sends)
        {
            if (send.active && send.amount > 0.0f)
            {
                if (busBuffers.count(send.targetTrackId))
                {
                    auto& busBuf = busBuffers[send.targetTrackId];
                    for (int ch = 0; ch < std::min(trackBuffer.getNumChannels(), busBuf.getNumChannels()); ++ch)
                    {
                        busBuf.addFrom(ch, 0, trackBuffer, ch, 0, trackBuffer.getNumSamples(), send.amount);
                    }
                }
            }
        }
        
        // Mix to Main
        for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
        {
            // If track is mono, mix to both L and R (or pan)
            // If track is stereo, mix L->L, R->R
            // If track has more channels, we usually just take first 2 for main mix
            
            int sourceCh = ch;
            if (trackBuffer.getNumChannels() == 1) sourceCh = 0;
            
            if (sourceCh < trackBuffer.getNumChannels())
            {
                bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, trackBuffer, sourceCh, 0, trackBuffer.getNumSamples(), track->volume);
            }
        }
    }
    
    // 2. Process Bus Tracks
    for (const auto& track : projectState.tracks)
    {
        if (track->mute) continue;
        if (track->type != TrackType::Bus) continue;
        
        if (busBuffers.count(track->id))
        {
            auto& busBuf = busBuffers[track->id];
            juce::MidiBuffer busMidi;
            
            // Process Inserts
            for (auto& slot : track->insertPlugins)
            {
                if (slot && slot->instance && !slot->bypassed)
                {
                    slot->instance->processBlock(busBuf, busMidi);
                }
            }
            
            // Mix Bus to Main
            for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
            {
                bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, busBuf, ch, 0, busBuf.getNumSamples(), track->volume);
            }
        }
    }
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
    // searchPath.add(juce::File("C:\\Program Files\\Common Files\\VST3"));
    for (const auto& path : pluginSearchPaths)
        searchPath.add(path);
    
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

void AudioEngine::scanPluginsAsync(std::function<void(const juce::String&)> onProgress, std::function<void()> onFinished)
{
    // Simple thread for scanning
    juce::Thread::launch([this, onProgress, onFinished] {
        juce::FileSearchPath searchPath;
        for (const auto& path : pluginSearchPaths)
            searchPath.add(path);
            
        juce::AudioPluginFormat* vst3Format = nullptr;
        for (int i = 0; i < pluginFormatManager.getNumFormats(); ++i) {
            if (pluginFormatManager.getFormat(i)->getName() == "VST3") {
                vst3Format = pluginFormatManager.getFormat(i);
                break;
            }
        }
        
        if (vst3Format)
        {
            juce::PluginDirectoryScanner scanner(knownPluginList, *vst3Format, searchPath, true, juce::File());
            juce::String pluginName;
            while (scanner.scanNextFile(true, pluginName))
            {
                if (onProgress) juce::MessageManager::callAsync([onProgress, pluginName] { onProgress(pluginName); });
            }
        }
        
        auto xml = knownPluginList.createXml();
        xml->writeTo(knownPluginListFile);
        
        if (onFinished) juce::MessageManager::callAsync(onFinished);
    });
}

void AudioEngine::addPluginSearchPath(const juce::File& path)
{
    if (std::find(pluginSearchPaths.begin(), pluginSearchPaths.end(), path) == pluginSearchPaths.end())
    {
        pluginSearchPaths.push_back(path);
        savePluginSearchPaths();
    }
}

void AudioEngine::removePluginSearchPath(int index)
{
    if (index >= 0 && index < pluginSearchPaths.size())
    {
        pluginSearchPaths.erase(pluginSearchPaths.begin() + index);
        savePluginSearchPaths();
    }
}

void AudioEngine::savePluginSearchPaths()
{
    juce::StringArray paths;
    for (const auto& p : pluginSearchPaths)
        paths.add(p.getFullPathName());
        
    juce::File appDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    pluginSearchPathsFile = appDir.getChildFile("AiceCube").getChildFile("plugin_paths.txt");
    pluginSearchPathsFile.replaceWithText(paths.joinIntoString("\n"));
}

void AudioEngine::loadPluginSearchPaths()
{
    juce::File appDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    pluginSearchPathsFile = appDir.getChildFile("AiceCube").getChildFile("plugin_paths.txt");
    
    if (pluginSearchPathsFile.existsAsFile())
    {
        juce::StringArray paths;
        paths.addLines(pluginSearchPathsFile.loadFileAsString());
        
        pluginSearchPaths.clear();
        for (const auto& p : paths)
            if (p.isNotEmpty())
                pluginSearchPaths.push_back(juce::File(p));
    }
    
    // Default if empty
    if (pluginSearchPaths.empty())
    {
        pluginSearchPaths.push_back(juce::File("C:\\Program Files\\Common Files\\VST3"));
    }
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
        
        if (currentSampleRate > 0)
            instance->prepareToPlay(currentSampleRate, currentBlockSize);
            
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
        
        if (currentSampleRate > 0)
            instance->prepareToPlay(currentSampleRate, currentBlockSize);
            
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

void AudioEngine::openPluginWindow(Track* track)
{
    if (track && track->instrumentPlugin && track->instrumentPlugin->instance)
    {
        showPluginWindow(track->instrumentPlugin->instance.get());
    }
}

void AudioEngine::togglePluginWindow(Track* track)
{
    if (track && track->instrumentPlugin && track->instrumentPlugin->instance)
    {
        auto* plugin = track->instrumentPlugin->instance.get();
        if (pluginWindows.find(plugin) != pluginWindows.end() && pluginWindows[plugin] != nullptr)
        {
            closePluginWindow(plugin);
        }
        else
        {
            showPluginWindow(plugin);
        }
    }
}

void AudioEngine::playMetronome(const juce::AudioSourceChannelInfo& bufferToFill, double startBeat, double samplesPerBeat)
{
    if (!projectState.metronomeEnabled) return;

    double endBeat = startBeat + (bufferToFill.numSamples / samplesPerBeat);
    
    int firstBeat = (int)std::ceil(startBeat);
    int lastBeat = (int)std::floor(endBeat);
    
    for (int beat = firstBeat; beat <= lastBeat; ++beat)
    {
        double beatTime = beat;
        int sampleOffset = (int)((beatTime - startBeat) * samplesPerBeat);
        
        if (sampleOffset >= 0 && sampleOffset < bufferToFill.numSamples)
        {
            float frequency = (beat % projectState.timeSignatureNumerator == 0) ? 1000.0f : 500.0f;
            float length = 0.05f; 
            int lengthSamples = (int)(length * currentSampleRate);
            
            for (int i = 0; i < lengthSamples; ++i)
            {
                if (sampleOffset + i >= bufferToFill.numSamples) break;
                
                float sample = std::sin(2.0f * juce::MathConstants<float>::pi * frequency * (i / currentSampleRate));
                sample *= 0.8f; 
                sample *= (1.0f - (float)i / lengthSamples);
                
                for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
                {
                    bufferToFill.buffer->addSample(ch, bufferToFill.startSample + sampleOffset + i, sample);
                }
            }
        }
    }
}

void AudioEngine::deleteTrack(int index)
{
    const juce::ScopedLock sl(processLock);
    projectState.removeTrack(index);
    updateGraph();
}


