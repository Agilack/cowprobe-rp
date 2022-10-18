
#ifndef USBMON_H
#define USBMON_H

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t  s64;
#define SETUP_LEN 8

/* taken from Linux, Documentation/usb/usbmon.txt */
struct usbmon_packet {
	u64 id;                 /*  0: URB ID - from submission to callback */
	unsigned char type;     /*  8: Same as text; extensible. */
	unsigned char xfer_type; /*    ISO (0), Intr, Control, Bulk (3) */
	unsigned char epnum;    /*     Endpoint number and transfer direction */
	unsigned char devnum;   /*     Device address */
	u16 busnum;             /* 12: Bus number */
	char flag_setup;        /* 14: Same as text */
	char flag_data;         /* 15: Same as text; Binary zero is OK. */
	s64 ts_sec;             /* 16: gettimeofday */
	s32 ts_usec;            /* 24: gettimeofday */
	int status;             /* 28: */
	unsigned int length;    /* 32: Length of data (submitted or actual) */
	unsigned int len_cap;   /* 36: Delivered length */
	union {                 /* 40: */
		unsigned char setup[SETUP_LEN]; /* Only for Control S-type */
		struct iso_rec {                /* Only for ISO */
			int error_count;
			int numdesc;
		} iso;
	} s;
	int interval;           /* 48: Only for Interrupt and ISO */
	int start_frame;        /* 52: For ISO */
	unsigned int xfer_flags; /* 56: copy of URB's transfer_flags */
	unsigned int ndesc;     /* 60: Actual number of ISO descriptors */
};

struct mon_bin_stats
{
	u32 queued;
	u32 dropped;
};

typedef struct mon_get_arg {
	struct usbmon_packet *hdr;
	unsigned char *data;
	size_t alloc;           /* Length of data (can be zero) */
} usbmon_pkt;

struct mon_mfetch_arg
{
	u32 *offvec; /* Vector of events fetched */
	u32 nfetch;  /* Number of events to fetch (out: fetched) */
	u32 nflush;  /* Number of events to flush */
};

#define MON_IOC_MAGIC      0x92
#define MON_IOCQ_URB_LEN   _IO(MON_IOC_MAGIC,   1)
#define MON_IOCG_STATS     _IOR(MON_IOC_MAGIC,  3, struct mon_bin_stats)
#define MON_IOCT_RING_SIZE _IO(MON_IOC_MAGIC,   4)
#define MON_IOCX_GET       _IOW(MON_IOC_MAGIC,  6, struct mon_get_arg)
#define MON_IOCX_GETX      _IOW(MON_IOC_MAGIC, 10, struct mon_get_arg)
#define MON_IOCX_MFETCH    _IOWR(MON_IOC_MAGIC, 7, struct mon_mfetch_arg)
#define MON_IOCH_MFLUSH    _IO(MON_IOC_MAGIC,   8)

#endif
/* EOF */

