#include "CRC32.h"

ulong ComputeCRC(ulong ulCRC, const void *pv, size_t cbLength)
{
	uint i;
	const byte *pbBuffer = (const byte *) pv;

	const uint cbAlignedOffset = 
		((cbLength < sizeof(ulong)) ? 0 : (size_t)((ulong)pv % sizeof(ulong)));
	const uint cbInitialUnalignedBytes = 
		((cbAlignedOffset == 0)          ? 0 : (sizeof(ulong) - cbAlignedOffset));
	const uint cbRunningLength = 
		((cbLength < sizeof(ulong)) ? 0 : ((cbLength - cbInitialUnalignedBytes) / 8) * 8);
	const uint cbEndUnalignedBytes = cbLength - cbInitialUnalignedBytes - cbRunningLength;

	for(i=0; i < cbInitialUnalignedBytes; ++i) 
		ulCRC = CrcTableOffset32[(ulCRC ^ *pbBuffer++) & 0x000000FF] ^ (ulCRC >> 8);   

	for(i=0; i < cbRunningLength/4; ++i)
	{
		ulCRC ^= *(ulong *)pbBuffer;
		ulCRC = CrcTableOffset56[ ulCRC        & 0x000000FF] ^
			CrcTableOffset48[(ulCRC >>  8) & 0x000000FF] ^
			CrcTableOffset40[(ulCRC >> 16) & 0x000000FF] ^ 
			CrcTableOffset32[(ulCRC >> 24) & 0x000000FF];
		pbBuffer += 4;
	}

	for(i=0; i < cbEndUnalignedBytes; ++i) 
		ulCRC = CrcTableOffset32[(ulCRC ^ *pbBuffer++) & 0x000000FF] ^ (ulCRC >> 8);

	return ulCRC;         
}