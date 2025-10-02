char cjk_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"회사 이름",					// 0
	"사용자ID 입력",				// 1
	"사용자ID 확인",				// 2
	"비밀번호 입력",				// 3
	"비밀번호 확인",				// 4
	"새 비밀번호 입력",				// 5
	"새 비밀변호 확인",				// 6
	"지문을 입력하세요",			// 7
	"지문을 다시 입력하세요",		// 8
	"카드를 입력하세요",			// 9
	"카드을 다시 입력하세요",		// 10
	"한번 더 입력하세요",			// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"초",							// 14
	"늘",							// 15
    "삭제하시겠습니까 ?",			// 16
	"구내 전화번호를 누르세요",		// 18
	"단축 번호를 누르세요",			// 19
	"정말로 실행하시겠습니까?",		// 20
	"잠시만 기다려 주십시오"		// 21
};
//	"New Time",						// 12
//	"New TID",						// 13

char cjk_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"처리중...",					// 0
	"시스템\n오류입니다",			// 1
	"입력 시간이\n초과되었습니다",	// 2
	"잘못된\n사용자ID입니다",		// 3
	"잘못된\n데이타입니다",			// 4
	"등록이 안된\n사용자입니다",	// 5
	"승인을 거부하였습니다",		// 6
	"승인하였습니다",				// 7	사용자를 확인\n하였습니다
	"이미 등록된\n사용자입니다",	// 8
	"이미 등록된\n카드입니다",		// 9
	"이미 등록된\n지문입니다",		// 10
	"비밀번호가\n틀립니다",			// 11
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
	"네트워크\n장애입니다",				// 66
	"서버로부터\n응답이 없습니다",		// 67
	"케이스 열림\n경고",				// 68
	"전원이\n나갔습니다",				// 69
	"전원을\n끕니다",					// 70
	"슬레이브\n초기화중",				// 71
};

char cjk_auth_msgs[9][MAX_MULTIMSG_LEN] = {
	"출입기간이\n아닙니다",				// 0
	"출입문이\n개방되어 있습니다",		// 1
	"출입문이\n잠겨 있습니다",			// 2
	"사용자 권한이\n없습니다",			// 3
	"정지된\n카드입니다",				// 4
	"출입방법이\n틀립니다",				// 5
	"Anti-Passback\n출입거부입니다",	// 6
	"이미 처리된\n카드입니다",			// 7
	"모든 출입이\n거부되고 있습니다"	// 8
};

char cjk_alarm_msgs[4][MAX_MULTIMSG_LEN] = {
	"산업보안 규정위반",
	"출입문이 강제개방되었습니다",
	"출입문이 열렸습니다",
	"출입문을 닫아주세요"
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

char cjk_menu_top_title[MAX_MSG_LEN] = "설정";

char cjk_menu_top[6][MAX_MSG_LEN] = {
	"사용자",
	"하드웨어",
	"출입 제어",
	"네트워크",
	"장치 설정",
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

char cjk_menu_user[3][MAX_MSG_LEN] = {
	"사용자 추가",
	"사용자 편집/삭제",
	"사용자 전체 삭제"
};

char cjk_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",
	"액세스 모드",
	"지문 1",
	"지문 2",
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
	"지문+암호"
};

char cjk_menu_hardware_class[12][MAX_MSG_LEN] = {
	"네트워크 포트",
	"기기",
	"입출력기기",
	"하위 장치",
	"리더",
	"입력",
	"출력",
	"출입문",
	"출입 구역",
	"화재 구역",
	"침입탐지 구역",
	"엘리베이터",
};

char cjk_menu_device[3][MAX_MSG_LEN] = {
	"입력, 출력",
	"마스터 네트워크 포트",
	"슬레이브 장치",
};

char cjk_menu_hardware[11][MAX_MSG_LEN] = {
	"주소",
	"장치번호",
	"네트워크 주소",
	"모델명",
	"할당",
	"유형",
	"채널",
	"네트워크 포트",
	"리더",
	"리더 장치",
	"기기",
};

char cjk_menu_unit_model[4][MAX_MSG_LEN] = {
	"가상",
	"K200-4",
	"K200-2",
	"자체"
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
	"일반",
	"주리더",
	"보조리더",
	"엘리베이터",
	"미정의"
};

char cjk_menu_input_type[7][MAX_MSG_LEN] = {
	"일반",
	"출구 버튼",
	"문 스위치",
	"엘리베이터",
	"침입탐지",
	"화재구역",
	"미정의"
};

char cjk_menu_output_type[5][MAX_MSG_LEN] = {
	"일반",
	"도어록",
	"경고 출력",
	"엘리베이터 버튼",
	"미정의"
};

char cjk_menu_input_output[1][MAX_MSG_LEN] = {
	"극성",
};

char cjk_menu_io_polarity[2][MAX_MSG_LEN] = {
	"정상",
	"반전"
};

char cjk_menu_armed[4][MAX_MSG_LEN] = {
	"방범 설정",
	"방범 모드",
	"경계",
	"침입"
};

char cjk_menu_fire_zone[4][MAX_MSG_LEN] = {
	"화재구역",
	"화재 입력",
	"화재시 여는 문",
	"경보 릴레이",
};

char cjk_menu_dev_model[6][MAX_MSG_LEN] = {
	"K100-4",
	"K100-2",
	"Wiegand 리더",
	"Wiegand 리더+키패드",
	"T35s-NonIo",
	"T40A",
};

char cjk_menu_door[13][MAX_MSG_LEN] = {
	"주리더",
	"보조 리더",
	"입력/출력 사용",
	"도어 록 릴레이",
	"퇴실 버튼 입력",
	"문상태 감지 입력",
	"경보",
	"경보 릴레이",
	"경보 액션",
	"문열림 시간",
	"확장 문열림 시간",
	"록 유형",
	"보조 록"
};

char cjk_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM 록",
	"데드볼트"
};

char cjk_menu_alarm[4][MAX_MSG_LEN] = {
	"경고 지연 시간",
	"경고 후 지연 시간",
	"장시간 문열림 시간",
	"경고 금지"
};

char cjk_menu_access_point[7][MAX_MSG_LEN] = {
	"지원 크리덴셜 포멧",
	"인증 모드",
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

char cjk_menu_server_auth[3][MAX_MSG_LEN] = {
	"장치 단독",
	"장치 단독 + 서버",
	"서버"
};

char cjk_menu_ap_alarm[3][MAX_MSG_LEN] = {
	"미등록 경고",
	"유효기간 만료 경고",
	"승인거부 경고"
};

char cjk_menu_reader[7][MAX_MSG_LEN] = {
	"스마트카드 리더",
	"EM 리더",
	"Wiegand 리더",
	"시리얼 리더",
	"IP 리더",
	"PIN 리더",
	"지문 리더"
};

char cjk_menu_readerop[6][MAX_MSG_LEN] = {
	"인증 모드",
	"인증 결과",
	"슬립 모드",
	"시간 표시",
	"안테나 출력",
	"근태 표시 시간"
};

char cjk_menu_authmode[5][MAX_MSG_LEN] = {
	"일반 모드",
	"카드",
	"카드+ PIN",
	"카드 or PIN",
	"PIN"
};

char cjk_menu_authresult[4][MAX_MSG_LEN] = {
	"LED, Beep 제어",
	"대기 시간",
	"표시 시간",
	"인증 금지"
};

char cjk_menu_ledbeep[4][MAX_MSG_LEN] = {
	"사용안함",
	"적색 LED + Beep",
	"청색 LED + Beep",
	"이중 라인 LED"
};

char cjk_menu_ac[4][MAX_MSG_LEN] = {
	"포멧 설정",
	"경고 액션",
	"입력 포멧",
	"Wiegand 출력 포멧"
};

char cjk_menu_credential[5][MAX_MSG_LEN] = {
	"32-bit 정수",
	"64-bit 정수",
	"Facility+CardNo",
	"BCD 숫자",
	"문자 스트링"
};

char cjk_menu_field_type[2][MAX_MSG_LEN] = {
	"카드번호",
	"Facility code"
};

char cjk_menu_cred_format[4][MAX_MSG_LEN] = {
	"스마트 카드 포멧",
	"Wiegand 카드 포멧",
	"PIN 포멧",
	"Wiegand PIN 포멧"
};

char cjk_menu_card_format[8][MAX_MSG_LEN] = {
	"일련번호(UID) 포멧",
	"블록 포멧",
	"EMV(ISO7816) 포멧",
	"EM 포멧",
	"시리얼리더 포멧",
	"IP리더 포멧",
	"코딩 포멧",
	"모바일(EMV) 카드"
};

char cjk_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"표준 26-bit 포멧",
	"35-bit HID Coporate 1000",
	"34-bit 포멧(패러티)",
	"66-bit 포멧(패러티)",
	"사용자 정의"
};

char cjk_menu_pin_format[5][MAX_MSG_LEN] = {
	"입력 간격",
	"입력종료 문자",
	"최대 길이",
	"Facility code",
	"PIN 번호",
};

char cjk_menu_wiegand_pin[4][MAX_MSG_LEN] = {
	"4-bit 버스트 모드",
	"8-bit 버스트 모드",
	"표준 26-bit 포멧",
	"고정 길이 모드"
};

char cjk_menu_format_setting[8][MAX_MSG_LEN] = {
	"크리덴셜",
	"카드 유형",
	"카드 앱",
	"출력 Wiegand 포멧",
	"Wiegand 패러티",
	"코딩",
	"카드 필드",
	"리더 설정"
};

char cjk_menu_card_type[5][MAX_MSG_LEN] = {
	"ISO14443A",
	"ISO14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM 카드"
};

char cjk_menu_cardtype_ext[3][MAX_MSG_LEN] = {
	"900MHz",
	"바코드",
	"기타 카드"
};

char cjk_menu_cardapp_block[2][MAX_MSG_LEN] = {
	"정상",
	"암호화"
};

char cjk_menu_cardapp_iso7816[3][MAX_MSG_LEN] = {
	"일반",
	"교통카드",
	"SK SAM"
};

char cjk_menu_cardapp_serial[2][MAX_MSG_LEN] = {
	"일반",
	"특수"
};

char cjk_menu_card_field[3][MAX_MSG_LEN] = {
	"포멧 유형",
	"간격",
	"길이"
};

char cjk_menu_field_coding[4][MAX_MSG_LEN] = {
	"이진 데이터",
	"BCD",
	"문자(ASCII)",
	"역순 이진 데이터"
};

char cjk_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"없슴",
	"짝수/홀수 패러티",
	"기타 패러티"
};

char cjk_menu_reader_setting[8][MAX_MSG_LEN] = {
	"블록번호",
	"섹터",
	"블록",
	"속도",
	"패러티",
	"스톱 비트", 
	"마이페어 키",
	"AID"
};

char cjk_menu_ip_reader[3][MAX_MSG_LEN] = {
	"리더 네트워크 설정",
	"주소",
	"포트"
};

char cjk_menu_action_idx[12][MAX_MSG_LEN] = {
	"입력(Exit)",
	"입력(Door)",
	"입력(Input)",
	"입력(Fire,Tamper)",
	"화재 경보",
	"문열림 경보",
	"문강제열림 경보",
	"미등록 카드",
	"유효기간 만료",
	"패스백 위반 탐지",
	"출입 거부",
	"침입"
};

char cjk_menu_action_cmd[4][MAX_MSG_LEN] = {
	"경고",
	"ON 시간",
	"OFF 시간",
	"반복 횟수"
};

char cjk_menu_setup[11][MAX_MSG_LEN] = {
	"관리자 비밀번호",
	"날짜 및 시간",
	"화면",
	"소리",
	"언어(Language)",
	"인증 제어",
	"전원 관리",
	"카메라",
	"BLE(Bluetooth Low Energy)",
	"IP 전화",
	"안테나 출력"
};

char cjk_menu_admin[2][MAX_MSG_LEN] = {
	"비밀번호 변경",
	"숫자만 사용"
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

char cjk_menu_sound[6][MAX_MSG_LEN] = {
	"사용자 음량",
	"미디어 음량",
	"통화 음량",
	"시스템 음량",
	"키패드 버튼음",
	"터치음"
};

char cjk_menu_sleepmode[3][MAX_MSG_LEN] = {
	"모드 설정",
	"대기 시간",
	"동영상" 
};

char cjk_menu_sleepact[3][MAX_MSG_LEN] = {
	"사용 안함",
	"디스플레이 꺼짐",
	"동영상 재생"
};

char cjk_menu_video[1][MAX_MSG_LEN] = {
	"USB 로부터 복사"
};

char cjk_menu_lockout[2][MAX_MSG_LEN] = {
	"탬퍼(분리 경고)",
	"키 잠금",
};

char cjk_menu_power_mgmt[2][MAX_MSG_LEN] = {
	"배터리 사용",
	"배터리시 록 사용"
};

char cjk_menu_cam[2][MAX_MSG_LEN] = {
	"초당 프레임수",
	"이벤트 영상 저장"
};

char cjk_menu_cam_control[3][MAX_MSG_LEN] = {
	"사용안함",
	"끄기(Off)",
	"켜기(On)"
};

char cjk_menu_ble[5][MAX_MSG_LEN] = {
	"비콘 수집",
	"비콘 신호수준",
	"비콘 간격",
	"MAC 주소 필터",
	"비콘 보기"
};

char cjk_menu_phone[6][MAX_MSG_LEN] = {
	"전화 계정",
	"계정 추가",
	"계정 편집",
	"계정 삭제",
	"계정 전체 삭제",
	"전화 걸기"
};

char cjk_menu_phone_reg[2][MAX_MSG_LEN] = {
	"이름",
	"SIP 계정"
};

char cjk_menu_call_state[3][MAX_MSG_LEN] = {
	"거는중",
	"연결중",
	"통화 종료"
};

char cjk_menu_network[10][MAX_MSG_LEN] = {
    "네트워크 설정",
    "IP 주소 설정",
    "서버 설정",
    "서버 주소",
    "서버 포트",
    "마스터 설정",
    "마스터 주소",
    "마스터 포트",
    "Ping 시험",
    "Ping 실행"
};

char cjk_menu_network_ip[8][MAX_MSG_LEN] = {
    "하드웨어(MAC) 주소",
    "자동으로 IP 주소 받기",
    "IP 주소",
    "서브넷 마스크",
    "기본 게이트웨이",
    "자동으로 DNS 서버 주소 받기",
    "기본 DNS 서버",
    "보조 DNS 서버",
};

char cjk_menu_network_serial[2][MAX_MSG_LEN] = {
	"시리얼 속도",
	"프로토콜",
};

char cjk_menu_network_type[2][MAX_MSG_LEN] = {
	"IP",
	"RS-485"
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

char cjk_menu_system[9][MAX_MSG_LEN] = {
	"마스터/슬레이브 장치",
	"상세 정보 보기",
	"초기화",
	"테스트",
	"가져오기(from USB)",
	"내보내기(to USB)",
	"내려받기",
	"장치 재초기화(리셋)",
	"전원 끄기(종료)"
};

char cjk_menu_download[4][MAX_MSG_LEN] = {
	"배경화면",
	"펌웨어",
	"OS 커널",
	"동영상"
};

char cjk_menu_mst_slv[2][MAX_MSG_LEN] = {
	"마스터 장치",
	"슬레이브 장치"
};

char cjk_menu_net_remote[2][MAX_MSG_LEN] = {
	"서버 연결",
	"마스터 연결"
};

char cjk_menu_init_config[4][MAX_MSG_LEN] = {
	"이벤트 초기화",
	"설정 초기화",
	"모두 초기화",
	"플래시 포멧"
};

char cjk_menu_test[4][MAX_MSG_LEN] = {
	"음성",
	"입력",
	"출입문 입출력",
	"카드 리더"
};

char cjk_menu_imexport[6][MAX_MSG_LEN] = {
	"펌웨어",
	"OS 커널",
	"자원 파일(아이콘,음성)",
	"배경 화면",
	"장치 설정 정보",	
	"사용자 데이타"
};

char cjk_menu_comm_stats[2][MAX_MSG_LEN] = {
	"통계 보기",
	"통계치 초기화"
};

char cjk_menu_bat_status[6][MAX_MSG_LEN] = {
	"연결 안됨",
	"충전중",
	"방전중",
	"충전 안함",
	"완료",
	"연결됨"
};

char cjk_menu_yesno[2][MAX_MSG_LEN] = {
	"예",
	"아니오"
};

char cjk_menu_noyes[2][MAX_MSG_LEN] = {
	"아니오",
	"예"
};

char cjk_menu_okcancel[5][MAX_MSG_LEN] = {
	"확인",
	"취소",
	"완료",
	"저장",
	"삭제"
};

char cjk_menu_enable[2][MAX_MSG_LEN] = {
	"활성화",
	"사용 안함"
};

char cjk_menu_unit[5][MAX_MSG_LEN] = {
	"비트",
	"숫자",
	"바이트",
	"초",
	"분"
};

char cjk_menu_other[15][MAX_MSG_LEN] = {
	"OS 커널 버젼",
	"펌웨어 버젼",
	"모델명",
	"무제한",
	"전체",
	"입력 전압",
	"배터리",
	"파일 크기",
	"미지정",		// Unspecified(null)
	"항상 부정",	// Never
	"항상 긍정",	// Always(all the time)
	"일정",
	"마스터 비밀번호",
	"이벤트 갯수",
	"사용자 수",
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

char _cjk_day_of_week_postfix[8] = "요일";

