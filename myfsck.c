/* $cmuPDL: readwrite.c,v 1.3 2010/02/27 11:38:39 rajas Exp $ */
/* $cmuPDL: readwrite.c,v 1.4 2014/01/26 21:16:20 avjaltad Exp $ */
/* readwrite.c
 *
 * Code to read and write sectors to a "disk" file.
 * This is a support file for the "fsck" storage systems laboratory.
 *
 * author: YOUR NAME HERE
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for memcpy() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include "genhd.h"
#include "ext2_fs.h"
#include <stdbool.h>

#if defined(__FreeBSD__)
#define lseek64 lseek
#endif

/* linux: lseek64 declaration needed here to eliminate compiler warning. */
extern int64_t lseek64(int, int64_t, int);

const unsigned int sector_size_bytes = 512;

static int device;  /* disk file descriptor */

/* print_sector: print the contents of a buffer containing one sector.
 *
 * inputs:
 *   char *buf: buffer must be >= 512 bytes.
 *
 * outputs:
 *   the first 512 bytes of char *buf are printed to stdout.
 *
 * modifies:
 *   (none)
 */
void print_sector (unsigned char *buf)
{
    int i;
    for (i = 0; i < sector_size_bytes; i++) {
        printf("%02x", buf[i]);
        if (!((i+1) % 32))
            printf("\n");      /* line break after 32 bytes */
        else if (!((i+1) % 4))
            printf(" ");   /* space after 4 bytes */
    }
}


/* read_sectors: read a specified number of sectors into a buffer.
 *
 * inputs:
 *   int64 start_sector: the starting sector number to read.
 *                       sector numbering starts with 0.
 *   int numsectors: the number of sectors to read.  must be >= 1.
 *   int device [GLOBAL]: the disk from which to read.
 *
 * outputs:
 *   void *into: the requested number of sectors are copied into here.
 *
 * modifies:
 *   void *into
 */
void read_sectors (int64_t start_sector, unsigned int num_sectors, void *into)
{
    ssize_t ret;
    int64_t lret;
    int64_t sector_offset;
    ssize_t bytes_to_read;

    if (num_sectors == 1) {
        //printf("Reading sector %"PRId64"\n", start_sector);
    } else {
        //printf("Reading sectors %"PRId64"--%"PRId64"\n",
        //       start_sector, start_sector + (num_sectors - 1));
    }

    sector_offset = start_sector * sector_size_bytes;

    if ((lret = lseek64(device, sector_offset, SEEK_SET)) != sector_offset) {
        fprintf(stderr, "Seek to position %"PRId64" failed: "
                "returned %"PRId64"\n", sector_offset, lret);
        exit(-1);
    }

    bytes_to_read = sector_size_bytes * num_sectors;

    if ((ret = read(device, into, bytes_to_read)) != bytes_to_read) {
        fprintf(stderr, "Read sector %"PRId64" length %d failed: "
                "returned %"PRId64"\n", start_sector, num_sectors, ret);
        exit(-1);
    }
}

/*
 * parse each partition
 *
 * */
void parse_partition(unsigned char *buf)
{
	unsigned char partition_type = buf[4];
	
	printf ("partition type: %02x\n", partition_type);
}


struct partition* find_first_ext_block(unsigned char *buf)
{
		int i=0;
		int base = 446;
		int length = 16;
		struct partition * pt;
		for(i=0; i < 5; i++)
		{
			pt = (struct partition*) (buf + base + length*i);
			if (pt->sys_ind == 5)
					break;
		}
		return pt;
}

/*
 * parse MBR, skip first 446 bytes
 *
 * */
void parse_MBR(unsigned char *buf, int p)
{
		
    int j;
	int length = 16;
    int base = 446;	
	unsigned char extend[sector_size_bytes];
	struct partition * pt;
	struct partition * ext;
	struct partition * ext_pt;
	unsigned int ext_start_sect;
	unsigned int logi_start_sect;
    int super_block_start_sect;
	bool is_print = true;
	if (p < 0)
	{
		printf ("-1\n");
		return;
	}
	if (p <= 4 && p > 0)
	{
		pt = (struct partition*) (buf + base + length*(p-1));
		printf("0x%02X %d %d\n", pt->sys_ind, pt->start_sect, pt->nr_sects);
		super_block_start_sect = pt->start_sect;
		//read_magic_number(super_block_start_sect);
		struct partition * base_pt = (struct partition*) (buf+base);

	}
	else if (p > 4)
	{
        pt = find_first_ext_block(buf);
		//pt = (struct partition*) (buf + base + length*3);
		ext_start_sect = pt->start_sect;
        
		for(j=5; j<=p; j++)
		{
			read_sectors(ext_start_sect, 1, extend);
			ext = (struct partition*) (&extend[base]);
			ext_pt = (struct partition*) (&extend[base+length]);
			logi_start_sect = ext_start_sect + ext->start_sect;
			if ((ext_pt -> sys_ind == 5))
			{
					ext_start_sect = pt->start_sect + ext_pt->start_sect;
			}
			else if( (ext_pt->sys_ind != 5))
			{
					is_print = false;
					break;
			}
			

		}
		if((j==p) || (is_print))
		    printf("0x%02X %d %d\n", ext->sys_ind, logi_start_sect, ext->nr_sects);
		else if(!is_print)
			printf("-1\n");
		
	}

}

/* write_sectors: write a buffer into a specified number of sectors.
 *
 * inputs:
 *   int64 start_sector: the starting sector number to write.
 *                	sector numbering starts with 0.
 *   int numsectors: the number of sectors to write.  must be >= 1.
 *   void *from: the requested number of sectors are copied from here.
 *
 * outputs:
 *   int device [GLOBAL]: the disk into which to write.
 *
 * modifies:
 *   int device [GLOBAL]
 */
void write_sectors (int64_t start_sector, unsigned int num_sectors, void *from)
{
    ssize_t ret;
    int64_t lret;
    int64_t sector_offset;
    ssize_t bytes_to_write;

    if (num_sectors == 1) {
        printf("Reading sector  %"PRId64"\n", start_sector);
    } else {
        printf("Reading sectors %"PRId64"--%"PRId64"\n",
               start_sector, start_sector + (num_sectors - 1));
    }

    sector_offset = start_sector * sector_size_bytes;

    if ((lret = lseek64(device, sector_offset, SEEK_SET)) != sector_offset) {
        fprintf(stderr, "Seek to position %"PRId64" failed: "
                "returned %"PRId64"\n", sector_offset, lret);
        exit(-1);
    }

    bytes_to_write = sector_size_bytes * num_sectors;

    if ((ret = write(device, from, bytes_to_write)) != bytes_to_write) {
        fprintf(stderr, "Write sector %"PRId64" length %d failed: "
                "returned %"PRId64"\n", start_sector, num_sectors, ret);
        exit(-1);
    }
}


void read_magic_number(int start_sector)
{
    unsigned char buf[sector_size_bytes*2];        /* temporary buffer */
	struct ext2_super_block* super_block = malloc(sector_size_bytes*2);
	unsigned char magic_num;
    read_sectors(start_sector, 2, super_block);
    read_sectors(start_sector, 2, buf);
	magic_num = super_block -> s_magic;
	unsigned int totalBlocks = super_block->s_blocks_count;
	printf("magic_number:0x%02X\n", magic_num);
	printf("total number of blocks: %d\n", totalBlocks);

	printf("start sector: %d\n", start_sector);
	
	/*
    int i;
	
    for (i = 0; i < sector_size_bytes*2; i++) {
        printf("%02x", buf[i]);
        if (!((i+1) % 32))
            printf("\n");      
        else if (!((i+1) % 4))
            printf(" ");       
			}
	*/
}

int main (int argc, char **argv)
{
    /* This is a sample program.  If you want to print out sector 57 of
     * the disk, then run the program as:
     *
     *    ./readwrite disk 57
     *
     * You'll of course want to replace this with your own functions.
     */

    unsigned char buf[sector_size_bytes];        /* temporary buffer */
    int           p;                     /* IN: sector to read */
	int start_sector = 0;
	char * disk;
	int c;
	while((c = getopt(argc, argv, "i:p:")) != -1)
	{
		switch(c)
		{
			case 'p':
					p = atoi(optarg);
					break;
			case 'i':
					disk = malloc(strlen(optarg)+1);
				    strcpy(disk, optarg+'\0');
					break;
		}	
	}

    if ((device = open(disk, O_RDWR)) == -1) {
        perror("Could not open device file");
        exit(-1);
    }
    //printf("Dumping sector %d:\n", the_sector);
    read_sectors(start_sector, 1, buf);
    //print_sector(buf);
	parse_MBR(buf, p);
    close(device);
    return 0;
}

/* EOF */
