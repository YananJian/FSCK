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

/*
 * parse MBR, skip first 446 bytes
 *
 * */
void parse_MBR(unsigned char *buf)
{
		
    int i,j;
	int length = 16;
	//int partition = 0;
    int base = 446;	
	unsigned char extend[sector_size_bytes];
	struct partition * pt;
	struct partition * ext;
	struct partition * ext_pt;
	unsigned int ext_start_sect;
	unsigned int logi_start_sect;
	for(i = 0; i < 4; i++)
	{
		pt = (struct partition*) (buf + base + length*i);
		printf("0x%02X %d %d\n", pt->sys_ind, pt->start_sect, pt->nr_sects);
	}
	ext_start_sect = pt->start_sect;


	for(j=0; j<2; j++)
	{
		read_sectors(ext_start_sect, 1, extend);
		ext = (struct partition*) (&extend[base]);
		ext_pt = (struct partition*) (&extend[base+length]);
		logi_start_sect = ext_start_sect + ext->start_sect;

		ext_start_sect = pt->start_sect + ext_pt->start_sect;
		printf("0x%02X %d %d\n", ext->sys_ind, logi_start_sect, ext->nr_sects);
	}

    //for (i = 446; i < sector_size_bytes; i++) {
        //printf("%02x", buf[i]);
	//	parts[partition/16][partition%16] = buf[i];
		/*
        if (partition % 16 == 0)
		{
			printf("\n");      
			partition = 0;
		}*/
		/*
		if ((ct)%4 == 1)
			printf("partition type: %02x", buf[i]);
			*/
        /*if (!((i+1) % 4))
            printf(" "); */
	//	partition += 1;
    //}
	

	//for(i = 0; i < 4; i++)
	//{
	//	parse_partition(parts[i]);
	//	printf ("\n");
	//}
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
    int           the_sector;                     /* IN: sector to read */

    if ((device = open(argv[1], O_RDWR)) == -1) {
        perror("Could not open device file");
        exit(-1);
    }

    the_sector = atoi(argv[2]);
    //printf("Dumping sector %d:\n", the_sector);
    read_sectors(the_sector, 1, buf);
    //print_sector(buf);
	parse_MBR(buf);

    close(device);
    return 0;
}

/* EOF */
