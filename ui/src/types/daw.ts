/**
 * Type definitions for the DAW state
 */

export interface Note {
  id: string;
  pitch: number;     // MIDI pitch (0-127)
  startBeat: number;
  lengthBeats: number;
  velocity: number;  // 0-127
}

export interface Clip {
  id: string;
  trackId: string;
  name: string;
  startBeat: number;
  lengthBeats: number;
  isMidi: boolean;
  color: string;     // CSS color
  notes?: Note[];    // For MIDI clips
  audioFile?: string; // For audio clips
  gain: number;
  fadeIn: number;
  fadeOut: number;
}

export type TrackType = 'midi' | 'audio' | 'bus' | 'master';

export interface Track {
  id: string;
  type: TrackType;
  name: string;
  color: string;
  volume: number;    // 0.0 - 2.0
  pan: number;       // -1.0 to 1.0
  mute: boolean;
  solo: boolean;
  arm: boolean;
  clips: Clip[];
}

export interface TransportState {
  isPlaying: boolean;
  playheadBeat: number;
  tempo: number;
  isLooping: boolean;
  loopStart: number;
  loopEnd: number;
  metronomeEnabled: boolean;
  timeSignatureNumerator: number;
  timeSignatureDenominator: number;
}

export interface ProjectState {
  tracks: Track[];
  selectedTrackIndex: number;
  transport: TransportState;
}

// Default values
export const defaultTransportState: TransportState = {
  isPlaying: false,
  playheadBeat: 0,
  tempo: 120,
  isLooping: false,
  loopStart: 0,
  loopEnd: 16,
  metronomeEnabled: false,
  timeSignatureNumerator: 4,
  timeSignatureDenominator: 4,
};

export const createDefaultClip = (trackId: string, startBeat: number): Clip => ({
  id: crypto.randomUUID(),
  trackId,
  name: 'New Clip',
  startBeat,
  lengthBeats: 4,
  isMidi: true,
  color: '#89b4fa',
  notes: [],
  gain: 1,
  fadeIn: 0,
  fadeOut: 0,
});

export const createDefaultTrack = (type: TrackType = 'midi'): Track => ({
  id: crypto.randomUUID(),
  type,
  name: type === 'midi' ? 'MIDI Track' : type === 'audio' ? 'Audio Track' : 'Track',
  color: '#89b4fa',
  volume: 1,
  pan: 0,
  mute: false,
  solo: false,
  arm: false,
  clips: [],
});

// Color palette for tracks
export const TRACK_COLORS = [
  '#f38ba8', // Red
  '#fab387', // Peach
  '#f9e2af', // Yellow
  '#a6e3a1', // Green
  '#94e2d5', // Teal
  '#89dceb', // Sky
  '#89b4fa', // Blue
  '#cba6f7', // Mauve
  '#f5c2e7', // Pink
];
