#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mount.h>
#include <sys/types.h>
#include <dirent.h>

FILE* cmd_pipe;

#define NEWLINE  fprintf(cmd_pipe, "ui_print\n")

void list_dir(const char* path) {
  DIR *dir;
  struct dirent *ent;

  fprintf(cmd_pipe, "ui_print ls %s\n", path); NEWLINE;
  sleep(1);
  if ((dir = opendir(path)) == NULL) {
    fprintf(cmd_pipe, "ui_print failed to open directory\n"); NEWLINE;
    sleep(5);
    return;
  }
  
  int i = 0;
  while ((ent = readdir (dir)) != NULL) {
    fprintf(cmd_pipe, "ui_print %s\n", ent->d_name); NEWLINE;
    //usleep(250000);
    sleep(1);
  }
  closedir (dir);
}

void list_file(const char* file) {
  FILE* fp = fopen(file, "r");
  char* line = 0;
  size_t len = 0;
  ssize_t read;
  
  if (!fp) {
    fprintf(cmd_pipe, "ui_print failed to open file\n"); NEWLINE;
    return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if (line[0] == '#') continue;
    fprintf(cmd_pipe, "ui_print %s\n", line); NEWLINE;
    //usleep(500000);
    sleep(2);
  }
  fclose(fp);
  if (line) free(line);
}

void dump_file(const char* input, const char* output, const char* name) {
  FILE* fin = fopen(input, "r");
  
  fprintf(cmd_pipe, "ui_print dump %s -> %s\n", input, output); NEWLINE;
  if (!fin) {
    fprintf(cmd_pipe, "ui_print reading faild from %s\n", input); NEWLINE;
    return;
  }

  FILE* fout = fopen(output, "wb");
  if (!fout) {
    fprintf(cmd_pipe, "ui_print writing failed to %s\n", output); NEWLINE;
    fclose(fin);
    return;
  }

  size_t size = 0;
  char buffer[128 * 1024];
  int in = 0;
  int out = 0;
  
  while ((size = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
    in += size;
    out += fwrite(buffer, 1, size, fout);
    if ((in % 10) == 0) {
      fprintf(cmd_pipe, "ui_print %s %d %d\n", name, in, out); NEWLINE;
    }
  }
  fprintf(cmd_pipe, "ui_print %s %d %d\n", name, in, out); NEWLINE;

  fclose(fin);
  fclose(fout);
}

void dump_dir(const char* path) {
  DIR *dir;
  struct dirent *ent;

  fprintf(cmd_pipe, "ui_print dump files from %s\n", path); NEWLINE;
  sleep(1);
  if ((dir = opendir(path)) == NULL) {
    fprintf(cmd_pipe, "ui_print failed to open directory\n"); NEWLINE;
    sleep(5);
    return;
  }
  
  int i = 0;
  while ((ent = readdir (dir)) != NULL) {
    if (ent->d_name[0] == '.') continue;
    char input[512];
    char output[512];
    snprintf(input, sizeof(input), "%s/%s", path, ent->d_name);
    snprintf(output, sizeof(output), "/sdcard/%s.bin", ent->d_name);
    dump_file(input, output, ent->d_name);
  }
  closedir (dir);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    return 128;
  }
  
  int fd = atoi(argv[2]);
  cmd_pipe = fdopen(fd, "wb");
  setlinebuf(cmd_pipe);

  fprintf(cmd_pipe, "ui_print 0: %s\n", argv[0]); NEWLINE;
  fprintf(cmd_pipe, "ui_print 1: %s\n", argv[1]); NEWLINE;
  fprintf(cmd_pipe, "ui_print 2: %s\n", argv[2]); NEWLINE;
  fprintf(cmd_pipe, "ui_print 3: %s\n", argv[3]); NEWLINE;
  sleep(2);
  
  //list_file("/fstab.qcom");
  fprintf(cmd_pipe, "ui_print mount sdcard\n"); NEWLINE;
  if (mount("/dev/block/mmcblk1p1", "/sdcard", "vfat", 0, "")) {
    fprintf(cmd_pipe, "ui_print mounting sdcard failed\n"); NEWLINE;
    sleep(5);
    return 0;
  }
  
//  list_dir("/");
//  list_dir("/sbin");
//  list_file("/fstab.qcom");
  list_file("/proc/mounts");
  sleep(5);
//  list_dir("/dev/block/bootdevice/by-name");
  dump_file("/proc/mounts", "/sdcard/mounts.txt", "mounts");
  dump_file("/fstab.qcom", "/sdcard/fstab.qcom", "fstab.qcom");
  dump_dir("/dev/block/bootdevice/by-name");
  sleep(5);

  return 0;
}

