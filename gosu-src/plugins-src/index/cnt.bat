@echo off
for %%i IN ( *.c *.h ) DO type %%i >> x.
type x. | find /V /C ""
del x > nul