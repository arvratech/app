// Request/Response definitions
#define DATA_REQ			0x12
#define DATA_IND			0x22
#define REQ_IND				0x23
#define RSP_CNF				0x24
#define SEG_REQ_IND			0x25
#define SEG_RSP_CNF			0x26

uv_poll_t		_pollSlvnetSio;
unsigned char	slvnetSioOpened;


void SlvnetSioSetAddress(void)
{
	unsigned long	arg;
	int		fd;

	slvAddress = devId(NULL);
	arg = devId(NULL);
	fd = _pollSlvnetSio.io_watcher.fd;
	ioctl(fd, MSP_CMD_SET_ADDRESS, &arg);
}

void *MainLoop(void);
void _OnSlvnetSioPoll(uv_poll_t *handle, int status, int events);

int SlvnetSioInit(void)
{
	int		fd;

printf("SlvnetSioInit...\n");
	fd = open("/dev/mspS", O_RDWR);
	if(fd < 0) {
		printf("SlvnetSioInit: can't open device(/dev/mspS)\n");
		return -1;
	}
	uv_poll_init(MainLoop(), &_pollSlvnetSio, fd);
	uv_poll_start(&_pollSlvnetSio, UV_READABLE, _OnSlvnetSioPoll);
	SlvnetSioSetAddress();
	slvnetSioOpened = 0;
printf("SlvnetSioInit end\n");
	return 0;
}	

void _SlvnetSioReadData(unsigned char *buf, int len);

void _OnSlvnetSioPoll(uv_poll_t *handle, int status, int events)
{
	unsigned char	buf[2060];
	int		fd, rval, i;

	fd = handle->io_watcher.fd;
	rval = read(fd, buf, 2060);
	if(rval > 0) {
		if(buf[0] == 0xaa) {
			_SlvnetSioResponse(buf+2, rval-2);
		} else if(buf[0] == 0xa8) {
			_SlvnetConnected2(buf+2);
		} else if(buf[0] == 0xa9) {
			_SlvnetDisconnected();
		}
	}
}

int SlvnetSioWriteData(unsigned char *buf, int length)
{
	unsigned char	_buf[2060];
	int		fd, rval;
int	i;

printf("Tx: %d[%02x", length, (int)buf[0]); for(i = 1;i < length;i++) printf("-%02x", (int)buf[i]); printf("]\n");
	fd = _pollSlvnetSio.io_watcher.fd;
	_buf[0] = 0xaa; _buf[1] = devId(NULL);
	memcpy(_buf+2, buf, length);
	rval = write(fd, _buf, length+2);
	return rval;
}

