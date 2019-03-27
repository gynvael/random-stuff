set breakpoint pending on
break ps_unicodes_char_index
r /usr/share/fonts/type1/texlive-fonts-recommended/pcrr8a.pfb `cat letter` > /dev/null

break *0x7FFFF7BA9B07
break *0x7FFFF7BA9B15
break *0x7FFFF7BA9B46
break *0x7FFFF7BA9B20
break *0x7FFFF7BA9B4F
break *0x7FFFF7BA9B25
break *0x7FFFF7BA9B54
break *0x7FFFF7BA9B29
break *0x7FFFF7BA9B57
break *0x7FFFF7BA9B60
break *0x7FFFF7BA9B32
break *0x7FFFF7BA9B65
break *0x7FFFF7BA9B70
break *0x7FFFF7BA9B78

display/i $rip

break *0x7FFFF7BA9B69
commands
quit
end

break *0x7FFFF7BA9B77
commands
quit
end

break *0x7FFFF7BA9B7A
commands
quit
end

while 1
  c
end


