with open("gamepad.txt") as f:
  lines = f.read().splitlines()

last_buttons = None
for ln in lines:
  tm, data = ln.split('\t')[:2]
  tm = float(tm)
  if tm < 1568814371.669264:
    continue
  data = bytearray(bytes.fromhex(data))
  buttons = data[3:6]
  if buttons != last_buttons:
    last_buttons = buttons

    action = None
    if buttons[0] & 0x02:
      action = 'reset'
    elif buttons[2] & 0x04:
      action = 'right'
    elif buttons[0] & 0x08:
      action = 'select'
    elif buttons[2] & 0x01:
      action = 'down'
    elif buttons[2] & 0x02:
      action = 'up'
    elif buttons[2] & 0x08:
      action = 'left'

    if action:
      print("handleButtons('%s', true);" % action)
    #print("%10.6f" % tm, action)


