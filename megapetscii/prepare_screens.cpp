#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <vector>

std::vector<uint8_t> load_marq(const char *fn)
{
	FILE *f;
	std::vector<uint8_t> picture;

	f = fopen(fn, "rb");
	if (!f)
	{
		printf("unable to open file '%s'\n", fn);
		return picture;
	}
	while(!feof(f))
	{
		char rivi[1024];
		fgets(rivi, 1024, f);
		if (strpbrk(rivi, "0123456789") == rivi)
		{
//			printf("'%s'\n",rivi);

			char *rivi_ptr = rivi;
			int accu = 0;
			bool last_push = false;
			while(1)
			{
				if (*rivi_ptr == ',')
				{
					picture.push_back(accu);
					accu = 0;
					last_push = true;
				}
				else if (*rivi_ptr == '\n' || *rivi_ptr == '\r')
				{
					if (!last_push)
						picture.push_back(accu);
					break;
				}
				else 
				{
					accu *= 10;
					accu += *rivi_ptr - '0';
					last_push = false;
				}
				rivi_ptr++;
			}
		}
	}
	fclose(f);

	return picture;
}

uint16_t bundlebase = 0x4E00;
const int numslots = 5;

uint16_t get_slot(int bundleidx, int bundlesize)
{
	int datasize = (int)ceilf(bundlesize/5.f) * 256;
	int datapos = bundlebase + (bundleidx % numslots) * datasize;
	return datapos;
}

using std::vector;
vector<uint8_t> pic;
int main(int argc, char *argv[])
{
	pic = load_marq(argv[1]);
	int picsize = (2 + 40*25 + 40*25);
	int picnum = pic.size() / picsize;
	printf("size: %lu. %i pics, should be %i\n", pic.size(), picnum, picnum * picsize);
	fflush(stdout);

	int columns_per_file = atoi(argv[2]);

	FILE *f2 = fopen("bundledata.h", "w");
	fprintf(f2, "uint16_t bundle_addrs[] = {");

	int i;
	bool file_end = false;
	int colidx = 0;
	int bundleidx = 0;
	while(!file_end)
	{
		char fn[256];
		sprintf(fn, "out/%02x.pet", bundleidx);
		FILE *f = fopen(fn, "wb");

		int datasize = (25*2);
		int bunchsize = datasize * columns_per_file;

		int addr = 0x10000;
		uint16_t addr_w;

		addr = get_slot(bundleidx, columns_per_file);

		fprintf(f2, "0x%04X, ", addr);

		printf("bundle %02X.pet goes to 0x%04X\n", bundleidx, addr);
		
		addr_w = addr;
		fwrite(&addr_w, 2, 1, f);

		for (i = 0; i < columns_per_file; i++)
		{
			char col[25*2];

			int pici = colidx / 40;
			int piccol = colidx % 40;

			if (pici >= picnum)
			{
				file_end = true;

				int dp;
				for (dp = 0; dp < 25; dp++)
				{
					col[dp] = 1;
					col[dp+25] = 1;
				}
			}
			else
			{
				int dp;
				for (dp = 0; dp < 25; dp++)
				{
					col[dp]    = pic[pici * picsize + 2 + piccol + dp * 40];
					col[dp+25] = pic[pici * picsize + 2 + piccol + dp * 40 + 40*25];
				}
			}
//			printf("writing column at: %04X\n", ftell(f));
			//fflush(stdout);
			fwrite(col, 1, 25*2, f);

			uint8_t bundleidx_byte = bundleidx;
			fwrite(&bundleidx_byte, 1, 1, f);


			if ((i % 5) == 4)
			{
//				printf("writing padding at: %04X\n", ftell(f));
			//	fflush(stdout);

				uint8_t stuffing = colidx;

/*
				stuffing ^= bundleidx_byte;

				if (bundleidx == 0x22)
					stuffing ^= 0xFF;

				if (bundleidx == 0x23)
					stuffing ^= 0xFF;

				if (bundleidx == 0x26)
					stuffing ^= 0xFF;
*/

/*
				if (bundleidx == 0x29)
					stuffing ^= 0xFF;
				if (bundleidx == 0x2B)
					stuffing ^= 0xFF;
				if (bundleidx == 0x31)
					stuffing ^= 0xFF;
				if (bundleidx == 0x34)
					stuffing ^= 0xFF;
				if (bundleidx == 0x45)
					stuffing ^= 0x57;
*/

				//stuffing = bundleidx_byte;
				printf("stuffing byte for bundle %02X: %02X\n", bundleidx, stuffing);

				fwrite(&stuffing, 1, 1, f);
			}


			colidx++;

		}
		fclose(f);

		bundleidx++;
	}

	fprintf(f2, "};\n");
	fprintf(f2, "const int bundle_addr_count = %i;\n", bundleidx);
	fclose(f2);

	return 0;
}
