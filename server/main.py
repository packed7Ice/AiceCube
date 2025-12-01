from fastapi import FastAPI
from pydantic import BaseModel
from typing import List, Optional
import random

app = FastAPI()

class GenerateRequest(BaseModel):
    action: str = "generate"
    role: str = "melody"
    tempo: float = 120.0
    barsStart: int = 1
    barsEnd: int = 4
    style: str = ""
    density: int = 3

class Note(BaseModel):
    pitch: int
    start: float
    duration: float
    velocity: int

class Sequence(BaseModel):
    notes: List[Note]

def generate_drums(req: GenerateRequest) -> List[Note]:
    notes = []
    length = (req.barsEnd - req.barsStart + 1) * 4.0
    
    # Simple Kick/Snare pattern
    for i in range(int(length)):
        # Kick on 1, 3
        if i % 4 == 0 or i % 4 == 2.5: # Simple syncopation
            notes.append(Note(pitch=36, start=float(i), duration=0.25, velocity=100))
        # Snare on 2, 4
        if i % 4 == 2:
            notes.append(Note(pitch=38, start=float(i), duration=0.25, velocity=90))
        # Hihats 8th notes
        notes.append(Note(pitch=42, start=float(i), duration=0.25, velocity=80))
        notes.append(Note(pitch=42, start=float(i)+0.5, duration=0.25, velocity=70))
        
    return notes

def generate_bass(req: GenerateRequest) -> List[Note]:
    notes = []
    length = (req.barsEnd - req.barsStart + 1) * 4.0
    root = 36 # C2
    
    # Simple bass line
    for i in range(int(length)):
        if i % 2 == 0:
            pitch = root if i % 4 == 0 else root + 7 # Root and Fifth
            if req.style == "dark": pitch = root + 3 # Minor third
            notes.append(Note(pitch=pitch, start=float(i), duration=0.5, velocity=100))
            
    return notes

def generate_melody(req: GenerateRequest) -> List[Note]:
    notes = []
    length = (req.barsEnd - req.barsStart + 1) * 4.0
    scale = [60, 62, 64, 65, 67, 69, 71, 72] # C Major
    if req.style == "dark": scale = [60, 62, 63, 65, 67, 68, 70, 72] # C Minor
    
    current_time = 0.0
    while current_time < length:
        dur = random.choice([0.5, 1.0, 0.25])
        if current_time + dur > length: break
        
        pitch = random.choice(scale)
        notes.append(Note(pitch=pitch, start=current_time, duration=dur, velocity=90))
        current_time += dur
        
    return notes

def generate_chords(req: GenerateRequest) -> List[Note]:
    notes = []
    length = (req.barsEnd - req.barsStart + 1) * 4.0
    
    # C - G - Am - F
    progression = [[60, 64, 67], [55, 59, 62], [57, 60, 64], [53, 57, 60]]
    if req.style == "dark": # Cm - Fm - G - Cm
        progression = [[60, 63, 67], [53, 56, 60], [55, 59, 62], [60, 63, 67]]

    for bar in range(req.barsEnd - req.barsStart + 1):
        chord = progression[bar % 4]
        start = float(bar * 4)
        for p in chord:
            notes.append(Note(pitch=p, start=start, duration=4.0, velocity=70))
            
    return notes

def generate_pad(req: GenerateRequest) -> List[Note]:
    # Similar to chords but longer, maybe different voicing
    return generate_chords(req)

def generate_fx(req: GenerateRequest) -> List[Note]:
    notes = []
    # Just a crash at start
    notes.append(Note(pitch=49, start=0.0, duration=4.0, velocity=100))
    return notes

@app.post("/generate")
def generate(req: GenerateRequest):
    print(f"Received request: {req}")
    
    notes = []
    if req.role == "drums":
        notes = generate_drums(req)
    elif req.role == "bass":
        notes = generate_bass(req)
    elif req.role == "melody":
        notes = generate_melody(req)
    elif req.role == "chords":
        notes = generate_chords(req)
    elif req.role == "pad":
        notes = generate_pad(req)
    else:
        notes = generate_fx(req)
        
    return {"notes": notes}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
