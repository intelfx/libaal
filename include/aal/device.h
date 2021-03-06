/* Copyright (C) 2001, 2002, 2003 by Hans Reiser, licensing governed by
   libaal/COPYING.
   
   device.h -- device functions declaration. */

#ifndef AAL_DEVICE_H
#define AAL_DEVICE_H

#include <aal/types.h>

extern aal_device_t *aal_device_open(struct aal_device_ops *ops, 
				     void *person,
				     uint32_t blksize,
				     int flags);

#ifndef ENABLE_MINIMAL
extern errno_t aal_device_sync(aal_device_t *device);

extern bool_t aal_device_readonly(aal_device_t *device);

extern errno_t aal_device_reopen(aal_device_t *device,
				 uint32_t blksize,
				 int flags);

extern errno_t aal_device_discard(aal_device_t *device,
				blk_t block,
				count_t count);

extern errno_t aal_device_write(aal_device_t *device, 
				void *buff, blk_t block,
				count_t count);

extern bool_t aal_device_equals(aal_device_t *device1, 
				aal_device_t *device2);
#endif

extern errno_t aal_device_read(aal_device_t *device, 
			       void *buff, blk_t block,
			       count_t count);

extern void aal_device_close(aal_device_t *device);

extern errno_t aal_device_set_bs(aal_device_t *device, 
				 uint32_t blksize);

extern count_t aal_device_len(aal_device_t *device);
extern uint32_t aal_device_get_bs(aal_device_t *device);

#endif
