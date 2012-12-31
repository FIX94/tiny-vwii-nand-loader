/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <cmath>
#include <x86intrin.h>

unsigned char dol_header[0x100];
#define ALIGN32(x)			(((x) + 31) & ~31)

int main()
{
	char nullBuf[1];
	nullBuf[0] = 0x00;
	memset(&dol_header, 0, 0x100);
	/* Read in shit */
	FILE *f = fopen("loader.bin", "rb");
	if(f == NULL)
	{
		printf("no loader\n");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	unsigned int size = ftell(f);
	rewind(f);
	unsigned char *buf = (unsigned char*)malloc(size);
	fread(buf, size, 1, f);
	fclose(f);
	f = fopen("stub.bin", "rb");
	if(f == NULL)
	{
		printf("no stub\n");
		free(buf);
		return -2;
	}
	fseek(f, 0, SEEK_END);
	unsigned int stub_size = ftell(f);
	rewind(f);
	unsigned char *stub_buf = (unsigned char*)malloc(stub_size);
	fread(stub_buf, stub_size, 1, f);
	fclose(f);
	/* Add stub text */
	((unsigned int*)dol_header)[0] = _bswap(0x100); /* Header size */
	((unsigned int*)dol_header)[18] = _bswap(0x80003400);
	((unsigned int*)dol_header)[36] = _bswap(ALIGN32(stub_size));
	/* Add NAND Loader text */
	((unsigned int*)dol_header)[1] = _bswap(0x100 + ALIGN32(stub_size));
	((unsigned int*)dol_header)[19] = _bswap(0x80004000);
	((unsigned int*)dol_header)[37] = _bswap(ALIGN32(size));
	/* Set our entry to the NAND Loader */
	((unsigned int*)dol_header)[56] = _bswap(0x80004000);
	/* Write it to a new file */
	f = fopen("00000001.app", "wb");
	fwrite(dol_header, 0x100, 1, f); /* Write header first */

	fwrite(stub_buf, stub_size, 1, f); /* Then stub */
	if(ALIGN32(stub_size) > stub_size)
	{
		for(int i = 0; i < (ALIGN32(stub_size) - stub_size); ++i)
			fwrite(nullBuf, 1, 1, f);
	}

	fwrite(buf, size, 1, f); /* and last nand loader */
	if(ALIGN32(size) > size)
	{
		for(int i = 0; i < (ALIGN32(size) - size); ++i)
			fwrite(nullBuf, 1, 1, f);
	}

	fclose(f);
	free(buf);
	free(stub_buf);
	printf("done\n");
	return 0;
}
