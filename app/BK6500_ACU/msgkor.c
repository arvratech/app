char kor_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"회사 이름",						// 0
	"사용자ID 입력:",					// 1
	"사용자ID 확인:",					// 2
	"암호 입력:",						// 3
	"암호 확인:",						// 4
	"새 암호 입력:",					// 5
	"새 암호 확인:",					// 6
	"지문을\n입력하십시오",				// 7
	"지문을 다시\n입력하십시오",		// 8
	"카드를\n입력하십시오",				// 9
	"카드을 다시\n입력하십시오",		// 10
	"한번 더\n입력하십시오",			// 11
	"YY/MM/DD",							// 12
	"HH:MM:SS",							// 13
	"초",								// 14
	"늘",								// 15
    "삭제하시겠습니까 ?",				// 16
	"구내 전화번호를 누르세요",			// 17
	"단축 번호를 누르세요",				// 18
	"정말로 실행하시겠습니까 ?",		// 19
	"잠시만 기다려 주십시오"			// 20
};
//	"New Time",							// 12
//	"New TID",							// 13

char kor_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"처리중...",						// 0
	"시스템\n오류입니다",				// 1
	"입력 시간이\n초과되었습니다",		// 2
	"잘못된\n사용자ID입니다",			// 3
	"잘못된\n데이터입니다",				// 4
	"미등록\n사용자입니다",				// 5
	"승인을 거부\n하였습니다",			// 6
	"승인하였습니다",					// 7	사용자를 확인\n하였습니다
	"이미 등록된\n사용자입니다",		// 8
	"이미 등록된\n카드입니다",			// 9
	"이미 등록된\n지문입니다",			// 10
	"암호가\n틀립니다",					// 11
	"데이터가\n틀립니다",				// 12
	"데이터를\n저장하였습니다",			// 13
	"사용자를\n삭제하였습니다",			// 14
	"전체 사용자를\n삭제하였습니다",	// 15
	"잘못된\n시간 설정입니다",			// 16
	"잘못된\n단말기ID입니다",			// 17
	"잘못된\n날짜입니다",				// 18
	"날짜를\n변경하였습니다",			// 19
	"잘못된\n시간입니다",				// 20
	"시간을\n변경하였습니다",			// 21
	"잘못된\nIP주소입니다",				// 22
	"잘못된 서브넷\n매스크입니다",		// 23
	"잘못된\n게이트웨이입니다",			// 24
	"잘못된\n서버 주소입니다",			// 25
	"잘못된\n서버 포트입니다",			// 26
	"전체를\n삭제하였습니다",			// 27
	"사용자가\n다 찼습니다",			// 28
	"전화번호\n오류입니다",				// 29
	"전화가 연결되지\n않았습니다",		// 30
	"통화중입니다",						// 31
	"응답하지 않습니다",				// 32
	"전화 끊김",						// 33
	"통화",								// 34
	"문이\n열렸습니다",					// 35
	"처리\n되었습니다",					// 36
	"실패\n하였습니다",					// 37
	"완료\n되었습니다",					// 38
	"사용할 수 없는\n카드입니다",		// 39
	"한장의 카드만\n대 주십시오",		// 40
	"등록이 되어\n있지 않습니다",		// 41
	"이미 등록되어\n있습니다",			// 42
	"데이터가\n다 찼습니다",			// 43
	"삭제하였습니다",					// 44
	"SD메모리 카드가\n없습니다",		// 45
	"SD메모리 준비가\n안되었습니다",	// 46
	"\n디렉토리 열기 오류",				// 47
	"\n디렉토리 읽기 오류",				// 48
	"\n디렉토리 쓰기 오류",				// 49
	"\n파일 열기 오류",					// 50
	"\n파일 읽기 오류",					// 51
	"\n파일 쓰기 오류",					// 52
	"\n파일 동기화 오류",				// 53
	"\n디렉토리가 없습니다",			// 54
	"\n파일이 없습니다",				// 55
	"잠시 후 시스템이\n재시작됩니다",	// 56
	"SD메모리를 안전하게\n제거할 수 있습니다", // 57
	"SD메모리 오류\n점검하십시오",		// 58
	"서버 접속 오류\n잠시 후 다시 하세요", // 59
	"이미 처리중입니다\n먼저 취소하세요"
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
	"모든 출입이\n거부되고 있습니다"	// 8
};

char kor_ta_msgs[MAX_TA_MSGS_COUNT][MAX_TAMSG_LEN] = {
	"출 근",		// 1
	"퇴 근",		// 2
	"외 출",		// 3
	"복 귀",		// 4
	"출 석",		// 5
	"조 퇴",		// 6
	"등 교",		// 7
	"하 교"			// 8
};
/*
char kor_ta_msgs[8][MAX_TAMSG_LEN] = {
	"출 석",		// 1
	"조 퇴",		// 2
	"등 교",		// 3
	"하 교",		// 4
	"출 입",		// 5
	"부 재",		// 6
	"휴 식",		// 7
	"복 귀"			// 8
};

char kor_menu_unit[17][MAX_MSG_LEN] = {
	"초",								// 14
	"분",
	"0.5초",
	"무제한",		// Unlimited(Never timeout)
	"늘",			// Always
	"미지정",		// Unspecified(null)
	"전체",			// All
	"늘",			// Always
	"항상 긍정",	// Always(all the time)
	"항상 부정",	// Never
	"항상 ON"
	"항상 OFF",
};
*/
char kor_menu_top_title[MAX_MSG_LEN] = "시스템 관리";

char kor_menu_top[5][MAX_MSG_LEN] = {
	"사용자", 
	"출입 제어",
	"네트워크",
	"환경 설정",
	"시스템"
};

char kor_menu_lang[2][MAX_MSG_LEN] = {
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

char kor_menu_user[6][MAX_MSG_LEN] = {
	"사용자 등록",
	"사용자 연속 등록",
	"사용자 변경",
	"사용자 삭제",
	"사용자 전체 삭제",
	"사용자 사진 등록"
};

char kor_menu_user_enroll[7][MAX_MSG_LEN] = {
	"저장 종료",	
	"액세스 모드",
	"지문1",
	"지문2",
	"카드",
	"암호",
	"인증 등급"
};

char kor_menu_access_mode[7][MAX_MSG_LEN] = {
	"지문",
	"카드",
	"암호",
	"카드+지문",
	"카드+암호",
	"카드+지문+암호",
	"암호+지문"
};

char kor_menu_security_level[10][MAX_MSG_LEN] = {
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

char kor_menu_ta_sub[1][MAX_MSG_LEN] = {
	"시간대",
};

char kor_menu_time_attend[4][MAX_MSG_LEN] = {
	"키",
	"화면표시",
	"항목",
	"타이머"	
};

char kor_menu_ta_key[2][MAX_MSG_LEN] = {
	"화살표 키",
//	"오른쪽 키"
	"F1-...-F4"
};

char kor_menu_ta_display[3][MAX_MSG_LEN] = {
	"표시안함",
	"문자",
	"아이콘"
};

char kor_menu_ta_item[3][MAX_MSG_LEN] = {
	"활성화",
	"문자 선택",
	"아이콘 선택"
};

char kor_menu_meal[9][MAX_MSG_LEN] = {
	"잠 김",
	"조 식", //	"출 입", ALARM_OUTPUT
	"중 식",
	"석 식",
	"야 식",
	"간 식",
	"중복 금지",
	"식권 인쇄",
	"그릇 수 입력"
};

char kor_menu_meal_sub[2][MAX_MSG_LEN] = {
	"시간대",
	"메뉴 개수"
};

char kor_menu_meal_funckey[4][MAX_MSG_LEN] = {
	"메뉴 1",
	"메뉴 2",
	"메뉴 3",
	"메뉴 4"
};

char kor_menu_event[3][MAX_MSG_LEN] = {
	"이벤트 삭제",
	"출력 유형",
	"이벤트 방지"
};

char kor_menu_event_output[3][MAX_MSG_LEN] = {
	"네트워크",
	"26 Wiegand",
	"34 Wiegand"
};

char kor_menu_event_mask[3][MAX_MSG_LEN] = {
	"록 이벤트",
	"출입 이벤트",
	"기타 이벤트"
};

char kor_menu_event_wiegand[2][MAX_MSG_LEN] = {
	"사용자ID",
	"카드 데이터"
};

char kor_menu_access_control[9][MAX_MSG_LEN] = {
	"하드웨어 구성",
	"전체 리더",
	"출입문",	
	"출입점(리더)",
	"입력",
	"출력(릴레이)",
	"출입방",
	"경보",
	"확장 입출력"
};

char kor_menu_hw_setup[6][MAX_MSG_LEN] = {
	"기기 구성",
	"기기 보기",
	"기기 입출력 보기",
	"리더 IP주소 보기",
	"입출력 기기 구성",
	"입출력 기기 보기"
};

char kor_menu_op_mode[3][MAX_MSG_LEN] = {
	"모드 선택",
	"출입 모드",
	"식수 모드"
};

char kor_menu_all_readers[6][MAX_MSG_LEN] = {
	"리더 설정",
	"리더 할당",
	"탬퍼(분리 경보)",
	"근태 설정",
	"식수 설정",
	"운용 모드"
};

char kor_menu_hw_config[19][MAX_MSG_LEN] = {
	"기기",
	"리더기",
	"Wiegand",
	"RS-485",
	"자체",
	"설정",
	"출입문",
	"유형",
	"IP",
	"채 널 1",
	"채 널 2",
	"출입점",
	"출입방",
	"화재구역",
	"경보작동",
	"입출력기기",
	"사용여부",
	"화재점",
	"IP"
};

char kor_menu_unit_type[3][MAX_MSG_LEN] = {
	"K-200",
	"K-220",
	"K-300(단독형)"
};

char kor_menu_iounit_type[2][MAX_MSG_LEN] = {
	"K-500(입력)",
	"K-700(출력)"
};

char kor_menu_access_door[7][MAX_MSG_LEN] = {
	"문열림 시간",
	"입력/출력",
	"경보 지연 시간",
	"경보 후 지연시간",
	"경보",
	"문담힘 일정",
	"문열림 일정"
};

char kor_menu_door_config[3][MAX_MSG_LEN] = {
	"출입문 해제",
	"정상(출구 미사용)",
	"출구"
};

char kor_menu_door_inout[5][MAX_MSG_LEN] = {
	"록(Lock)",
	"퇴실 버튼",
	"문상태 감지",
	"보조 록",
	"록 유형"
};

char kor_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM 록",
	"데드볼트"
};

char kor_menu_door_alarm[2][MAX_MSG_LEN] = {
	"장시간 문 열림",
	"문 강제 열림"
};

char kor_menu_door_alarm_info[2][MAX_MSG_LEN] = {
	"활성화",
	"문열림 경보 시간"
};

char kor_menu_access_point[1][MAX_MSG_LEN] = {
	"인증 모드"
};

char kor_menu_authorization_mode[4][MAX_MSG_LEN] = {
	"정상 인증",
	"권한 생략 인증",
	"모두 거부",
	"카드 모두 승인"
};

char kor_menu_access_zone[6][MAX_MSG_LEN] = {
	"추가",
	"출입점 정의",
	"패스백 설정",
	"출구방",
	"입구방",
	"외부"
};

char kor_menu_passback[6][MAX_MSG_LEN] = {
	"팻스백 삭제",
	"패스백 모드",
	"출구에서 팩스백",
	"패스백 타임아웃",
	"하드 패스백 일정",
	"소프트 패스백 일정"
};

char kor_menu_passback_mode[3][MAX_MSG_LEN] = {
	"사용 안함",
	"하드(문잠김 유지)",
	"소프트(경보만 발생)"
};

char kor_menu_passback_setting[4][MAX_MSG_LEN] = {
	"패스백 사용",
	"반복 출입 지연 시간",
	"패스백 리셋 시간",
	"패스백 전체 리셋"
};

char kor_menu_binary_input[4][MAX_MSG_LEN] = {
	"극성",
	"경보 지연 시간",
	"경보 후 지연 시간",
	"경보 금지"
};

char kor_menu_binary_output[1][MAX_MSG_LEN] = {
	"극성"
};

char kor_menu_io_polarity[2][MAX_MSG_LEN] = {
	"정상(Normal Open)",
	"반전(Normal Close)"
};

char kor_menu_armed[4][MAX_MSG_LEN] = {
	"방범 설정",
	"방범 모드",
	"경계",
	"침입"
};

char kor_menu_alarm[4][MAX_MSG_LEN] = {
	"화재구역",
	"화재점",
	"경보 작동",
	"출력 명령"
};

char kor_menu_fire_zone[3][MAX_MSG_LEN] = {
	"화재 입력",
	"화재시 여는 문",
	"경보 작동"
};

char kor_menu_action_index[12][MAX_MSG_LEN] = {
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

char kor_menu_output_command[3][MAX_MSG_LEN] = {
	"반복 횟수",
	"ON 시간",
	"OFF 시간"
};

char kor_menu_readers[3][MAX_MSG_LEN] = {
	"지문 리더",
	"PIN 리더",
	"카드 리더"
};

char kor_menu_card_readers[5][MAX_MSG_LEN] = {
	"Mifare 리더",
	"EM 리더",
	"Wiegand 리더",
	"시리얼 리더",
	"IP 리더"
};

char kor_menu_wiegand_reader[2][MAX_MSG_LEN] = {
	"Wiegand 카드 리더",
	"Wiegand PIN 리더"
};

char kor_menu_fp_reader[4][MAX_MSG_LEN] = {
	"지문 노출량",
	"인증 등급",
	"1:N 인증",
	"빠른 캡춰"	
};

char kor_menu_mifare_reader[4][MAX_MSG_LEN] = {
	"카드 포멧",
	"특수 카드 설정",
	"Mifare Type(A/B)",
	"카드 보기"
};

char kor_menu_mifare_format[3][MAX_MSG_LEN] = {
	"일련번호",
	"특수 카드",
	"티머니(T-money)"
};

char kor_menu_mifare_type[3][MAX_MSG_LEN] = {
	"Type A",
	"Type B",
	"Type A + Type B"
};

char kor_menu_special_card[6][MAX_MSG_LEN] = {
	"섹터 번호",
	"블록 번호",
	"데이터 위치",
	"데이터 길이",
	"데이터 유형",
	"데이터 순서"
};

char kor_menu_data_type[3][MAX_MSG_LEN] = {
	"숫자",
	"문자",
	"이진 데이터"
};

char kor_menu_byte_order[2][MAX_MSG_LEN] = {
	"정상 순서",
	"역 순서"
};

char kor_menu_em_format[3][MAX_MSG_LEN] = {
	"32-bit",
	"24-bit",
	"20-bit"
};

char kor_menu_external_format[3][MAX_MSG_LEN] = {
	"900MHz 리더",
	"바코드 리더",
	"EM 리더"
};

char kor_menu_ip_reader[3][MAX_MSG_LEN] = {
	"리더기 네트워크 설정",
	"주소",
	"포트"
};

char kor_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"정상",
	"카드 번호만 사용",
	"패러티 추가"
};

char kor_menu_duress[2][MAX_MSG_LEN] = {
	"협박모드 숫자",
	"협박모드 경보"
};

char kor_menu_setup[8][MAX_MSG_LEN] = {
	"관리자",
	"시각 설정",
	"화면",
	"음성",
	"잠금 설정",
	"언어(Language)",
	"카메라",
	"도어폰"
};

char kor_menu_admin[2][MAX_MSG_LEN] = {
	"관리자 암호",
	"관리자 지문"
};

char kor_menu_wiegand_type[2][MAX_MSG_LEN] = {
	"26 Wiegand",
	"34 Wiegand"
};

char kor_menu_datetime[2][MAX_MSG_LEN] = {
	"날짜 설정",
	"시간 설정"
};

char kor_menu_monitor[6][MAX_MSG_LEN] = {
	"조명 시간",
	"콘트라스트",
	"밝기",
	"흑백 모드",
	"시간 표기",
	"날짜 표기"
};

char kor_menu_time_notation[2][MAX_MSG_LEN] = {
	"12 시간",
	"24 시간"
};

char kor_menu_date_notation[3][MAX_MSG_LEN] = {
	"표시 안함",
	"MM/DD",
	"MM월DD일"
};

char kor_menu_sound[3][MAX_MSG_LEN] = {
	"인증 음량",
	"관리 기능 음량",
	"버튼 음량"
};

char kor_menu_lockout[2][MAX_MSG_LEN] = {
	"키 잠금",
	"탬퍼(분리 경보)"	
};

char kor_menu_cam[7][MAX_MSG_LEN] = {
	"카메라 동작 모드",
	"카메라 켜기/끄기",
	"카메라 채널",
	"카메라 방향",
	"카메라 보기",
	"이벤트 영상 저장",
	"카메라 채널 할당" 
};

char kor_menu_cam_mode[3][MAX_MSG_LEN] = {
	"사용안함",
	"자동",
	"수동"
};

char kor_menu_cam_run[2][MAX_MSG_LEN] = {
	"끄기(Off)",
	"켜기(On)"
};

char kor_menu_cam_channel[4][MAX_MSG_LEN] = {
	"0 채널",
	"1 채널",
	"2 채널",
	"3 채널"
};

char kor_menu_cam_direction[2][MAX_MSG_LEN] = {
	"세로",
	"가로"
};

char kor_menu_doorphone[8][MAX_MSG_LEN] = {
	"운용 모드",
	"전화 번호",
	"문열기 번호",
	"음량 조절",
	"침묵 시간",
	"톤 정의",
	"톤 점검",
	"하드웨어 설정"
};

char kor_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"사용 안함",
	"인터폰",
	"일반 전화",
	"IP 전화"
};

char kor_menu_phone_number[3][MAX_MSG_LEN] = {
	"다이얼링 모드",
	"단축 번호",
	"IP 전화번호"
};

char kor_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"표준",
	"단축 번호",
	"단일 번호"
};

char kor_menu_ip_phone_number[5][MAX_MSG_LEN] = {
	"번호 등록",
	"번호 변경",
	"번호 삭제",
	"번호 전체 삭제",
	"번호 보기"
};

char kor_menu_ip_phone_item[1][MAX_MSG_LEN] = {
	"IP 주소"
};

char kor_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"스피커",
	"회선 감도",
	"마이크 감도"
};

char kor_menu_doorphone_tone[3][MAX_MSG_LEN] = {
	"링백톤",
	"통화중",
	"전화끊김음"
};

char kor_menu_tone_info[2][MAX_MSG_LEN] = {
	"High 시간",
	"Low 시간"
};

char kor_menu_doorphone_hw_setup[2][MAX_MSG_LEN] = {
	"스위칭 속도",
	"배경 소음(BGN)"
};

char kor_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char kor_menu_network[6][MAX_MSG_LEN] = {
	"장치 ID",
	"MAC 주소",
	"네트워크 모드",
	"IP 주소 설정",
	"서버 설정",
	"Ping 시험"
};

char kor_menu_network_serial[2][MAX_MSG_LEN] = {
	"시리얼 속도",
	"시리얼 포트"
};

char kor_menu_serial_port[3][MAX_MSG_LEN] = {
	"포트 0(RS-485)",
	"포트 1(RS-232)",
	"포트 2"
};

char kor_menu_network_tcpip[7][MAX_MSG_LEN] = {
	"자동 주소 받기",
	"장치 IP 주소",
	"서브넷 매스크",
	"게이트웨이",
	"서버 주소",
	"서버 포트",
	"IP 주소"
};

char kor_menu_network_mode[3][MAX_MSG_LEN] = {
	"시리얼",
	"다이얼업",
	"TCP/IP"
};

char kor_menu_system[7][MAX_MSG_LEN] = {
	"상세 정보 보기",
	"초기화",
	"가져오기(from SD메모리)",
	"내보내기(to SD메모리)",
	"SD메모리 안전하게 제거",
	"통신 통계",
	"장치 재초기화(리셋)"
};

char kor_menu_init_device[3][MAX_MSG_LEN] = {
	"이벤트 초기화",
	"설정 초기화",
	"모두 초기화"
};

char kor_menu_imexport[6][MAX_MSG_LEN] = {
	"펌웨어",
	"자원 파일(아이콘,음성)",
	"고객정보 설정값",
	"사용자 데이터",
	"단말기 설정 정보",
	"K-300(단독형) 펌웨어"
};

char kor_menu_comm_stats[2][MAX_MSG_LEN] = {
	"통계 보기",
	"통계치 초기화"
};

char kor_menu_downfirmware[5][MAX_MSG_LEN] = {
	"펌웨어 가져오기",
	"다운로드 시작",
	"미완료만 재시도",
	"다운로드 취소",
	"다운로드 상태 보기"
};

char kor_menu_inuse[2][MAX_MSG_LEN] = {
	"미사용",
	"사용"
};

char kor_menu_yesno[2][MAX_MSG_LEN] = {
	"예",
	"아니오"
};

char kor_menu_noyes[2][MAX_MSG_LEN] = {
	"아니오",
	"예"
};

char kor_menu_okcancel[2][MAX_MSG_LEN] = {
	"확인",
	"취소"
};

char kor_menu_enable[MAX_MSG_LEN] = "예";

char kor_menu_other[24][MAX_MSG_LEN] = {
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
	"이벤트 갯수"
};

char _kor_day_of_weeks[7][4] = {
	"일", "월", "화", "수", "목", "금", "토"
};
