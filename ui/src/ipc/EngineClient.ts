/**
 * WebSocket client for Engine IPC communication
 */

export type MessageType = 'command' | 'state' | 'event' | 'response';

export interface Command {
  type: 'command';
  id: string;
  command: string;
  payload: Record<string, unknown>;
}

export interface StateUpdate {
  type: 'state';
  scope: string;
  data: Record<string, unknown>;
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

export interface EngineResponse {
  type: 'response';
  id: string;
  success: boolean;
  data?: Record<string, unknown>;
  error?: string;
}

type MessageHandler = (message: StateUpdate | EngineResponse) => void;

class EngineClient {
  private ws: WebSocket | null = null;
  private url: string;
  private messageHandlers: Set<MessageHandler> = new Set();
  private reconnectInterval: number = 2000;
  private isConnecting: boolean = false;
  private pendingCommands: Map<string, (response: EngineResponse) => void> = new Map();
  
  constructor(url: string = 'ws://localhost:9001') {
    this.url = url;
  }
  
  connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      if (this.ws?.readyState === WebSocket.OPEN) {
        resolve();
        return;
      }
      
      if (this.isConnecting) {
        // Wait for existing connection attempt
        const checkConnection = setInterval(() => {
          if (this.ws?.readyState === WebSocket.OPEN) {
            clearInterval(checkConnection);
            resolve();
          }
        }, 100);
        return;
      }
      
      this.isConnecting = true;
      
      try {
        this.ws = new WebSocket(this.url);
        
        this.ws.onopen = () => {
          console.log('[IPC] Connected to Engine');
          this.isConnecting = false;
          resolve();
        };
        
        this.ws.onclose = () => {
          console.log('[IPC] Disconnected from Engine');
          this.isConnecting = false;
          this.scheduleReconnect();
        };
        
        this.ws.onerror = (error) => {
          console.error('[IPC] WebSocket error:', error);
          this.isConnecting = false;
          reject(error);
        };
        
        this.ws.onmessage = (event) => {
          try {
            const message = JSON.parse(event.data);
            this.handleMessage(message);
          } catch (e) {
            console.error('[IPC] Failed to parse message:', e);
          }
        };
      } catch (e) {
        this.isConnecting = false;
        reject(e);
      }
    });
  }
  
  private scheduleReconnect(): void {
    setTimeout(() => {
      console.log('[IPC] Attempting to reconnect...');
      this.connect().catch(() => {
        // Will retry via onclose handler
      });
    }, this.reconnectInterval);
  }
  
  private handleMessage(message: StateUpdate | EngineResponse): void {
    // Check if it's a response to a pending command
    if (message.type === 'response' && this.pendingCommands.has(message.id)) {
      const handler = this.pendingCommands.get(message.id)!;
      this.pendingCommands.delete(message.id);
      handler(message);
    }
    
    // Notify all handlers
    this.messageHandlers.forEach(handler => handler(message));
  }
  
  onMessage(handler: MessageHandler): () => void {
    this.messageHandlers.add(handler);
    return () => this.messageHandlers.delete(handler);
  }
  
  async sendCommand(command: string, payload: Record<string, unknown> = {}): Promise<EngineResponse> {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      await this.connect();
    }
    
    const id = crypto.randomUUID();
    const cmd: Command = {
      type: 'command',
      id,
      command,
      payload,
    };
    
    return new Promise((resolve, reject) => {
      this.pendingCommands.set(id, resolve);
      
      // Timeout after 10 seconds
      setTimeout(() => {
        if (this.pendingCommands.has(id)) {
          this.pendingCommands.delete(id);
          reject(new Error('Command timeout'));
        }
      }, 10000);
      
      this.ws!.send(JSON.stringify(cmd));
    });
  }
  
  // Convenience methods for transport commands
  async play(): Promise<void> {
    await this.sendCommand('transport.play');
  }
  
  async stop(): Promise<void> {
    await this.sendCommand('transport.stop');
  }
  
  async setTempo(tempo: number): Promise<void> {
    await this.sendCommand('transport.setTempo', { tempo });
  }
  
  async setPlayhead(beat: number): Promise<void> {
    await this.sendCommand('transport.setPlayhead', { beat });
  }
  
  async setLoop(enabled: boolean, start?: number, end?: number): Promise<void> {
    await this.sendCommand('transport.setLoop', { enabled, start, end });
  }
  
  async toggleMetronome(): Promise<void> {
    await this.sendCommand('transport.toggleMetronome');
  }
  
  get isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN;
  }
  
  disconnect(): void {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }
}

// Singleton instance
export const engineClient = new EngineClient();
