#ifndef _SC_MIFARE_CRC_H_
#define _SC_MIFARE_CRC_H_


/*! 
 *****************************************************************************
 *  \brief  Calculate CRC according to CCITT standard.
 *
 *  This function takes \a length bytes from \a buf and calculates the CRC
 *  for this data. The result is returned.
 *  \note This implementation calculates the CRC with LSB first, i.e. all
 *  bytes are "read" from right to left.
 *
 *  \param[in] preloadValue : Initial value of CRC calculation.
 *  \param[in] buf : buffer to calculate the CRC for.
 *  \param[in] length : size of the buffer.
 *
 *  \return 16 bit long crc value.
 *
 *****************************************************************************
 */
unsigned short crcCalculateCcitt(unsigned short preloadValue, const unsigned char* buf, unsigned short length);


#endif

