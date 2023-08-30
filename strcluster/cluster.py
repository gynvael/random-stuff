#!/usr/bin/python
# Lame string clustering by Gynvael Coldwind.
import sys
from Levenshtein import distance

def usage_and_exit():
  sys.exit("usage: cluster.py <fname> [--cutoff=7]")

fname = None
CUTOFF = 7

for argv in sys.argv[1:]:
  if argv in {'-h', '--help'}:
    usage_and_exit()

  if argv.startswith('--cutoff='):
    CUTOFF = int(argv.split('=')[-1])
    continue

  if fname is None:
    fname = argv
    continue

  usage_and_exit()

def print_next(w):
  if len(w) == 0:
    return None

  main_word = w.pop()
  similar = [ main_word ]

  i = 0
  while i < len(w):
    candidate_word = w[i]
    if distance(main_word, candidate_word) < CUTOFF:
      w.pop(i)
      similar.append(candidate_word)
    else:
      i += 1

  return similar

if fname:
  with open(sys.argv[1]) as f:
    words = list(set(f.read().splitlines()))
else:
  words = list(set(sys.stdin.read().splitlines()))

while True:
  s = print_next(words)
  if s is None:
    break
  print(s)
