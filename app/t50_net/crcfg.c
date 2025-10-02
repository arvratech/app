void _CrSetDefault(CREDENTIAL_READER_CFG *cfg)
{
	cfg->readers		= 0x80;
	cfg->scFormat		= 0;
	cfg->wiegandFormats	= 0x00;
	cfg->authMode		= 1;
	cfg->ledBeep		= 1;
	cfg->option			= 0x00;
	SetDefaultIPAddress(cfg->ipReaderIpAddress);
	cfg->ipReaderPort	= 6000;
}

int _CrEncode(CREDENTIAL_READER_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->readers;
	*p++ = cfg->scFormat;
	*p++ = cfg->wiegandFormats;
	*p++ = cfg->authMode;
	*p++ = cfg->ledBeep;
	*p++ = cfg->option;
	memcpy(p, cfg->ipReaderIpAddress, 4); p += 4;
	SHORTtoBYTE(cfg->ipReaderPort, p); p += 2;
	return p - (unsigned char *)buf;
}

int _CrDecode(CREDENTIAL_READER_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->readers		= *p++;
	cfg->scFormat		= *p++;
	cfg->wiegandFormats	= *p++;
	cfg->authMode		= *p++;
	cfg->ledBeep		= *p++;
	cfg->option			= *p++;
	memcpy(cfg->ipReaderIpAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &cfg->ipReaderPort); p += 2;
	return p - (unsigned char *)buf;
}

BOOL crIsEnableScReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x80) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableScReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x80;
	else	sys_cfg->credentialReaders[0].readers &= 0x7f;
}

BOOL crIsEnableEmReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x40) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableEmReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x40;
	else	sys_cfg->credentialReaders[0].readers &= 0xbf;
}

BOOL crIsEnableWiegandReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x20) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableWiegandReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x20;
	else	sys_cfg->credentialReaders[0].readers &= 0xdf;
}

BOOL crIsEnableSerialReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x10) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableSerialReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x10;
	else	sys_cfg->credentialReaders[0].readers &= 0xef;
}

BOOL crIsEnableIpReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x08) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableIpReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x08;
	else	sys_cfg->credentialReaders[0].readers &= 0xf7;
}

BOOL crIsEnablePinReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x02) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnablePinReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x02;
	else	sys_cfg->credentialReaders[0].readers &= 0xfd;
}

BOOL crIsEnableFpRedaer(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x01) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableFpReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x01;
	else	sys_cfg->credentialReaders[0].readers &= 0xfe;
}

int crScFormat(void *cr)
{
	int		rval;

	rval = sys_cfg->credentialReaders[0].scFormat;
	if(rval > 2) rval = 0;
	return rval;
}

void crSetScFormat(void *cr, int cf)
{
	sys_cfg->credentialReaders[0].scFormat = cf;
}

BOOL crIsEnableWiegandFormat(void *cr, int wfId)
{
	unsigned char	val;
	BOOL	enable;

	val = 0x80 >> wfId;
	if(sys_cfg->credentialReaders[0].wiegandFormats & val) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableWiegandFormat(void *cr, int wfId, BOOL enable)
{
	unsigned char	val;

	val = 0x80 >> wfId;
	if(enable) sys_cfg->credentialReaders[0].wiegandFormats |= val;
	else	   sys_cfg->credentialReaders[0].wiegandFormats &= ~val;
}

BOOL crIsEnableCardReaders(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0xf1) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

int crMainKeypad(void *cr)
{
	int		mode;

	mode = crAuthMode(cr);
	if(crIsEnableScReader(cr) && (mode == 0 || mode > 2)) mode = 1;
	else	mode = 0;
	return mode;
}

// 0:General  1:Card  2:Card+PIN  3:Card or PIN  4:PIN
int crAuthMode(void *cr)
{
	BOOL	cf, pf;
	int		val;

	cf = crIsEnableCardReaders(cr);
	pf = crIsEnablePinReader(cr);
	val = sys_cfg->credentialReaders[0].authMode;
	if(!cf && !pf || val > 4) val = 0;
	else if(cf && !pf && val > 1) val = 0;
	else if(!cf && pf && (val > 0 && val < 4)) val = 0;
	return val;
}

void crSetAuthMode(void *cr, int authMode)
{
	sys_cfg->credentialReaders[0].authMode = authMode;
}

int crLedBeep(void *cr)
{
	int		val;

	val = sys_cfg->credentialReaders[0].ledBeep;
	return val;
}

void crSetLedBeep(void *cr, int ledBeep)
{
	sys_cfg->credentialReaders[0].ledBeep = ledBeep;
}
