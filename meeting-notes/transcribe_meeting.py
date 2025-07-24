import whisper
import math
import os

from pydub import AudioSegment
from tqdm import tqdm

# 필요한 라이브러리 설치:
# pip install pydub tqdm

model = whisper.load_model("medium")

AUDIO_FILE = "test.m4a"
SEGMENT_LENGTH = 60 * 1000  # 60초 (ms 단위)

# 오디오 불러오기
audio = AudioSegment.from_file(AUDIO_FILE)
duration_ms = len(audio)
total_segments = math.ceil(duration_ms / SEGMENT_LENGTH)

print(f"총 길이: {duration_ms/1000:.1f}초 ({total_segments} 조각으로 분할)")

final_text = ""

for i in tqdm(range(total_segments), desc="진행률", unit="조각"):
    start = i * SEGMENT_LENGTH
    end = min((i + 1) * SEGMENT_LENGTH, duration_ms)
    chunk = audio[start:end]
    chunk.export("temp_chunk.mp3", format="mp3")

    # Whisper로 해당 조각 변환
    result = model.transcribe("temp_chunk.mp3", language="ko")
    final_text += result["text"] + "\n"

# 임시 파일 삭제
if os.path.exists("temp_chunk.mp3"):
    os.remove("temp_chunk.mp3")

base_filename = os.path.splitext(os.path.basename(AUDIO_FILE))[0]
output_filename = f"{base_filename}-whisper-script.txt"

# 결과 저장
with open(output_filename, "w", encoding="utf-8") as f:
    f.write(final_text)

print(f"✅ 전체 회의 스크립트 저장 완료! → {output_filename}")
