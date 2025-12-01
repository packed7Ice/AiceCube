from fastapi import FastAPI
from pydantic import BaseModel
from typing import List, Optional

app = FastAPI()

class AgentCommandRequest(BaseModel):
    type: str
    track: int
    barsStart: int
    barsEnd: int
    mood: str
    intensity: float
    extra: Optional[str] = None

class Note(BaseModel):
    pitch: int
    start: float
    duration: float
    velocity: int

class Sequence(BaseModel):
    notes: List[Note]

@app.post("/generate_melody", response_model=Sequence)
async def generate_melody(cmd: AgentCommandRequest):
    print(f"Received command: {cmd}")
    
    # Dummy logic for now
    notes = []
    
    # Create a simple C major scale or similar based on mood
    base_pitch = 60 # C4
    if cmd.mood == "dark":
        base_pitch = 57 # A3 (Minor-ish feel if we stick to natural minor scale intervals, but let's just shift down)
    
    # Generate 4 notes per bar
    num_bars = cmd.barsEnd - cmd.barsStart + 1
    total_beats = num_bars * 4
    
    for i in range(total_beats):
        note = Note(
            pitch=base_pitch + (i % 12), # Just climbing up chromatic for test
            start=float(i),
            duration=0.5,
            velocity=100
        )
        notes.append(note)
        
    return Sequence(notes=notes)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
