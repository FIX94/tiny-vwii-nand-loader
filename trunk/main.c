/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on dhewg's geckoloader stub */
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "utils.h"
#include "ios.h"
#include "cache.h"
#include "memory.h"

#define IOCTL_ES_LAUNCH					0x08
#define IOCTL_ES_OPENCONTENT			0x09
#define IOCTL_ES_READCONTENT			0x0A
#define IOCTL_ES_CLOSECONTENT			0x0B
#define IOCTL_ES_GETVIEWCNT				0x12
#define IOCTL_ES_GETVIEWS				0x13
#define IOCTL_ES_GETTITLEID				0x20
#define IOCTL_ES_SEEKCONTENT			0x23

#define TITLE_ID(x,y)	(((u64)(x) << 32) | (y))

extern const u8 app_booter_bin[];
extern const u32 app_booter_bin_size;

static struct ioctlv vecs[16] ALIGNED(64);
s32 cfd ALIGNED(32);
int es_fd = -1;

typedef void (*entry)(void);

static int es_init(void)
{
	es_fd = ios_open("/dev/es", 0);
	return es_fd;
}

static void es_close(void)
{
	if(es_fd >= 0)
		ios_close(es_fd);
	es_fd = -1;
}

static int es_launchtitle(u64 titleID)
{
	static u64 xtitleID __attribute__((aligned(32)));
	static u32 cntviews __attribute__((aligned(32)));
	static u8 tikviews[0xd8*4] __attribute__((aligned(32)));

	xtitleID = titleID;
	vecs[0].data = &xtitleID;
	vecs[0].len = 8;
	vecs[1].data = &cntviews;
	vecs[1].len = 4;
	int ret = ios_ioctlv(es_fd, IOCTL_ES_GETVIEWCNT, 1, 1, vecs);
	if(ret<0) return ret;
	if(cntviews>4) return -1;

	vecs[2].data = tikviews;
	vecs[2].len = 0xd8*cntviews;
	ret = ios_ioctlv(es_fd, IOCTL_ES_GETVIEWS, 2, 1, vecs);
	if(ret<0) return ret;
	vecs[1].data = tikviews;
	vecs[1].len = 0xd8;
	ret = ios_ioctlvreboot(es_fd, IOCTL_ES_LAUNCH, 2, 0, vecs);
	return ret;
}

static int es_opencontent(u32 content_num)
{
	vecs[0].data = &content_num;
	vecs[0].len = 4;
	cfd = ios_ioctlv(es_fd, IOCTL_ES_OPENCONTENT, 1, 0, vecs);
	return cfd;
}

static int es_seekcontent(s32 place, s32 from)
{
	vecs[0].data = &cfd;
	vecs[0].len = 4;
	vecs[1].data = &place;
	vecs[1].len = 4;
	vecs[2].data = &from;
	vecs[2].len = 4;
	return ios_ioctlv(es_fd, IOCTL_ES_SEEKCONTENT, 3, 0, vecs);
}

static int es_readcontent(void *data, u32 size)
{
	vecs[0].data = &cfd;
	vecs[0].len = 4;
	vecs[1].data = data;
	vecs[1].len = size;
	return ios_ioctlv(es_fd, IOCTL_ES_READCONTENT, 1, 1, vecs);
}

static int es_closecontent()
{
	vecs[0].data = &cfd;
	vecs[0].len = 4;
	return ios_ioctlv(es_fd, IOCTL_ES_CLOSECONTENT, 1, 0, vecs);
}

void memoryset(u8 *dst, const u8 value, u32 size)
{
	u32 i;
	for(i = 0; i < size; ++i)
		dst[i] = value;
}

void memorycopy(u8 *dst, const u8 *src, u32 size)
{
	u32 i;
	for(i = 0; i < size; ++i)
		dst[i] = src[i];
}

static void *dol_data = (void*)0x91000000; /* 32MB Max */
static void *appbooter_entry = (void*)0x93000000;

void _main(void)
{
	u32 size = 0;
	es_init();
	/* load up dol */
	es_opencontent(2); //stub is 1, real dol is 2
	if(cfd < 0) goto fail;
	size = es_seekcontent(0, 2);
	es_seekcontent(0, 0);
	es_readcontent(dol_data, size);
	sync_after_write(dol_data, size);
	es_closecontent();
	if(size == 0) goto fail;
	es_close();
	/* Boot our good old appbooter stuff */
	memorycopy(appbooter_entry, app_booter_bin, app_booter_bin_size);
	sync_after_write(appbooter_entry, app_booter_bin_size);
	entry EntryPoint = (entry)appbooter_entry;
	EntryPoint();

fail:
	es_launchtitle(TITLE_ID(1,2));
	while(1) usleep(500);
}

#define SYSCALL_VECTOR	((u8*)0x80000C00)
void __init_syscall()
{
	u8* sc_vector = SYSCALL_VECTOR;
	u32 bytes = (u32)DCFlashInvalidate - (u32)__temp_abe;
	u8* from = (u8*)__temp_abe;
	for ( ; bytes != 0 ; --bytes )
	{
		*sc_vector = *from;
		sc_vector++;
		from++;
	}

	sync_after_write(SYSCALL_VECTOR, 0x100);
	ICInvalidateRange(SYSCALL_VECTOR, 0x100);
}
