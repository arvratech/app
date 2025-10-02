#include <stdio.h>
#include "hw.h"
#include "cf.h"
#include "syscfg.h"
#include "sccfg.h"


int CrScReaderEnable(void)
{
	void	*cr;

	cr = crsGet(0);
	return crIsEnableScReader(cr);
}

int CrScReaderFormat(void)
{
	void	*cr;

	cr = crsGet(0);
	return crScFormat(cr);
}

int CrIsEnableHynixEmvReader(void)
{
    void    *cr;

    cr = crsGet(0);
    return crIsEnableHynixEmvReader(cr);
}

int CrRfoNormalLevel(void)
{
	return syscfgScRfoNormalLevel(NULL);
}

void CrSetScReader(int enable)
{
}

int CrUidFormatCardType(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_UID);
	return cfCardCardType(cf);
}

int CrBlockFormatCardType(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_BLOCK);
	return cfCardCardType(cf);
}

int CrBlockFormatCardApp(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_BLOCK);
	return cfCardCardApp(cf);
}

int CrIso7816FormatCardType(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_UID);
	return cfCardCardType(cf);
}

int CrIso7816FormatCardApp(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_UID);
	return cfCardCardApp(cf);
}

unsigned char *CrMifareKey(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_BLOCK);
	return cfCardMifareKey(cf);
}

int CrBlockNumber(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_BLOCK);
	return cfCardBlockNoSingle(cf);
}

int CrBlockDataLength(void)
{
	CF_CARD		*cf;

    cf = cfCardsGet(CFCARD_SC_BLOCK);
	return cfCardBlockNoSingle(cf);
}

unsigned char *CrAID(void)
{
	CF_CARD		*cf;
	int		length;

    cf = cfCardsGet(CFCARD_SC_FILE);
	return cfCardGetAid(cf, &length);
}

int CrAIDLength(void)
{
	CF_CARD		*cf;
	int		length;

    cf = cfCardsGet(CFCARD_SC_FILE);
	cfCardGetAid(cf, &length);
	return length;
}

int CrIso7816SFI(void)
{
	return 0;
}

int CrAFI(void)
{
	return 0;
}

int CrIso7816RecordNumber(void)
{
	return 0;
}

int CrIso7816ReadLength(void)
{
	return 0;
}

int CrIso7816DataOffset(void)
{
	return 0;
}

int CrIso7816DataLength(void)
{
	return 0;
}

int CrIso7816EnableSelectMF(void)
{
	return 0;
}
