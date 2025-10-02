#ifndef AS3911_ERRNO_H
#define AS3911_ERRNO_H


// application specific errorcodes

#define ERR_NONE			0		// Success
#define ERR_TIMEOUT			1		// Operation timeout out
#define ERR_COLLISION		2		// Bit collision error
#define ERR_CRC				3		// CRC incorrect
#define ERR_PARITY			4		// Rarity bit incorrect
#define ERR_HARD_FRAMING	5		// Some frame timing parameters or signal shape requirements have been violated, and the received data has been corrupted.
#define ERR_SOFT_FRAMING	6		// Some frame timing parameters have been violated, but the data has been correctly received.
#define ERR_NOTFOUND		7		// transponder not found
#define ERR_NOTUNIQUE		8		// transponder not unique - more than one transponder in field/
#define ERR_NOTSUPP			9		// requested operation not supported */
#define ERR_WRITE			10		// write error/
#define ERR_OVERFLOW		11		// fifo over or underflow error */
#define ERR_DONE			13		// transfer has already finished */
#define ERR_INTERNAL		14		// internal error */
#define ERR_RF_COLLISION	15		// ingroup error codes RF collision during RF collision avoidance occured */
#define ERR_PROTOCOL		16		// appended by KDK
#define ERR_DATA_TOO_BIG	17		// ingroup error codes RF collision during RF collision avoidance occured */
#define ERR_DATA_OVER		18		// appended by KDK
#define ERR_REQUEST			19		// appended by KDK
#define ERR_PARAM			20		// appended by KDK
#define ERR_NOMSG			21		// appended by KDK
#define ERR_INVALID_FORMAT	22		// appended by KDK
#define ERR_TRANSMISSION	23
#define ERR_AUTHENTICATE	24		// appended by KDK
#define ERR_EMV_PROTOCOL	25		// appended by KDK
#define ERR_EMV_STATUS		26		// appended by KDK
#define ERR_FILE_NOT_FOUND	27		// appended by KDK
#define ERR_READ_RECORD		28		// appended by KDK


#define ERR_CHIP			99		// appended by KDK


#endif
