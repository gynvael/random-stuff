@echo off
php cm_3.cpp | g++ -x c++ - -o cm_3.exe
strip cm_3.exe
