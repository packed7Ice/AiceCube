/**
 * Track Headers Component - Track list with controls
 */

import type { Track, TrackType } from '../types/daw';
import './TrackHeaders.css';

interface TrackHeadersProps {
  tracks: Track[];
  selectedTrackIndex: number;
  onSelectTrack: (index: number) => void;
  onAddTrack: (type: TrackType) => void;
  onDeleteTrack: (index: number) => void;
  onRenameTrack: (index: number, name: string) => void;
  onToggleMute: (index: number) => void;
  onToggleSolo: (index: number) => void;
  onToggleArm: (index: number) => void;
}

export function TrackHeaders({
  tracks,
  selectedTrackIndex,
  onSelectTrack,
  onAddTrack,
  onDeleteTrack,
  onRenameTrack,
  onToggleMute,
  onToggleSolo,
  onToggleArm,
}: TrackHeadersProps) {
  
  const handleNameDoubleClick = (index: number, currentName: string) => {
    const newName = prompt('トラック名を入力:', currentName);
    if (newName && newName !== currentName) {
      onRenameTrack(index, newName);
    }
  };
  
  return (
    <div className="track-headers">
      {/* Header area (matches ruler height) */}
      <div className="track-headers-ruler">
        <button 
          className="add-track-btn"
          onClick={() => onAddTrack('midi')}
          title="MIDIトラックを追加"
        >
          + MIDI
        </button>
        <button 
          className="add-track-btn audio"
          onClick={() => onAddTrack('audio')}
          title="オーディオトラックを追加"
        >
          + Audio
        </button>
      </div>
      
      {/* Track list */}
      <div className="track-headers-list">
        {tracks.map((track, index) => (
          <div
            key={track.id}
            className={`track-header ${selectedTrackIndex === index ? 'selected' : ''}`}
            onClick={() => onSelectTrack(index)}
          >
            {/* Color indicator */}
            <div 
              className="track-color-indicator"
              style={{ backgroundColor: track.color }}
            />
            
            {/* Track info */}
            <div className="track-info">
              <span 
                className="track-name"
                onDoubleClick={() => handleNameDoubleClick(index, track.name)}
              >
                {track.name}
              </span>
              <span className="track-type">
                {track.type === 'midi' ? '🎹' : track.type === 'audio' ? '🎵' : '🔊'}
              </span>
            </div>
            
            {/* Track controls */}
            <div className="track-controls">
              <button
                className={`track-btn mute ${track.mute ? 'active' : ''}`}
                onClick={(e) => { e.stopPropagation(); onToggleMute(index); }}
                title="ミュート"
              >
                M
              </button>
              <button
                className={`track-btn solo ${track.solo ? 'active' : ''}`}
                onClick={(e) => { e.stopPropagation(); onToggleSolo(index); }}
                title="ソロ"
              >
                S
              </button>
              <button
                className={`track-btn arm ${track.arm ? 'active' : ''}`}
                onClick={(e) => { e.stopPropagation(); onToggleArm(index); }}
                title="録音アーム"
              >
                R
              </button>
            </div>
            
            {/* Delete button */}
            <button
              className="track-delete-btn"
              onClick={(e) => { e.stopPropagation(); onDeleteTrack(index); }}
              title="トラックを削除"
            >
              ×
            </button>
          </div>
        ))}
        
        {/* Empty state */}
        {tracks.length === 0 && (
          <div className="track-headers-empty">
            <span>トラックがありません</span>
            <span className="hint">上のボタンでトラックを追加</span>
          </div>
        )}
      </div>
    </div>
  );
}
