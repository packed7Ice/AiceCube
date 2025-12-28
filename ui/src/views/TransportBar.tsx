/**
 * TransportBar - Transport controls (Play/Stop/Record/Tempo/Loop)
 */

import { useTransportState } from '../state/useEngineState';
import './TransportBar.css';

export function TransportBar() {
  const {
    transport,
    play,
    stop,
    setTempo,
    setPlayhead,
    toggleLoop,
    toggleMetronome,
  } = useTransportState();
  
  // Format playhead position as bars:beats
  const formatPosition = (beat: number) => {
    const bar = Math.floor(beat / transport.timeSignatureNumerator) + 1;
    const beatInBar = Math.floor(beat % transport.timeSignatureNumerator) + 1;
    const ticks = Math.floor((beat % 1) * 100);
    return `${bar}.${beatInBar}.${ticks.toString().padStart(2, '0')}`;
  };
  
  const handleTempoChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value) && value >= 20 && value <= 999) {
      setTempo(value);
    }
  };
  
  const handleRewind = () => {
    setPlayhead(0);
  };
  
  return (
    <div className="transport-bar">
      {/* Position Display */}
      <div className="transport-section position-section">
        <div className="position-display">
          <span className="position-label">位置</span>
          <span className="position-value">{formatPosition(transport.playheadBeat)}</span>
        </div>
      </div>
      
      {/* Transport Controls */}
      <div className="transport-section controls-section">
        <button 
          className="transport-btn rewind-btn" 
          onClick={handleRewind}
          title="先頭に戻る"
        >
          ⏮
        </button>
        
        <button
          className={`transport-btn stop-btn ${!transport.isPlaying ? 'active' : ''}`}
          onClick={stop}
          title="停止"
        >
          ⏹
        </button>
        
        <button
          className={`transport-btn play-btn ${transport.isPlaying ? 'active' : ''}`}
          onClick={play}
          title="再生"
        >
          ▶
        </button>
        
        <button
          className="transport-btn record-btn"
          title="録音"
        >
          ⏺
        </button>
      </div>
      
      {/* Tempo Section */}
      <div className="transport-section tempo-section">
        <label className="tempo-label">BPM</label>
        <input
          type="number"
          className="tempo-input"
          value={transport.tempo}
          onChange={handleTempoChange}
          min={20}
          max={999}
          step={1}
        />
      </div>
      
      {/* Time Signature */}
      <div className="transport-section time-sig-section">
        <span className="time-sig">
          {transport.timeSignatureNumerator}/{transport.timeSignatureDenominator}
        </span>
      </div>
      
      {/* Loop Toggle */}
      <div className="transport-section loop-section">
        <button
          className={`transport-btn loop-btn ${transport.isLooping ? 'active' : ''}`}
          onClick={toggleLoop}
          title="ループ"
        >
          🔁
        </button>
        {transport.isLooping && (
          <span className="loop-range">
            {Math.floor(transport.loopStart)} - {Math.floor(transport.loopEnd)}
          </span>
        )}
      </div>
      
      {/* Metronome Toggle */}
      <div className="transport-section metronome-section">
        <button
          className={`transport-btn metronome-btn ${transport.metronomeEnabled ? 'active' : ''}`}
          onClick={toggleMetronome}
          title="メトロノーム"
        >
          🔔
        </button>
      </div>
    </div>
  );
}
