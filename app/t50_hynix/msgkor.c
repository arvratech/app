char cjk_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ȸ�� �̸�",					// 0
	"�����ID �Է�",				// 1
	"�����ID Ȯ��",				// 2
	"��й�ȣ �Է�",				// 3
	"��й�ȣ Ȯ��",				// 4
	"�� ��й�ȣ �Է�",				// 5
	"�� ��к�ȣ Ȯ��",				// 6
	"������ �Է��ϼ���",			// 7
	"������ �ٽ� �Է��ϼ���",		// 8
	"ī�带 �Է��ϼ���",			// 9
	"ī���� �ٽ� �Է��ϼ���",		// 10
	"�ѹ� �� �Է��ϼ���",			// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"��",							// 14
	"��",							// 15
    "�����Ͻðڽ��ϱ� ?",			// 16
	"���� ��ȭ��ȣ�� ��������",		// 18
	"���� ��ȣ�� ��������",			// 19
	"������ �����Ͻðڽ��ϱ�?",		// 20
	"��ø� ��ٷ� �ֽʽÿ�"		// 21
};
//	"New Time",						// 12
//	"New TID",						// 13

char cjk_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ó����...",					// 0
	"�ý���\n�����Դϴ�",			// 1
	"�Է� �ð���\n�ʰ��Ǿ����ϴ�",	// 2
	"�߸���\n�����ID�Դϴ�",		// 3
	"�߸���\n����Ÿ�Դϴ�",			// 4
	"����� �ȵ�\n������Դϴ�",	// 5
	"������ �ź��Ͽ����ϴ�",		// 6
	"�����Ͽ����ϴ�",				// 7	����ڸ� Ȯ��\n�Ͽ����ϴ�
	"�̹� ��ϵ�\n������Դϴ�",	// 8
	"�̹� ��ϵ�\nī���Դϴ�",		// 9
	"�̹� ��ϵ�\n�����Դϴ�",		// 10
	"��й�ȣ��\nƲ���ϴ�",			// 11
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
	"��Ʈ��ũ\n����Դϴ�",				// 66
	"�����κ���\n������ �����ϴ�",		// 67
	"���̽� ����\n���",				// 68
	"������\n�������ϴ�",				// 69
	"������\n���ϴ�",					// 70
	"�����̺�\n�ʱ�ȭ��",				// 71
};

char cjk_auth_msgs[9][MAX_MULTIMSG_LEN] = {
	"���ԱⰣ��\n�ƴմϴ�",				// 0
	"���Թ���\n����Ǿ� �ֽ��ϴ�",		// 1
	"���Թ���\n��� �ֽ��ϴ�",			// 2
	"����� ������\n�����ϴ�",			// 3
	"������\nī���Դϴ�",				// 4
	"���Թ����\nƲ���ϴ�",				// 5
	"Anti-Passback\n���԰ź��Դϴ�",	// 6
	"�̹� ó����\nī���Դϴ�",			// 7
	"��� ������\n�źεǰ� �ֽ��ϴ�"	// 8
};

char cjk_alarm_msgs[4][MAX_MULTIMSG_LEN] = {
	"������� ��������",
	"���Թ��� ��������Ǿ����ϴ�",
	"���Թ��� ���Ƚ��ϴ�",
	"���Թ��� �ݾ��ּ���"
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

char cjk_menu_top_title[MAX_MSG_LEN] = "����";

char cjk_menu_top[6][MAX_MSG_LEN] = {
	"�����",
	"�ϵ����",
	"���� ����",
	"��Ʈ��ũ",
	"��ġ ����",
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

char cjk_menu_user[3][MAX_MSG_LEN] = {
	"����� �߰�",
	"����� ����/����",
	"����� ��ü ����"
};

char cjk_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",
	"�׼��� ���",
	"���� 1",
	"���� 2",
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
	"����+��ȣ"
};

char cjk_menu_hardware_class[12][MAX_MSG_LEN] = {
	"��Ʈ��ũ ��Ʈ",
	"���",
	"����±��",
	"���� ��ġ",
	"����",
	"�Է�",
	"���",
	"���Թ�",
	"���� ����",
	"ȭ�� ����",
	"ħ��Ž�� ����",
	"����������",
};

char cjk_menu_device[3][MAX_MSG_LEN] = {
	"�Է�, ���",
	"������ ��Ʈ��ũ ��Ʈ",
	"�����̺� ��ġ",
};

char cjk_menu_hardware[11][MAX_MSG_LEN] = {
	"�ּ�",
	"��ġ��ȣ",
	"��Ʈ��ũ �ּ�",
	"�𵨸�",
	"�Ҵ�",
	"����",
	"ä��",
	"��Ʈ��ũ ��Ʈ",
	"����",
	"���� ��ġ",
	"���",
};

char cjk_menu_unit_model[4][MAX_MSG_LEN] = {
	"����",
	"K200-4",
	"K200-2",
	"��ü"
};

char cjk_menu_iounit_model[2][MAX_MSG_LEN] = {
    "K500-I/24",
    "K700-R/12"
};

char cjk_menu_unit_ch[4][MAX_MSG_LEN] = {
	"CH1",
	"CH2",
	"CH3",
	"CH4",
};

char cjk_menu_reader_type[5][MAX_MSG_LEN] = {
	"�Ϲ�",
	"�ָ���",
	"��������",
	"����������",
	"������"
};

char cjk_menu_input_type[7][MAX_MSG_LEN] = {
	"�Ϲ�",
	"�ⱸ ��ư",
	"�� ����ġ",
	"����������",
	"ħ��Ž��",
	"ȭ�籸��",
	"������"
};

char cjk_menu_output_type[5][MAX_MSG_LEN] = {
	"�Ϲ�",
	"�����",
	"��� ���",
	"���������� ��ư",
	"������"
};

char cjk_menu_input_output[1][MAX_MSG_LEN] = {
	"�ؼ�",
};

char cjk_menu_io_polarity[2][MAX_MSG_LEN] = {
	"����",
	"����"
};

char cjk_menu_armed[4][MAX_MSG_LEN] = {
	"��� ����",
	"��� ���",
	"���",
	"ħ��"
};

char cjk_menu_fire_zone[4][MAX_MSG_LEN] = {
	"ȭ�籸��",
	"ȭ�� �Է�",
	"ȭ��� ���� ��",
	"�溸 ������",
};

char cjk_menu_dev_model[6][MAX_MSG_LEN] = {
	"K100-4",
	"K100-2",
	"Wiegand ����",
	"Wiegand ����+Ű�е�",
	"T35s-NonIo",
	"T40A",
};

char cjk_menu_door[13][MAX_MSG_LEN] = {
	"�ָ���",
	"���� ����",
	"�Է�/��� ���",
	"���� �� ������",
	"��� ��ư �Է�",
	"������ ���� �Է�",
	"�溸",
	"�溸 ������",
	"�溸 �׼�",
	"������ �ð�",
	"Ȯ�� ������ �ð�",
	"�� ����",
	"���� ��"
};

char cjk_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM ��",
	"���庼Ʈ"
};

char cjk_menu_alarm[4][MAX_MSG_LEN] = {
	"��� ���� �ð�",
	"��� �� ���� �ð�",
	"��ð� ������ �ð�",
	"��� ����"
};

char cjk_menu_access_point[7][MAX_MSG_LEN] = {
	"���� ũ������ ����",
	"���� ���",
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

char cjk_menu_server_auth[3][MAX_MSG_LEN] = {
	"��ġ �ܵ�",
	"��ġ �ܵ� + ����",
	"����"
};

char cjk_menu_ap_alarm[3][MAX_MSG_LEN] = {
	"�̵�� ���",
	"��ȿ�Ⱓ ���� ���",
	"���ΰź� ���"
};

char cjk_menu_reader[7][MAX_MSG_LEN] = {
	"����Ʈī�� ����",
	"EM ����",
	"Wiegand ����",
	"�ø��� ����",
	"IP ����",
	"PIN ����",
	"���� ����"
};

char cjk_menu_readerop[6][MAX_MSG_LEN] = {
	"���� ���",
	"���� ���",
	"���� ���",
	"�ð� ǥ��",
	"���׳� ���",
	"���� ǥ�� �ð�"
};

char cjk_menu_authmode[5][MAX_MSG_LEN] = {
	"�Ϲ� ���",
	"ī��",
	"ī��+ PIN",
	"ī�� or PIN",
	"PIN"
};

char cjk_menu_authresult[4][MAX_MSG_LEN] = {
	"LED, Beep ����",
	"��� �ð�",
	"ǥ�� �ð�",
	"���� ����"
};

char cjk_menu_ledbeep[4][MAX_MSG_LEN] = {
	"������",
	"���� LED + Beep",
	"û�� LED + Beep",
	"���� ���� LED"
};

char cjk_menu_ac[4][MAX_MSG_LEN] = {
	"���� ����",
	"��� �׼�",
	"�Է� ����",
	"Wiegand ��� ����"
};

char cjk_menu_credential[5][MAX_MSG_LEN] = {
	"32-bit ����",
	"64-bit ����",
	"Facility+CardNo",
	"BCD ����",
	"���� ��Ʈ��"
};

char cjk_menu_field_type[2][MAX_MSG_LEN] = {
	"ī���ȣ",
	"Facility code"
};

char cjk_menu_cred_format[4][MAX_MSG_LEN] = {
	"����Ʈ ī�� ����",
	"Wiegand ī�� ����",
	"PIN ����",
	"Wiegand PIN ����"
};

char cjk_menu_card_format[8][MAX_MSG_LEN] = {
	"�Ϸù�ȣ(UID) ����",
	"��� ����",
	"EMV(ISO7816) ����",
	"EM ����",
	"�ø��󸮴� ����",
	"IP���� ����",
	"�ڵ� ����",
	"�����(EMV) ī��"
};

char cjk_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"ǥ�� 26-bit ����",
	"35-bit HID Coporate 1000",
	"34-bit ����(�з�Ƽ)",
	"66-bit ����(�з�Ƽ)",
	"����� ����"
};

char cjk_menu_pin_format[5][MAX_MSG_LEN] = {
	"�Է� ����",
	"�Է����� ����",
	"�ִ� ����",
	"Facility code",
	"PIN ��ȣ",
};

char cjk_menu_wiegand_pin[4][MAX_MSG_LEN] = {
	"4-bit ����Ʈ ���",
	"8-bit ����Ʈ ���",
	"ǥ�� 26-bit ����",
	"���� ���� ���"
};

char cjk_menu_format_setting[8][MAX_MSG_LEN] = {
	"ũ������",
	"ī�� ����",
	"ī�� ��",
	"��� Wiegand ����",
	"Wiegand �з�Ƽ",
	"�ڵ�",
	"ī�� �ʵ�",
	"���� ����"
};

char cjk_menu_card_type[5][MAX_MSG_LEN] = {
	"ISO14443A",
	"ISO14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM ī��"
};

char cjk_menu_cardtype_ext[3][MAX_MSG_LEN] = {
	"900MHz",
	"���ڵ�",
	"��Ÿ ī��"
};

char cjk_menu_cardapp_block[2][MAX_MSG_LEN] = {
	"����",
	"��ȣȭ"
};

char cjk_menu_cardapp_iso7816[3][MAX_MSG_LEN] = {
	"�Ϲ�",
	"����ī��",
	"SK SAM"
};

char cjk_menu_cardapp_serial[2][MAX_MSG_LEN] = {
	"�Ϲ�",
	"Ư��"
};

char cjk_menu_card_field[3][MAX_MSG_LEN] = {
	"���� ����",
	"����",
	"����"
};

char cjk_menu_field_coding[4][MAX_MSG_LEN] = {
	"���� ������",
	"BCD",
	"����(ASCII)",
	"���� ���� ������"
};

char cjk_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"����",
	"¦��/Ȧ�� �з�Ƽ",
	"��Ÿ �з�Ƽ"
};

char cjk_menu_reader_setting[8][MAX_MSG_LEN] = {
	"��Ϲ�ȣ",
	"����",
	"���",
	"�ӵ�",
	"�з�Ƽ",
	"���� ��Ʈ", 
	"������� Ű",
	"AID"
};

char cjk_menu_ip_reader[3][MAX_MSG_LEN] = {
	"���� ��Ʈ��ũ ����",
	"�ּ�",
	"��Ʈ"
};

char cjk_menu_action_idx[12][MAX_MSG_LEN] = {
	"�Է�(Exit)",
	"�Է�(Door)",
	"�Է�(Input)",
	"�Է�(Fire,Tamper)",
	"ȭ�� �溸",
	"������ �溸",
	"���������� �溸",
	"�̵�� ī��",
	"��ȿ�Ⱓ ����",
	"�н��� ���� Ž��",
	"���� �ź�",
	"ħ��"
};

char cjk_menu_action_cmd[4][MAX_MSG_LEN] = {
	"���",
	"ON �ð�",
	"OFF �ð�",
	"�ݺ� Ƚ��"
};

char cjk_menu_setup[11][MAX_MSG_LEN] = {
	"������ ��й�ȣ",
	"��¥ �� �ð�",
	"ȭ��",
	"�Ҹ�",
	"���(Language)",
	"���� ����",
	"���� ����",
	"ī�޶�",
	"BLE(Bluetooth Low Energy)",
	"IP ��ȭ",
	"���׳� ���"
};

char cjk_menu_admin[2][MAX_MSG_LEN] = {
	"��й�ȣ ����",
	"���ڸ� ���"
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

char cjk_menu_sound[6][MAX_MSG_LEN] = {
	"����� ����",
	"�̵�� ����",
	"��ȭ ����",
	"�ý��� ����",
	"Ű�е� ��ư��",
	"��ġ��"
};

char cjk_menu_sleepmode[3][MAX_MSG_LEN] = {
	"��� ����",
	"��� �ð�",
	"������" 
};

char cjk_menu_sleepact[3][MAX_MSG_LEN] = {
	"��� ����",
	"���÷��� ����",
	"������ ���"
};

char cjk_menu_video[1][MAX_MSG_LEN] = {
	"USB �κ��� ����"
};

char cjk_menu_lockout[2][MAX_MSG_LEN] = {
	"����(�и� ���)",
	"Ű ���",
};

char cjk_menu_power_mgmt[2][MAX_MSG_LEN] = {
	"���͸� ���",
	"���͸��� �� ���"
};

char cjk_menu_cam[2][MAX_MSG_LEN] = {
	"�ʴ� �����Ӽ�",
	"�̺�Ʈ ���� ����"
};

char cjk_menu_cam_control[3][MAX_MSG_LEN] = {
	"������",
	"����(Off)",
	"�ѱ�(On)"
};

char cjk_menu_ble[5][MAX_MSG_LEN] = {
	"���� ����",
	"���� ��ȣ����",
	"���� ����",
	"MAC �ּ� ����",
	"���� ����"
};

char cjk_menu_phone[6][MAX_MSG_LEN] = {
	"��ȭ ����",
	"���� �߰�",
	"���� ����",
	"���� ����",
	"���� ��ü ����",
	"��ȭ �ɱ�"
};

char cjk_menu_phone_reg[2][MAX_MSG_LEN] = {
	"�̸�",
	"SIP ����"
};

char cjk_menu_call_state[3][MAX_MSG_LEN] = {
	"�Ŵ���",
	"������",
	"��ȭ ����"
};

char cjk_menu_network[10][MAX_MSG_LEN] = {
    "��Ʈ��ũ ����",
    "IP �ּ� ����",
    "���� ����",
    "���� �ּ�",
    "���� ��Ʈ",
    "������ ����",
    "������ �ּ�",
    "������ ��Ʈ",
    "Ping ����",
    "Ping ����"
};

char cjk_menu_network_ip[8][MAX_MSG_LEN] = {
    "�ϵ����(MAC) �ּ�",
    "�ڵ����� IP �ּ� �ޱ�",
    "IP �ּ�",
    "����� ����ũ",
    "�⺻ ����Ʈ����",
    "�ڵ����� DNS ���� �ּ� �ޱ�",
    "�⺻ DNS ����",
    "���� DNS ����",
};

char cjk_menu_network_serial[2][MAX_MSG_LEN] = {
	"�ø��� �ӵ�",
	"��������",
};

char cjk_menu_network_type[2][MAX_MSG_LEN] = {
	"IP",
	"RS-485"
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

char cjk_menu_system[9][MAX_MSG_LEN] = {
	"������/�����̺� ��ġ",
	"�� ���� ����",
	"�ʱ�ȭ",
	"�׽�Ʈ",
	"��������(from USB)",
	"��������(to USB)",
	"�����ޱ�",
	"��ġ ���ʱ�ȭ(����)",
	"���� ����(����)"
};

char cjk_menu_download[4][MAX_MSG_LEN] = {
	"���ȭ��",
	"�߿���",
	"OS Ŀ��",
	"������"
};

char cjk_menu_mst_slv[2][MAX_MSG_LEN] = {
	"������ ��ġ",
	"�����̺� ��ġ"
};

char cjk_menu_net_remote[2][MAX_MSG_LEN] = {
	"���� ����",
	"������ ����"
};

char cjk_menu_init_config[4][MAX_MSG_LEN] = {
	"�̺�Ʈ �ʱ�ȭ",
	"���� �ʱ�ȭ",
	"��� �ʱ�ȭ",
	"�÷��� ����"
};

char cjk_menu_test[4][MAX_MSG_LEN] = {
	"����",
	"�Է�",
	"���Թ� �����",
	"ī�� ����"
};

char cjk_menu_imexport[6][MAX_MSG_LEN] = {
	"�߿���",
	"OS Ŀ��",
	"�ڿ� ����(������,����)",
	"��� ȭ��",
	"��ġ ���� ����",	
	"����� ����Ÿ"
};

char cjk_menu_comm_stats[2][MAX_MSG_LEN] = {
	"��� ����",
	"���ġ �ʱ�ȭ"
};

char cjk_menu_bat_status[6][MAX_MSG_LEN] = {
	"���� �ȵ�",
	"������",
	"������",
	"���� ����",
	"�Ϸ�",
	"�����"
};

char cjk_menu_yesno[2][MAX_MSG_LEN] = {
	"��",
	"�ƴϿ�"
};

char cjk_menu_noyes[2][MAX_MSG_LEN] = {
	"�ƴϿ�",
	"��"
};

char cjk_menu_okcancel[5][MAX_MSG_LEN] = {
	"Ȯ��",
	"���",
	"�Ϸ�",
	"����",
	"����"
};

char cjk_menu_enable[2][MAX_MSG_LEN] = {
	"Ȱ��ȭ",
	"��� ����"
};

char cjk_menu_unit[5][MAX_MSG_LEN] = {
	"��Ʈ",
	"����",
	"����Ʈ",
	"��",
	"��"
};

char cjk_menu_other[15][MAX_MSG_LEN] = {
	"OS Ŀ�� ����",
	"�߿��� ����",
	"�𵨸�",
	"������",
	"��ü",
	"�Է� ����",
	"���͸�",
	"���� ũ��",
	"������",		// Unspecified(null)
	"�׻� ����",	// Never
	"�׻� ����",	// Always(all the time)
	"����",
	"������ ��й�ȣ",
	"�̺�Ʈ ����",
	"����� ��",
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

char _cjk_day_of_week_postfix[8] = "����";

