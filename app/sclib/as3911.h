/*
 * \brief This part of the AS9311 module abstracts Communication with an ISO14443-A
 * or ISO14443-B card including features required for EMV. This part of the AS3911
 * module consists of functions to configure paramters of the datatarnsmission to
 * and from an RFID card and functions to actually perfrom the transmissions.
 *
 * \section sec_as3911RfidCom_1 Receiver Deadtime
 *
 * After the data transmission from the reader is completed the receivers will be kept
 * disbabled until the receiver dead time has expired. This dead time ensures that
 * the receiver does not pick up noise still present from the transmit operation.
 *
 * For EMV the standard specifies that the PCD must ignore any load modulation produced
 * by the PICC for a certain time immediately after the completion of the PCD message.
 * The receveiver dead time can be used to implement this EMV deaf time.
 *
 * \section sec_as3911RfidCom_2 Frame Delay Time
 *
 * The frame delay time is a forced delay between the end of the last message received
 * from a card and the beginning of the next message send by the PCD.
 *
 * If a frame delay time greater than zero is configured then as3911Transmit() and
 * as3911Transceive() make sure that this frame delay time is kept. If necessary they
 * will wait with starting the data transmission until the set frame delay time has
 * passed.
 *
 * If no card response was received after the last message transmitted by the PCD
 * (as3911Receive() or as3911Transceive() returned with a timeout error), then no frame
 * delay time will be applied to the next data transmission.
 *
 * \section sec_as3911RfidCom_3 Receive Timeout
 *
 * The receive timeout specifies the time the AS3911 will wait for a card response
 * after it has completed the transmission of the reader message. After the receive 
 * timeout expires the receiver will be disabled internally and no further card
 * responses can be received. If a receive timeout occures while a reception is in
 * progress, then this reception continues until the message from the card has been
 * completely received. So the receive timeout specifies a timeout for the start of
 * the response from the RFID card.
 *
 * If as3911Transceive is used then the receive timeout can be specified as a paramter
 * to the functino call. If as3911Transmit() and as3911Receive() are used seperately
 * then the receive timeout must be set via as3911SetReceiveTimeout() prior to the call
 * to as3911Transmit(). The currently active receive timeout is used until a new
 * receive timeout is set.
 *
 * \section sec_as3911RfidCom_4 Error Handling
 *
 * To support EMV special error handling capabilities are build into the AS3911 RFID
 * communication code.
 * 
 * If EMV error handling is disabled, then no special error handling takes place and
 * any reception errors are reported via the return values of as3911Transmit() and
 * as3911Transceive() respectively.
 *
 * If EMV error handling is enabled then a second parameter defines the details of the
 * error handling namely the transmission error threshold. If a transmission error
 *(parity error, crc error, error, data encoding error, or frame timing error) occures
 * during reception of a frame whose overall length is less than or equal to the
 * tranmsission error threshold, then this received frame is ignored and the receiver
 * is reenabled to receive another card response. Also when EMV error handling is
 * enabled then any frame with residual bits will be ignored no matter how long that
 * frame was.
 *
 * This is handled transparent to the caller. I.e. the caller will not be informed
 * that an errornous frame has been ignored. Instead he will either get a timeout
 * error reported if no further frame follows the ingored frame, or he will be informed
 * about the content and error status of any frame received subsequent to the ignored
 * one.
 *
 * The transmission error threshold is compared against the received message length
 * as reported by the AS3911. This length may or may not include the CRC bytes depending
 * on the selected CRC processing mode 
 * (see usage of ::AS3911RequestFlags_t in as3911Transceive()).
 *
 * \section sec_as3911RfidCom_5 Modulation Depth Adjustment
 *
 * The AS3911 RFID communication module can adjust the modulation depth for the PCD to
 * PICC communication basedon the the measured RF amplitude (on the RFI pins) or on the
 * phase difference between the RFO and RFI pins. Both of these parameters depend on
 * the coupling between the PCD and PICC antennas. So this provides a method to adjust
 * the modulation depth based on the strength of the coupling between the PCD and PICC.
 * 
 * To adjust the modulation depth, the selected parameter is measured and then a table
 * lookup is performed to derive the apropriate RF modulation on driver level.
 * This lookup table defines a piece wise linear function. I.e. if the measured paramter
 * value is not found in the table then a linear interpolation between the two closest
 * values is performed. If the measured parameter value is out of the table range
 * (either lower than the smallest x table value or higher than the highest x table value),
 * then the RF on modulation driver level associated with the lowest or highest table
 * entry is used.
 *
 * The modulation depth adjustment mode and the lookup table can be configured via
 * as3911SetModulationLevelMode(). An adjustment of the modulation depth can be performed
 * at any time by a call to as3911AdjustModulationLevel(). This adjustment is done
 * automatically when as3911Transmit() or as3911Transceive() is called.
 *
 * \section sec_as3911RfidCom_6 Data Transmission and Reception
 *
 * Data can be transceived either by using as3911Transmit() and as3911Receive() directly
 * as a two step process or by calling as3911Transceive(). The later of which will perform
 * the complete transmit and receive opertion and also allows to specify some additional
 * options not available when using as3911Transmit() and as3911Receive() separately.
 *
 * If the two step approach is used then care must be taken that as3911Receive() is called
 * in time not to loose any data received from the PICC. Also the user must then take care
 * that any necessary interupts are enabled. The necessary interrupts are
 * ::AS3911_IRQ_MASK_TXE and ::AS3911_IRQ_MASK_WL for as3911Transmit(),
 * and ::AS3911_IRQ_MASK_RXS, ::AS3911_IRQ_MASK_RXE, and ::AS3911_IRQ_MASK_WL for data
 * reception with as3911Receive(). Additionally error interrupts should be enabled as needed.
 */

#ifndef _AS3911_H
#define _AS3911_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <stddef.h>
#include "ams_types.h"
#include "as3911_def.h"
#include "as3911_errno.h"
#include "as3911_io.h"
#include "as3911_stream.h"
#include "as3911_modulation_adjustment.h"
#include "as3911_interrupt.h"

extern unsigned char	opControl;


int  as3911Open(void);
void as3911Close(void);


/*! \ingroup as3911RfidCom
 *****************************************************************************
 * \brief Enable or disable emv layer 4 exception processing.
 *
 * Enable or disable reception error processing as required by the EMV standard
 * for a PICC that has been activated.
 * 
 * \param [in] enableEmvExceptionProcessing
 *****************************************************************************
 */
void as3911EnableEmvExceptionProcessing(bool_t enableEmvExceptionProcessing);


/*! \ingroup as3911RfidCom
 *****************************************************************************
 * \brief Check EMV exception processing status.
 *
 * \return true: EMV layer 4 exception processing is enabled.
 * \return false: EMV layer 4 exception processing is disabled.
 *****************************************************************************
 */
bool_t as3911EmvExceptionProcessingIsEnabled(void);

/*! \ingroup as3911RfidCom
 *****************************************************************************
 * \brief Set the transmission error threshold.
 *
 * If EMV exception processing is enabled, then all messages with a transmission
 * error (CRC error, parity error, residual bits error) and an overall length
 * lower than or equal to the transmission error threshold will be ignored.
 * 
 * \param[in] transmissionErrorThreshold Transmission error threshold (bytes).
 *****************************************************************************
 */
void as3911SetTransmissionErrorThreshold(int transmissionErrorThreshold);

/*! \ingroup as3911RfidCom
 *****************************************************************************
 * \brief Get the transmission error threshold.
 *
 * \return The transmission error threshold.
 *****************************************************************************
 */
int as3911GetTransmissionErrorThreshold(void);

/*! \ingroup as3911RfidCom
 *****************************************************************************
 * \brief Set the receiver deadtime.
 *
 * The receiver dead time is the time for which the receiver is still disabled
 * after the end of the message send by the AS3911. The as3911 will not process
 * any card response received during the receiver dead time.
 *
 * In NFCIP1 mode the receiver deadtime has a granularity of 512 carrier cycles
 * and ranges from ?? to 130560 carrier cycles (~9.3ms). In all other modes the
 * receiver deadtime has a granularity of 64 carrier cycles and ranges from 256
 * to 16320 carrier cycles (~1.2ms). Using values below the minimum value will
 * lead to the minimum value being used. Using values above the maximum allowed
 * value leads to undefined behavior.
 *
 * \param[in] receiverDeadtime Receiver deadtime in carrier cycles.
 *****************************************************************************
 */
void as3911SetMaskReceiveTime(int maskReceiveTime);
void as3911SetNoResponseTime(int noResponseTime);
void as3911SetFrameDelayTime(int frameDelayTime);

int  as3911TransceiveBitData(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout);
int  as3911TransceiveRawBitData(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout);
int  as3911TransceiveStream(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout);
int  as3911TransceiveData(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int maxLen, int *rxLen, int timeout);
int  as3911TransmitData(unsigned char *buf, int len, int timeout);
int  as3911PollData(unsigned char *buf, int maxLen, int *len);

void as3911ExecuteCommandAndGetResult(int cmd, int resreg, int sleeptime, unsigned char* result);
void as3911OnField(void);
void as3911OffField(void);
void as3911SetRfoNormalLevel(int rfoLevel);
int  as3911AdjustRegulators(int *result_mV);
void as3911MeasureRF(unsigned char *result);
void as3911MeasureAntennaResonance(unsigned char *result);
void as3911CalibrateAntenna(unsigned char *result);
void as3911CalibrateModulationDepth(unsigned char *result);
int  as3911MeasureVoltage(int mpsv);
void as3911SetVoltage(int VoltVal);
void as3911SetRxChannel(int Mode);
void as3911SetFirstStageGainReduction(int AMVal, int PMVal);
void as3911SetSecondStageGainReduction(int AMVal, int PMVal);

void emvInitLayer4(void);
int  emvTransceiveApdu(unsigned char *apdu, int apduLen, unsigned char *rxBuf, int maxLen, int *rxLen);


#endif
