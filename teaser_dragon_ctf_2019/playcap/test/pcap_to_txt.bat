"c:\Program Files\Wireshark\tshark.exe" -r playcap.pcap -Y "((usb.transfer_type == 0x01) && (frame.len == 91))" -e "frame.time_epoch" -e "usb.capdata" -Tfields > gamepad.txt
