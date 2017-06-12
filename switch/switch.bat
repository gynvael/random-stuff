@echo off
:: A simple environment switcher by Gynvael Coldwind (assume MIT license).
:: Make sure that you use the full path to the Python executable here, as well
:: as the full path to the switch-worker.py and switch-worker-output.bat.
d:\bin\Python2712_64\python.exe d:\commands\switch-worker.py %*
call d:\commands\switch-worker-output.bat
