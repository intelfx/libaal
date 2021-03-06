/* Copyright (C) 2001, 2002, 2003 by Hans Reiser, licensing governed by
   libaal/COPYING.
   
   file.c -- standard file device abstraction layer. It is used files functions
   to read or write into device. */

#ifndef ENABLE_MINIMAL

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#ifdef HAVE_LINUX_FALLOC_H
#  include <linux/falloc.h>
#endif

#ifdef HAVE_LINUX_TYPES_H
#  include <linux/types.h>
#endif

/* BLKGETSIZE & BLKGETSIZE64 defines: */
#include <sys/mount.h>

/* BLKDISCARD define: */
#if defined(__linux__) && !defined(BLKDISCARD)
#  define BLKDISCARD _IO(0x12,119)
#endif

#include <aal/libaal.h>

/* Function for saving last error message into device assosiated buffer */
static void file_error(
	aal_device_t *device)	    /* device, error will be saved into */
{
	char *error;
    
	if(!device)
		return;

	if ((error = strerror(errno)))
		aal_strncpy(device->error, error, aal_strlen(error));
}

/* Opens actual file, initializes aal_device_t instance and returns it to caller
   function. */
errno_t file_open(
	aal_device_t *device,
	void *person,               /* name of file to be used as file device */
	uint32_t blksize,	    /* used blocksize */
	int flags)		    /* flags file will be opened with */
{
	int fd;
	char *filename;

	if (!device)
		return -EINVAL;
	
	if (!person || aal_strlen((char *)person) == 0) 
		return -EINVAL;
    
	/* Initializing device entity (file descripror in the case of file
	   device). */
	if (!(device->entity = aal_calloc(sizeof(int), 0)))
		return -ENOMEM;

	/* Opening specified file with specified flags */
	filename = (char *)person;

#if defined(O_LARGEFILE)
	flags |= O_LARGEFILE;
#endif

	if ((fd = open(filename, flags)) == -1)
		goto error_free_entity;

	*(int *)device->entity = fd;
	
	aal_strncpy(device->name, filename,
		    sizeof(device->name));
	
	return 0;
    
 error_free_entity:
	aal_free(device->entity);
	return -EINVAL;
}

/* Closes file device. Close opened file descriptor, frees all assosiated
   memory.  It is usualy called at end for work any utility from reiser4progs
   set. */
void file_close(
	aal_device_t *device)	     /* file device to be closed */
{
	if (!device) 
		return;

	/* Closing entity (file descriptor) */
	close(*((int *)device->entity));
	aal_free(device->entity);
}

/* Handler for "read" operation for use with file device. See bellow for
   understanding where it is used. */
static errno_t file_read(
	aal_device_t *device,	    /* file device for reading */
	void *buff,		    /* buffer data will be placed in */
	blk_t block,		    /* block number to be read from */
	count_t count)		    /* number of blocks to be read */
{
	off_t off, len;
	
	if (!device || !buff)
		return -EINVAL;
    
	/* Positioning inside file. As configure script defines
	   __USE_FILE_OFFSET64 macro inside config.h file, lseek64 function will
	   be actually used. */
	off = (off_t)block * (off_t)device->blksize;
	
	if (lseek(*((int *)device->entity), off, SEEK_SET) == (off_t)-1) {
		file_error(device);
		return errno;
	}

	/* Reading data form file */
	len = (off_t)count * (off_t)device->blksize;
	
	if (read(*((int *)device->entity), buff, len) <= 0) {
		file_error(device);
		return errno;
	}
    
	return 0;
}

/* Handler for "write" operation for use with file device. See bellow for
   understanding where it is used. */
static errno_t file_write(
	aal_device_t *device,	    /* file device, data will be wrote onto */
	void *buff,		    /* buffer, data stored in */
	blk_t block,		    /* start position for writing */
	count_t count)		    /* number of blocks to be write */
{
	off_t off, len;
	
	if (!device || !buff)
		return -EINVAL;
	
	/* Positioning inside file */
	off = (off_t)block * (off_t)device->blksize;
	
	if (lseek(*((int *)device->entity), off, SEEK_SET) == (off_t)-1) {
		file_error(device);
		return errno;
	}
    
	/* Writing into file */
	len = (off_t)count * (off_t)device->blksize;
	
	if (write((*(int *)device->entity), buff, len) <= 0) {
		file_error(device);
		return errno;
	}
	
	return 0;
}

/* Handler for "discard" operation for use with file device. See below for
   understanding where it is used. */
static errno_t file_discard(
	aal_device_t *device,	    /* file device to discard */
	blk_t block,		    /* start position for discarding */
	count_t count)		    /* number of blocks to be discarded */
{
	struct stat st;
	int ret;

	if (!device)
		return -EINVAL;

	/* Stat the file */
	if(stat(device->name, &st) != 0) {
		file_error(device);
		return errno;
	}

	/* Discard or punch hole depending on whether this is a block device */
	if (S_ISBLK(st.st_mode)) {
#ifdef BLKDISCARD
		__u64 range[2];

		range[0] = (__u64)block * device->blksize;
		range[1] = (__u64)count * device->blksize;

		ret = ioctl(*((int *)device->entity), BLKDISCARD, &range);
#else
		errno = EOPNOTSUPP;
		ret = -1;
#endif
	} else {
#if defined(HAVE_FALLOCATE) && defined(FALLOC_FL_PUNCH_HOLE)
		off_t blk, len;

		blk = (off_t)block * device->blksize;
		len = (off_t)count * device->blksize;

		ret = fallocate(*((int *)device->entity),
				FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
				blk, len);
#else
		errno = EOPNOTSUPP;
		ret = -1;
#endif
	}

	if (ret != 0) {
		file_error(device);
		return errno;
	}

	return 0;
}

/* Handler for "sync" operation for use with file device. See bellow for
   understanding where it is used. */
static errno_t file_sync(
	aal_device_t *device)	    /* file device to be synchronized */
{
	if (!device) 
		return -EINVAL;
	
	/* As this is file device, we are using fsync function for synchronizing
	   file. */
	if (fsync(*((int *)device->entity))) {
		file_error(device);
		return errno;
	}

	return 0;
}

/* Handler for "equals" operation for use with file device. See bellow for
   understanding where it is used. */
static errno_t file_equals(
	aal_device_t *device1,	    /* the first device for comparing */
	aal_device_t *device2)	    /* the second one */
{
	char *file1, *file2;
	
	if (!device1 || !device2)
		return 0;

	file1 = (char *)device1->data;
	file2 = (char *)device2->data;
	
	/* Devices are comparing by comparing their file names */
	return !aal_strncmp(file1, file2, aal_strlen(file1));
}

/* Handler for "len" operation for use with file device. See bellow for
   understanding where it is used. */
static count_t file_len(
	aal_device_t *device)	    /* file device, lenght will be obtained from */
{
	uint64_t size;
	off_t max_off = 0;

	if (!device) 
		return INVAL_BLK;
    
#ifdef BLKGETSIZE64
	if (ioctl(*((int *)device->entity), BLKGETSIZE64, &size) >= 0) {
		return size / device->blksize;
	}
    
#endif

#ifdef BLKGETSIZE    
	{
		unsigned long l_size;
		
		if (ioctl(*((int *)device->entity), BLKGETSIZE, &l_size) >= 0) {
			size = l_size;
			return size * 512 / device->blksize;
		}
	}
    
#endif
    
	if ((max_off = lseek(*((int *)device->entity), 
			     0, SEEK_END)) == (off_t)-1) 
	{
		file_error(device);
		return INVAL_BLK;
	}
    
	return (count_t)(max_off / device->blksize);
}

/* Initializing the file device operations. They are used when any operation of
   enumerated bellow is performed on a file device. Here is the heart of file
   the device. It is pretty simple. The same as in linux implemented abstraction
   from interrupt controller. */
struct aal_device_ops file_ops = {
	.open   = file_open,        /* handler for "open" operation */
	.close  = file_close,       /* handler for "create" operation */
	.read   = file_read,	    /* handler for "read" operation */	    
	.write  = file_write,	    /* handler for "write" operation */
	.discard = file_discard,    /* handler for "discard" operation */
	.sync   = file_sync,	    /* handler for "sync" operation */
	.equals = file_equals,	    /* handler for comparing two devices */
	.len    = file_len	    /* handler for length obtaining */
};
#endif
