#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <linux/types.h>
#include <strings.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/mman.h>

#define DEVICE_FILE "/dev/isp-device"
#define MEM_DEVICE_FILE "/dev/memory_dev"

typedef unsigned int uint32; 
typedef unsigned char uint8;

struct msg_header {
    uint32 msg_id;
    uint32 msg_length;
};

struct isp_io_msg
{
    struct msg_header header;
    uint8 data[128]; 
};


#define ISP_IO_MSG_INIT				    (0x01)		//To initialize ISP module
#define ISP_IO_MSG_DEINIT				(0x02)		//To de-initialize ISP module
#define ISP_IO_MSG_START				(0x03)		//To ask ISP module start to work, and go into running state.
#define ISP_IO_MSG_STOP				    (0x04)		//To ask ISP module stop to work, and go into stop state.
#define ISP_IO_MSG_SET_RAWBUF	        (0x05)	    //Configure sensor RAW data buffer queue.
#define ISP_IO_MSG_SET_FRAME_BUF	    (0x06)		//Set output frame buffer to ISP, after ISP filled this buffer, ISP must response the ACK message.
#define ISP_IO_MSG_SET_FRAME_INFO	    (0x07)		//Configure ISP output channel frame buffer attribution, such as resolution, pixel format, etc.
#define ISP_IO_MSG_SET_RAWINFO          (0x08)      //Set raw information
#define ISP_IO_MSG_SET_GAIN             (0x09)      //Set raw information
#define ISP_IO_MSG_SET_EXPTIME          (0x0a)      //Set raw information
#define ISP_IO_MSG_MASK					(0xff)



struct memory_space {
	unsigned int addr;
	unsigned int size;
};

struct isp_msg_rawbuf
{
    uint32 sensor_type;
    uint32 rawbuf_addr;     /* RAW buffer address   */
    uint32 rawbuf_size;     /* RAW buffer size      */
    uint32 queue_num;       /* RAW frame queue size */
};

struct isp_msg_start {
    uint32 sensor;
};

struct isp_msg_stop {
    uint32 sensor;
};

struct isp_msg_gain {
    uint32 sensor_type;
    uint32 gain;
};

struct isp_msg_exptime {
    uint32 sensor_type;
    uint32 exptime;
};

enum sensor_type_t {
    SENSOR_TYPE_A = 1 << 0,
    SENSOR_TYPE_B = 1 << 1
};

enum SnsResolution_s{
    SNS_RES_VGA = 1,
    SNS_RES_720P,
    SNS_RES_1080P
};

#define ISP_IO_MSG_CMD		_IOW ('i', 4, struct isp_io_msg)
#define MEMORY_IO_MEMORY_SPACE		_IOWR ('i', 4, struct memory_space)

struct isp_msg_start start;
struct isp_msg_stop stop;

struct isp_msg_gain    gain_val;
struct isp_msg_exptime exptime_val;


struct isp_msg_rawbuf rawbuf;

struct memory_space g_mem_space;
struct isp_io_msg g_isp_msg;

enum SnsResolution_s resolution	= SNS_RES_VGA;

#define CONFIG_SENSOR 


int open_device(const char *pathname)
{
	int fd;
	
	if ((fd = open(pathname, O_RDWR | O_SYNC)) == -1) {
		fprintf(stderr, "open %s failed, [%s]\n", pathname, strerror(errno));
		return -1;
	}
	printf("character device %s opened.\n", pathname); 
	fflush(stdout);
	return fd;
}

int close_device(int fd)
{
	close(fd);
	return 0;
}

static int mem_get_memory_space(int fd, struct memory_space *space)
{
	if (ioctl(fd, MEMORY_IO_MEMORY_SPACE, space) < 0 ) {
		fprintf(stderr, "failed to set map table: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static int isp_send_msg(int fd, struct isp_io_msg *isp_msg)
{
	if (ioctl(fd, ISP_IO_MSG_CMD, isp_msg) < 0 ) {
		fprintf(stderr, "failed to set map table: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}


#define MAP_SIZE 399*1024*1024
int get_base_address(int fd, void **addr)
{
    /* map one page */
    *addr = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == (void *) -1) {
		fprintf(stderr, "mmap failed, [%s]\n", strerror(errno));
		return -1;
    }
    printf("Memory mapped at address %p.\n", *addr); 
    fflush(stdout);
	return 0;
}

int release_address(void *addr)
{
	if (munmap(addr, MAP_SIZE) == -1) {
		fprintf(stderr, "munmap failed, [%s]\n", strerror(errno));
		return -1;
    }
}

void isp_send_init(int dev_fd, 
	struct isp_io_msg *isp_msg)
{

	isp_msg->header.msg_id = ISP_IO_MSG_INIT;
	isp_msg->header.msg_length = 0;

	isp_send_msg(dev_fd, isp_msg);
}

void send_sensora_rawbuf(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_SET_RAWBUF;
	isp_msg->header.msg_length = sizeof(struct isp_msg_rawbuf);

	rawbuf.sensor_type = SENSOR_TYPE_A;
	rawbuf.rawbuf_addr = 0x9f000000;
    rawbuf.queue_num  = 60;

    if (resolution == SNS_RES_VGA) {
        rawbuf.rawbuf_size = 0x96000 * rawbuf.queue_num;
    } else if (resolution == SNS_RES_720P) {
        rawbuf.rawbuf_size = 0x1C2000 * rawbuf.queue_num;
    } else if (resolution == SNS_RES_1080P) {
        rawbuf.rawbuf_size = 0x3F4800 * rawbuf.queue_num;
    }

	memcpy(isp_msg->data, &rawbuf, sizeof(struct isp_msg_rawbuf));
	isp_send_msg(dev_fd, isp_msg);
}

void send_sensorb_rawbuf(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_SET_RAWBUF;
	isp_msg->header.msg_length = sizeof(struct isp_msg_rawbuf);

	rawbuf.sensor_type = SENSOR_TYPE_B;
	rawbuf.rawbuf_addr = 0xA2000000; //0x9f000000 + 0x3000000;
	rawbuf.rawbuf_size = 0x3000000;
	rawbuf.queue_num  = 3;
	memcpy(isp_msg->data, &rawbuf, sizeof(struct isp_msg_rawbuf));
	isp_send_msg(dev_fd, isp_msg);
}

void send_start(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_START;
	isp_msg->header.msg_length = sizeof(struct isp_msg_start);
	start.sensor = SENSOR_TYPE_A;

	memcpy(isp_msg->data, &start, sizeof(struct isp_msg_start));
	isp_send_msg(dev_fd, isp_msg);
}

void send_stop(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_STOP;
	isp_msg->header.msg_length = sizeof(struct isp_msg_stop);

	stop.sensor = SENSOR_TYPE_A;// | SENSOR_TYPE_B;
	memcpy(isp_msg->data, &stop, sizeof(struct isp_msg_stop));
	isp_send_msg(dev_fd, isp_msg);
}

#ifdef CONFIG_SENSOR
void send_set_gain(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_SET_GAIN;
	isp_msg->header.msg_length = sizeof(struct isp_msg_gain);

	gain_val.sensor_type = SENSOR_TYPE_A;
	gain_val.gain        = 60000;
	memcpy(isp_msg->data, &gain_val, sizeof(struct isp_msg_gain));
	isp_send_msg(dev_fd, isp_msg);
}

void send_set_exptime(int dev_fd, struct isp_io_msg *isp_msg)
{
	isp_msg->header.msg_id = ISP_IO_MSG_SET_EXPTIME;
	isp_msg->header.msg_length = sizeof(struct isp_msg_exptime);

	exptime_val.sensor_type = SENSOR_TYPE_A;// | SENSOR_TYPE_B;
	exptime_val.exptime     = 200000;
	memcpy(isp_msg->data, &exptime_val, sizeof(struct isp_msg_exptime));
	isp_send_msg(dev_fd, isp_msg);
}
#endif


static int save_raw_file(char *filename, unsigned char *rawfile, unsigned int size)
{
	FILE *fp;

	fp = fopen(filename, "wb");
	if (!fp) {
		printf("fopen %s error\n", filename);
		return -1;
	}
	printf("save raw file %s\n", filename);	
	fseek(fp, 0, SEEK_SET);
	fwrite(rawfile, 1, size, fp);
	printf("save raw finish\n");
	fclose(fp);
	return 0;
}

static void usage(char *name)
{
 	fprintf(stderr, "Usage: %s [-f 10] [-r 1]\n", name);
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
	int dev_fd;
	int mem_fd;
	struct isp_io_msg *isp_msg = &g_isp_msg;
	void *mem_base;
	int ch;
	int err_flag = 0;
	int  size;
	int config = 0;
	char filename[64];

    int save_frame_num = 50;
    int buf_size = 640 * 480 * 2;
    int skip_frame = 3;

	printf("isp demo\n");
	opterr = 0;
	while((ch = getopt(argc, argv, "f:r:c")) != -1)
	{
		printf("optind:%d, ch=%d, optarg=%s\n", optind, ch, optarg);
		switch(ch)
		{
			case 'f':
				save_frame_num = atoi(optarg);
                printf("parse save_frame_num %d\n",save_frame_num);
				break;
			case 'r':
				resolution = atoi(optarg);
                printf("parse resolution %d\n", resolution);
				break;
			case 'c':
				config = 1;
				break;
			case '?':
				usage(basename(argv[0]));
				break;
			default:
			break;
		}
	}

    if (save_frame_num > 50) {
        printf("must change sleep time and change the limit of buf num\n");
    }

    switch (resolution) {
        case SNS_RES_VGA:
            buf_size = 640 * 480 * 2;
            break;
        case SNS_RES_720P:
            buf_size = 1280 * 720 * 2;
            break;
        case SNS_RES_1080P:
            buf_size = 1920 * 1080 * 2;
            break;
        default:
            printf("input r param error\n");
			resolution = SNS_RES_VGA;
            buf_size = 640 * 480 * 2;
            break;
	}

	printf("ope parse finish\n");
	/*Memory Share Device*/
	mem_fd = open_device(MEM_DEVICE_FILE);
	if (mem_fd == -1)
		return -1;

	dev_fd = open_device(DEVICE_FILE);
	if (dev_fd == -1)
		return -1;

	mem_get_memory_space(mem_fd, &g_mem_space);
	printf("memory share addr [ 0x%x ]\n", g_mem_space.addr);
	printf("memory share size [ 0x%x ]\n", g_mem_space.size);
	get_base_address(mem_fd, (void**)&mem_base);
	printf("mmap base addr mem_base=%p\n", mem_base);

	if (config) {		
		isp_send_init(dev_fd, isp_msg);
		send_sensora_rawbuf(dev_fd, isp_msg);
		//send_sensorb_rawbuf(dev_fd, isp_msg);
#ifdef  CONFIG_SENSOR
        //send_set_gain(dev_fd, isp_msg);
        //send_set_exptime(dev_fd, isp_msg);
#endif
		send_start(dev_fd, isp_msg);

		sleep(1);

		send_stop(dev_fd, isp_msg);

        printf("buf_size 0x%x,save_frame_num %d\n", buf_size, save_frame_num);

        if (resolution == SNS_RES_VGA) {
            save_raw_file("ov2710_VPA.raw", mem_base + buf_size * skip_frame, buf_size * save_frame_num);
		} else if (resolution == SNS_RES_720P) {
            save_raw_file("ov2710_720p.raw", mem_base + buf_size * skip_frame, buf_size * save_frame_num);
        } else if (resolution == SNS_RES_1080P) {
            save_raw_file("ov2710_1080p.raw", mem_base + buf_size * skip_frame, buf_size * save_frame_num);
        }
	}

	printf("finish put any key to end\n");
	getchar();
	release_address(mem_base);
	fsync(mem_fd);
	close_device(mem_fd);
	close_device(dev_fd);
	return 0;
}



