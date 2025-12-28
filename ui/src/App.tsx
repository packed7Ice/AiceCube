/**
 * AiceCube - Modern DAW Application
 * Simplified version for debugging
 */

import { useState, useEffect } from 'react';
import './App.css';

// Simplified TransportBar for testing
function TransportBar() {
  const [isPlaying, setIsPlaying] = useState(false);
  const [tempo, setTempo] = useState(120);
  
  return (
    <div className="transport-bar" style={{
      display: 'flex',
      alignItems: 'center',
      gap: '1rem',
      padding: '0.75rem 1.5rem',
      background: 'linear-gradient(180deg, #1e1e2e 0%, #181825 100%)',
      borderBottom: '1px solid #313244',
      height: '56px',
    }}>
      <button 
        onClick={() => setIsPlaying(!isPlaying)}
        style={{
          width: '40px',
          height: '40px',
          background: isPlaying ? '#a6e3a1' : '#313244',
          color: isPlaying ? '#1e1e2e' : '#cdd6f4',
          border: 'none',
          borderRadius: '6px',
          fontSize: '1.1rem',
          cursor: 'pointer',
        }}
      >
        {isPlaying ? '⏹' : '▶'}
      </button>
      
      <div style={{ color: '#cdd6f4' }}>
        <span style={{ fontSize: '0.75rem', color: '#6c7086' }}>BPM </span>
        <input
          type="number"
          value={tempo}
          onChange={(e) => setTempo(Number(e.target.value))}
          style={{
            width: '70px',
            padding: '0.4rem',
            background: '#11111b',
            border: '1px solid #313244',
            borderRadius: '4px',
            color: '#f9e2af',
            fontSize: '0.9rem',
            textAlign: 'center',
          }}
        />
      </div>
      
      <span style={{ color: '#a6e3a1', fontFamily: 'monospace' }}>
        1.1.00
      </span>
    </div>
  );
}

function App() {
  const [isConnected, setIsConnected] = useState(false);
  
  useEffect(() => {
    // Simple connection attempt
    const ws = new WebSocket('ws://localhost:9001');
    ws.onopen = () => setIsConnected(true);
    ws.onclose = () => setIsConnected(false);
    ws.onerror = () => setIsConnected(false);
    
    return () => ws.close();
  }, []);
  
  return (
    <div className="app" style={{
      display: 'flex',
      flexDirection: 'column',
      height: '100vh',
      background: '#1e1e2e',
    }}>
      {/* Transport Bar */}
      <TransportBar />
      
      {/* Main Content Area */}
      <div style={{
        display: 'flex',
        flex: 1,
        overflow: 'hidden',
      }}>
        {/* Track Headers */}
        <div style={{
          width: '200px',
          background: '#181825',
          borderRight: '1px solid #313244',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: '#6c7086',
        }}>
          トラック一覧
        </div>
        
        {/* Timeline */}
        <div style={{
          flex: 1,
          background: '#11111b',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: '#6c7086',
        }}>
          <div style={{ textAlign: 'center' }}>
            <div style={{ fontSize: '3rem', opacity: 0.3 }}>🎹</div>
            <div>タイムライン</div>
          </div>
        </div>
      </div>
      
      {/* Bottom Panel */}
      <div style={{
        height: '200px',
        background: '#181825',
        borderTop: '1px solid #313244',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        color: '#6c7086',
      }}>
        ミキサー / ブラウザ
      </div>
      
      {/* Connection Status */}
      <div style={{
        position: 'fixed',
        bottom: '12px',
        right: '12px',
        display: 'flex',
        alignItems: 'center',
        gap: '0.5rem',
        padding: '0.4rem 0.75rem',
        background: 'rgba(0, 0, 0, 0.6)',
        backdropFilter: 'blur(8px)',
        borderRadius: '20px',
        fontSize: '0.75rem',
      }}>
        <span style={{
          width: '8px',
          height: '8px',
          borderRadius: '50%',
          background: isConnected ? '#a6e3a1' : '#f38ba8',
          boxShadow: isConnected ? '0 0 8px #a6e3a1' : '0 0 8px #f38ba8',
        }} />
        <span style={{ color: '#a6adc8' }}>
          {isConnected ? 'Engine 接続済' : 'Engine に接続中...'}
        </span>
      </div>
    </div>
  );
}

export default App;
