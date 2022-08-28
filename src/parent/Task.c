/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCMCU.com --------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ����STC�����ϼ�����            */
/*---------------------------------------------------------------------*/

#include	"Task.h"

//========================================================================
//                               ���ر�������
//========================================================================

// static TASK_COMPONENTS Task_Comps[]=
// {
// //״̬  ����  ����  ����
// 	{0, 8000, 8000, App_Run},				/* task 1 Period�� 10ms */
// 	// {0, 1500, 1500, TestRun},				/* task 1 Period�� 3000ms */
// 	{0, 3000, 3000, Child_Run},				/* task 1 Period�� 3000ms */
// };

// u8 Tasks_Max = sizeof(Task_Comps)/sizeof(Task_Comps[0]);

u8 Tasks_Max = 0;
TASK_COMPONENTS* Task_Comps;

void Task_Init(TASK_COMPONENTS* Tasks, u8 Tasks_Size){
	Task_Comps = Tasks;
	Tasks_Max = Tasks_Size;
}


//========================================================================
// ����: Task_Handler_Callback
// ����: �����ǻص�����.
// ����: None.
// ����: None.
// �汾: V1.0, 2012-10-22
//========================================================================
void Task_Marks_Handler_Callback(void)
{
	u8 i;
	for(i=0; i<Tasks_Max; i++)
	{
		if(Task_Comps[i].TIMCount)    /* If the time is not 0 */
		{
			Task_Comps[i].TIMCount--;  /* Time counter decrement */
			if(Task_Comps[i].TIMCount == 0)  /* If time arrives */
			{
				/*Resume the timer value and try again */
				Task_Comps[i].TIMCount = Task_Comps[i].TRITime;  
				Task_Comps[i].Run = 1;    /* The task can be run */
			}
		}
	}
}

//========================================================================
// ����: Task_Pro_Handler_Callback
// ����: ������ص�����.
// ����: None.
// ����: None.
// �汾: V1.0, 2012-10-22
//========================================================================
void Task_Pro_Handler_Callback(void)
{
	u8 i;
	for(i=0; i<Tasks_Max; i++)
	{
		if(Task_Comps[i].Run) /* If task can be run */
		{
			Task_Comps[i].Run = 0;    /* Flag clear 0 */
			Task_Comps[i].TaskHook();  /* Run task */
		}
	}
}


