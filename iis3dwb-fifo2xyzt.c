#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define TEMP_FILE       "fifo.bin"
#define TEMP_FILE_META  "fifo.bin.meta"

void quits() {
  exit(EXIT_SUCCESS);
}

void quitf() {
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  uint32_t duration;
  uint64_t nsamples;
  double fs;
  double odr;
  double afactor;
  double tfactor;

  FILE *fp = fopen(TEMP_FILE_META, "rb");
  fread(&duration, sizeof(uint32_t), 1, fp);
  fread(&nsamples, sizeof(uint64_t), 1, fp);
  fread(&fs, sizeof(double), 1, fp);
  fread(&odr, sizeof(double), 1, fp);
  fread(&afactor, sizeof(double), 1, fp);
  fread(&tfactor, sizeof(double), 1, fp);
  fclose(fp);

  printf("duration:	%u\n", duration);
  printf("nsamples:	%lu\n", nsamples);
  printf("fs:		%lf\n", fs);
  printf("odr:		%lf\n", odr);
  printf("afactor:	%lf\n", afactor);
  printf("tfactor:	%lf\n", tfactor);

  fp = fopen(TEMP_FILE, "rb");
  fseek(fp, 0, SEEK_END);
  const int64_t fifo_bin_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  bool error = false;
  
  if ((fifo_bin_size % 7) == 0) {

    // this is not == but >=
    // because the sensor read is not hard limited by nsamples
    // it might collect and eventually a little more samples than nsamples
    // might be written to the file, it is trimmed below
    if ((fifo_bin_size / 7) >= nsamples) {

      printf("%s looks OK. Processing...\n", TEMP_FILE);

      FILE *fpx = fopen("x.bin", "wb");
      FILE *fpy = fopen("y.bin", "wb");
      FILE *fpz = fopen("z.bin", "wb");
      FILE *fpt = fopen("t.bin", "wb");

      uint8_t sample[7];
      uint32_t t0 = 0;
      uint32_t timestamp = 0;
      bool last_acc = true;

      // reading nsamples times
      // this might not read all the file (see the comment above)
      for (uint64_t i = 0; i < nsamples; i++) {

        if (fread(sample, sizeof(uint8_t), 7, fp) == 7) {

          uint8_t tag = sample[0];
          uint8_t tag_sensor = (tag >> 3);

          // accelerometer
          if (tag_sensor == 0x02) {

            if (last_acc) continue;

            double x = afactor * ((int16_t) ((sample[2] << 8) | sample[1]));
            double y = afactor * ((int16_t) ((sample[4] << 8) | sample[3]));
            double z = afactor * ((int16_t) ((sample[6] << 8) | sample[5]));
            double t = tfactor * (timestamp - t0);

            fwrite(&x, sizeof(double), 1, fpx);
            fwrite(&y, sizeof(double), 1, fpy);
            fwrite(&z, sizeof(double), 1, fpz);
            fwrite(&t, sizeof(double), 1, fpt);

            last_acc = true;

          // temperature
          } else if (tag_sensor == 0x03) {

            // temperature data is not collected
            printf("ERROR. temperature data encountered in FIFO.\n");
            error = true;
            break;

          // timestamp
          } else if (tag_sensor == 0x04) {

            timestamp = (sample[4] << 24) | 
                        (sample[3] << 16) |
                        (sample[2] << 8) | 
                         sample[1];

            if (t0 == 0) {
              t0 = timestamp;
            }

            last_acc = false;

          } else {

            printf("ERROR. invalid tag_sensor value:  0x%02X\n", tag_sensor);
            error = true;
            break;

          }

        } else {

          printf("ERROR. fread failed.\n");
          error = true;
          break;

        }

      }

      fclose(fpt);
      fclose(fpz);
      fclose(fpy);
      fclose(fpx);
      fclose(fp);

    } else {

      printf("ERROR. %s contains less samples than required %lu\n", 
          TEMP_FILE, 
          nsamples);
      error = true;

    }

  } else {

    printf("ERROR. The size of %s is invalid, not a multiple of 7.\n", 
        TEMP_FILE);
    error = true;

  }
  
  if (error) {

    printf("ERROR. Do not use the output files.\n");
    quitf();

  } else {

    printf("SUCCESS. x,y,z,t files are created.\n");
    quits();

  }

}
