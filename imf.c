#include <u.h>
#include <libc.h>

void
main(int argc, char **argv)
{
	int n, m, fd;
	uchar buf[65536], wse[4] = {0x01, 0x20, 0, 0};

	fd = 0;
	if(argc > 1){
		fd = open(argv[1], OREAD);
		if(fd < 0)
			sysfatal("open: %r");
	}
	if(read(fd, buf, 2) != 2)
		sysfatal("short read: %r");
	m = buf[1]<<8 | buf[0];
	if(m & 3)
		sysfatal("invalid imf size");
	write(1, wse, sizeof wse);
	while(n = read(fd, buf, m > sizeof buf ? sizeof buf : m), n > 0){
		write(1, buf, n);
		m -= n;
	}
	exits(nil);
}
