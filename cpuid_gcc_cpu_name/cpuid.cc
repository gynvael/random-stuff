// Works on Windows / Linux with GCC / MinGW GCC.
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cpuid.h>

static void GetCPUInfoPart(unsigned int leaf, void* buffer) {
  uint8_t *buf = (uint8_t*)buffer;
  __get_cpuid(leaf,
      (unsigned int*)buf,
      (unsigned int*)(buf + 4),
      (unsigned int*)(buf + 8),
      (unsigned int*)(buf + 12)
  );
}

std::string GetCPUInfo() {
  char buffer[0x38]{};
  GetCPUInfoPart(0x80000002, buffer);
  GetCPUInfoPart(0x80000003, buffer + 0x10);
  GetCPUInfoPart(0x80000004, buffer + 0x20);
  return buffer;
}

int main() {
  std::string cpu_info = GetCPUInfo();
  puts(cpu_info.c_str());
}
