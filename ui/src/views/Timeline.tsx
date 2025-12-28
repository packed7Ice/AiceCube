/**
 * Timeline Component - Canvas-based arrangement view
 */

import { useRef, useEffect, useState, useCallback } from 'react';
import { 
  renderTimeline, 
  pixelToBeat, 
  getTrackBounds,
  TIMELINE_LAYOUT,
} from '../render/TimelineCanvas';
import type { TimelineViewport } from '../render/TimelineCanvas';
import type { Track, TransportState } from '../types/daw';
import './Timeline.css';

interface TimelineProps {
  tracks: Track[];
  transport: TransportState;
  selectedTrackIndex: number;
  selectedClipId: string | null;
  onSelectTrack: (index: number) => void;
  onSelectClip: (clipId: string | null) => void;
  onAddClip: (trackIndex: number, startBeat: number) => void;
  onMoveClip: (clipId: string, trackIndex: number, startBeat: number) => void;
  onSetPlayhead: (beat: number) => void;
}

export function Timeline({
  tracks,
  transport,
  selectedTrackIndex,
  selectedClipId,
  onSelectTrack,
  onSelectClip,
  onAddClip,
  onMoveClip,
  onSetPlayhead,
}: TimelineProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const [viewport, setViewport] = useState<TimelineViewport>({
    scrollX: 0,
    scrollY: 0,
    zoom: 1,
    beatsPerBar: transport.timeSignatureNumerator,
  });
  
  // Dragging state
  const [isDragging, setIsDragging] = useState(false);
  const [dragType, setDragType] = useState<'clip' | 'playhead' | 'scroll' | null>(null);
  const [dragStart, setDragStart] = useState({ x: 0, y: 0 });
  const [dragClipId, setDragClipId] = useState<string | null>(null);
  const [dragClipOriginal, setDragClipOriginal] = useState<{ trackIndex: number; startBeat: number } | null>(null);
  
  // Canvas rendering
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    
    // Handle DPI scaling
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    ctx.scale(dpr, dpr);
    
    // Render
    renderTimeline({
      ctx,
      width: rect.width,
      height: rect.height,
      viewport,
      tracks,
      transport,
      selectedTrackIndex,
      selectedClipId,
    });
  }, [viewport, tracks, transport, selectedTrackIndex, selectedClipId]);
  
  // Handle resize
  useEffect(() => {
    const container = containerRef.current;
    if (!container) return;
    
    const observer = new ResizeObserver(() => {
      // Trigger re-render by updating a dummy state
      setViewport(prev => ({ ...prev }));
    });
    
    observer.observe(container);
    return () => observer.disconnect();
  }, []);
  
  // Find clip at position
  const findClipAt = useCallback((x: number, y: number): { clip: { id: string; trackIndex: number } | null } => {
    const beat = pixelToBeat(x, viewport);
    
    for (let i = 0; i < tracks.length; i++) {
      const { y: trackY, height: trackHeight } = getTrackBounds(i, viewport);
      
      if (y >= trackY && y < trackY + trackHeight) {
        for (const clip of tracks[i].clips) {
          if (beat >= clip.startBeat && beat < clip.startBeat + clip.lengthBeats) {
            return { clip: { id: clip.id, trackIndex: i } };
          }
        }
      }
    }
    
    return { clip: null };
  }, [tracks, viewport]);
  
  // Find track at Y position
  const findTrackAt = useCallback((y: number): number => {
    for (let i = 0; i < tracks.length; i++) {
      const { y: trackY, height: trackHeight } = getTrackBounds(i, viewport);
      if (y >= trackY && y < trackY + trackHeight) {
        return i;
      }
    }
    return -1;
  }, [tracks, viewport]);
  
  // Mouse handlers
  const handleMouseDown = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    const rect = canvasRef.current?.getBoundingClientRect();
    if (!rect) return;
    
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    
    // Check if clicking on ruler (playhead)
    if (y < TIMELINE_LAYOUT.headerHeight) {
      const beat = Math.max(0, pixelToBeat(x, viewport));
      onSetPlayhead(beat);
      setIsDragging(true);
      setDragType('playhead');
      setDragStart({ x, y });
      return;
    }
    
    // Check if clicking on a clip
    const { clip } = findClipAt(x, y);
    if (clip) {
      onSelectClip(clip.id);
      onSelectTrack(clip.trackIndex);
      
      // Start dragging
      const track = tracks[clip.trackIndex];
      const clipData = track.clips.find(c => c.id === clip.id);
      if (clipData) {
        setIsDragging(true);
        setDragType('clip');
        setDragClipId(clip.id);
        setDragClipOriginal({ trackIndex: clip.trackIndex, startBeat: clipData.startBeat });
        setDragStart({ x, y });
      }
      return;
    }
    
    // Check if clicking on a track
    const trackIndex = findTrackAt(y);
    if (trackIndex >= 0) {
      onSelectTrack(trackIndex);
      onSelectClip(null);
    }
    
    // Middle mouse button for scrolling
    if (e.button === 1) {
      setIsDragging(true);
      setDragType('scroll');
      setDragStart({ x, y });
    }
  }, [viewport, findClipAt, findTrackAt, onSelectClip, onSelectTrack, onSetPlayhead, tracks]);
  
  const handleMouseMove = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (!isDragging) return;
    
    const rect = canvasRef.current?.getBoundingClientRect();
    if (!rect) return;
    
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    
    if (dragType === 'playhead') {
      const beat = Math.max(0, pixelToBeat(x, viewport));
      onSetPlayhead(beat);
    } else if (dragType === 'scroll') {
      const dx = x - dragStart.x;
      const dy = y - dragStart.y;
      setViewport(prev => ({
        ...prev,
        scrollX: Math.max(0, prev.scrollX - dx),
        scrollY: Math.max(0, prev.scrollY - dy),
      }));
      setDragStart({ x, y });
    } else if (dragType === 'clip' && dragClipId && dragClipOriginal) {
      // Clip dragging is handled on mouse up
    }
  }, [isDragging, dragType, dragStart, viewport, dragClipId, dragClipOriginal, onSetPlayhead]);
  
  const handleMouseUp = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (isDragging && dragType === 'clip' && dragClipId && dragClipOriginal) {
      const rect = canvasRef.current?.getBoundingClientRect();
      if (rect) {
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        
        const newTrackIndex = findTrackAt(y);
        const newStartBeat = Math.max(0, Math.round(pixelToBeat(x, viewport) * 4) / 4); // Snap to 16th notes
        
        if (newTrackIndex >= 0) {
          onMoveClip(dragClipId, newTrackIndex, newStartBeat);
        }
      }
    }
    
    setIsDragging(false);
    setDragType(null);
    setDragClipId(null);
    setDragClipOriginal(null);
  }, [isDragging, dragType, dragClipId, dragClipOriginal, findTrackAt, viewport, onMoveClip]);
  
  // Double-click to add clip
  const handleDoubleClick = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    const rect = canvasRef.current?.getBoundingClientRect();
    if (!rect) return;
    
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    
    // Don't add clips in ruler area
    if (y < TIMELINE_LAYOUT.headerHeight) return;
    
    const trackIndex = findTrackAt(y);
    if (trackIndex >= 0) {
      const beat = Math.max(0, Math.floor(pixelToBeat(x, viewport)));
      onAddClip(trackIndex, beat);
    }
  }, [findTrackAt, viewport, onAddClip]);
  
  // Zoom with mouse wheel
  const handleWheel = useCallback((e: React.WheelEvent<HTMLCanvasElement>) => {
    if (e.ctrlKey || e.metaKey) {
      // Zoom
      e.preventDefault();
      const zoomDelta = e.deltaY > 0 ? 0.9 : 1.1;
      setViewport(prev => ({
        ...prev,
        zoom: Math.max(0.5, Math.min(4, prev.zoom * zoomDelta)),
      }));
    } else {
      // Scroll
      setViewport(prev => ({
        ...prev,
        scrollX: Math.max(0, prev.scrollX + e.deltaX),
        scrollY: Math.max(0, prev.scrollY + e.deltaY),
      }));
    }
  }, []);
  
  return (
    <div ref={containerRef} className="timeline-container">
      <canvas
        ref={canvasRef}
        className="timeline-canvas"
        onMouseDown={handleMouseDown}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
        onMouseLeave={handleMouseUp}
        onDoubleClick={handleDoubleClick}
        onWheel={handleWheel}
      />
      
      {/* Zoom controls */}
      <div className="timeline-zoom-controls">
        <button onClick={() => setViewport(prev => ({ ...prev, zoom: Math.min(4, prev.zoom * 1.2) }))}>
          +
        </button>
        <span>{Math.round(viewport.zoom * 100)}%</span>
        <button onClick={() => setViewport(prev => ({ ...prev, zoom: Math.max(0.5, prev.zoom * 0.8) }))}>
          −
        </button>
      </div>
    </div>
  );
}
