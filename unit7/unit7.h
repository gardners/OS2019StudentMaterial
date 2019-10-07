#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

extern u_int32_t p_start;
extern u_int32_t p_size;

extern u_int32_t f_reserved_sectors;
extern u_int32_t f_root_dir_sector;
extern u_int32_t f_fat_sectors;
extern u_int32_t f_fat1_sector;
extern u_int32_t f_fat2_sector;
extern u_int32_t f_sectors_per_fat;
extern u_int32_t f_clusters;
extern u_int32_t f_rootdir_sector;
extern u_int32_t f_rootdir_cluster;
extern unsigned char f_sectors_per_cluster;

struct my_dirent {
  char name[13];
  u_int32_t length;
  u_int8_t attribs;
  u_int32_t cluster;
};

void dump_bytes(char *m,unsigned char *b,int count,int focus);
void wu7_examine_file_system(void);
void sdcard_readsector(const u_int32_t sector_number);
extern u_int8_t sector_buffer[512];
void sdcard_writesector(const u_int32_t sector_number);
void my_opendir(void);
struct my_dirent *my_readdir(void);
int my_open(char *filename);
int my_read(unsigned char *buffer,int count);
struct my_dirent *my_findfile(char *name);
