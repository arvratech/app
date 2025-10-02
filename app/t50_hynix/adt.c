// pv: 0=Lock  1=Unlock  2=PulseUnlock  3=ExtendedPulseUnlock
// priority: 1(highest) .. 8(lowest)
void _adPresentValue(void *self, int *pPv, int *pPiority)
{
	ACCESS_DOOR		*ad = self;
	int		i, pv, priority;

	priority = 1;
	for(i = 0;i < 4;i++) {
		pv = ad->priorityArray[i] >> 4;
		if(pv < 4) break;
		priority++;
		pv = ad->priorityArray[i] & 0x0f;
		if(pv < 4) break;
		priority++;
	}
	if(priority > 8) {
		pv = 0; priority = 8;
	}
	*pPv = pv; *pPriority = priority;
}

void _adSetPresentValue(void *ad, int pv, int priority)
{
	ACCESS_DOOR		*ad = self;
	int		i, val, opv, opriorita, add;

	opriority = 1;
	for(i = 0;i < 4;i++) {
		opv = ad->priorityArray[i] >> 4;
		if(opv < 4) break;
		opriority++;
		opv = ad->priorityArray[i] & 0x0f;
		if(opv < 4) break;
		opriority++;
	}
	i = (priority-1) >> 1;
	val = ad->priorityArray[i];
	if(priority & 1) val = (val & 0x0f) | (pv << 4);
	else			 val = (val & 0xf0) | pv;
	ad->priorityArray[i] = val;
}

