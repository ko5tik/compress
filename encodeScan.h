#pragma ident "@(#) encodeScan.h 1.2@(#) Copyright © by Konstantin Priblouda 1997 all rigths reserved"

/* encodes scan as fast as possible , returns 
   amount of bytes actually encoded */

int encodeScan(void* scanPtr, /* pointer to the scan data */
	       int scanWidth, /* width of scan */
	       int scanHeight, /* height of scan */
	       void* destPtr, /* destination pointer */
	       void* huffPtr /* huffman data pointer */
	       ) ;
