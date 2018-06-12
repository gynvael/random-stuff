// Link this against kernel32.dll directly.
#include <windows.h>
#include <stdio.h>
#include <vector>

extern "C" DWORD GetConsoleCommandHistoryLengthA(char *exename);
extern "C" DWORD GetConsoleCommandHistoryA(VOID *buffer, DWORD buffer_len, char *exename);

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: history <exename>");
    return 1;
  }

  DWORD len = GetConsoleCommandHistoryLengthA(argv[1]);

  if (len == 0) {
    fprintf(stderr, "no history found\n");
    return 2;
  }

  printf("History length: %u\n", (unsigned int)len);

  std::vector<char> buffer(len + 1);
  DWORD ret = GetConsoleCommandHistoryA(&buffer[0], len, argv[1]);
  // ret should be equal to len if all went fine.
  // buffer will contain a series of NUL-terminated strings.

  if (ret != len) {
    fprintf(stderr, "returned length doesn't match initially reported one\n");
    // Continue.
  }

  // Probably the slowest way to print it ;)
  for (size_t i = 0; i < buffer.size(); i++) {
    if (buffer[i] == '\0') {
      putchar('\n');
      continue;
    }
    putchar(buffer[i]);
  }

  return 0;
}
