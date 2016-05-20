#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <climits>

#define OUTBUFSIZ  100000

z_stream z;
Bytef outbuf[OUTBUFSIZ];

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

Bytef  *zcodecom(int mode, Bytef *inbuf, int inbuf_len, int *resultbuf_len)
{
    int count, status;
    Bytef *resultbuf;
    int total_count = 0;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (mode == 0) {
	deflateInit(&z, 1);
    } else {
	inflateInit(&z);
    }

    z.next_out = outbuf;
    z.avail_out = OUTBUFSIZ;
    z.next_in = inbuf;
    z.avail_in = inbuf_len;

    resultbuf = (Bytef*) malloc(OUTBUFSIZ);

    while (1) {
		if (mode == 0) {
			status = deflate(&z, Z_FINISH);
		} else {
			status = inflate(&z, Z_NO_FLUSH);
		}
		if (status == Z_STREAM_END) break;
		if (status != Z_OK) {
			std::cout << "status !Z_OK" << std::endl;
			if (mode == 0) {
				deflateEnd(&z);
			} else {
				inflateEnd(&z);
			}
			*resultbuf_len = 0;
			return(resultbuf);
		}
		if (z.avail_out == 0) {
			resultbuf = (Bytef*) realloc(resultbuf, total_count + OUTBUFSIZ);
			memcpy(resultbuf + total_count, outbuf, OUTBUFSIZ);
			total_count += OUTBUFSIZ;
			z.next_out = outbuf;
			z.avail_out = OUTBUFSIZ;
		}
    }
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
		resultbuf = (Bytef*) realloc(resultbuf, total_count + OUTBUFSIZ);
		memcpy(resultbuf + total_count, outbuf, count);
		total_count += count;
    }
    if (mode == 0) {
		deflateEnd(&z);
    } else {
		inflateEnd(&z);
    }
    *resultbuf_len = total_count;
    return(resultbuf);
}

Bytef  *zencode(Bytef *inbuf, int inbuf_len, int *resultbuf_len)
{
	return zcodecom(0, inbuf, inbuf_len, resultbuf_len);
}

Bytef  *zdecode(Bytef *inbuf, int inbuf_len, int *resultbuf_len)
{
	return zcodecom(1, inbuf, inbuf_len, resultbuf_len);
}

void pm9screw_ext_fopen(FILE *fp, int header_len, int key_len, short * pm9screw_mycryptkey)
{
	struct	stat	stat_buf;
	Bytef	*datap, *newdatap;
	int	datalen, newdatalen;
	int	cryptkey_len = key_len;

	int	i;

	fstat(fileno(fp), &stat_buf);
	datalen = stat_buf.st_size - header_len;
	datap = (Bytef*)malloc(datalen);
	fseek ( fp , header_len , SEEK_SET );
	fread(datap, datalen, 1, fp);
	fclose(fp);
	
	std::cout << "|_ cryptkey length " << cryptkey_len << std::endl;
	std::cout << "|_ read " << datalen << " bytes" << std::endl;

	for(i=0; i<datalen; i++) {
		datap[i] = (Bytef)pm9screw_mycryptkey[(datalen - i) % cryptkey_len] ^ (~(datap[i]));
	}
	
	newdatap = zdecode(datap, datalen, &newdatalen);
	
	std::cout << "|_ decoded count " << newdatalen << " bytes" << std::endl << std::endl;

	for(i=0; i<newdatalen; i++) {
		std::cout << newdatap[i];
	}
	
	free(datap);
	free(newdatap);
}

int main( int argc, const char* argv[] ) {
	std::string hex(argv[3]);
	int len = hex.length() / 4, k = 0;
	short * key = new short[len];

	for (int i=0; i<hex.length(); i+=4) {
		unsigned short v = 0;
		std::stringstream ss;
		std::string h = hex.substr(i, 4);
		ss << std::hex << h.c_str();
		ss >> v;
		v = swap_endian<short>(v);
		key[k++] = v;
 	}

	FILE * f = fopen(argv[1], "r");
	std::cout << "IN: " << argv[1] << std::endl;
	pm9screw_ext_fopen(f, atoi(argv[2]), k, key);
	return 0;
}