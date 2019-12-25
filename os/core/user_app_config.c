//#include "user_app_config.h"
//freertos kernel head file
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
//freertos app head file
#include "user_app_common.h"
int freertos_app_add();

const sys_base_event_t sys_init_ok = SYS_INIT_OK;//ö��
const sys_base_event_t flash_init_ok = FLASH_INIT_OK;//ö��
const sys_base_event_t fs_mount_ok = FS_MOUNT_OK;//ö��

//��ʼ���������ں˲�������������app
int software_init(){
	//�����¼���
	sys_base_event_group = xEventGroupCreate();
	//���¼���λ
	xEventGroupSetBits(sys_base_event_group,sys_init_ok);
	//������Դ��������ʼ��
	serial_sem = xSemaphoreCreateBinary();
	//post
	xSemaphoreGive(serial_sem);
	freertos_app_add();
}
//����app,��������						
int freertos_app_add(){
	xTaskCreate(	show_sys_clock,
							"show_sys_clock_task",	
							128,
							NULL,
							3,
							&RCC_SYS_CLOCK );
	xTaskCreate(	open_led,
							"open_led_task",	
							128,
							NULL,
							3,
							&OPEN_LED_Task_TCB );
	xTaskCreate(	close_led,
							"close_led_task",	
							128,
							NULL,
							3,
							&CLOSE_LED_Task_TCB );
	xTaskCreate(	usart_heart_beat,
							"usart_heart_beat_task",	
							128,
							NULL,
							3,
							&USART_HEART_BEAT );					
	xTaskCreate(	pwr_mode_switch,
							"pwr_mode_switch_task",	
							128,
							NULL,
							3,
							&PWR_MODE_SWITCH );
	xTaskCreate(	pwr_mode_send_msg,
							"pwr_mode_send_msg_task",	
							128,
							NULL,
							3,
							&PWR_MODE_SEND_MSG );
	xTaskCreate(	crc_cal,
							"crc_cal_task",	
							128,
							NULL,
							3,
							&CRC_CAL );
  xTaskCreate(	crc_post_binary_sem,
							"crc_post_binary_sem_task",	
							128,
							NULL,
							3,
							&CRC_POST_BINARY_SEM );
	xTaskCreate(	flash_test,
							"flash_size_task",	
							128,
							NULL,
							3,
							&FLASH_SIZE_TASK_PCB );		
	xTaskCreate(fs_test,
							"fs_test_task",	
							256,
							NULL,
							3,
							&FS_TEST_TASK_PCB );
	return 0;
}