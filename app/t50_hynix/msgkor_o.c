char cjk_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"회사 이름",					// 0
	"사용자ID 입력",				// 1
	"사용자ID 확인",				// 2
	"암호 입력",					// 3
	"암호 확인",					// 4
	"새 암호 입력",					// 5
	"새 암호 확인",					// 6
	"지문을입력하십시오",			// 7
	"지문을 다시입력하십시오",		// 8
	"카드를입력하십시오",			// 9
	"카드을 다시입력하십시오",		// 10
	"한번 더입력하십시오",			// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"초",							// 14
	"늘",							// 15
    "삭제하시겠습니까 ?",			// 16
	"구내 전화번호를 누르세요",		// 17
	"단축 번호를 누르세요",			// 18
	"정말로 실행하시겠습니까?",		// 19
	"잠시만 기다려 주십시오"		// 20
};
//	"New Time",					// 12
//	"New TID",					// 13

char cjk_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"처리중...",					// 0
	"시스템\n오류입니다",			// 1
	"입력 시간이\n초과되었습니다",	// 2
	"잘못된\n사용자ID입니다",		// 3
	"잘못된\n데이타입니다",			// 4
	"미등록\n사용자입니다",			// 5
	"승인을 거부하였습니다",		// 6
	"승인하였습니다",				// 7	사용자를 확인\n하였습니다
	"이미 등록된\n사용자입니다",	// 8
	"이미 등록된\n카드입니다",		// 9
	"이미 등록된\n지문입니다",		// 10
	"암호가\n틀립니다",				// 11
	"데이타가\n틀립니다",			// 12
	"데이타를\n저장하였습니다",		// 13
	"사용자를\n삭제하였습니다",		// 14
	"전체 사용자를\n삭제하였습니다",	// 15
	"잘못된\n시간 설정입니다",		// 16
	"잘못된\n단말기ID입니다",		// 17
	"잘못된\n날짜입니다",			// 18
	"날짜를\n변경하였습니다",		// 19
	"잘못된\n시간입니다",				// 20
	"시간을\n변경하였습니다",			// 21
	"잘못된\nIP주소입니다",			// 22
	"잘못된 서브넷\n매스크입니다",		// 23
	"잘못된\n게이트웨이입니다",			// 24
	"잘못된\n서버 주소입니다",			// 25
	"잘못된\n서버 포트입니다",			// 26
	"전체를\n삭제하였습니다",			// 27
	"사용자가\n다 찼습니다",			// 28
	"잘못된\n전화번호입니다",			// 29
	"전화가 연결되지\n않았습니다",		// 30
	"통화중입니다",					// 31
	"없는 번호입니다",				// 32
	"응답하지 않습니다",				// 33
	"통화를\n종료하였습니다",			// 34
	"통화를\n종료하였습니다",			// 35
	"문이\n열렸습니다",				// 36
	"처리\n되었습니다",				// 37
	"실패\n하였습니다",				// 38
	"완료\n되었습니다",				// 39
	"사용할 수 없는\n카드입니다",		// 40
	"한장의 카드만\n대 주십시오",		// 41
	"등록이 되어\n있지 않습니다",		// 42
	"이미 등록되어\n있습니다",			// 43
	"데이타가\n다 찼습니다",			// 44
	"삭제하였습니다",					// 45
	"SD메모리 카드가\n없습니다",		// 46
	"SD메모리 준비가\n안되었습니다",	// 47
	"\n디렉토리 열기 오류",			// 48
	"\n디렉토리 읽기 오류",			// 49
	"\n디렉토리 쓰기 오류",			// 50
	"\n파일 열기 오류",				// 51
	"\n파일 읽기 오류",				// 52
	"\n파일 쓰기 오류",				// 53
	"\n파일 동기화 오류",				// 54
	"\n디렉토리가 없습니다",			// 55
	"\n파일이 없습니다",				// 56
	"지원하지 않는\n보안방식입니다",	// 57
	"잠시 후 시스템이\n재시작됩니다",	// 58
	"SD메모리를 안전하게\n제거할 수 있습니다", // 59
	"SD메모리 오류\n점검하십시오",		// 60
	"서버 접속 오류\n잠시 후 다시 하세요", // 61
	"주장치로부터\n응답이 없습니다",		// 62
	"주장치의 쓰기 충돌이\n발생했습니다", 	// 63
	"주장치로부터\n오류가 있습니다"	,	// 64
	"네트워크를\n사용할 수 없습니다",	// 65
	"케이스 열림\n경고",				// 66
	"전원이\n나갔습니다",				// 66
};

char cjk_auth_msgs[9][MAX_MULTIMSG_LEN] = {
	"출입기간이\n아닙니다",				// 0
	"출입문이\n개방되었습니다",			// 1
	"출입문 권한이\n없습니다",			// 2
	"사용자 권한이\n없습니다",			// 3
	"분실된\n카드입니다",				// 4
	"출입방법이\n틀립니다",				// 5
	"Anti-Passback\n출입거부입니다",	// 6
	"이미 처리된\n카드입니다",			// 7
	"모든 출입이\n거부되고 있습니다"		// 8
};

char cjk_ta_msgs[MAX_TA_MSGS_COUNT][MAX_TAMSG_LEN] = {
	"출 근",		// 1
	"퇴 근",		// 2
	"외 출",		// 3
	"복 귀",		// 4
	"출 석",		// 5
	"조 퇴",		// 6
	"등 교",		// 7
	"하 교"		// 8
};
/*
char cjk_ta_msgs[8][MAX_TAMSG_LEN] = {
	"출 석",		// 1
	"조 퇴",		// 2
	"등 교",		// 3
	"하 교",		// 4
	"출 입",		// 5
	"부 재",		// 6
	"휴 식",		// 7
	"복 귀"		// 8
};
*/
char cjk_menu_top_title[MAX_MSG_LEN] = "설정";

char cjk_menu_top[6][MAX_MSG_LEN] = {
	"사용자",
	"하드웨어",
	"출입 제어",
	"네트워크",
	"환경 설정",
	"시스템"
};

char cjk_menu_lang[3][MAX_MSG_LEN] = {
	"영어-ENG",
#ifdef _CHN
	"중국어-CHN",
#else
#ifdef _JPN
	"일본어-JPN",
#else
#ifdef _TWN
	"중국어:번체-TWN",
#else
	"한국어-KOR",
#endif
#endif
#endif
};

char cjk_menu_user[4][MAX_MSG_LEN] = {
	"사용자 추가",
	"사용자 편집",
	"사용자 삭제",
	"사용자 전체 삭제"
};

char cjk_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",
	"액세스 모드",
	"지문1",
	"지문2",
	"카드",
	"암호",
	"인증 등급"
};

char cjk_menu_user_regex[4][MAX_MSG_LEN] = {
	"출입 권한",
	"유효기간 시작일",
	"유효기간 종료일",
	"카드 상태"
};

char cjk_menu_access_mode[7][MAX_MSG_LEN] = {
	"지문",
	"카드",
	"암호",
	"카드+지문",
	"카드+암호",
	"카드+지문+암호",
	"암호+지문"
};

char cjk_menu_security_level[10][MAX_MSG_LEN] = {
	"단말기 설정",
	"가장 낮음",
	"매우 낮음",
	"낮음",
	"약간 낮음",
	"보통",
	"약간 높음",
	"높음",
	"매우 높음",
	"가장 높음"
};

char cjk_menu_time_attend[3][MAX_MSG_LEN] = {
	"타이머",
	"취소",
	"근태 항목"
};

char cjk_menu_ta_item[3][MAX_MSG_LEN] = {
	"활성화",
	"문자 선택",
	"시간대"
};

char cjk_menu_meal[9][MAX_MSG_LEN] = {
	"잠 김",
	"조 식",
	"중 식",
	"석 식",
	"야 식",
	"간 식",
	"중복 금지",
	"식권 인쇄",
	"그릇 수 입력"
};

char cjk_menu_meal_sub[2][MAX_MSG_LEN] = {
	"시간대",
	"메뉴 개수"
};

char cjk_menu_meal_funckey[4][MAX_MSG_LEN] = {
	"메뉴 1",
	"메뉴 2",
	"메뉴 3",
	"메뉴 4"
};

char cjk_menu_event[3][MAX_MSG_LEN] = {
	"이벤트 삭제",
	"출력 유형",
	"이벤트 방지"
};

char cjk_menu_event_output[3][MAX_MSG_LEN] = {
	"네트워크",
	"26 Wiegand",
	"34 Wiegand"
};

char cjk_menu_event_mask[3][MAX_MSG_LEN] = {
	"록 이벤트",
	"출입 이벤트",
	"기타 이벤트"
};

char cjk_menu_event_wiegand[2][MAX_MSG_LEN] = {
	"사용자ID",
	"카드 데이타"
};

char cjk_menu_hardware[4][MAX_MSG_LEN] = {
	"출입문",
	"리더기",	
	"입력",
	"출력"
};

char cjk_menu_access_door[7][MAX_MSG_LEN] = {
	"리더기",
	"도어 록 릴레이",
	"퇴실 버튼 입력",
	"문상태 감지 입력",
	"문열림 시간",
	"록 유형",
	"경고"
};

char cjk_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM 록",
	"데드볼트"
};

char cjk_menu_door_alarm[3][MAX_MSG_LEN] = {
	"장시간 문 열림 경고",
	"문 강제 열림 경고",
	"장시간 문열림 시간"
};

char cjk_menu_alarm[4][MAX_MSG_LEN] = {
	"경고 지연 시간",
	"경고 후 지연 시간",
	"경고 출력",
	"경고 금지"
};

char cjk_menu_access_point[7][MAX_MSG_LEN] = {
	"인증 모드",
	"지원하는 카드 포멧",
	"암호 지원",
	"경고",
	"서버 인증 모드",
	"리더기 설정",
	"근태"
};

char cjk_menu_auth_mode[4][MAX_MSG_LEN] = {
	"정상 인증",
	"권한 생략 인증",
	"모두 거부",
	"카드 모두 승인"
};

char cjk_menu_ap_alarm[3][MAX_MSG_LEN] = {
	"미등록 경고",
	"유효기간 만료 경고",
	"승인거부 경고"
};

char cjk_menu_server_auth[3][MAX_MSG_LEN] = {
	"장치 단독",
	"장치 단독 + 서버",
	"서버"
};

char cjk_menu_reader_setting[6][MAX_MSG_LEN] = {
	"모드",
	"Type A / Type B",
	"속도",
	"패러티",
	"스톱 비트", 
	"IP 주소 및 포트"
};

char cjk_menu_fp_reader[4][MAX_MSG_LEN] = {
	"인증 등급",
	"1:N 인증",
	"지문 노출량",
	"빠른 캡춰"	
};

char cjk_menu_reader[3][MAX_MSG_LEN] = {
	"지문 리더",
	"PIN 리더",
	"카드 리더"
};

char cjk_menu_card_reader[5][MAX_MSG_LEN] = {
	"스마트 카드 리더",
	"EM 리더",
	"Wiegand 리더",
	"시리얼 리더",
	"IP 리더"
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
	"극성",
	"유형"
};

char cjk_menu_io_polarity[2][MAX_MSG_LEN] = {
	"정상(Normal Open)",
	"반전(Normal Close)"
};

char cjk_menu_bi_type[5][MAX_MSG_LEN] = {
	"일반",
	"퇴실버튼",
	"문상태 감지",
	"엘리베이터",
	"침입존"
};

char cjk_menu_bo_type[3][MAX_MSG_LEN] = {
	"일반",
	"도어 록",
	"엘리베이터 버튼",
};

char cjk_menu_access_control[3][MAX_MSG_LEN] = {
	"카드 포멧",
	"스마트 카드 구성",
	"출력 명령"
};

char cjk_menu_credential[7][MAX_MSG_LEN] = {
	"32-bit 정수",
	"64-bit 정수",
	"Facility(8-bit)+CardNo(16-bit)",
	"Facility(16-bit)+CardNo(32-bit)",
	"Facility(32-bit)+CardNo(32-bit)",
	"BCD 숫자",
	"문자 스트링"
};

char cjk_menu_field_type[2][MAX_MSG_LEN] = {
	"카드번호",
	"Facility code"
};

char cjk_menu_cred_format[3][MAX_MSG_LEN] = {
	"지문",
	"비밀번호",
	"스마트카드 포멧"
};

char cjk_menu_card_format[7][MAX_MSG_LEN] = {
	"일련번호(UID)",
	"블록",
	"ISO7816-4 파일",
	"EM 포멧",
	"Wiegand 포멧",
	"시리얼리더 포멧",
	"IP리더 포멧"
};

char cjk_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"표준 26-bit 포멧",
	"HID 37-bit 포멧",
	"HID 37-bit 포멧(Facility)",
	"35-bit HID Coporate 1000",
	"사용자 정의"
};

char cjk_menu_format_setting[6][MAX_MSG_LEN] = {
	"크리덴셜",
	"카드 유형",
	"카드 앱",
	"출력 Wiegand 포멧",
	"Wiegand 패러티",
	"필드 유형"
};

char cjk_menu_card_type[5][MAX_MSG_LEN] = {
	"ISo14443A",
	"ISo14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM 카드"
};

char cjk_menu_card_field[4][MAX_MSG_LEN] = {
	"포멧 유형",
	"간격",
	"길이",
	"코딩"
};

char cjk_menu_field_coding[3][MAX_MSG_LEN] = {
	"이진 데이터",
	"BCD",
	"문자(ASCII)"
};

char cjk_menu_mifare_blk[2][MAX_MSG_LEN] = {
	"섹터",
	"블록"
};

char cjk_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"없슴",
	"짝수/홀수 패러티",
	"기타 패러티"
};

char cjk_menu_output_command[3][MAX_MSG_LEN] = {
	"반복 횟수",
	"ON 시간",
	"OFF 시간"
};

char cjk_menu_armed[4][MAX_MSG_LEN] = {
	"방범 설정",
	"방범 모드",
	"경계",
	"침입"
};

char cjk_menu_byte_order[2][MAX_MSG_LEN] = {
	"정상 순서",
	"역 순서"
};

char cjk_menu_em_format[3][MAX_MSG_LEN] = {
	"32-bit",
	"24-bit",
	"20-bit"
};

char cjk_menu_external_format[3][MAX_MSG_LEN] = {
	"900MHz 리더",
	"바코드 리더",
	"EM 리더"
};

char cjk_menu_ip_reader[3][MAX_MSG_LEN] = {
	"리더기 네트워크 설정",
	"주소",
	"포트"
};

char cjk_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"정상",
	"카드 번호만 사용",
	"패러티 추가"
};

char cjk_menu_duress[2][MAX_MSG_LEN] = {
	"협박모드 숫자",
	"협박모드 경고"
};

char cjk_menu_setup[8][MAX_MSG_LEN] = {
	"관리자",
	"날짜 및 시간",
	"화면",
	"음성",
	"잠금 설정",
	"언어(Language)",
	"카메라",
	"도어폰"
};

char cjk_menu_admin[2][MAX_MSG_LEN] = {
	"관리자 암호",
	"관리자 지문"
};

char cjk_menu_wiegand_type[2][MAX_MSG_LEN] = {
	"26 Wiegand",
	"34 Wiegand"
};

char cjk_menu_datetime[3][MAX_MSG_LEN] = {
	"날짜 설정",
	"시간 설정",
	"24시간 형식 사용"
};

char cjk_menu_display[3][MAX_MSG_LEN] = {
	"조명 시간",
	"콘트라스트",
	"밝기"
};

char cjk_menu_time_notation[2][MAX_MSG_LEN] = {
	"오후 1:00",
	"13:00"
};

char cjk_menu_date_notation[3][MAX_MSG_LEN] = {
	"표시 안함",
	"MM/DD",
	"MM월DD일"
};

char cjk_menu_sound[3][MAX_MSG_LEN] = {
	"인증 음량",
	"관리 기능 음량",
	"버튼 음량"
};

char cjk_menu_lockout[2][MAX_MSG_LEN] = {
	"탬퍼(분리 경고)",
	"키 잠금"
};

char cjk_menu_cam[4][MAX_MSG_LEN] = {
	"카메라 제어",
	"카메라 채널",
	"카메라 보기",
	"이벤트 영상 저장"
};

char cjk_menu_cam_control[3][MAX_MSG_LEN] = {
	"사용안함",
	"끄기(Off)",
	"켜기(On)"
};

char cjk_menu_doorphone[8][MAX_MSG_LEN] = {
	"운용 모드",
	"전화 번호",
	"음량 조절",
	"침묵 시간",
	"문열기 번호",
	"톤 정의",
	"톤 점검",
	"하드웨어 설정"
};

char cjk_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"사용 안함",
	"IP 전화",
	"인터폰",
	"일반 전화"
};

char cjk_menu_phone_number[3][MAX_MSG_LEN] = {
	"다이얼링 모드",
	"IP 전화번호",
	"단축 번호"
};

char cjk_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"표준",
	"단일 번호",
	"단축 번호"
};

char cjk_menu_ip_phone_item[2][MAX_MSG_LEN] = {
	"전화번호",
	"IP 주소"
};

char cjk_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"스피커 음량",
	"마이크 감도",
	"회선 감도"
};

char cjk_menu_doorphone_tone[3][MAX_MSG_LEN] = {
	"링백톤",
	"통화중",
	"전화끊김음"
};

char cjk_menu_tone_info[2][MAX_MSG_LEN] = {
	"High 시간",
	"Low 시간"
};

char cjk_menu_doorphone_hw_setup[2][MAX_MSG_LEN] = {
	"스위칭 속도",
	"배경 소음(BGN)"
};

char cjk_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char cjk_menu_network[7][MAX_MSG_LEN] = {
	"기기 주소",
	"네트워크 유형",
	"IP 주소 설정",
	"마스터 설정",
	"서버 설정",
	"와이파이(Wi-Fi)",
	"Ping 시험"
};

char cjk_menu_network_ip[2][MAX_MSG_LEN] = {
	"유선 LAN",
	"와이파이(Wi-Fi)"
};

char cjk_menu_network_serial[2][MAX_MSG_LEN] = {
	"시리얼 속도",
	"시리얼 포트"
};

char cjk_menu_serial_port[3][MAX_MSG_LEN] = {
	"포트 0(RS-485)",
	"포트 1(RS-232)",
	"포트 2"
};

char cjk_menu_network_tcpip[7][MAX_MSG_LEN] = {
	"MAC 주소",
	"자동 주소 받기",
	"IP 주소",
	"서브넷 매스크",
	"게이트웨이",
	"서버 주소",
	"서버 포트"
};

char cjk_menu_network_type[2][MAX_MSG_LEN] = {
	"시리얼(RS-485)",
	"IP 망"
};

char cjk_menu_network_wifi[8][MAX_MSG_LEN] = {
	"SSID",
	"상태",
	"신호 세기",
	"채널",
	"보안",
	"저장 안함",
	"연결",
	"변경 없슴"
};

char cjk_menu_wpa_state[12][MAX_MSG_LEN] = {
	"초기화 안됨",
	"비활성화",
	"연결 안됨",
	"휴면 상태",
	"찾는중...",
	"인증중...",
	"연결중...",
	"연결중...",
	"인증중...",
	"인증중...",
	"연결됨",
	"IP 주소 받는 중..."
};

char cjk_menu_wifi_security[8][MAX_MSG_LEN] = {
	"설정 안함",
	"WPS",
	"WEP",
	"WPA PSK",
	"WPA PSK",
	"WPA2 PSK",
	"802.1x EAP"
	"802.1x EAP"
};

char cjk_menu_wifi_state[6][MAX_MSG_LEN] = {
	"사용 안함",
	"저장됨",
	"범위내에 없슴",
	"(으)로 보안",
	"인증 오류",
	"오류 발생"
};

char cjk_menu_system[5][MAX_MSG_LEN] = {
	"상세 정보 보기",
	"초기화",
	"가져오기(from USB)",
	"내보내기(to USB)",
	"장치 재초기화(리셋)"
};

char cjk_menu_init_config[4][MAX_MSG_LEN] = {
	"이벤트 초기화",
	"설정 초기화",
	"모두 초기화",
	"플래시 포멧"
};
char cjk_menu_imexport[4][MAX_MSG_LEN] = {
	"펌웨어",
	"자원 파일(아이콘,음성)",
	"장치 설정 정보",	
	"사용자 데이타"
};

char cjk_menu_comm_stats[2][MAX_MSG_LEN] = {
	"통계 보기",
	"통계치 초기화"
};

char cjk_menu_yesno[2][MAX_MSG_LEN] = {
	"예",
	"아니오"
};

char cjk_menu_noyes[2][MAX_MSG_LEN] = {
	"아니오",
	"예"
};

char cjk_menu_okcancel[4][MAX_MSG_LEN] = {
	"확인",
	"취소",
	"완료",
	"저장"
};

char cjk_menu_enable[2][MAX_MSG_LEN] = {
	"활성화",
	"사용 안함"
};

char cjk_menu_hw_config[9][MAX_MSG_LEN] = {
	"자체",
	"기기",
	"출입문",
	"리더기",
	"입력",
	"출력",
	"Wiegand",
	"RS-485",
	"CH"
};

char cjk_menu_unit[5][MAX_MSG_LEN] = {
	"바이트",
	"비트",
	"숫자",
	"초",
	"분"
};

char cjk_menu_other[25][MAX_MSG_LEN] = {
	"펌웨어 버젼",
	"모델명",
	"사용자 등록수",
	"지문 등록수",
	"분",
	"무제한",
	"식      권",
	"0:스트링(null)",
	"IP 전화번호 등록수",
	"전체",
	"0.5초",
	"미지정",		// Unspecified(null)
	"항상 부정",	// Never
	"항상 긍정",	// Always(all the time)
	"일정",
	"항상 OFF",
	"항상 ON",
	"대기",
	"성공",
	"실패",
	"연결 안됨",
	"다운로드 중",
	"다운로드 완료",
	"마스터 비밀번호",
	"총 이벤트 갯수"
};

char _cjk_day_of_weeks[7][4] = {
	"일", "월", "화", "수", "목", "금", "토"
};

char _cjk_am_pm[2][6] = {
	"오전", "오후"
};

char _cjk_year_mon_day[3][4] = {
	"년", "월", "일"
};


char car_no[20] = "부산34바6325";
