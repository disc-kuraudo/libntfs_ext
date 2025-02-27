/**
 * SACD Ripper - http://code.google.com/p/sacd-ripper/
 *
 * Copyright (c) 2010-2011 by respective authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __SYS_STORAGE_H__
#define __SYS_STORAGE_H__

#ifndef __lv2ppu__
#error you need the psl1ght/lv2 ppu compatible compiler!
#endif

#include <stdint.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/thread.h>

//#include "io_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

//static typedef int sys_io_buffer_t;
//static typedef int sys_io_block_t;

#define BD_DEVICE                              0x0101000000000006ULL

/* The generic packet command opcodes for CD/DVD Logical Units,
 * From Table 57 of the SFF8090 Ver. 3 (Mt. Fuji) draft standard. */
#define GPCMD_GET_CONFIGURATION                0x46
#define GPCMD_GET_EVENT_STATUS_NOTIFICATION    0x4a
#define GPCMD_MODE_SELECT_10                   0x55
#define GPCMD_MODE_SENSE_10                    0x5a
#define GPCMD_READ_CD                          0xbe
#define GPCMD_READ_DVD_STRUCTURE               0xad
#define GPCMD_READ_TRACK_RZONE_INFO            0x52
#define GPCMD_READ_TOC_PMA_ATIP                0x43
#define GPCMD_REPORT_KEY                       0xa4
#define GPCMD_SEND_KEY                         0xa3
#define GPCMD_START_STOP_UNIT                  0x1b

#define LV2_STORAGE_SEND_ATAPI_COMMAND         (1)

struct lv2_atapi_cmnd_block
{
    uint8_t  pkt[32];    /* packet command block           */
    uint32_t pktlen;     /* should be 12 for ATAPI 8020    */
    uint32_t blocks;
    uint32_t block_size;
    uint32_t proto;     /* transfer mode                  */
    uint32_t in_out;    /* transfer direction             */
    uint32_t unknown;
} __attribute__((packed));

typedef struct
{
    uint8_t     name[7];
    uint8_t     unknown01;
    uint32_t    unknown02; // random nr?
    uint32_t    zero01; 
    uint32_t    unknown03; // 0x28?
    uint32_t    unknown04; // 0xd000e990?
    uint8_t     zero02[16];
    uint64_t    total_sectors;
    uint32_t    sector_size;
    uint32_t    unknown05;
    uint8_t     writable;
    uint8_t     unknown06[3];
    uint32_t    unknown07;
} __attribute__((packed)) device_info_t;

enum lv2_atapi_proto
{
    ATAPI_NON_DATA_PROTO     = 0,
    ATAPI_PIO_DATA_IN_PROTO  = 1,
    ATAPI_PIO_DATA_OUT_PROTO = 2,
    ATAPI_DMA_PROTO          = 3
};

enum lv2_atapi_in_out
{
    ATAPI_DIR_WRITE = 0,   /* memory -> device */
    ATAPI_DIR_READ  = 1    /* device -> memory */
};

static inline void sys_storage_init_atapi_cmnd(
    struct lv2_atapi_cmnd_block *atapi_cmnd
    , uint32_t block_size
    , uint32_t proto
    , uint32_t type)
{
    memset(atapi_cmnd, 0, sizeof(struct lv2_atapi_cmnd_block));
    atapi_cmnd->pktlen     = 12;
    atapi_cmnd->blocks     = 1;
    atapi_cmnd->block_size = block_size;     /* transfer size is block_size * blocks */
    atapi_cmnd->proto      = proto;
    atapi_cmnd->in_out     = type;
}

static inline int sys_storage_send_atapi_command(uint32_t fd, struct lv2_atapi_cmnd_block *atapi_cmnd, uint8_t *buffer)
{
    uint64_t tag;
    return Lv2Syscall7(616
                , fd
                , LV2_STORAGE_SEND_ATAPI_COMMAND
                , (uint64_t) atapi_cmnd
                , sizeof(struct lv2_atapi_cmnd_block)
                , (uint64_t) buffer
                , atapi_cmnd->block_size
                , (uint64_t) &tag);
  
}


static inline int sys_storage_send_device_cmd(uint32_t fd, uint32_t cmd, void * cmd_block, uint32_t cmd_size, void * data_buffer, uint32_t len_data_buffer)
{
    return Lv2Syscall6(604
                , fd
                , cmd
                , (uint64_t) cmd_block
                , cmd_size
                , (uint64_t) data_buffer
                , len_data_buffer);
}

#if 0
static inline int sys_storage_async_configure(uint32_t fd, sys_io_buffer_t io_buffer, sys_event_queue_t equeue_id, void * sys_event)
{
    lv2syscall4(605
                , fd
                , io_buffer
                , equeue_id
                , (uint64_t) sys_event);

    return_to_user_prog(int);
}


static inline int sys_storage_async_send_device_command(uint32_t fd, uint64_t cmd,
    const void *cmdbuf, uint32_t cmdbuf_size, void *databuf, uint32_t databuf_size, uint64_t user_data) 
{
    lv2syscall7(619
                , fd
                , cmd
                , (uint64_t) cmdbuf
                , cmdbuf_size
                , (uint64_t) databuf
                , databuf_size
                , user_data);

    return_to_user_prog(int);
}
#endif

static inline int sys_storage_get_device_info(uint64_t device, device_info_t *device_info)
{
    return Lv2Syscall2(609, device, (uint64_t) device_info);
}

static inline int sys_storage_open(uint64_t id, int *fd)
{
    return Lv2Syscall4(600, id, 0, (uint64_t) fd, 0);
}

static inline int sys_storage_close(int fd)
{
    return Lv2Syscall1(601, fd);
}

static inline int sys_storage_read(int fd, uint32_t start_sector, uint32_t sectors, uint8_t *bounce_buf, uint32_t *sectors_read)
{
    return Lv2Syscall7(602, fd, 0, start_sector, sectors, (uint64_t) bounce_buf, (uint64_t) sectors_read, 0);
}

static inline int sys_storage_write(int fd, uint32_t start_sector, uint32_t sectors, uint8_t *bounce_buf, uint32_t *sectors_read)
{
    return Lv2Syscall7(603, fd, 0, start_sector, sectors, (uint64_t) bounce_buf, (uint64_t) sectors_read, 0);
}

/*
static inline int sys_storage_async_read(int fd, uint32_t start_sector, uint32_t sectors, sys_io_block_t bounce_buf, uint64_t user_data)
{
    lv2syscall7(606, fd, 0, start_sector, sectors, bounce_buf, user_data, 0);

    return_to_user_prog(int);
    
}
*/

/**
 * In order to execute syscall 864 the access rights of LV2 need to be patched
 */
static inline int sys_storage_reset_bd(void)
{
    return Lv2Syscall2(864, 0x5004, 0x29);
}

/**
 * In order to execute syscall 864 the access rights of LV2 need to be patched
 */
static inline int sys_storage_authenticate_bd(void)
{
    return Lv2Syscall2(864, 0x5004, 0x46);
}

/**
 * In order to execute syscall 864 the access rights of LV2 need to be patched
 */
static inline int sys_storage_ctrl_bd(int _func)
{
    uint64_t func[0x18/8];
    func[0]= (uint64_t) _func;
    
    return Lv2Syscall2(864, 0x5007, (uint64_t) &func[0]);
}


#ifdef __cplusplus
};
#endif
#endif /* _SYS_STORAGE_H__ */
