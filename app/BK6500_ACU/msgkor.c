char kor_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ȸ�� �̸�",						// 0
	"�����ID �Է�:",					// 1
	"�����ID Ȯ��:",					// 2
	"��ȣ �Է�:",						// 3
	"��ȣ Ȯ��:",						// 4
	"�� ��ȣ �Է�:",					// 5
	"�� ��ȣ Ȯ��:",					// 6
	"������\n�Է��Ͻʽÿ�",				// 7
	"������ �ٽ�\n�Է��Ͻʽÿ�",		// 8
	"ī�带\n�Է��Ͻʽÿ�",				// 9
	"ī���� �ٽ�\n�Է��Ͻʽÿ�",		// 10
	"�ѹ� ��\n�Է��Ͻʽÿ�",			// 11
	"YY/MM/DD",							// 12
	"HH:MM:SS",							// 13
	"��",								// 14
	"��",								// 15
    "�����Ͻðڽ��ϱ� ?",				// 16
	"���� ��ȭ��ȣ�� ��������",			// 17
	"���� ��ȣ�� ��������",				// 18
	"������ �����Ͻðڽ��ϱ� ?",		// 19
	"��ø� ��ٷ� �ֽʽÿ�"			// 20
};
//	"New Time",							// 12
//	"New TID",							// 13

char kor_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"ó����...",						// 0
	"�ý���\n�����Դϴ�",				// 1
	"�Է� �ð���\n�ʰ��Ǿ����ϴ�",		// 2
	"�߸���\n�����ID�Դϴ�",			// 3
	"�߸���\n�������Դϴ�",				// 4
	"�̵��\n������Դϴ�",				// 5
	"������ �ź�\n�Ͽ����ϴ�",			// 6
	"�����Ͽ����ϴ�",					// 7	����ڸ� Ȯ��\n�Ͽ����ϴ�
	"�̹� ��ϵ�\n������Դϴ�",		// 8
	"�̹� ��ϵ�\nī���Դϴ�",			// 9
	"�̹� ��ϵ�\n�����Դϴ�",			// 10
	"��ȣ��\nƲ���ϴ�",					// 11
	"�����Ͱ�\nƲ���ϴ�",				// 12
	"�����͸�\n�����Ͽ����ϴ�",			// 13
	"����ڸ�\n�����Ͽ����ϴ�",			// 14
	"��ü ����ڸ�\n�����Ͽ����ϴ�",	// 15
	"�߸���\n�ð� �����Դϴ�",			// 16
	"�߸���\n�ܸ���ID�Դϴ�",			// 17
	"�߸���\n��¥�Դϴ�",				// 18
	"��¥��\n�����Ͽ����ϴ�",			// 19
	"�߸���\n�ð��Դϴ�",				// 20
	"�ð���\n�����Ͽ����ϴ�",			// 21
	"�߸���\nIP�ּ��Դϴ�",				// 22
	"�߸��� �����\n�Ž�ũ�Դϴ�",		// 23
	"�߸���\n����Ʈ�����Դϴ�",			// 24
	"�߸���\n���� �ּ��Դϴ�",			// 25
	"�߸���\n���� ��Ʈ�Դϴ�",			// 26
	"��ü��\n�����Ͽ����ϴ�",			// 27
	"����ڰ�\n�� á���ϴ�",			// 28
	"��ȭ��ȣ\n�����Դϴ�",				// 29
	"��ȭ�� �������\n�ʾҽ��ϴ�",		// 30
	"��ȭ���Դϴ�",						// 31
	"�������� �ʽ��ϴ�",				// 32
	"��ȭ ����",						// 33
	"��ȭ",								// 34
	"����\n���Ƚ��ϴ�",					// 35
	"ó��\n�Ǿ����ϴ�",					// 36
	"����\n�Ͽ����ϴ�",					// 37
	"�Ϸ�\n�Ǿ����ϴ�",					// 38
	"����� �� ����\nī���Դϴ�",		// 39
	"������ ī�常\n�� �ֽʽÿ�",		// 40
	"����� �Ǿ�\n���� �ʽ��ϴ�",		// 41
	"�̹� ��ϵǾ�\n�ֽ��ϴ�",			// 42
	"�����Ͱ�\n�� á���ϴ�",			// 43
	"�����Ͽ����ϴ�",					// 44
	"SD�޸� ī�尡\n�����ϴ�",		// 45
	"SD�޸� �غ�\n�ȵǾ����ϴ�",	// 46
	"\n���丮 ���� ����",				// 47
	"\n���丮 �б� ����",				// 48
	"\n���丮 ���� ����",				// 49
	"\n���� ���� ����",					// 50
	"\n���� �б� ����",					// 51
	"\n���� ���� ����",					// 52
	"\n���� ����ȭ ����",				// 53
	"\n���丮�� �����ϴ�",			// 54
	"\n������ �����ϴ�",				// 55
	"��� �� �ý�����\n����۵˴ϴ�",	// 56
	"SD�޸𸮸� �����ϰ�\n������ �� �ֽ��ϴ�", // 57
	"SD�޸� ����\n�����Ͻʽÿ�",		// 58
	"���� ���� ����\n��� �� �ٽ� �ϼ���", // 59
	"�̹� ó�����Դϴ�\n���� ����ϼ���"
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
	"��� ������\n�źεǰ� �ֽ��ϴ�"	// 8
};

char kor_ta_msgs[MAX_TA_MSGS_COUNT][MAX_TAMSG_LEN] = {
	"�� ��",		// 1
	"�� ��",		// 2
	"�� ��",		// 3
	"�� ��",		// 4
	"�� ��",		// 5
	"�� ��",		// 6
	"�� ��",		// 7
	"�� ��"			// 8
};
/*
char kor_ta_msgs[8][MAX_TAMSG_LEN] = {
	"�� ��",		// 1
	"�� ��",		// 2
	"�� ��",		// 3
	"�� ��",		// 4
	"�� ��",		// 5
	"�� ��",		// 6
	"�� ��",		// 7
	"�� ��"			// 8
};

char kor_menu_unit[17][MAX_MSG_LEN] = {
	"��",								// 14
	"��",
	"0.5��",
	"������",		// Unlimited(Never timeout)
	"��",			// Always
	"������",		// Unspecified(null)
	"��ü",			// All
	"��",			// Always
	"�׻� ����",	// Always(all the time)
	"�׻� ����",	// Never
	"�׻� ON"
	"�׻� OFF",
};
*/
char kor_menu_top_title[MAX_MSG_LEN] = "�ý��� ����";

char kor_menu_top[5][MAX_MSG_LEN] = {
	"�����", 
	"���� ����",
	"��Ʈ��ũ",
	"ȯ�� ����",
	"�ý���"
};

char kor_menu_lang[2][MAX_MSG_LEN] = {
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

char kor_menu_user[6][MAX_MSG_LEN] = {
	"����� ���",
	"����� ���� ���",
	"����� ����",
	"����� ����",
	"����� ��ü ����",
	"����� ���� ���"
};

char kor_menu_user_enroll[7][MAX_MSG_LEN] = {
	"���� ����",	
	"�׼��� ���",
	"����1",
	"����2",
	"ī��",
	"��ȣ",
	"���� ���"
};

char kor_menu_access_mode[7][MAX_MSG_LEN] = {
	"����",
	"ī��",
	"��ȣ",
	"ī��+����",
	"ī��+��ȣ",
	"ī��+����+��ȣ",
	"��ȣ+����"
};

char kor_menu_security_level[10][MAX_MSG_LEN] = {
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

char kor_menu_ta_sub[1][MAX_MSG_LEN] = {
	"�ð���",
};

char kor_menu_time_attend[4][MAX_MSG_LEN] = {
	"Ű",
	"ȭ��ǥ��",
	"�׸�",
	"Ÿ�̸�"	
};

char kor_menu_ta_key[2][MAX_MSG_LEN] = {
	"ȭ��ǥ Ű",
//	"������ Ű"
	"F1-...-F4"
};

char kor_menu_ta_display[3][MAX_MSG_LEN] = {
	"ǥ�þ���",
	"����",
	"������"
};

char kor_menu_ta_item[3][MAX_MSG_LEN] = {
	"Ȱ��ȭ",
	"���� ����",
	"������ ����"
};

char kor_menu_meal[9][MAX_MSG_LEN] = {
	"�� ��",
	"�� ��", //	"�� ��", ALARM_OUTPUT
	"�� ��",
	"�� ��",
	"�� ��",
	"�� ��",
	"�ߺ� ����",
	"�ı� �μ�",
	"�׸� �� �Է�"
};

char kor_menu_meal_sub[2][MAX_MSG_LEN] = {
	"�ð���",
	"�޴� ����"
};

char kor_menu_meal_funckey[4][MAX_MSG_LEN] = {
	"�޴� 1",
	"�޴� 2",
	"�޴� 3",
	"�޴� 4"
};

char kor_menu_event[3][MAX_MSG_LEN] = {
	"�̺�Ʈ ����",
	"��� ����",
	"�̺�Ʈ ����"
};

char kor_menu_event_output[3][MAX_MSG_LEN] = {
	"��Ʈ��ũ",
	"26 Wiegand",
	"34 Wiegand"
};

char kor_menu_event_mask[3][MAX_MSG_LEN] = {
	"�� �̺�Ʈ",
	"���� �̺�Ʈ",
	"��Ÿ �̺�Ʈ"
};

char kor_menu_event_wiegand[2][MAX_MSG_LEN] = {
	"�����ID",
	"ī�� ������"
};

char kor_menu_access_control[9][MAX_MSG_LEN] = {
	"�ϵ���� ����",
	"��ü ����",
	"���Թ�",	
	"������(����)",
	"�Է�",
	"���(������)",
	"���Թ�",
	"�溸",
	"Ȯ�� �����"
};

char kor_menu_hw_setup[6][MAX_MSG_LEN] = {
	"��� ����",
	"��� ����",
	"��� ����� ����",
	"���� IP�ּ� ����",
	"����� ��� ����",
	"����� ��� ����"
};

char kor_menu_op_mode[3][MAX_MSG_LEN] = {
	"��� ����",
	"���� ���",
	"�ļ� ���"
};

char kor_menu_all_readers[6][MAX_MSG_LEN] = {
	"���� ����",
	"���� �Ҵ�",
	"����(�и� �溸)",
	"���� ����",
	"�ļ� ����",
	"��� ���"
};

char kor_menu_hw_config[19][MAX_MSG_LEN] = {
	"���",
	"������",
	"Wiegand",
	"RS-485",
	"��ü",
	"����",
	"���Թ�",
	"����",
	"IP",
	"ä �� 1",
	"ä �� 2",
	"������",
	"���Թ�",
	"ȭ�籸��",
	"�溸�۵�",
	"����±��",
	"��뿩��",
	"ȭ����",
	"IP"
};

char kor_menu_unit_type[3][MAX_MSG_LEN] = {
	"K-200",
	"K-220",
	"K-300(�ܵ���)"
};

char kor_menu_iounit_type[2][MAX_MSG_LEN] = {
	"K-500(�Է�)",
	"K-700(���)"
};

char kor_menu_access_door[7][MAX_MSG_LEN] = {
	"������ �ð�",
	"�Է�/���",
	"�溸 ���� �ð�",
	"�溸 �� �����ð�",
	"�溸",
	"������ ����",
	"������ ����"
};

char kor_menu_door_config[3][MAX_MSG_LEN] = {
	"���Թ� ����",
	"����(�ⱸ �̻��)",
	"�ⱸ"
};

char kor_menu_door_inout[5][MAX_MSG_LEN] = {
	"��(Lock)",
	"��� ��ư",
	"������ ����",
	"���� ��",
	"�� ����"
};

char kor_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM ��",
	"���庼Ʈ"
};

char kor_menu_door_alarm[2][MAX_MSG_LEN] = {
	"��ð� �� ����",
	"�� ���� ����"
};

char kor_menu_door_alarm_info[2][MAX_MSG_LEN] = {
	"Ȱ��ȭ",
	"������ �溸 �ð�"
};

char kor_menu_access_point[1][MAX_MSG_LEN] = {
	"���� ���"
};

char kor_menu_authorization_mode[4][MAX_MSG_LEN] = {
	"���� ����",
	"���� ���� ����",
	"��� �ź�",
	"ī�� ��� ����"
};

char kor_menu_access_zone[6][MAX_MSG_LEN] = {
	"�߰�",
	"������ ����",
	"�н��� ����",
	"�ⱸ��",
	"�Ա���",
	"�ܺ�"
};

char kor_menu_passback[6][MAX_MSG_LEN] = {
	"�ֽ��� ����",
	"�н��� ���",
	"�ⱸ���� �ѽ���",
	"�н��� Ÿ�Ӿƿ�",
	"�ϵ� �н��� ����",
	"����Ʈ �н��� ����"
};

char kor_menu_passback_mode[3][MAX_MSG_LEN] = {
	"��� ����",
	"�ϵ�(����� ����)",
	"����Ʈ(�溸�� �߻�)"
};

char kor_menu_passback_setting[4][MAX_MSG_LEN] = {
	"�н��� ���",
	"�ݺ� ���� ���� �ð�",
	"�н��� ���� �ð�",
	"�н��� ��ü ����"
};

char kor_menu_binary_input[4][MAX_MSG_LEN] = {
	"�ؼ�",
	"�溸 ���� �ð�",
	"�溸 �� ���� �ð�",
	"�溸 ����"
};

char kor_menu_binary_output[1][MAX_MSG_LEN] = {
	"�ؼ�"
};

char kor_menu_io_polarity[2][MAX_MSG_LEN] = {
	"����(Normal Open)",
	"����(Normal Close)"
};

char kor_menu_armed[4][MAX_MSG_LEN] = {
	"��� ����",
	"��� ���",
	"���",
	"ħ��"
};

char kor_menu_alarm[4][MAX_MSG_LEN] = {
	"ȭ�籸��",
	"ȭ����",
	"�溸 �۵�",
	"��� ���"
};

char kor_menu_fire_zone[3][MAX_MSG_LEN] = {
	"ȭ�� �Է�",
	"ȭ��� ���� ��",
	"�溸 �۵�"
};

char kor_menu_action_index[12][MAX_MSG_LEN] = {
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

char kor_menu_output_command[3][MAX_MSG_LEN] = {
	"�ݺ� Ƚ��",
	"ON �ð�",
	"OFF �ð�"
};

char kor_menu_readers[3][MAX_MSG_LEN] = {
	"���� ����",
	"PIN ����",
	"ī�� ����"
};

char kor_menu_card_readers[5][MAX_MSG_LEN] = {
	"Mifare ����",
	"EM ����",
	"Wiegand ����",
	"�ø��� ����",
	"IP ����"
};

char kor_menu_wiegand_reader[2][MAX_MSG_LEN] = {
	"Wiegand ī�� ����",
	"Wiegand PIN ����"
};

char kor_menu_fp_reader[4][MAX_MSG_LEN] = {
	"���� ���ⷮ",
	"���� ���",
	"1:N ����",
	"���� ĸ��"	
};

char kor_menu_mifare_reader[4][MAX_MSG_LEN] = {
	"ī�� ����",
	"Ư�� ī�� ����",
	"Mifare Type(A/B)",
	"ī�� ����"
};

char kor_menu_mifare_format[3][MAX_MSG_LEN] = {
	"�Ϸù�ȣ",
	"Ư�� ī��",
	"Ƽ�Ӵ�(T-money)"
};

char kor_menu_mifare_type[3][MAX_MSG_LEN] = {
	"Type A",
	"Type B",
	"Type A + Type B"
};

char kor_menu_special_card[6][MAX_MSG_LEN] = {
	"���� ��ȣ",
	"��� ��ȣ",
	"������ ��ġ",
	"������ ����",
	"������ ����",
	"������ ����"
};

char kor_menu_data_type[3][MAX_MSG_LEN] = {
	"����",
	"����",
	"���� ������"
};

char kor_menu_byte_order[2][MAX_MSG_LEN] = {
	"���� ����",
	"�� ����"
};

char kor_menu_em_format[3][MAX_MSG_LEN] = {
	"32-bit",
	"24-bit",
	"20-bit"
};

char kor_menu_external_format[3][MAX_MSG_LEN] = {
	"900MHz ����",
	"���ڵ� ����",
	"EM ����"
};

char kor_menu_ip_reader[3][MAX_MSG_LEN] = {
	"������ ��Ʈ��ũ ����",
	"�ּ�",
	"��Ʈ"
};

char kor_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"����",
	"ī�� ��ȣ�� ���",
	"�з�Ƽ �߰�"
};

char kor_menu_duress[2][MAX_MSG_LEN] = {
	"���ڸ�� ����",
	"���ڸ�� �溸"
};

char kor_menu_setup[8][MAX_MSG_LEN] = {
	"������",
	"�ð� ����",
	"ȭ��",
	"����",
	"��� ����",
	"���(Language)",
	"ī�޶�",
	"������"
};

char kor_menu_admin[2][MAX_MSG_LEN] = {
	"������ ��ȣ",
	"������ ����"
};

char kor_menu_wiegand_type[2][MAX_MSG_LEN] = {
	"26 Wiegand",
	"34 Wiegand"
};

char kor_menu_datetime[2][MAX_MSG_LEN] = {
	"��¥ ����",
	"�ð� ����"
};

char kor_menu_monitor[6][MAX_MSG_LEN] = {
	"���� �ð�",
	"��Ʈ��Ʈ",
	"���",
	"��� ���",
	"�ð� ǥ��",
	"��¥ ǥ��"
};

char kor_menu_time_notation[2][MAX_MSG_LEN] = {
	"12 �ð�",
	"24 �ð�"
};

char kor_menu_date_notation[3][MAX_MSG_LEN] = {
	"ǥ�� ����",
	"MM/DD",
	"MM��DD��"
};

char kor_menu_sound[3][MAX_MSG_LEN] = {
	"���� ����",
	"���� ��� ����",
	"��ư ����"
};

char kor_menu_lockout[2][MAX_MSG_LEN] = {
	"Ű ���",
	"����(�и� �溸)"	
};

char kor_menu_cam[7][MAX_MSG_LEN] = {
	"ī�޶� ���� ���",
	"ī�޶� �ѱ�/����",
	"ī�޶� ä��",
	"ī�޶� ����",
	"ī�޶� ����",
	"�̺�Ʈ ���� ����",
	"ī�޶� ä�� �Ҵ�" 
};

char kor_menu_cam_mode[3][MAX_MSG_LEN] = {
	"������",
	"�ڵ�",
	"����"
};

char kor_menu_cam_run[2][MAX_MSG_LEN] = {
	"����(Off)",
	"�ѱ�(On)"
};

char kor_menu_cam_channel[4][MAX_MSG_LEN] = {
	"0 ä��",
	"1 ä��",
	"2 ä��",
	"3 ä��"
};

char kor_menu_cam_direction[2][MAX_MSG_LEN] = {
	"����",
	"����"
};

char kor_menu_doorphone[8][MAX_MSG_LEN] = {
	"��� ���",
	"��ȭ ��ȣ",
	"������ ��ȣ",
	"���� ����",
	"ħ�� �ð�",
	"�� ����",
	"�� ����",
	"�ϵ���� ����"
};

char kor_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"��� ����",
	"������",
	"�Ϲ� ��ȭ",
	"IP ��ȭ"
};

char kor_menu_phone_number[3][MAX_MSG_LEN] = {
	"���̾� ���",
	"���� ��ȣ",
	"IP ��ȭ��ȣ"
};

char kor_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"ǥ��",
	"���� ��ȣ",
	"���� ��ȣ"
};

char kor_menu_ip_phone_number[5][MAX_MSG_LEN] = {
	"��ȣ ���",
	"��ȣ ����",
	"��ȣ ����",
	"��ȣ ��ü ����",
	"��ȣ ����"
};

char kor_menu_ip_phone_item[1][MAX_MSG_LEN] = {
	"IP �ּ�"
};

char kor_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"����Ŀ",
	"ȸ�� ����",
	"����ũ ����"
};

char kor_menu_doorphone_tone[3][MAX_MSG_LEN] = {
	"������",
	"��ȭ��",
	"��ȭ������"
};

char kor_menu_tone_info[2][MAX_MSG_LEN] = {
	"High �ð�",
	"Low �ð�"
};

char kor_menu_doorphone_hw_setup[2][MAX_MSG_LEN] = {
	"����Ī �ӵ�",
	"��� ����(BGN)"
};

char kor_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char kor_menu_network[6][MAX_MSG_LEN] = {
	"��ġ ID",
	"MAC �ּ�",
	"��Ʈ��ũ ���",
	"IP �ּ� ����",
	"���� ����",
	"Ping ����"
};

char kor_menu_network_serial[2][MAX_MSG_LEN] = {
	"�ø��� �ӵ�",
	"�ø��� ��Ʈ"
};

char kor_menu_serial_port[3][MAX_MSG_LEN] = {
	"��Ʈ 0(RS-485)",
	"��Ʈ 1(RS-232)",
	"��Ʈ 2"
};

char kor_menu_network_tcpip[7][MAX_MSG_LEN] = {
	"�ڵ� �ּ� �ޱ�",
	"��ġ IP �ּ�",
	"����� �Ž�ũ",
	"����Ʈ����",
	"���� �ּ�",
	"���� ��Ʈ",
	"IP �ּ�"
};

char kor_menu_network_mode[3][MAX_MSG_LEN] = {
	"�ø���",
	"���̾��",
	"TCP/IP"
};

char kor_menu_system[7][MAX_MSG_LEN] = {
	"�� ���� ����",
	"�ʱ�ȭ",
	"��������(from SD�޸�)",
	"��������(to SD�޸�)",
	"SD�޸� �����ϰ� ����",
	"��� ���",
	"��ġ ���ʱ�ȭ(����)"
};

char kor_menu_init_device[3][MAX_MSG_LEN] = {
	"�̺�Ʈ �ʱ�ȭ",
	"���� �ʱ�ȭ",
	"��� �ʱ�ȭ"
};

char kor_menu_imexport[6][MAX_MSG_LEN] = {
	"�߿���",
	"�ڿ� ����(������,����)",
	"������ ������",
	"����� ������",
	"�ܸ��� ���� ����",
	"K-300(�ܵ���) �߿���"
};

char kor_menu_comm_stats[2][MAX_MSG_LEN] = {
	"��� ����",
	"���ġ �ʱ�ȭ"
};

char kor_menu_downfirmware[5][MAX_MSG_LEN] = {
	"�߿��� ��������",
	"�ٿ�ε� ����",
	"�̿ϷḸ ��õ�",
	"�ٿ�ε� ���",
	"�ٿ�ε� ���� ����"
};

char kor_menu_inuse[2][MAX_MSG_LEN] = {
	"�̻��",
	"���"
};

char kor_menu_yesno[2][MAX_MSG_LEN] = {
	"��",
	"�ƴϿ�"
};

char kor_menu_noyes[2][MAX_MSG_LEN] = {
	"�ƴϿ�",
	"��"
};

char kor_menu_okcancel[2][MAX_MSG_LEN] = {
	"Ȯ��",
	"���"
};

char kor_menu_enable[MAX_MSG_LEN] = "��";

char kor_menu_other[24][MAX_MSG_LEN] = {
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
	"�̺�Ʈ ����"
};

char _kor_day_of_weeks[7][4] = {
	"��", "��", "ȭ", "��", "��", "��", "��"
};
