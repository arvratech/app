#ifndef _FUNC_H_
#define _FUNC_H_


int  funcResetState(void);
int  funcSetCode(int FuncState, int FuncKey);
int  GetFuncState(void);
int  IsMealsMode(void);
int  MealsIsActive(void);
int  AuthIsEnable(void);
void FuncKeyInitialize(void);
void FuncKeyReset(void);
int  MealsFuncKeyIsActive(int FuncKey);
void InitOperationMode(void);
void InitMealsMode(void);
int  GetMealsNumber(void);
void ClearMealsNumber(void);
int  IsArmMode(void);
int  IsUseLocalIO(void);
int  IsBlackWhiteMode(void);


#endif
