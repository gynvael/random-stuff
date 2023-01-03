# AT+CPBR=1,10
# AT+CPBW=2, "222222", 129, "004100410041004101040106"
import sys

vcards = []

def encode_at(s):
  return ''.join(f"{ord(c):04x}" for c in s)

def decode_str(s):
  options, data = s.split(":")
  charset = "UTF-8"
  encoding = "PLAIN"
  for option in options.split(';'):
    if not option:
      continue
    k, v = option.split("=")
    if k == "CHARSET":
      chatset = v
    elif k == "ENCODING":
      encoding = v
    else:
      sys.exit("UNKNOWN_OPTION: " + s)

  if charset != "UTF-8":
    sys.exit("UNKNOWN_CHARSET: " + charset)

  if encoding == "PLAIN":
    return data

  if encoding == "QUOTED-PRINTABLE":
    d = []
    i = 0
    while i < len(data):
      if data[i] == '=':
        if data[i+1] == '\n':   # We can just skip =\n 
          i += 2
        else:  # =41 needs to be converted to a byte 0x41.
          d.append(int(data[i+1:i+3], 16))
          i += 3
      else:  # Just copy the character.
        d.append(ord(data[i]))
        i += 1
    return bytes(d).decode()

  sys.exit("UNKNOWN_ENCODING: " + encoding)


def process_vcard(lines):
  vcard = {}
  last = None

  for ln in lines:
    # Skip known fine to ignore lines.
    if (ln == "BEGIN:VCARD" or
        ln == "END:VCARD" or
        ln.startswith("VERSION") or
        ln.startswith("EMAIL;") or
        ln.startswith("PHOTO;") or
        ln.startswith("NOTE") or
        ln.startswith("ADR;")):
      last = None
      continue

    if ln.startswith("N:") or ln.startswith("N;"):
      last = "name"
      vcard["name"] = ln[1:]
      continue

    if ln.startswith("FN:") or ln.startswith("FN;"):
      last = "full_name"
      vcard["full_name"] = ln[2:]
      continue

    if ln.startswith("TEL;"):
      last = "tel"
      vcard["tel"] = ln.split(";", 1)[1]
      continue

    if ln.startswith("="):
      if last is None:
        continue  # Skip!
      vcard[last] += "\n" + ln
      continue
 
    sys.exit("UNKNOWN: " + ln)

  if "full_name" not in vcard:
    # Skip.
    #print(vcard)
    return None

  vcard["full_name"] = decode_str(vcard["full_name"])
  vcard["name"] = decode_str(vcard["name"])

  return vcard

with open("PIM00002.vcf") as f:
  vcard = []
  for ln in f:
    ln = ln.strip()
    vcard.append(ln)

    if "END:VCARD" in ln:
      v = process_vcard(vcard)
      if v is not None:
        vcards.append(v)
      vcard = []

n = 1
for vc in vcards:
  if "tel" not in vc:
    #print(vc)
    continue

  fn = vc["full_name"]
  if '/' in fn:
    fn, _ = fn.split('/')
    if _ not in 'WOMH':
      #print(_)
      pass

  while '  ' in fn:
    fn = fn.replace('  ', ' ')

  tel = vc["tel"]
  if tel.startswith("CELL:"):
    tel = tel[5:]

  # AT+CPBW=2, "222222", 129, "004100410041004101040106"

  # Truncate too long names.
  if len(fn) > 20:
    #print(fn[:20])
    fn = fn[:20]

  print(f'AT+CPBW={n}, "{tel}", 129, "{encode_at(fn)}"')

  n += 1


