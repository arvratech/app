#ifndef _UI_DLG_H_
#define _UI_DLG_H_


// Text Input definitions
#define TI_ALPHANUMERIC			 0
#define TI_ALPHANUMERIC_PASSWORD 1
#define TI_NUMERIC				 2
#define TI_NUMERIC_PASSWORD		 3
#define TI_HEXANUMERIC			 4
#define TI_IP_ADDRESS			 5


void *AlertDialog(int requestCode, char *intent, BOOL yesNo);
void *AlertClearTopDialog(int requestCode, char *intent, BOOL btnYesNo);
void *WaitDialog(int requestCode, char *intent);
void *WaitClearTopDialog(int requestCode, char *intent);
void *FieldAlertDialog(int requestCode, char *intent);
void *ProgressDialog(int requestCode, char *title, char *message);
void *TextInputDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *));
void *UuidInputDialog(int requestCode, char *intent);
void *PasswordDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *));
void *PasswordClearTopDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *));
//void PasswordDialogOnExternalKey(GACTIVITY *act, int code);
void *DigitInputDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *));
void *DigitInputClearTopDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *));
//void DigitInputDialogOnExternalKey(GACTIVITY *act, int code);
void *SpinnerDialog(int requestCode, char *intent);
void *CheckDialog(int requestCode, char *intent);
void *SliderDialog(int requestCode, char *intent);
void *PickerDialog(int requestCode, char *intent);
void *DatePickerDialog(int requestCode, int year, int month, int day);
void *TimePickerDialog(int requestCode, int hour, int minute, BOOL hour24View);
void *TimeRangeDialog(int requestCode, char *intent);


#endif
