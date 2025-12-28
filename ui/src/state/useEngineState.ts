/**
 * State management for Engine connection
 */

import { useState, useEffect, useCallback } from 'react';
import { engineClient, TransportState, StateUpdate, EngineResponse } from '../ipc/EngineClient';

// Default transport state
const defaultTransportState: TransportState = {
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

export function useEngineConnection() {
  const [isConnected, setIsConnected] = useState(false);
  const [connectionError, setConnectionError] = useState<string | null>(null);
  
  useEffect(() => {
    const connect = async () => {
      try {
        await engineClient.connect();
        setIsConnected(true);
        setConnectionError(null);
      } catch (e) {
        setConnectionError('Engine に接続できません');
        setIsConnected(false);
      }
    };
    
    connect();
    
    // Check connection status periodically
    const interval = setInterval(() => {
      setIsConnected(engineClient.isConnected);
    }, 1000);
    
    return () => {
      clearInterval(interval);
    };
  }, []);
  
  return { isConnected, connectionError };
}

export function useTransportState() {
  const [transport, setTransport] = useState<TransportState>(defaultTransportState);
  
  useEffect(() => {
    const unsubscribe = engineClient.onMessage((message) => {
      if (message.type === 'state' && (message as StateUpdate).scope === 'transport') {
        setTransport(prev => ({
          ...prev,
          ...(message as StateUpdate).data as Partial<TransportState>,
        }));
      }
      
      // Also handle transport data in response
      if (message.type === 'response' && (message as EngineResponse).success) {
        const data = (message as EngineResponse).data;
        if (data && 'isPlaying' in data) {
          setTransport(prev => ({
            ...prev,
            ...data as Partial<TransportState>,
          }));
        }
      }
    });
    
    return unsubscribe;
  }, []);
  
  const play = useCallback(async () => {
    try {
      await engineClient.play();
    } catch (e) {
      console.error('Play failed:', e);
    }
  }, []);
  
  const stop = useCallback(async () => {
    try {
      await engineClient.stop();
    } catch (e) {
      console.error('Stop failed:', e);
    }
  }, []);
  
  const setTempo = useCallback(async (tempo: number) => {
    try {
      await engineClient.setTempo(tempo);
    } catch (e) {
      console.error('Set tempo failed:', e);
    }
  }, []);
  
  const setPlayhead = useCallback(async (beat: number) => {
    try {
      await engineClient.setPlayhead(beat);
    } catch (e) {
      console.error('Set playhead failed:', e);
    }
  }, []);
  
  const toggleLoop = useCallback(async () => {
    try {
      await engineClient.setLoop(!transport.isLooping, transport.loopStart, transport.loopEnd);
    } catch (e) {
      console.error('Toggle loop failed:', e);
    }
  }, [transport.isLooping, transport.loopStart, transport.loopEnd]);
  
  const toggleMetronome = useCallback(async () => {
    try {
      await engineClient.toggleMetronome();
    } catch (e) {
      console.error('Toggle metronome failed:', e);
    }
  }, []);
  
  return {
    transport,
    play,
    stop,
    setTempo,
    setPlayhead,
    toggleLoop,
    toggleMetronome,
  };
}
