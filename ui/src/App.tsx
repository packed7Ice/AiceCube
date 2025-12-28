/**
 * AiceCube - Modern DAW Application
 * Incremental test with Timeline component
 */

import { useState } from 'react';
import { Timeline } from './views/Timeline';
import type { Track, TransportState } from './types/daw';
import { defaultTransportState, TRACK_COLORS } from './types/daw';
import './App.css';

// Create simple demo data
const createDemoTracks = (): Track[] => [
  {
    id: 'track-1',
    type: 'midi',
    name: 'Demo Track 1',
    color: TRACK_COLORS[6],
    volume: 1,
    pan: 0,
    mute: false,
    solo: false,
    arm: false,
    clips: [
      {
        id: 'clip-1',
        trackId: 'track-1',
        name: 'Intro',
        startBeat: 0,
        lengthBeats: 4,
        isMidi: true,
        color: TRACK_COLORS[6],
        notes: [
          { id: '1', pitch: 60, startBeat: 0, lengthBeats: 1, velocity: 100 },
          { id: '2', pitch: 64, startBeat: 1, lengthBeats: 1, velocity: 100 },
          { id: '3', pitch: 67, startBeat: 2, lengthBeats: 1, velocity: 100 },
          { id: '4', pitch: 72, startBeat: 3, lengthBeats: 1, velocity: 100 },
        ],
        gain: 1,
        fadeIn: 0,
        fadeOut: 0,
      },
      {
        id: 'clip-2',
        trackId: 'track-1',
        name: 'Verse',
        startBeat: 8,
        lengthBeats: 8,
        isMidi: true,
        color: TRACK_COLORS[3],
        notes: [],
        gain: 1,
        fadeIn: 0,
        fadeOut: 0,
      },
    ],
  },
  {
    id: 'track-2',
    type: 'midi',
    name: 'Demo Track 2',
    color: TRACK_COLORS[7],
    volume: 1,
    pan: 0,
    mute: false,
    solo: false,
    arm: false,
    clips: [],
  },
];

function App() {
  const [tracks] = useState<Track[]>(createDemoTracks);
  const [transport, setTransport] = useState<TransportState>(defaultTransportState);
  const [selectedTrackIndex, setSelectedTrackIndex] = useState(0);
  const [selectedClipId, setSelectedClipId] = useState<string | null>(null);
  const [isPlaying, setIsPlaying] = useState(false);
  
  const handlePlay = () => {
    setIsPlaying(true);
    setTransport(prev => ({ ...prev, isPlaying: true }));
  };
  
  const handleStop = () => {
    setIsPlaying(false);
    setTransport(prev => ({ ...prev, isPlaying: false }));
  };
  
  return (
    <div className="app">
      {/* Transport Bar */}
      <div className="transport-bar">
        <div className="transport-section controls">
          <button
            className={`transport-btn ${!isPlaying ? 'active' : ''}`}
            onClick={handleStop}
          >
            ⏹
          </button>
          <button
            className={`transport-btn play ${isPlaying ? 'active' : ''}`}
            onClick={handlePlay}
          >
            ▶
          </button>
        </div>
        <div className="transport-section">
          <span style={{ color: '#cdd6f4' }}>BPM: {transport.tempo}</span>
        </div>
      </div>
      
      {/* Main Content */}
      <div className="main-content">
        {/* Track Headers placeholder */}
        <div className="track-headers-panel" style={{ 
          width: '200px', 
          background: '#181825',
          borderRight: '1px solid #313244',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: '#6c7086'
        }}>
          Track Headers
        </div>
        
        {/* Timeline */}
        <div className="timeline-panel">
          <Timeline
            tracks={tracks}
            transport={transport}
            selectedTrackIndex={selectedTrackIndex}
            selectedClipId={selectedClipId}
            onSelectTrack={setSelectedTrackIndex}
            onSelectClip={setSelectedClipId}
            onAddClip={(trackIndex, startBeat) => console.log('Add clip', trackIndex, startBeat)}
            onMoveClip={(clipId, trackIndex, startBeat) => console.log('Move clip', clipId, trackIndex, startBeat)}
            onSetPlayhead={(beat) => setTransport(prev => ({ ...prev, playheadBeat: beat }))}
          />
        </div>
      </div>
      
      {/* Bottom Panel */}
      <div className="bottom-panel">
        <span className="placeholder-text">Bottom Panel</span>
      </div>
    </div>
  );
}

export default App;
