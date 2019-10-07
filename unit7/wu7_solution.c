#include "unit7.h"

u_int32_t p_start=0;
u_int32_t p_size=0;

u_int32_t f_reserved_sectors=0;
u_int32_t f_sectors_per_fat=0;
u_int32_t f_clusters=0;
unsigned char f_sectors_per_cluster=0;
u_int32_t f_fat1_sector=0;
u_int32_t f_fat2_sector=0;
u_int32_t f_rootdir_sector=0;
u_int32_t f_rootdir_cluster=0;

u_int32_t extract_uint32(int offset)
{
  return sector_buffer[offset+0]
    +(sector_buffer[offset+1]<<8)
    +(sector_buffer[offset+2]<<16)
    +(sector_buffer[offset+3]<<24);
}

u_int32_t extract_uint16(int offset)
{
  return sector_buffer[offset+0]
    +(sector_buffer[offset+1]<<8);
}

void extract_filename(int offset, char *dest)
{
  // Extract 8.3 filename to a coherent string
  int o=0;
  for(int i=0;i<8;i++) dest[i]=sector_buffer[offset+i];
  o=8;
  while(o&&dest[o-1]==' ') o--;
  dest[o++]='.';
  for(int i=0;i<3;i++) dest[o++]=sector_buffer[offset+8+i];
  while(o&&dest[o-1]==' ') o--;
  dest[o]=0;
  
}

void wu7_examine_file_system(void)
{
  // XXX - First, read the Master Boot Record, which is in the first sector of the disk.
  // Within that, find the partition entry that has a FAT32 partition in it (partition type
  // will be 0x0c), and then use extract_uint32() to get the start and size of the partition
  // into p_start and p_size.
  // Complexity guide: My solution was 6 lines long.

  // Then read the first sector of the FAT32 partition, and use extract_uint32(), extract_uint16()
  // or simply reading bytes from sector_buffer[] to get the values for:
  // f_sectors_per_fat, f_rootdir_cluster, f_reserved_sectors and f_sectors_per_cluster.
  // Then use those values to compute the values of f_fat1_Sector, f_fat2_sector, f_rootdir_sector
  // and f_clusters (this last one can be calculated simple as the number of sectors per fat multiplied
  // by the number of 32-bit values (i.e., 4 bytes long each) that can be packed into a 512 byte sector).
  // Complexity guide: My solution was 11 lines long.

}

u_int32_t dir_sector=0;
u_int32_t dir_sector_offset=0;
u_int32_t dir_sector_max=0;
void my_opendir(void)
{
  // XXX - Use the three convenient variables above to point to the start of
  // the first sector of the root directory of the file system.
  // Then work out the last valid sector number of the directory.  This will
  // be the last sector in the cluster, as for simplicity, we are assuming
  // that the directory is only one cluster long.  You will thus need to know
  // how many sectors in a cluster, and add one less than that to the starting
  // sector of the root directory.
  // Complexity guide: My solution was 3 lines long.
}

struct my_dirent return_structure;
struct my_dirent *my_readdir(void)
{
  // XXX - First, find the next directory entry in the directory that is valid.
  // A valid directory entry has a file name in it that doesn't begin with the null
  // character or the special chartacter 0xe5 that is used to indicate a deleted file.
  // If you get to the end of a sector, you will need to read the next sector, and try
  // looking there.  Don't forget to use sdcard_readsector() to read the sector before
  // fishing around in sector_buffer[] for the bytes of the directory entry.  I suggest
  // that you use dir_sector_offset to keep track of the directory entry you are looking at.
  // You would abort by returning NULL if you reach the end of the directory, i.e.,
  // you go past the end of dir_sector_max.
  // Complexity guide: My solution was 11 lines long.
  
  // At this point you have the directory entry located at offset dir_sector_offset in
  // sector_buffer[]. You can now use the convenience functions extract_uint32(), extract_uint16()
  // and extract_filename() that I have provided for you to extract the necessary values
  // into return_structure, where required. To get the attribs field, you don't need any of those,
  // because it is a single byte long. To get the cluster out, you will need to use extract_uint16()
  // on the two separate places where the halves of the cluster number are located, and then use
  // some addition and bit-shifting to combine the two halves to make a valid cluster number.
  // This is the trickiest part of this checkpoint.
  // Your solution will look like a series of return_structure.member=extract....() lines,
  // plus a call to extract_filename().
  // Complexity guide: My solution was 5 lines long.

  // XXX - Finally, advance dir_sector_offset (and dir_sector if the offset goes past the end
  // of the sector), so that it is pointing at the next directory entry, ready for the next call
  // to this function.
  // Complexity guide: My solution was 5 lines long.

  // And really last of all, we return a pointer to the return_structure, which I have done for
  // you here:
  return &return_structure;
}

u_int32_t file_cluster=-1;
u_int32_t cluster_offset=-1;
u_int32_t file_length_remaining=-1;

int my_open(char *filename)
{
  // XXX - First, find the file by using my_opendir() and my_readdir() to
  // iterate through the directory. You will probably want to use some
  // variable like the following, and test if de->name is equal to the
  // filename that has been passed in.  strcmp() is a handy function
  // for this.
  // Complexity guide: My solution was 5 lines long.
  struct my_dirent *de=NULL;

  // XXX - Next, abort if you couldn't find the file.  You can detect this
  // by seeing if my_readdir() has gotten to the end of the directory, and
  // stopped returning new directory entry structures.  You should return -1
  // in this case.
  // Complexity guide: My solution was 1 line long.

  // XXX - Finally, record the cluster where the file begins, reset the offset
  // in the cluster to zero, and set the remaining file length to the
  // length field of the directory entry of the file. Then return 0 to indicate
  // success.
  // You can stash these values in the three convenient variables that I have
  // defined for you just before this function.
  // Complexity guide: My solution was 4 lines long.
  return -1;
}

int my_read(unsigned char *buffer,int count)
{
  // XXX - First check that you have not reached the end of the file.
  // You should check both for illegal cluster values, and end of
  // cluster chain values, as well as that you haven't reached the
  // end of the file as reported by the file length field of the file's
  // directory entry.
  // Complexity guide: My solution was 4 lines long.
  int offset=0;
  if (!file_cluster) return 0;
  if (file_cluster&0xf0000000) return 0;
  if (!file_length_remaining) return 0;

  // XXX - Now you know you aren't yet at the end of the file.
  // You need to read the file data one sector at a time, and output it
  // into the buffer.  To write data to a specific offset in the buffer,
  // you can use something like:
  // bcopy(&sector_buffer[the offset in the sector buffer you want to read from],
  //       &buffer[the offset in the buffer you want to write to],
  //       the number of bytes you want to copy);
  // You need to remember to advance the sector each time, and to follow the
  // chain of clusters for the file from the FAT after you have read all the
  // sectors in the current cluster.  To get the checkpoint, you MUST demonstrate
  // in your code that you are reading the next cluster value from the FAT.
  // Complexity guide: My solution was 23 lines long.
  
}

struct my_dirent *my_findfile(char *name)
{
  // It's up to you whether you use these, but they are one way to hold the
  // path segments
#define MAX_LEVELS 16  
  char paths[MAX_LEVELS][16];
  int path_count=0;

  // XXX - Seperate out each path segment, which will be delineated by / or the end
  // of the string
  // Complexity guide: My solution was 12 lines long.
 

  // XXX - Now starting at the root directory, use my_readdir() to check each directory
  // entry to see if it matches the one you are looking for. If it matches, you need
  // to get the starting cluster of the sub-directory, and start iterating through that
  // in the same way. This is repeated until you find the file you are looking for.
  // At that point, you should return the directory entry structure that my_readdir()
  // gave you as the result.  If you can't find the file, you should return NULL instead.
  // Complexity guide: My solution was 23 lines long.
  
  return NULL;
}
