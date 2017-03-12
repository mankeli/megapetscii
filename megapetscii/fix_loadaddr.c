#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	FILE *f1, *f2;
	f1 = fopen(argv[1], "rb");
	f2 = fopen(argv[2], "wb");
	uint16_t loadaddr;
	uint16_t addr;
	fread(&loadaddr, 2, 1, f1);
	printf("cur loadaddr: %04X\n", loadaddr);
	loadaddr+=0x20;
	// (int)strtol(data, NULL, 0)
	printf("new loadaddr: %04X\n", loadaddr);
	fwrite(&loadaddr, 2, 1, f2);
	addr = loadaddr; 
	while(1)
	{
		char tmp = fgetc(f1);
		if (feof(f1))
			break;
		fputc(tmp, f2);
		addr++;
	}
	fclose(f1);
	fclose(f2);

	printf("final addr: %04X\n", addr);

}