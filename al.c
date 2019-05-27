#include <u.h>
#include <libc.h>
#include <bio.h>

enum{
	Mchr = 0x20,
	Msca = 0x40,
	Matk = 0x60,
	Msus = 0x80,
	Mwav = 0xe0,
	Cchr = Mchr + 3,
	Csca = Msca + 3,
	Catk = Matk + 3,
	Csus = Msus + 3,
	Cwav = Mwav + 3,
	FreqL = 0xa0,
	FreqH = 0xb0
};
uchar inst[] = {Mchr, Cchr, Msca, Csca, Matk, Catk, Msus, Csus, Mwav, Cwav};
uchar *out;
Biobuf *bf, *of;

#define	PBIT16(p,v)	(p)[0]=(v);(p)[1]=(v)>>8

u8int
get8(void)
{
	u8int v;

	if(Bread(bf, &v, 1) != 1)
		sysfatal("get8: short read: %r");
	return v;
}

u16int
get16(void)
{
	u8int v;

	v = get8();
	return get8()<<8 | v;
}

u32int
get32(void)
{
	u16int v;

	v = get16();
	return get16()<<16 | v;
}

void
main(int argc, char **argv)
{
	int n, fd;
	u8int *i, *p, v, blk;

	fd = 0;
	if(argc > 1){
		fd = open(argv[1], OREAD);
		if(fd < 0)
			sysfatal("open: %r");
	}
	bf = Bfdopen(fd, OREAD);
	of = Bfdopen(1, OWRITE);
	if(bf == nil || of == nil)
		sysfatal("Bfdopen: %r");
	n = get32();
	out = mallocz(2 + 4 * (nelem(inst) + n*2 + 1 + 1), 1);
	p = out + 2;
	*p++ = 0x01;
	*p++ = 0x20;
	p += 2;
	get16();
	i = inst;
	while(i < inst+nelem(inst)){
		*p++ = *i++;
		*p++ = get8();
		p += 2;
	}
	Bseek(bf, 6, 1);
	blk = (get8() & 7) << 2 | 0x20;
	while(n--){
		v = get8();
		if(v == 0){
			*p = FreqH;
			p += 2;
			PBIT16(p, 1);
			p += 2;
		}else{
			*p++ = FreqL;
			*p++ = v;
			p += 2;
			*p++ = FreqH;
			*p++ = blk;
			PBIT16(p, 1);
			p += 2;
		}
	}
	*p++ = FreqH;
	p += 3;
	n = p-out - 2;
	PBIT16(out, n);
	Bwrite(of, out, p-out);
	exits(nil);
}
