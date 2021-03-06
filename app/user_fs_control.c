/* Standard includes. */
#include <stdio.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
/* Library includes. */
#include "stm32f10x_it.h"
//bsp includes
#include "user_fs_control.h"
#include "ff.h"
#include "user_app_common.h"
extern const sys_base_event_t fs_mount_ok;
extern const sys_base_event_t flash_init_ok;
extern const sys_base_event_t fs_file_operate_ok;
static EventBits_t r_event;//事件返回值
FATFS *fs,fatfs;//fs操作句柄
const int work_buff_len = 512;
char work[work_buff_len];
int clust_size = 0;
xTaskHandle FS_TEST_TASK_PCB;//打印系统FLASH线程句柄
/***attention!!!!**/
/*****
freertos的任务堆栈很小，所以尽量不要在任务堆栈里面定义较大变量，以免任务堆栈溢出！！！
*********/
static FIL fp;
char buffer[20];
int count=0;

//fatfs文件系统使用:1)mkfs	\
2) mount	\
3)文件操作
//我们想让fs运行前，先完成裸板的flash测试\
为此，我们等到了flash测试完成事件，才会执行fs_init的相关工作�
void fs_init(){
		//等待事件
		r_event = xEventGroupWaitBits(sys_base_event_group,//事件组句柄
											flash_init_ok,//等待的事件
											pdFALSE,//true 等到后清除事件位
											pdTRUE,//true逻辑与等待
											portMAX_DELAY);//等待时间	
		if(r_event & flash_init_ok == flash_init_ok){//flash测试完成了
			//挂载文件系统，没有则创建
//			fs = &fatfs;
//			r_event = f_mount(fs, "FLASH",1);
//			if(FR_NO_FILESYSTEM == r_event){
//				xprintf_s("not exist filesystem,create it on flash...\n");
//				r_event = f_mkfs("FLASH", 0, work, work_buff_len);
//				configASSERT(!r_event);
//				xprintf_s("   %d   ",r_event);
//				r_event = f_mount(fs, "FLASH",1);
//				configASSERT(!r_event);
//			}
//			//其他故障，assert
//			configASSERT(!r_event);
//			r_event = f_getfree("FLASH",&clust_size,&fs);
//			configASSERT(!r_event);
//			xprintf_s("fs mount success,size:%d clust\n",clust_size);
			
			//挂载文件系统，没有则创建
			fs = &fatfs;
			r_event = f_mount(fs, "OUT_FLASH",1);
			if(FR_NO_FILESYSTEM == r_event){
				xprintf_s("not exist filesystem,create it on flash...\n");
				r_event = f_mkfs("OUT_FLASH", 0, work, work_buff_len);
				configASSERT(!r_event);
				xprintf_s("   %d   ",r_event);
				r_event = f_mount(fs, "OUT_FLASH",1);
				configASSERT(!r_event);
			}
			//其他故障，assert
			configASSERT(!r_event);
			r_event = f_getfree("OUT_FLASH",&clust_size,&fs);
			configASSERT(!r_event);
			xprintf_s("fs mount success,size:%d clust\n",clust_size);
		}
		//fs初始化完成，事件置位
		xEventGroupSetBits(sys_base_event_group,fs_mount_ok);
}

void fs_test(){
  	fs_init();
	//等待事件
		r_event = xEventGroupWaitBits(sys_base_event_group,//事件组句柄
											fs_mount_ok,//等待的事件
											pdFALSE,//true 等到后清除事件位
											pdTRUE,//true逻辑与等待
											portMAX_DELAY);//等待时间	
		if(r_event & fs_mount_ok == fs_mount_ok){//fs已经挂载
//			r_event = f_open(&fp, "FLASH:hello.txt", FA_CREATE_NEW | FA_WRITE);
//			configASSERT(!r_event);
//			f_write(&fp, "Hello, World!\n你好世界", 30, &count);
//			configASSERT(count == 30);
//			f_close(&fp);
//			r_event = f_open(&fp, "FLASH:hello.txt", FA_READ);
//			configASSERT(!r_event);
//			f_read(&fp,buffer,30,&count);
//			f_close(&fp);
//			xprintf_s("read text:%s\n",buffer);
			//fs基本读写完成，事件置位
			xEventGroupSetBits(sys_base_event_group,fs_file_operate_ok);
			//任务完成，删除任务  删除自身，参数填NULL
			vTaskDelete(NULL);
		}
}