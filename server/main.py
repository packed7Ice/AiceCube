from fastapi import FastAPI
from pydantic import BaseModel
from typing import List
from dataclasses import dataclass
import random

app = FastAPI()


# =========================
# リクエスト / レスポンスモデル
# =========================

class GenerateRequest(BaseModel):
    action: str = "generate"
    role: str = "melody"   # "melody" / "bass" / "drums" / "chords" / "pad" / "fx"
    tempo: float = 120.0
    barsStart: int = 1
    barsEnd: int = 4
    style: str = ""        # "dark" / "lofi" / "energetic" など
    density: int = 3       # 1〜5 くらいを想定


class Note(BaseModel):
    pitch: int      # MIDIノート番号
    start: float    # 開始位置（小節単位ではなく "拍" 単位で扱う想定）
    duration: float
    velocity: int   # 1〜127


class Sequence(BaseModel):
    notes: List[Note]


# =========================
# 曲全体のコンテキスト（コード進行など）
# =========================

@dataclass
class ChordDef:
    root: int          # MIDIノート番号
    quality: str       # "maj" / "min" / "dim" など
    duration_bars: float


@dataclass
class SongContext:
    key_root: int      # 例: C=60, ここではC3=48など
    scale: str         # "major" / "minor"
    tempo: float
    chords: List[ChordDef]
    bars_start: int
    bars_end: int


def generate_chord_progression(style: str, bars: int, key_root: int) -> List[ChordDef]:
    """
    スタイルに応じた簡易コード進行を生成する。
    非常にシンプルな「雰囲気用」実装。
    """
    chords: List[ChordDef] = []

    # degree は「キーからの半音オフセット」として超ざっくり扱う
    if style in ("dark", "tense", "ambient"):
        # i - VI - VII - iv っぽい雰囲気
        pattern = [
            (0, "min"),
            (5, "min"),
            (7, "maj"),
            (3, "min"),
        ]
        scale_type = "minor"
    else:
        # I - V - vi - IV 的な王道
        pattern = [
            (0, "maj"),
            (7, "maj"),
            (9, "min"),
            (5, "maj"),
        ]
        scale_type = "major"

    bars_per_chord = max(1, bars // len(pattern))
    used_bars = 0

    for degree, quality in pattern:
        if used_bars >= bars:
            break
        root = key_root + degree
        chords.append(ChordDef(root=root, quality=quality, duration_bars=bars_per_chord))
        used_bars += bars_per_chord

    # 足りなかったら最後のコードで埋める
    while used_bars < bars and chords:
        last = chords[-1]
        chords.append(ChordDef(root=last.root, quality=last.quality, duration_bars=1))
        used_bars += 1

    return chords, scale_type


def build_song_context(req: GenerateRequest) -> SongContext:
    """
    リクエストから SongContext（キー, スケール, コード進行）を構築する。
    """
    total_bars = req.barsEnd - req.barsStart + 1

    # キーは雑に C をベースにしつつ、スタイルによってオクターブを変えるなどしても良い
    # ここでは C3 (48) をベースにする
    key_root = 48

    chords, scale_type = generate_chord_progression(req.style, total_bars, key_root)

    return SongContext(
        key_root=key_root,
        scale=scale_type,
        tempo=req.tempo,
        chords=chords,
        bars_start=req.barsStart,
        bars_end=req.barsEnd,
    )


# =========================
# 共通ユーティリティ
# =========================

def humanize_notes(notes: List[Note], timing_amount: float = 0.02, vel_amount: int = 8) -> List[Note]:
    """
    タイミングとベロシティを少しランダムにずらして、人間味を出す。
    timing_amount: 開始位置の最大ずれ（拍単位）
    vel_amount   : ベロシティ変化の最大値
    """
    out: List[Note] = []
    for n in notes:
        start = n.start + random.uniform(-timing_amount, timing_amount)
        vel = n.velocity + random.randint(-vel_amount, vel_amount)
        vel = max(1, min(127, vel))

        out.append(
            Note(
                pitch=n.pitch,
                start=start,
                duration=n.duration,
                velocity=vel,
            )
        )
    return out


def minor_scale(root: int) -> List[int]:
    # ハーモニックマイナー的な例
    intervals = [0, 2, 3, 5, 7, 8, 11]
    return [root + i for i in intervals]


def major_scale(root: int) -> List[int]:
    intervals = [0, 2, 4, 5, 7, 9, 11]
    return [root + i for i in intervals]


# =========================
# 各パート生成
# =========================

def generate_drums(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    notes: List[Note] = []

    num_bars = ctx.bars_end - ctx.bars_start + 1
    density = max(1, min(5, req.density))

    # barsStart に応じたオフセット（拍単位）
    bar_offset_beats = (ctx.bars_start - 1) * 4.0

    kick = 36
    snare = 38
    hihat = 42

    for bar in range(num_bars):
        bar_start = bar_offset_beats + bar * 4.0

        # Kick: 4つ打ち
        for beat in [0.0, 1.0, 2.0, 3.0]:
            notes.append(Note(pitch=kick, start=bar_start + beat, duration=0.1, velocity=100))

        # Snare: 2,4拍
        for beat in [1.0, 3.0]:
            notes.append(Note(pitch=snare, start=bar_start + beat, duration=0.1, velocity=110))

        # Hihat: density に応じて細かさを変える
        if density <= 2:
            step = 1.0   # 4分
        elif density == 3:
            step = 0.5   # 8分
        else:
            step = 0.25  # 16分

        t = 0.0
        while t < 4.0:
            notes.append(Note(pitch=hihat, start=bar_start + t, duration=0.05, velocity=70))
            t += step

    return notes


def generate_bass(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    notes: List[Note] = []
    density = max(1, min(5, req.density))

    bar_offset_beats = (ctx.bars_start - 1) * 4.0

    cur_bar = ctx.bars_start
    current_beat = bar_offset_beats

    for chord in ctx.chords:
        bars_for_chord = int(chord.duration_bars)
        for i in range(bars_for_chord):
            bar_start = current_beat

            # コードルートを2オクターブ下げてベース音域へ
            root = chord.root - 24
            fifth = root + 7

            # 1拍目: root
            notes.append(Note(pitch=root, start=bar_start + 0.0, duration=0.9, velocity=90))

            # density に応じて他の拍に音を足す
            if density >= 3:
                notes.append(Note(pitch=fifth, start=bar_start + 2.0, duration=0.9, velocity=85))
            if density >= 4:
                notes.append(Note(pitch=root, start=bar_start + 1.0, duration=0.5, velocity=80))
            if density >= 5:
                notes.append(Note(pitch=fifth, start=bar_start + 3.0, duration=0.5, velocity=80))

            current_beat += 4.0
            cur_bar += 1

    return notes


def generate_melody(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    notes: List[Note] = []

    total_bars = ctx.bars_end - ctx.bars_start + 1
    total_beats = total_bars * 4.0
    start_beat = (ctx.bars_start - 1) * 4.0

    # スケール選択
    if ctx.scale == "minor":
        scale = minor_scale(ctx.key_root + 12)  # 1オクターブ上でメロディ
    else:
        scale = major_scale(ctx.key_root + 12)

    # 簡単なモチーフ（4音）を作る
    motif = []
    t = 0.0
    for _ in range(4):
        pitch = random.choice(scale)
        length = random.choice([0.25, 0.5, 0.75])
        motif.append((pitch, length))
        t += length

    cur = start_beat
    end = start_beat + total_beats

    while cur < end:
        use_motif = motif.copy()

        # たまにトランスポーズしてバリエーション
        if random.random() < 0.3:
            transpose = random.choice([-2, 2])
            use_motif = [(p + transpose, d) for (p, d) in use_motif]

        for (pitch, length) in use_motif:
            if cur >= end:
                break
            notes.append(
                Note(
                    pitch=pitch,
                    start=cur,
                    duration=length,
                    velocity=100,
                )
            )
            cur += length

    return notes


def generate_chords(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    """
    SongContext の chords をそのまま鳴らす。
    """
    notes: List[Note] = []
    bar_offset_beats = (ctx.bars_start - 1) * 4.0

    current_beat = bar_offset_beats

    for chord in ctx.chords:
        bars_for_chord = int(chord.duration_bars)
        duration_beats = bars_for_chord * 4.0

        # ごく単純な3和音
        # メジャー: root, M3, P5 / マイナー: root, m3, P5
        if chord.quality == "maj":
            intervals = [0, 4, 7]
        elif chord.quality == "min":
            intervals = [0, 3, 7]
        else:
            intervals = [0, 3, 6]  # dim 的な例

        for interval in intervals:
            pitch = chord.root + interval
            notes.append(
                Note(
                    pitch=pitch,
                    start=current_beat,
                    duration=duration_beats,
                    velocity=70,
                )
            )

        current_beat += duration_beats

    return notes


def generate_pad(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    """
    chords をベースに、少し高い音域で長めのサステインを鳴らす。
    """
    notes: List[Note] = []
    bar_offset_beats = (ctx.bars_start - 1) * 4.0
    current_beat = bar_offset_beats

    for chord in ctx.chords:
        bars_for_chord = int(chord.duration_bars)
        duration_beats = bars_for_chord * 4.0

        # 1オクターブ上で鳴らす
        if chord.quality == "maj":
            intervals = [12, 16, 19]
        elif chord.quality == "min":
            intervals = [12, 15, 19]
        else:
            intervals = [12, 15, 18]

        for interval in intervals:
            pitch = chord.root + interval
            notes.append(
                Note(
                    pitch=pitch,
                    start=current_beat,
                    duration=duration_beats,
                    velocity=60,
                )
            )

        current_beat += duration_beats

    return notes

def generate_fx(req: GenerateRequest, ctx: SongContext) -> List[Note]:
    """
    とりあえずシンプルなクラッシュ＋上昇音など。
    """
    notes: List[Note] = []
    start_beat = (ctx.bars_start - 1) * 4.0

    # Crash
    notes.append(
        Note(
            pitch=49,  # Crash Cymbal
            start=start_beat,
            duration=4.0,
            velocity=110,
        )
    )

    # 簡単なライザー的なもの
    end_beat = (ctx.bars_end) * 4.0
    steps = 8
    for i in range(steps):
        t = start_beat + (end_beat - start_beat) * (i / steps)
        pitch = 72 + i  # C5 から上昇
        notes.append(Note(pitch=pitch, start=t, duration=0.2, velocity=90))

    return notes

# =========================
# FastAPI エンドポイント
# =========================

@app.post("/generate")
def generate(req: GenerateRequest):
    print(f"Received request: {req}")

    ctx = build_song_context(req)

    if req.role == "drums":
        notes = generate_drums(req, ctx)
    elif req.role == "bass":
        notes = generate_bass(req, ctx)
    elif req.role == "melody":
        notes = generate_melody(req, ctx)
    elif req.role == "chords":
        notes = generate_chords(req, ctx)
    elif req.role == "pad":
        notes = generate_pad(req, ctx)
    else:
        notes = generate_fx(req, ctx)

    # 今のところ action は細かく使っていないが、
    # 将来的に "clear" などを追加する余地を残しておく。
    # ここでは一律 humanize をかける（FX も含めて問題なければこのままでOK）
    notes = humanize_notes(notes)

    return {"notes": notes}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
