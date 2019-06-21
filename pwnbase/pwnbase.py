#!/usr/bin/python
# Helper CTF script by Gynvael Coldwind
# IMPORTANT: Just use pwntools, seriously.
import random
import sys
import socket
import telnetlib
import os
import time
import threading
from struct import pack, unpack
from time import time

from keystone import *
from capstone import *
from unicorn import *
from unicorn.x86_const import *

ks = Ks(KS_ARCH_X86, KS_MODE_64)
cs = Cs(CS_ARCH_X86, CS_MODE_64)
uc = Uc(UC_ARCH_X86, UC_MODE_64)

REGS = [
    ( "rip", UC_X86_REG_RIP),
    ( "rax", UC_X86_REG_RAX),
    ( "rbx", UC_X86_REG_RBX),
    ( "rcx", UC_X86_REG_RCX),
    ( "rdx", UC_X86_REG_RDX),
    ( "rsi", UC_X86_REG_RSI),
    ( "rdi", UC_X86_REG_RDI),
    ( "rbp", UC_X86_REG_RBP),
    ( "rsp", UC_X86_REG_RSP),
    ( "r8" , UC_X86_REG_R8),
    ( "r9" , UC_X86_REG_R9),
    ( "r10", UC_X86_REG_R10),
    ( "r11", UC_X86_REG_R11),
    ( "r12", UC_X86_REG_R12),
    ( "r13", UC_X86_REG_R13),
    ( "r14", UC_X86_REG_R14),
    ( "r15", UC_X86_REG_R15),
]

REGS_MAP = dict(REGS)

class Regs(object):
  def __init__(self, uc):
    self.d = {}
    self.uc = uc

  def __getattr__(self, name):
    if name in self.d:
      return self.d[name]

    if name in REGS_MAP:
      r = self.uc.reg_read(REGS_MAP[name])
      self.d[name] = r
      return r

    raise AttributeError()

  def __setattr__(self, name, value):
    if name not in REGS_MAP:
      return super(Regs, self).__setattr__(name, value)

    self.d[name] = value
    self.uc.reg_write(REGS_MAP[name], value)


def recvuntil(sock, txt):
  d = ""
  while d.find(txt) == -1:
    try:
      dnow = sock.recv(1)
      if len(dnow) == 0:
        return ("DISCONNECTED", d)
    except socket.timeout:
      return ("TIMEOUT", d)
    except socket.error as msg:
      return ("ERROR", d)
    d += dnow
  return ("OK", d)

def recvall(sock, n):
  d = ""
  while len(d) != n:
    try:
      dnow = sock.recv(n - len(d))
      if len(dnow) == 0:
        return ("DISCONNECTED", d)
    except socket.timeout:
      return ("TIMEOUT", d)
    except socket.error as msg:
      return ("ERROR", d)
    d += dnow
  return ("OK", d)

# Proxy object for sockets.
class gsocket(object):
  def __init__(self, *p):
    self._sock = socket.socket(*p)

  def __getattr__(self, name):
    return getattr(self._sock, name)

  def recvall(self, n):
    err, ret = recvall(self._sock, n)
    if err != "OK":
      return False
    return ret

  def recvuntil(self, txt):
    err, ret = recvuntil(self._sock, txt)
    if err != "OK":
      return False
    return ret

# Base for any of my ROPs.
def db(v):
  return pack("<B", v)

def dw(v):
  return pack("<H", v)

def dd(v):
  return pack("<I", v)

def dq(v):
  return pack("<Q", v)

def rb(v):
  return unpack("<B", v[0])[0]

def rw(v):
  return unpack("<H", v[:2])[0]

def rd(v):
  return unpack("<I", v[:4])[0]

def rq(v):
  return unpack("<Q", v[:8])[0]

def go():
  global HOST
  global PORT
  s = gsocket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect((HOST, PORT))

  # Put your code here!

  # Interactive sockets.
  t = telnetlib.Telnet()
  t.sock = s
  t.interact()

  # Python console.
  # Note: you might need to modify ReceiverClass if you want
  #       to parse incoming packets.
  #ReceiverClass(s).start()
  #dct = locals()
  #for k in globals().keys():
  #  if k not in dct:
  #    dct[k] = globals()[k]
  #code.InteractiveConsole(dct).interact()

  s.close()

HOST = '127.0.0.1'
PORT = 13337
go()


