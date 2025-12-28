/**
 * Timeline Canvas Renderer
 * Handles all Canvas 2D drawing for the timeline view
 */

import type { Track, Clip, TransportState } from '../types/daw';

// Theme colors
const COLORS = {
  background: '#11111b',
  gridMajor: '#313244',
  gridMinor: '#1e1e2e',
  playhead: '#f38ba8',
  loopRange: 'rgba(203, 166, 247, 0.15)',
  loopBorder: '#cba6f7',
  beatNumber: '#6c7086',
  clipDefault: '#89b4fa',
  clipSelected: '#f9e2af',
  clipBorder: 'rgba(255, 255, 255, 0.2)',
  trackSeparator: '#313244',
};

// Layout constants
const LAYOUT = {
  headerHeight: 24,       // Ruler height
  trackHeight: 80,        // Height per track
  minBeatWidth: 20,       // Minimum pixels per beat at zoom 1
  maxBeatWidth: 100,      // Maximum pixels per beat
};

export interface TimelineViewport {
  scrollX: number;        // Scroll position in pixels
  scrollY: number;
  zoom: number;           // Zoom level (1 = default)
  beatsPerBar: number;    // Time signature numerator
}

export interface TimelineRenderContext {
  ctx: CanvasRenderingContext2D;
  width: number;
  height: number;
  viewport: TimelineViewport;
  tracks: Track[];
  transport: TransportState;
  selectedTrackIndex: number;
  selectedClipId: string | null;
}

/**
 * Calculate pixels per beat based on zoom level
 */
export function getPixelsPerBeat(zoom: number): number {
  return LAYOUT.minBeatWidth * zoom;
}

/**
 * Convert beat position to pixel X position
 */
export function beatToPixelX(beat: number, viewport: TimelineViewport): number {
  const pxPerBeat = getPixelsPerBeat(viewport.zoom);
  return beat * pxPerBeat - viewport.scrollX;
}

/**
 * Convert pixel X position to beat
 */
export function pixelToBeat(pixelX: number, viewport: TimelineViewport): number {
  const pxPerBeat = getPixelsPerBeat(viewport.zoom);
  return (pixelX + viewport.scrollX) / pxPerBeat;
}

/**
 * Get track Y position and height
 */
export function getTrackBounds(trackIndex: number, viewport: TimelineViewport): { y: number; height: number } {
  const y = LAYOUT.headerHeight + trackIndex * LAYOUT.trackHeight - viewport.scrollY;
  return { y, height: LAYOUT.trackHeight };
}

/**
 * Main render function
 */
export function renderTimeline(context: TimelineRenderContext): void {
  const { ctx, width, height } = context;
  
  // Clear canvas
  ctx.fillStyle = COLORS.background;
  ctx.fillRect(0, 0, width, height);
  
  // Draw components
  drawGrid(context);
  drawLoopRange(context);
  drawTracks(context);
  drawClips(context);
  drawPlayhead(context);
  drawRuler(context);
}

/**
 * Draw background grid
 */
function drawGrid(context: TimelineRenderContext): void {
  const { ctx, width, height, viewport, transport } = context;
  const pxPerBeat = getPixelsPerBeat(viewport.zoom);
  const beatsPerBar = transport.timeSignatureNumerator;
  
  // Calculate visible beat range
  const startBeat = Math.floor(viewport.scrollX / pxPerBeat);
  const endBeat = Math.ceil((viewport.scrollX + width) / pxPerBeat);
  
  // Draw beat lines
  for (let beat = startBeat; beat <= endBeat; beat++) {
    const x = beatToPixelX(beat, viewport);
    const isBarLine = beat % beatsPerBar === 0;
    
    ctx.strokeStyle = isBarLine ? COLORS.gridMajor : COLORS.gridMinor;
    ctx.lineWidth = isBarLine ? 1 : 0.5;
    
    ctx.beginPath();
    ctx.moveTo(x, LAYOUT.headerHeight);
    ctx.lineTo(x, height);
    ctx.stroke();
  }
  
  // Draw track separators
  const { tracks } = context;
  ctx.strokeStyle = COLORS.trackSeparator;
  ctx.lineWidth = 1;
  
  for (let i = 0; i <= tracks.length; i++) {
    const { y } = getTrackBounds(i, viewport);
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(width, y);
    ctx.stroke();
  }
}

/**
 * Draw loop range highlight
 */
function drawLoopRange(context: TimelineRenderContext): void {
  const { ctx, height, viewport, transport } = context;
  
  if (!transport.isLooping) return;
  
  const startX = beatToPixelX(transport.loopStart, viewport);
  const endX = beatToPixelX(transport.loopEnd, viewport);
  
  // Fill loop region
  ctx.fillStyle = COLORS.loopRange;
  ctx.fillRect(startX, LAYOUT.headerHeight, endX - startX, height - LAYOUT.headerHeight);
  
  // Draw loop boundaries
  ctx.strokeStyle = COLORS.loopBorder;
  ctx.lineWidth = 2;
  ctx.setLineDash([4, 4]);
  
  ctx.beginPath();
  ctx.moveTo(startX, LAYOUT.headerHeight);
  ctx.lineTo(startX, height);
  ctx.moveTo(endX, LAYOUT.headerHeight);
  ctx.lineTo(endX, height);
  ctx.stroke();
  
  ctx.setLineDash([]);
}

/**
 * Draw track backgrounds
 */
function drawTracks(context: TimelineRenderContext): void {
  const { ctx, width, viewport, tracks, selectedTrackIndex } = context;
  
  tracks.forEach((_, index) => {
    const { y, height } = getTrackBounds(index, viewport);
    
    // Highlight selected track
    if (index === selectedTrackIndex) {
      ctx.fillStyle = 'rgba(137, 180, 250, 0.05)';
      ctx.fillRect(0, y, width, height);
    }
  });
}

/**
 * Draw clips on tracks
 */
function drawClips(context: TimelineRenderContext): void {
  const { ctx, viewport, tracks, selectedClipId } = context;
  
  tracks.forEach((track, trackIndex) => {
    const { y, height } = getTrackBounds(trackIndex, viewport);
    const clipHeight = height - 8; // Padding
    const clipY = y + 4;
    
    track.clips.forEach(clip => {
      const x = beatToPixelX(clip.startBeat, viewport);
      const clipWidth = clip.lengthBeats * getPixelsPerBeat(viewport.zoom);
      const isSelected = clip.id === selectedClipId;
      
      // Clip background
      ctx.fillStyle = clip.color || COLORS.clipDefault;
      roundRect(ctx, x, clipY, clipWidth, clipHeight, 4);
      ctx.fill();
      
      // Selection highlight
      if (isSelected) {
        ctx.strokeStyle = COLORS.clipSelected;
        ctx.lineWidth = 2;
        roundRect(ctx, x, clipY, clipWidth, clipHeight, 4);
        ctx.stroke();
      } else {
        ctx.strokeStyle = COLORS.clipBorder;
        ctx.lineWidth = 1;
        roundRect(ctx, x, clipY, clipWidth, clipHeight, 4);
        ctx.stroke();
      }
      
      // Clip name
      ctx.fillStyle = '#cdd6f4';
      ctx.font = '11px Inter, sans-serif';
      ctx.textBaseline = 'top';
      
      // Clip text with padding
      const textX = x + 6;
      const textY = clipY + 4;
      const maxTextWidth = clipWidth - 12;
      
      if (maxTextWidth > 20) {
        ctx.save();
        ctx.beginPath();
        ctx.rect(x, clipY, clipWidth, clipHeight);
        ctx.clip();
        ctx.fillText(clip.name, textX, textY, maxTextWidth);
        ctx.restore();
      }
      
      // Draw MIDI notes preview if clip has notes
      if (clip.isMidi && clip.notes && clip.notes.length > 0) {
        drawMidiPreview(ctx, clip, x, clipY, clipWidth, clipHeight, viewport);
      }
    });
  });
}

/**
 * Draw miniature MIDI notes in clip
 */
function drawMidiPreview(
  ctx: CanvasRenderingContext2D,
  clip: Clip,
  clipX: number,
  clipY: number,
  _clipWidth: number,
  clipHeight: number,
  viewport: TimelineViewport
): void {
  if (!clip.notes || clip.notes.length === 0) return;
  
  const pxPerBeat = getPixelsPerBeat(viewport.zoom);
  const noteAreaY = clipY + 18; // Below clip name
  const noteAreaHeight = clipHeight - 22;
  
  // Find pitch range
  const pitches = clip.notes.map(n => n.pitch);
  const minPitch = Math.min(...pitches);
  const maxPitch = Math.max(...pitches);
  const pitchRange = Math.max(maxPitch - minPitch, 12);
  
  ctx.fillStyle = 'rgba(255, 255, 255, 0.4)';
  
  clip.notes.forEach(note => {
    const noteX = clipX + (note.startBeat - clip.startBeat) * pxPerBeat;
    const noteWidth = Math.max(note.lengthBeats * pxPerBeat, 2);
    const noteY = noteAreaY + noteAreaHeight - (((note.pitch - minPitch) / pitchRange) * noteAreaHeight);
    const noteHeight = 3;
    
    ctx.fillRect(noteX, noteY, noteWidth, noteHeight);
  });
}

/**
 * Draw playhead
 */
function drawPlayhead(context: TimelineRenderContext): void {
  const { ctx, height, viewport, transport } = context;
  const x = beatToPixelX(transport.playheadBeat, viewport);
  
  // Playhead line
  ctx.strokeStyle = COLORS.playhead;
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(x, 0);
  ctx.lineTo(x, height);
  ctx.stroke();
  
  // Playhead triangle at top
  ctx.fillStyle = COLORS.playhead;
  ctx.beginPath();
  ctx.moveTo(x - 6, 0);
  ctx.lineTo(x + 6, 0);
  ctx.lineTo(x, 10);
  ctx.closePath();
  ctx.fill();
}

/**
 * Draw ruler (beat/bar numbers)
 */
function drawRuler(context: TimelineRenderContext): void {
  const { ctx, width, viewport, transport } = context;
  const pxPerBeat = getPixelsPerBeat(viewport.zoom);
  const beatsPerBar = transport.timeSignatureNumerator;
  
  // Ruler background
  ctx.fillStyle = '#181825';
  ctx.fillRect(0, 0, width, LAYOUT.headerHeight);
  
  // Calculate visible beat range
  const startBeat = Math.floor(viewport.scrollX / pxPerBeat);
  const endBeat = Math.ceil((viewport.scrollX + width) / pxPerBeat);
  
  ctx.fillStyle = COLORS.beatNumber;
  ctx.font = '10px Inter, sans-serif';
  ctx.textBaseline = 'middle';
  
  // Draw bar numbers
  for (let beat = startBeat; beat <= endBeat; beat++) {
    if (beat % beatsPerBar === 0) {
      const bar = Math.floor(beat / beatsPerBar) + 1;
      const x = beatToPixelX(beat, viewport);
      ctx.fillText(String(bar), x + 4, LAYOUT.headerHeight / 2);
    }
  }
}

/**
 * Helper: Draw rounded rectangle
 */
function roundRect(
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  width: number,
  height: number,
  radius: number
): void {
  ctx.beginPath();
  ctx.moveTo(x + radius, y);
  ctx.lineTo(x + width - radius, y);
  ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
  ctx.lineTo(x + width, y + height - radius);
  ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
  ctx.lineTo(x + radius, y + height);
  ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
  ctx.lineTo(x, y + radius);
  ctx.quadraticCurveTo(x, y, x + radius, y);
  ctx.closePath();
}

// Re-export layout constants for external use
export const TIMELINE_LAYOUT = LAYOUT;
