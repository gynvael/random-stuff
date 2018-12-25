// Gynvael's attempt to dump the full 32KB ROM (E-DDC).
// Don't expect pretty code, this is an ad-hoc tool.
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void usage_and_exit() {
  puts("Usage: eddc <i2c_device> [--rand]\n"
       "E.g. : eddc /dev/i2c-3\n"
       "\n"
       "Options:\n"
       "  --rand            use single byte \"random\" access (by default\n"
       "                    sequential 256-byte-as-a-time access is used)\n"
       "\n"
       "Note: Output file is always called eddc_dump.bin");
  exit(1);
}

int main(int argc, char **argv) {
  puts("E-DDC/DisplayID ROM dumper (by Gynvael Coldwind)");

  if (argc != 2 && argc != 3) {
    usage_and_exit();
  }

  const char *dev_path = argv[1];

  bool seq_mode = true;
  if (argc == 3) {
    if (strcmp(argv[2], "--rand") == 0) {
      seq_mode = false;
    }
  }

  int i2c = open(dev_path, O_RDWR);
  if (i2c == -1) {
    perror("Failed to open I2C device");
    return 2;
  }

  int funcs = 0;
  if (ioctl(i2c, I2C_FUNCS, &funcs) < 0) {
    perror("Fail at I2C_FUNCS:");
    close(i2c);
    return 1;
  }

  printf("I2C function/feature word: %x\n", funcs);
  if (!(funcs & I2C_FUNC_I2C)) {
    puts("Warning, I2C_FUNC_I2C not present, that's not good. Trying anyway.");
  }

  FILE *f = fopen("eddc_dump.bin", "wb");
  if (f == nullptr) {
    perror("Failed to open eddc_dump.bin for writing");
    close(i2c);
    return 3;
  }

  for (int j = 0; j < 128; j++) {  // 32 KB in 256-byte segments.
    uint8_t segment = (uint8_t)j;
    uint8_t output[256]{};

    if (seq_mode) {
      uint8_t offset = 0;
      i2c_msg msg[] = {
          { 0x30, 0, 1, &segment },
          { 0x50, 0, 1, &offset },
          { 0x50, I2C_M_RD, sizeof(output), output }
      };

      i2c_rdwr_ioctl_data data;
      data.msgs = msg;
      data.nmsgs = sizeof(msg) / sizeof(*msg);

      if (ioctl(i2c, I2C_RDWR, &data) < 0) {
        perror("I2C_RDWR failed");
        puts("If you're using BCM2835 (e.g. Raspberry Pi) use i2c_gpio instead "
             " of the default i2c_bcm2835 module.");
        close(i2c);
        fclose(f);
        return 1;
      }
    } else {
      for (int i = 0; i < 256; i++) {
        uint8_t out;
        uint8_t offset = (uint8_t)i;

        i2c_msg msg[] = {
          { 0x30, 0, 1, &segment },
          { 0x50, 0, 1, &offset },
          { 0x50, I2C_M_RD, sizeof(out), &out }
        };

        i2c_rdwr_ioctl_data data;
        data.msgs = msg;
        data.nmsgs = sizeof(msg) / sizeof(*msg);

        if (ioctl(i2c, I2C_RDWR, &data) < 0) {
          perror("I2C_RDWR failed");
          puts("If you're using BCM2835 (e.g. Raspberry Pi) use i2c_gpio instead "
               " of the default i2c_bcm2835 module.");
          close(i2c);
          fclose(f);
          return 1;
        }

        output[i] = out;
      }
    }

    fwrite(output, 1, 256, f);
    fflush(f);
    putchar('.'); fflush(stdout);
  }
  putchar('\n');

  close(i2c);
  fclose(f);

  puts("Done!");
  puts("Note: If the first 256 bytes are repeated in the file and there is no "
       "other data, it might mean only 256 bytes are present and the segment "
       "register is a fake one or doesn't exist.");

  return 0;
}
