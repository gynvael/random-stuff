#!/usr/bin/python3

# Fair warning - this is a very simple hacked up script.
# Use at your own risk.
# And expect to have to fix stuff.

from google.cloud import texttospeech
import sys
import os

def show_usage_and_exit():
  sys.exit("usage: tts.py [-de|-en] <textfile>")

if len(sys.argv) not in {2, 3}:
  show_usage_and_exit()

VOICE_NAME = "en-US-Wavenet-F"
SPEAKING_RATE = 1.25

if len(sys.argv) == 3:
  if sys.argv[1] == "-de":
    VOICE_NAME = "de-DE-Wavenet-F"
    SPEAKING_RATE = 1.0
  elif sys.argv[1] == "-en":
    pass  # Default.
  else:
    show_usage_and_exit()

text_fname = sys.argv[-1]
print(f"Processing {text_fname}...")

output_file = os.path.splitext(os.path.basename(text_fname))[0] + ".mp3"
print(f"Will output file {output_file}")

def text_to_mp3(fname, text):
  language_code = "-".join(VOICE_NAME.split("-")[:2])

  text_input = texttospeech.SynthesisInput(
      text=text
  )

  voice_params = texttospeech.VoiceSelectionParams(
      language_code=language_code, name=VOICE_NAME
  )

  audio_config = texttospeech.AudioConfig(
      audio_encoding=texttospeech.AudioEncoding.MP3,
      speaking_rate=SPEAKING_RATE
  )

  # TODO: Perhaps it's enough to keep a reference to the client?
  client = texttospeech.TextToSpeechClient()

  response = client.synthesize_speech(
      input=text_input,
      voice=voice_params,
      audio_config=audio_config
  )

  with open(fname, "wb") as out:
    out.write(response.audio_content)
    print(f'Audio content written to "{fname}"')


with open(text_fname, "r", encoding="utf-8") as t:
  text = t.read()

text = iter(text.splitlines())

f = open("parts.txt", "w")

part = 1
while True:
  t = ""
  final = False
  while len(t) < 5000:
    prev_t = t
    t += "\n"
    try:
      t += next(text)
    except StopIteration:
      final = True
      break

  if len(prev_t.strip()) == 0:
    break

  if len(prev_t) == 0 and not final:
    sys.exit("handle this case where the line is more than 5k...")

  fname = "part_%.3i.mp3" % part
  f.write(f"file '{fname}'\n")

  text_to_mp3(fname, prev_t)
  part += 1
  t = ""

f.close()

print("Merging...")

# TODO: Change this to subprocess.something later on.
os.system(f"ffmpeg -y -f concat -i parts.txt -c copy {output_file}")

# TODO: Remove all create part_%.3i.mp3 files and parts.txt.

print("Done: output.mp3")

