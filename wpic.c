#include <u.h>
#include <libc.h>
#include <bio.h>
#include <draw.h>

u32int pal[256] = {
	0x000000, 0x0000aa, 0x00aa00, 0x00aaaa, 0xaa0000, 0xaa00aa, 0xaa5500,
	0xaaaaaa, 0x555555, 0x5555ff, 0x55ff55, 0x55ffff, 0xff5555, 0xff55ff,
	0xffff55, 0xffffff, 0xeeeeee, 0xdedede, 0xd2d2d2, 0xc2c2c2, 0xb6b6b6,
	0xaaaaaa, 0x999999, 0x8d8d8d, 0x7d7d7d, 0x717171, 0x656565, 0x555555,
	0x484848, 0x383838, 0x2c2c2c, 0x202020, 0xff0000, 0xee0000, 0xe20000,
	0xd60000, 0xca0000, 0xbe0000, 0xb20000, 0xa50000, 0x990000, 0x890000,
	0x7d0000, 0x710000, 0x650000, 0x590000, 0x4c0000, 0x400000, 0xffdada,
	0xffbaba, 0xff9d9d, 0xff7d7d, 0xff5d5d, 0xff4040, 0xff2020, 0xff0000,
	0xffaa5d, 0xff9940, 0xff8920, 0xff7900, 0xe66d00, 0xce6100, 0xb65500,
	0x9d4c00, 0xffffda, 0xffffba, 0xffff9d, 0xffff7d, 0xfffa5d, 0xfff640,
	0xfff620, 0xfff600, 0xe6da00, 0xcec600, 0xb6ae00, 0x9d9d00, 0x858500,
	0x716d00, 0x595500, 0x404000, 0xd2ff5d, 0xc6ff40, 0xb6ff20, 0xa1ff00,
	0x91e600, 0x81ce00, 0x75b600, 0x619d00, 0xdaffda, 0xbeffba, 0x9dff9d,
	0x81ff7d, 0x61ff5d, 0x40ff40, 0x20ff20, 0x00ff00, 0x00ff00, 0x00ee00,
	0x00e200, 0x00d600, 0x04ca00, 0x04be00, 0x04b200, 0x04a500, 0x049900,
	0x048900, 0x047d00, 0x047100, 0x046500, 0x045900, 0x044c00, 0x044000,
	0xdaffff, 0xbaffff, 0x9dffff, 0x7dfffa, 0x5dffff, 0x40ffff, 0x20ffff,
	0x00ffff, 0x00e6e6, 0x00cece, 0x00b6b6, 0x009d9d, 0x008585, 0x007171,
	0x005959, 0x004040, 0x5dbeff, 0x40b2ff, 0x20aaff, 0x009dff, 0x008de6,
	0x007dce, 0x006db6, 0x005d9d, 0xdadaff, 0xbabeff, 0x9d9dff, 0x7d81ff,
	0x5d61ff, 0x4040ff, 0x2024ff, 0x0004ff, 0x0000ff, 0x0000ee, 0x0000e2,
	0x0000d6, 0x0000ca, 0x0000be, 0x0000b2, 0x0000a5, 0x000099, 0x000089,
	0x00007d, 0x000071, 0x000065, 0x000059, 0x00004c, 0x000040, 0x282828,
	0xffe234, 0xffd624, 0xffce18, 0xffc208, 0xffb600, 0xb620ff, 0xaa00ff,
	0x9900e6, 0x8100ce, 0x7500b6, 0x61009d, 0x500085, 0x440071, 0x340059,
	0x280040, 0xffdaff, 0xffbaff, 0xff9dff, 0xff7dff, 0xff5dff, 0xff40ff,
	0xff20ff, 0xff00ff, 0xe200e6, 0xca00ce, 0xb600b6, 0x9d009d, 0x850085,
	0x6d0071, 0x590059, 0x400040, 0xffeade, 0xffe2d2, 0xffdac6, 0xffd6be,
	0xffceb2, 0xffc6a5, 0xffbe9d, 0xffba91, 0xffb281, 0xffa571, 0xff9d61,
	0xf2955d, 0xea8d59, 0xde8955, 0xd28150, 0xca7d4c, 0xbe7948, 0xb67144,
	0xaa6940, 0xa1653c, 0x9d6138, 0x915d34, 0x895930, 0x81502c, 0x754c28,
	0x6d4824, 0x5d4020, 0x553c1c, 0x483818, 0x403018, 0x382c14, 0x28200c,
	0x610065, 0x006565, 0x006161, 0x00001c, 0x00002c, 0x302410, 0x480048,
	0x500050, 0x000034, 0x1c1c1c, 0x4c4c4c, 0x5d5d5d, 0x404040, 0x303030,
	0x343434, 0xdaf6f6, 0xbaeaea, 0x9ddede, 0x75caca, 0x48c2c2, 0x20b6b6,
	0x20b2b2, 0x00a5a5, 0x009999, 0x008d8d, 0x008585, 0x007d7d, 0x007979,
	0x007575, 0x007171, 0x006d6d, 0x990089
};
u32int bg = 0x777777;
int dx = 64, dy = 64;
int wall, sprite;
Biobuf *bi, *bo;
int fd;

#define	GBIT16(p)	((p)[0]|((p)[1]<<8))

void
put24(u32int v)
{
	uchar u[3];

	u[0] = v;
	u[1] = v>>8;
	u[2] = v>>16;
	Bwrite(bo, u, 3);
}

void
wrwall(uchar *s)
{
	int n;
	uchar *p, *e;

	n = dx * dy;
	e = s + dy;
	while(s < e){
		p = s + n;
		while(s < p){
			put24(pal[*s]);
			s += dy;
		}
		s -= n - 1;
	}
}

void
wrspr(uchar *s)
{
	int n, m, lx, rx;
	u32int u[64*64], *p, *qp, *e;
	uchar *c, *d, *qs;

	p = u;
	while(p < u + nelem(u))
		*p++ = bg;
	c = s;
	lx = GBIT16(c), c+=2;
	rx = GBIT16(c), c+=2;
	p = u + lx;
	e = u + rx;
	while(p <= e){
		d = s + GBIT16(c), c+=2;
		n = GBIT16(d) / 2, d+=2;
		while(n != 0){
			qs = s + (s16int)GBIT16(d), d+=2;
			m = GBIT16(d) / 2, d+=2;
			qs += m;
			qp = p + 64 * m;
			n -= m;
			while(n-- > 0){
				*qp = pal[*qs++];
				qp += 64;
			}
			n = GBIT16(d) / 2, d+=2;
		}
		p++;
	}
	p = u;
	while(p < u + nelem(u))
		put24(*p++);
}

void
wrvga(uchar *s)
{
	int n;
	uchar *e;

	n = dx * dy / 4;
	e = s + n;
	while(s < e){
		put24(pal[s[0]]);
		put24(pal[s[n]]);
		put24(pal[s[n*2]]);
		put24(pal[s[n*3]]);
		s++;
	}
}

static vlong
bsize(void)
{
	vlong n;
	Dir *d;

	d = dirfstat(Bfildes(bi));
	if(d == nil)
		sysfatal("bstat: %r");
	n = d->length;
	free(d);
	return n;
}

void
getpal(char *f)
{
	int i;
	uchar u[4];
	Biobuf *b;

	if((b = Bopen(f, OREAD)) == nil)
		sysfatal("getpal: %r");
	for(i=0; i<256; i++){
		if(Bread(b, u, 3) != 3)
			sysfatal("getpal: palette too short: %r");
		pal[i] = u[2]<<16 | u[1]<<8 | u[0];
	}
	Bterm(b);
}

void
usage(void)
{
	fprint(2, "usage: %s [-sw] [-b bg] [-p pal] [-x dx] [-y dy] [pic]\n", argv0);
	exits("usage");
}

void
main(int argc, char **argv)
{
	int n;
	char c[9];
	uchar *s;

	ARGBEGIN{
	case 'b': bg = strtoul(EARGF(usage()), nil, 0); break;
	case 'p': getpal(EARGF(usage())); break;
	case 's': sprite++; break;
	case 'w': wall++; break;
	case 'x': dx = strtoul(EARGF(usage()), nil, 0); break;
	case 'y': dy = strtoul(EARGF(usage()), nil, 0); break;
	default:
		usage();
	}ARGEND
	if(*argv != nil){
		fd = open(*argv, OREAD);
		if(fd < 0)
			sysfatal("open: %r");
	}
	bi = Bfdopen(fd, OREAD);
	bo = Bfdopen(1, OWRITE);
	if(bi == nil || bo == nil)
		sysfatal("Bfdopen: %r");
	n = sprite ? bsize() : dx * dy;
	s = mallocz(n, 1);
	if(s == nil)
		sysfatal("mallocz: %r");
	if(Bread(bi, s, n) != n)
		sysfatal("Bread: short read");
	Bprint(bo, "%11s %11d %11d %11d %11d ",
		chantostr(c, RGB24), 0, 0, dx, dy);
	if(wall)
		wrwall(s);
	else if(sprite)
		wrspr(s);
	else
		wrvga(s);
	free(s);
	exits(nil);
}
