char cjk_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ȸ�� �̸�",					// 0
	"�����ID �Է�",				// 1
	"�����ID Ȯ��",				// 2
	"��ȣ �Է�",					// 3
	"��ȣ Ȯ��",					// 4
	"�� ��ȣ �Է�",					// 5
	"�� ��ȣ Ȯ��",					// 6
	"�������Է��Ͻʽÿ�",			// 7
	"������ �ٽ��Է��Ͻʽÿ�",		// 8
	"ī�带�Է��Ͻʽÿ�",			// 9
	"ī���� �ٽ��Է��Ͻʽÿ�",		// 10
	"�ѹ� ���Է��Ͻʽÿ�",			// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"��",							// 14
	"��",							// 15
    "�����Ͻðڽ��ϱ� ?",			// 16
	"���� ��ȭ��ȣ�� ��������",		// 17
	"���� ��ȣ�� ��������",			// 18
	"������ �����Ͻðڽ��ϱ�?",		// 19
	"��ø� ��ٷ� �ֽʽÿ�"		// 20
};
//	"New Time",					// 12
//	"New TID",					// 13

char cjk_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ó����...",					// 0
	"�ý���\n�����Դϴ�",			// 1
	"�Է� �ð���\n�ʰ��Ǿ����ϴ�",	// 2
	"�߸���\n�����ID�Դϴ�",		// 3
	"�߸���\n����Ÿ�Դϴ�",			// 4
	"�̵��\n������Դϴ�",			// 5
	"������ �ź��Ͽ����ϴ�",		// 6
	"�����Ͽ����ϴ�",				// 7	����ڸ� Ȯ��\n�Ͽ����ϴ�
	"�̹� ��ϵ�\n������Դϴ�",	// 8
	"�̹� ��ϵ�\nī���Դϴ�",		// 9
	"�̹� ��ϵ�\n�����Դϴ�",		// 10
	"��ȣ��\nƲ���ϴ�",				// 11
	"����Ÿ��\nƲ���ϴ�",			// 12
	"����Ÿ��\n�����Ͽ����ϴ�",		// 13
	"����ڸ�\n�����Ͽ����ϴ�",		// 14
	"��ü ����ڸ�\n�����Ͽ����ϴ�",	// 15
	"�߸���\n�ð� �����Դϴ�",		// 16
	"�߸���\n�ܸ���ID�Դϴ�",		// 17
	"�߸���\n��¥�Դϴ�",			// 18
	"��¥��\n�����Ͽ����ϴ�",		// 19
	"�߸���\n�ð��Դϴ�",				// 20
	"�ð���\n�����Ͽ����ϴ�",			// 21
	"�߸���\nIP�ּ��Դϴ�",			// 22
	"�߸��� �����\n�Ž�ũ�Դϴ�",		// 23
	"�߸���\n����Ʈ�����Դϴ�",			// 24
	"�߸���\n���� �ּ��Դϴ�",			// 25
	"�߸���\n���� ��Ʈ�Դϴ�",			// 26
	"��ü��\n�����Ͽ����ϴ�",			// 27
	"����ڰ�\n�� á���ϴ�",			// 28
	"�߸���\n��ȭ��ȣ�Դϴ�",			// 29
	"��ȭ�� �������\n�ʾҽ��ϴ�",		// 30
	"��ȭ���Դϴ�",					// 31
	"���� ��ȣ�Դϴ�",				// 32
	"�������� �ʽ��ϴ�",				// 33
	"��ȭ��\n�����Ͽ����ϴ�",			// 34
	"��ȭ��\n�����Ͽ����ϴ�",			// 35
	"����\n���Ƚ��ϴ�",				// 36
	"ó��\n�Ǿ����ϴ�",				// 37
	"����\n�Ͽ����ϴ�",				// 38
	"�Ϸ�\n�Ǿ����ϴ�",				// 39
	"����� �� ����\nī���Դϴ�",		// 40
	"������ ī�常\n�� �ֽʽÿ�",		// 41
	"����� �Ǿ�\n���� �ʽ��ϴ�",		// 42
	"�̹� ��ϵǾ�\n�ֽ��ϴ�",			// 43
	"����Ÿ��\n�� á���ϴ�",			// 44
	"�����Ͽ����ϴ�",					// 45
	"SD�޸� ī�尡\n�����ϴ�",		// 46
	"SD�޸� �غ�\n�ȵǾ����ϴ�",	// 47
	"\n���丮 ���� ����",			// 48
	"\n���丮 �б� ����",			// 49
	"\n���丮 ���� ����",			// 50
	"\n���� ���� ����",				// 51
	"\n���� �б� ����",				// 52
	"\n���� ���� ����",				// 53
	"\n���� ����ȭ ����",				// 54
	"\n���丮�� �����ϴ�",			// 55
	"\n������ �����ϴ�",				// 56
	"�������� �ʴ�\n���ȹ���Դϴ�",	// 57
	"��� �� �ý�����\n����۵˴ϴ�",	// 58
	"SD�޸𸮸� �����ϰ�\n������ �� �ֽ��ϴ�", // 59
	"SD�޸� ����\n�����Ͻʽÿ�",		// 60
	"���� ���� ����\n��� �� �ٽ� �ϼ���", // 61
	"����ġ�κ���\n������ �����ϴ�",		// 62
	"����ġ�� ���� �浹��\n�߻��߽��ϴ�", 	// 63
	"����ġ�κ���\n������ �ֽ��ϴ�"	,	// 64
	"��Ʈ��ũ��\n����� �� �����ϴ�",	// 65
	"���̽� ����\n���",				// 66
	"������\n�������ϴ�",				// 66
};

char cjk_auth_msgs[9][MAX_MULTIMSG_LEN] = {
	"���ԱⰣ��\n�ƴմϴ�",				// 0
	"���Թ���\n����Ǿ����ϴ�",			// 1
	"���Թ� ������\n�����ϴ�",			// 2
	"����� ������\n�����ϴ�",			// 3
	"�нǵ�\nī���Դϴ�",				// 4
	"���Թ����\nƲ���ϴ�",				// 5
	"Anti-Passback\n���԰ź��Դϴ�",	// 6
	"�̹� ó����\nī���Դϴ�",			// 7
	"��� ������\n�źεǰ� �ֽ��ϴ�"		// 8
};

char cjk_ta_msgs[MAX_TA_MSGS_COUNT][MAX_TAMSG_LEN] = {
	"�� ��",		// 1
	"�� ��",		// 2
	"�� ��",		// 3
	"�� ��",		// 4
	"�� ��",		// 5
	"�� ��",		// 6
	"�� ��",		// 7
	"�� ��"		// 8
};
/*
char cjk_ta_msgs[8][MAX_TAMSG_LEN] = {
	"�� ��",		// 1
	"�� ��",		// 2
	"�� ��",		// 3
	"�� ��",		// 4
	"�� ��",		// 5
	"�� ��",		// 6
	"�� ��",		// 7
	"�� ��"		// 8
};
*/
char cjk_menu_top_title[MAX_MSG_LEN] = "����";

char cjk_menu_top[6][MAX_MSG_LEN] = {
	"�����",
	"�ϵ����",
	"���� ����",
	"��Ʈ��ũ",
	"ȯ�� ����",
	"�ý���"
};

char cjk_menu_lang[3][MAX_MSG_LEN] = {
	"����-ENG",
#ifdef _CHN
	"�߱���-CHN",
#else
#ifdef _JPN
	"�Ϻ���-JPN",
#else
#ifdef _TWN
	"�߱���:��ü-TWN",
#else
	"�ѱ���-KOR",
#endif
#endif
#endif
};

char cjk_menu_user[4][MAX_MSG_LEN] = {
	"����� �߰�",
	"����� ����",
	"����� ����",
	"����� ��ü ����"
};

char cjk_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",
	"�׼��� ���",
	"����1",
	"����2",
	"ī��",
	"��ȣ",
	"���� ���"
};

char cjk_menu_user_regex[4][MAX_MSG_LEN] = {
	"���� ����",
	"��ȿ�Ⱓ ������",
	"��ȿ�Ⱓ ������",
	"ī�� ����"
};

char cjk_menu_access_mode[7][MAX_MSG_LEN] = {
	"����",
	"ī��",
	"��ȣ",
	"ī��+����",
	"ī��+��ȣ",
	"ī��+����+��ȣ",
	"��ȣ+����"
};

char cjk_menu_security_level[10][MAX_MSG_LEN] = {
	"�ܸ��� ����",
	"���� ����",
	"�ſ� ����",
	"����",
	"�ణ ����",
	"����",
	"�ణ ����",
	"����",
	"�ſ� ����",
	"���� ����"
};

char cjk_menu_time_attend[3][MAX_MSG_LEN] = {
	"Ÿ�̸�",
	"���",
	"���� �׸�"
};

char cjk_menu_ta_item[3][MAX_MSG_LEN] = {
	"Ȱ��ȭ",
	"���� ����",
	"�ð���"
};

char cjk_menu_meal[9][MAX_MSG_LEN] = {
	"�� ��",
	"�� ��",
	"�� ��",
	"�� ��",
	"�� ��",
	"�� ��",
	"�ߺ� ����",
	"�ı� �μ�",
	"�׸� �� �Է�"
};

char cjk_menu_meal_sub[2][MAX_MSG_LEN] = {
	"�ð���",
	"�޴� ����"
};

char cjk_menu_meal_funckey[4][MAX_MSG_LEN] = {
	"�޴� 1",
	"�޴� 2",
	"�޴� 3",
	"�޴� 4"
};

char cjk_menu_event[3][MAX_MSG_LEN] = {
	"�̺�Ʈ ����",
	"��� ����",
	"�̺�Ʈ ����"
};

char cjk_menu_event_output[3][MAX_MSG_LEN] = {
	"��Ʈ��ũ",
	"26 Wiegand",
	"34 Wiegand"
};

char cjk_menu_event_mask[3][MAX_MSG_LEN] = {
	"�� �̺�Ʈ",
	"���� �̺�Ʈ",
	"��Ÿ �̺�Ʈ"
};

char cjk_menu_event_wiegand[2][MAX_MSG_LEN] = {
	"�����ID",
	"ī�� ����Ÿ"
};

char cjk_menu_hardware[4][MAX_MSG_LEN] = {
	"���Թ�",
	"������",	
	"�Է�",
	"���"
};

char cjk_menu_access_door[7][MAX_MSG_LEN] = {
	"������",
	"���� �� ������",
	"��� ��ư �Է�",
	"������ ���� �Է�",
	"������ �ð�",
	"�� ����",
	"���"
};

char cjk_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM ��",
	"���庼Ʈ"
};

char cjk_menu_door_alarm[3][MAX_MSG_LEN] = {
	"��ð� �� ���� ���",
	"�� ���� ���� ���",
	"��ð� ������ �ð�"
};

char cjk_menu_alarm[4][MAX_MSG_LEN] = {
	"��� ���� �ð�",
	"��� �� ���� �ð�",
	"��� ���",
	"��� ����"
};

char cjk_menu_access_point[7][MAX_MSG_LEN] = {
	"���� ���",
	"�����ϴ� ī�� ����",
	"��ȣ ����",
	"���",
	"���� ���� ���",
	"������ ����",
	"����"
};

char cjk_menu_auth_mode[4][MAX_MSG_LEN] = {
	"���� ����",
	"���� ���� ����",
	"��� �ź�",
	"ī�� ��� ����"
};

char cjk_menu_ap_alarm[3][MAX_MSG_LEN] = {
	"�̵�� ���",
	"��ȿ�Ⱓ ���� ���",
	"���ΰź� ���"
};

char cjk_menu_server_auth[3][MAX_MSG_LEN] = {
	"��ġ �ܵ�",
	"��ġ �ܵ� + ����",
	"����"
};

char cjk_menu_reader_setting[6][MAX_MSG_LEN] = {
	"���",
	"Type A / Type B",
	"�ӵ�",
	"�з�Ƽ",
	"���� ��Ʈ", 
	"IP �ּ� �� ��Ʈ"
};

char cjk_menu_fp_reader[4][MAX_MSG_LEN] = {
	"���� ���",
	"1:N ����",
	"���� ���ⷮ",
	"���� ĸ��"	
};

char cjk_menu_reader[3][MAX_MSG_LEN] = {
	"���� ����",
	"PIN ����",
	"ī�� ����"
};

char cjk_menu_card_reader[5][MAX_MSG_LEN] = {
	"����Ʈ ī�� ����",
	"EM ����",
	"Wiegand ����",
	"�ø��� ����",
	"IP ����"
};

char cjk_menu_sc_type[3][MAX_MSG_LEN] = {
	"Type A",
	"Type B",
	"Type A + Type B"
};

char cjk_menu_binary_inputs[4][MAX_MSG_LEN] = {
	"Exit",
	"Input1",
	"Input2",
	"Input3"
};

char cjk_menu_binary_outputs[2][MAX_MSG_LEN] = {
	"Lock",
	"Relay",
};

char cjk_menu_binary_io[2][MAX_MSG_LEN] = {
	"�ؼ�",
	"����"
};

char cjk_menu_io_polarity[2][MAX_MSG_LEN] = {
	"����(Normal Open)",
	"����(Normal Close)"
};

char cjk_menu_bi_type[5][MAX_MSG_LEN] = {
	"�Ϲ�",
	"��ǹ�ư",
	"������ ����",
	"����������",
	"ħ����"
};

char cjk_menu_bo_type[3][MAX_MSG_LEN] = {
	"�Ϲ�",
	"���� ��",
	"���������� ��ư",
};

char cjk_menu_access_control[3][MAX_MSG_LEN] = {
	"ī�� ����",
	"����Ʈ ī�� ����",
	"��� ���"
};

char cjk_menu_credential[7][MAX_MSG_LEN] = {
	"32-bit ����",
	"64-bit ����",
	"Facility(8-bit)+CardNo(16-bit)",
	"Facility(16-bit)+CardNo(32-bit)",
	"Facility(32-bit)+CardNo(32-bit)",
	"BCD ����",
	"���� ��Ʈ��"
};

char cjk_menu_field_type[2][MAX_MSG_LEN] = {
	"ī���ȣ",
	"Facility code"
};

char cjk_menu_cred_format[3][MAX_MSG_LEN] = {
	"����",
	"��й�ȣ",
	"����Ʈī�� ����"
};

char cjk_menu_card_format[7][MAX_MSG_LEN] = {
	"�Ϸù�ȣ(UID)",
	"���",
	"ISO7816-4 ����",
	"EM ����",
	"Wiegand ����",
	"�ø��󸮴� ����",
	"IP���� ����"
};

char cjk_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"ǥ�� 26-bit ����",
	"HID 37-bit ����",
	"HID 37-bit ����(Facility)",
	"35-bit HID Coporate 1000",
	"����� ����"
};

char cjk_menu_format_setting[6][MAX_MSG_LEN] = {
	"ũ������",
	"ī�� ����",
	"ī�� ��",
	"��� Wiegand ����",
	"Wiegand �з�Ƽ",
	"�ʵ� ����"
};

char cjk_menu_card_type[5][MAX_MSG_LEN] = {
	"ISo14443A",
	"ISo14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM ī��"
};

char cjk_menu_card_field[4][MAX_MSG_LEN] = {
	"���� ����",
	"����",
	"����",
	"�ڵ�"
};

char cjk_menu_field_coding[3][MAX_MSG_LEN] = {
	"���� ������",
	"BCD",
	"����(ASCII)"
};

char cjk_menu_mifare_blk[2][MAX_MSG_LEN] = {
	"����",
	"���"
};

char cjk_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"����",
	"¦��/Ȧ�� �з�Ƽ",
	"��Ÿ �з�Ƽ"
};

char cjk_menu_output_command[3][MAX_MSG_LEN] = {
	"�ݺ� Ƚ��",
	"ON �ð�",
	"OFF �ð�"
};

char cjk_menu_armed[4][MAX_MSG_LEN] = {
	"��� ����",
	"��� ���",
	"���",
	"ħ��"
};

char cjk_menu_byte_order[2][MAX_MSG_LEN] = {
	"���� ����",
	"�� ����"
};

char cjk_menu_em_format[3][MAX_MSG_LEN] = {
	"32-bit",
	"24-bit",
	"20-bit"
};

char cjk_menu_external_format[3][MAX_MSG_LEN] = {
	"900MHz ����",
	"���ڵ� ����",
	"EM ����"
};

char cjk_menu_ip_reader[3][MAX_MSG_LEN] = {
	"������ ��Ʈ��ũ ����",
	"�ּ�",
	"��Ʈ"
};

char cjk_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"����",
	"ī�� ��ȣ�� ���",
	"�з�Ƽ �߰�"
};

char cjk_menu_duress[2][MAX_MSG_LEN] = {
	"���ڸ�� ����",
	"���ڸ�� ���"
};

char cjk_menu_setup[8][MAX_MSG_LEN] = {
	"������",
	"��¥ �� �ð�",
	"ȭ��",
	"����",
	"��� ����",
	"���(Language)",
	"ī�޶�",
	"������"
};

char cjk_menu_admin[2][MAX_MSG_LEN] = {
	"������ ��ȣ",
	"������ ����"
};

char cjk_menu_wiegand_type[2][MAX_MSG_LEN] = {
	"26 Wiegand",
	"34 Wiegand"
};

char cjk_menu_datetime[3][MAX_MSG_LEN] = {
	"��¥ ����",
	"�ð� ����",
	"24�ð� ���� ���"
};

char cjk_menu_display[3][MAX_MSG_LEN] = {
	"���� �ð�",
	"��Ʈ��Ʈ",
	"���"
};

char cjk_menu_time_notation[2][MAX_MSG_LEN] = {
	"���� 1:00",
	"13:00"
};

char cjk_menu_date_notation[3][MAX_MSG_LEN] = {
	"ǥ�� ����",
	"MM/DD",
	"MM��DD��"
};

char cjk_menu_sound[3][MAX_MSG_LEN] = {
	"���� ����",
	"���� ��� ����",
	"��ư ����"
};

char cjk_menu_lockout[2][MAX_MSG_LEN] = {
	"����(�и� ���)",
	"Ű ���"
};

char cjk_menu_cam[4][MAX_MSG_LEN] = {
	"ī�޶� ����",
	"ī�޶� ä��",
	"ī�޶� ����",
	"�̺�Ʈ ���� ����"
};

char cjk_menu_cam_control[3][MAX_MSG_LEN] = {
	"������",
	"����(Off)",
	"�ѱ�(On)"
};

char cjk_menu_doorphone[8][MAX_MSG_LEN] = {
	"��� ���",
	"��ȭ ��ȣ",
	"���� ����",
	"ħ�� �ð�",
	"������ ��ȣ",
	"�� ����",
	"�� ����",
	"�ϵ���� ����"
};

char cjk_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"��� ����",
	"IP ��ȭ",
	"������",
	"�Ϲ� ��ȭ"
};

char cjk_menu_phone_number[3][MAX_MSG_LEN] = {
	"���̾� ���",
	"IP ��ȭ��ȣ",
	"���� ��ȣ"
};

char cjk_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"ǥ��",
	"���� ��ȣ",
	"���� ��ȣ"
};

char cjk_menu_ip_phone_item[2][MAX_MSG_LEN] = {
	"��ȭ��ȣ",
	"IP �ּ�"
};

char cjk_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"����Ŀ ����",
	"����ũ ����",
	"ȸ�� ����"
};

char cjk_menu_doorphone_tone[3][MAX_MSG_LEN] = {
	"������",
	"��ȭ��",
	"��ȭ������"
};

char cjk_menu_tone_info[2][MAX_MSG_LEN] = {
	"High �ð�",
	"Low �ð�"
};

char cjk_menu_doorphone_hw_setup[2][MAX_MSG_LEN] = {
	"����Ī �ӵ�",
	"��� ����(BGN)"
};

char cjk_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char cjk_menu_network[7][MAX_MSG_LEN] = {
	"��� �ּ�",
	"��Ʈ��ũ ����",
	"IP �ּ� ����",
	"������ ����",
	"���� ����",
	"��������(Wi-Fi)",
	"Ping ����"
};

char cjk_menu_network_ip[2][MAX_MSG_LEN] = {
	"���� LAN",
	"��������(Wi-Fi)"
};

char cjk_menu_network_serial[2][MAX_MSG_LEN] = {
	"�ø��� �ӵ�",
	"�ø��� ��Ʈ"
};

char cjk_menu_serial_port[3][MAX_MSG_LEN] = {
	"��Ʈ 0(RS-485)",
	"��Ʈ 1(RS-232)",
	"��Ʈ 2"
};

char cjk_menu_network_tcpip[7][MAX_MSG_LEN] = {
	"MAC �ּ�",
	"�ڵ� �ּ� �ޱ�",
	"IP �ּ�",
	"����� �Ž�ũ",
	"����Ʈ����",
	"���� �ּ�",
	"���� ��Ʈ"
};

char cjk_menu_network_type[2][MAX_MSG_LEN] = {
	"�ø���(RS-485)",
	"IP ��"
};

char cjk_menu_network_wifi[8][MAX_MSG_LEN] = {
	"SSID",
	"����",
	"��ȣ ����",
	"ä��",
	"����",
	"���� ����",
	"����",
	"���� ����"
};

char cjk_menu_wpa_state[12][MAX_MSG_LEN] = {
	"�ʱ�ȭ �ȵ�",
	"��Ȱ��ȭ",
	"���� �ȵ�",
	"�޸� ����",
	"ã����...",
	"������...",
	"������...",
	"������...",
	"������...",
	"������...",
	"�����",
	"IP �ּ� �޴� ��..."
};

char cjk_menu_wifi_security[8][MAX_MSG_LEN] = {
	"���� ����",
	"WPS",
	"WEP",
	"WPA PSK",
	"WPA PSK",
	"WPA2 PSK",
	"802.1x EAP"
	"802.1x EAP"
};

char cjk_menu_wifi_state[6][MAX_MSG_LEN] = {
	"��� ����",
	"�����",
	"�������� ����",
	"(��)�� ����",
	"���� ����",
	"���� �߻�"
};

char cjk_menu_system[5][MAX_MSG_LEN] = {
	"�� ���� ����",
	"�ʱ�ȭ",
	"��������(from USB)",
	"��������(to USB)",
	"��ġ ���ʱ�ȭ(����)"
};

char cjk_menu_init_config[4][MAX_MSG_LEN] = {
	"�̺�Ʈ �ʱ�ȭ",
	"���� �ʱ�ȭ",
	"��� �ʱ�ȭ",
	"�÷��� ����"
};
char cjk_menu_imexport[4][MAX_MSG_LEN] = {
	"�߿���",
	"�ڿ� ����(������,����)",
	"��ġ ���� ����",	
	"����� ����Ÿ"
};

char cjk_menu_comm_stats[2][MAX_MSG_LEN] = {
	"��� ����",
	"���ġ �ʱ�ȭ"
};

char cjk_menu_yesno[2][MAX_MSG_LEN] = {
	"��",
	"�ƴϿ�"
};

char cjk_menu_noyes[2][MAX_MSG_LEN] = {
	"�ƴϿ�",
	"��"
};

char cjk_menu_okcancel[4][MAX_MSG_LEN] = {
	"Ȯ��",
	"���",
	"�Ϸ�",
	"����"
};

char cjk_menu_enable[2][MAX_MSG_LEN] = {
	"Ȱ��ȭ",
	"��� ����"
};

char cjk_menu_hw_config[9][MAX_MSG_LEN] = {
	"��ü",
	"���",
	"���Թ�",
	"������",
	"�Է�",
	"���",
	"Wiegand",
	"RS-485",
	"CH"
};

char cjk_menu_unit[5][MAX_MSG_LEN] = {
	"����Ʈ",
	"��Ʈ",
	"����",
	"��",
	"��"
};

char cjk_menu_other[25][MAX_MSG_LEN] = {
	"�߿��� ����",
	"�𵨸�",
	"����� ��ϼ�",
	"���� ��ϼ�",
	"��",
	"������",
	"��      ��",
	"0:��Ʈ��(null)",
	"IP ��ȭ��ȣ ��ϼ�",
	"��ü",
	"0.5��",
	"������",		// Unspecified(null)
	"�׻� ����",	// Never
	"�׻� ����",	// Always(all the time)
	"����",
	"�׻� OFF",
	"�׻� ON",
	"���",
	"����",
	"����",
	"���� �ȵ�",
	"�ٿ�ε� ��",
	"�ٿ�ε� �Ϸ�",
	"������ ��й�ȣ",
	"�� �̺�Ʈ ����"
};

char _cjk_day_of_weeks[7][4] = {
	"��", "��", "ȭ", "��", "��", "��", "��"
};

char _cjk_am_pm[2][6] = {
	"����", "����"
};

char _cjk_year_mon_day[3][4] = {
	"��", "��", "��"
};


char car_no[20] = "�λ�34��6325";
