#pragma once
#include "MusicData.h"
#include "ProjectState.h"

class ProjectSerializer
{
public:
    static juce::String toJSON(const ProjectState& state)
    {
        juce::DynamicObject* root = new juce::DynamicObject();
        root->setProperty("tempo", state.tempo);
        root->setProperty("timeSignatureNum", state.timeSignatureNumerator);
        root->setProperty("timeSignatureDenom", state.timeSignatureDenominator);
        
        juce::Array<juce::var> tracksArray;
        for (const auto& track : state.tracks)
        {
            tracksArray.add(trackToVar(*track));
        }
        root->setProperty("tracks", tracksArray);
        
        return juce::JSON::toString(juce::var(root));
    }
    
    static void fromJSON(ProjectState& state, const juce::String& jsonString)
    {
        auto rootVar = juce::JSON::parse(jsonString);
        if (rootVar.isObject())
        {
            state.tempo = rootVar["tempo"];
            state.timeSignatureNumerator = rootVar["timeSignatureNum"];
            state.timeSignatureDenominator = rootVar["timeSignatureDenom"];
            
            state.tracks.clear();
            auto tracksArray = rootVar["tracks"].getArray();
            if (tracksArray)
            {
                for (auto& t : *tracksArray)
                {
                    auto track = std::make_shared<Track>();
                    varToTrack(t, *track);
                    state.tracks.push_back(track);
                }
            }
        }
    }

private:
    static juce::var trackToVar(const Track& track)
    {
        juce::DynamicObject* obj = new juce::DynamicObject();
        obj->setProperty("name", track.name);
        obj->setProperty("type", (int)track.type);
        obj->setProperty("volume", track.volume);
        obj->setProperty("pan", track.pan);
        obj->setProperty("mute", track.mute);
        obj->setProperty("solo", track.solo);
        
        juce::Array<juce::var> clipsArray;
        for (const auto& clip : track.clips)
        {
            clipsArray.add(clipToVar(clip));
        }
        obj->setProperty("clips", clipsArray);
        
        // Plugins & Automation
        // Instrument
        if (track.instrumentPlugin)
        {
             if (track.instrumentPlugin->instance)
             {
                 track.instrumentPlugin->instance->getStateInformation(const_cast<juce::MemoryBlock&>(track.instrumentPlugin->state));
                 obj->setProperty("instrumentId", track.instrumentPlugin->identifier);
                 obj->setProperty("instrumentState", track.instrumentPlugin->state.toBase64Encoding());
             }
             else if (track.instrumentPlugin->identifier.isNotEmpty())
             {
                 obj->setProperty("instrumentId", track.instrumentPlugin->identifier);
                 // If we have state but no instance (e.g. failed load), save it back
                 if (track.instrumentPlugin->state.getSize() > 0)
                    obj->setProperty("instrumentState", track.instrumentPlugin->state.toBase64Encoding());
             }
        }
        
        juce::Array<juce::var> insertsArray;
        for (const auto& slot : track.insertPlugins)
        {
            juce::DynamicObject* slotObj = new juce::DynamicObject();
            if (slot)
            {
                if (slot->instance)
                {
                    slot->instance->getStateInformation(slot->state);
                    slotObj->setProperty("id", slot->identifier);
                    slotObj->setProperty("state", slot->state.toBase64Encoding());
                }
                else
                {
                    slotObj->setProperty("id", slot->identifier);
                    if (slot->state.getSize() > 0)
                        slotObj->setProperty("state", slot->state.toBase64Encoding());
                }
            }
            insertsArray.add(juce::var(slotObj));
        }
        obj->setProperty("inserts", insertsArray);
        
        // Automation
        juce::Array<juce::var> curvesArray;
        for (const auto& curve : track.automationCurves)
        {
            curvesArray.add(automationCurveToVar(curve));
        }
        obj->setProperty("automation", curvesArray);

        obj->setProperty("id", track.id.toString());
        
        juce::Array<juce::var> sendsArray;
        for (const auto& send : track.sends)
        {
            juce::DynamicObject* sObj = new juce::DynamicObject();
            sObj->setProperty("targetId", send.targetTrackId.toString());
            sObj->setProperty("amount", send.amount);
            sObj->setProperty("active", send.active);
            sendsArray.add(juce::var(sObj));
        }
        obj->setProperty("sends", sendsArray);

        return juce::var(obj);
    }
    
    static void varToTrack(const juce::var& v, Track& track)
    {
        if (v.hasProperty("id"))
            track.id = juce::Uuid(v["id"].toString());
        else
            track.id = juce::Uuid();
            
        track.name = v["name"].toString();
        track.type = (TrackType)(int)v["type"];
        track.volume = (float)v["volume"];
        track.pan = (float)v["pan"];
        track.mute = (bool)v["mute"];
        track.solo = (bool)v["solo"];
        
        auto sendsArray = v["sends"].getArray();
        if (sendsArray)
        {
            for (auto& s : *sendsArray)
            {
                Send send;
                send.targetTrackId = juce::Uuid(s["targetId"].toString());
                send.amount = (float)s["amount"];
                send.active = (bool)s["active"];
                track.sends.push_back(send);
            }
        }
        
        auto clipsArray = v["clips"].getArray();
        if (clipsArray)
        {
            for (auto& c : *clipsArray)
            {
                Clip clip;
                varToClip(c, clip);
                track.clips.push_back(clip);
            }
        }
        
        // Instrument
        juce::String instId = v["instrumentId"].toString();
        if (instId.isNotEmpty())
        {
            track.instrumentPlugin = std::make_shared<PluginSlot>();
            track.instrumentPlugin->identifier = instId;
            
            juce::String stateStr = v["instrumentState"].toString();
            if (stateStr.isNotEmpty())
            {
                track.instrumentPlugin->state.fromBase64Encoding(stateStr);
            }
        }
        
        // Inserts
        auto insertsArray = v["inserts"].getArray();
        if (insertsArray)
        {
            for (auto& i : *insertsArray)
            {
                auto slot = std::make_shared<PluginSlot>();
                // Handle both old format (string ID) and new format (object)
                if (i.isObject())
                {
                    slot->identifier = i["id"].toString();
                    juce::String stateStr = i["state"].toString();
                    if (stateStr.isNotEmpty())
                    {
                        slot->state.fromBase64Encoding(stateStr);
                    }
                }
                else
                {
                    slot->identifier = i.toString();
                }
                
                if (slot->identifier.isNotEmpty())
                    track.insertPlugins.push_back(slot);
                else
                    track.insertPlugins.push_back(nullptr); // Preserve slot index? Or just push back.
                    // Original logic pushed empty string for empty slot.
                    // Let's stick to vector of shared_ptr, null if empty?
                    // The original code: insertsArray.add("");
                    // So if identifier is empty, it was an empty slot.
                    // But here we are iterating and pushing back.
                    // If we want to maintain 3 slots, we should handle it.
                    // For now, just push back.
            }
        }
        
        auto curvesArray = v["automation"].getArray();
        if (curvesArray)
        {
            for (auto& c : *curvesArray)
            {
                AutomationCurve curve;
                varToAutomationCurve(c, curve);
                track.automationCurves.push_back(curve);
            }
        }
    }
    
    static juce::var clipToVar(const Clip& clip)
    {
        juce::DynamicObject* obj = new juce::DynamicObject();
        obj->setProperty("name", clip.name);
        obj->setProperty("startBeat", clip.startBeat);
        obj->setProperty("lengthBeats", clip.lengthBeats);
        obj->setProperty("isMidi", clip.isMidi);
        
        if (clip.isMidi)
        {
            juce::Array<juce::var> notesArray;
            auto& seq = clip.midiSequence;
            for (int i = 0; i < seq.getNumEvents(); ++i)
            {
                auto* event = seq.getEventPointer(i);
                if (event->message.isNoteOn())
                {
                    juce::DynamicObject* noteObj = new juce::DynamicObject();
                    noteObj->setProperty("pitch", event->message.getNoteNumber());
                    noteObj->setProperty("velocity", event->message.getVelocity());
                    noteObj->setProperty("start", event->message.getTimeStamp());
                    
                    // Find note off
                    double duration = 1.0; // Default
                    // Simple search for matching note off (not robust but ok for simple save)
                    for (int j = i + 1; j < seq.getNumEvents(); ++j)
                    {
                        auto* off = seq.getEventPointer(j);
                        if (off->message.isNoteOff() && off->message.getNoteNumber() == event->message.getNoteNumber())
                        {
                            duration = off->message.getTimeStamp() - event->message.getTimeStamp();
                            break;
                        }
                    }
                    noteObj->setProperty("duration", duration);
                    notesArray.add(juce::var(noteObj));
                }
            }
            obj->setProperty("notes", notesArray);
        }
        else
        {
            obj->setProperty("audioPath", clip.audioFile.getFullPathName());
        }
        
        return juce::var(obj);
    }
    
    static void varToClip(const juce::var& v, Clip& clip)
    {
        clip.name = v["name"].toString();
        clip.startBeat = (double)v["startBeat"];
        clip.lengthBeats = (double)v["lengthBeats"];
        clip.isMidi = (bool)v["isMidi"];
        
        if (clip.isMidi)
        {
            auto notesArray = v["notes"].getArray();
            if (notesArray)
            {
                for (auto& n : *notesArray)
                {
                    int pitch = (int)n["pitch"];
                    int vel = (int)n["velocity"];
                    double start = (double)n["start"];
                    double dur = (double)n["duration"];
                    
                    clip.midiSequence.addEvent(juce::MidiMessage::noteOn(1, pitch, (juce::uint8)vel), start);
                    clip.midiSequence.addEvent(juce::MidiMessage::noteOff(1, pitch), start + dur);
                }
                clip.midiSequence.updateMatchedPairs();
            }
        }
        else
        {
            clip.audioFile = juce::File(v["audioPath"].toString());
        }
    }
    
    static juce::var automationCurveToVar(const AutomationCurve& curve)
    {
        juce::DynamicObject* obj = new juce::DynamicObject();
        obj->setProperty("paramId", curve.parameterID);
        obj->setProperty("active", curve.active);
        
        juce::Array<juce::var> pointsArray;
        for (const auto& p : curve.points)
        {
            juce::DynamicObject* pObj = new juce::DynamicObject();
            pObj->setProperty("time", p.time);
            pObj->setProperty("value", p.value);
            pointsArray.add(juce::var(pObj));
        }
        obj->setProperty("points", pointsArray);
        return juce::var(obj);
    }
    
    static void varToAutomationCurve(const juce::var& v, AutomationCurve& curve)
    {
        curve.parameterID = v["paramId"].toString();
        curve.active = (bool)v["active"];
        
        auto pointsArray = v["points"].getArray();
        if (pointsArray)
        {
            for (auto& p : *pointsArray)
            {
                curve.points.push_back({ (double)p["time"], (float)p["value"] });
            }
        }
    }
};
