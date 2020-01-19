#include "core/task.h"
#include "core/list.h"
#include "core/mem.h"
#include "arch/arm-cortex-m3/arch.h"
tasklist wait_task,block_task,extinct_task;
TASKHANDLE cur_task;
TASKHANDLE idle_task;
#define slot	1 //时间片
#define TIMESLICE 1
//append 不会创建listnode
static err_t task_append(tasklist dest,TASKHANDLE t){
	 spnode  p=NULL;
	 if(dest==NULL||t==NULL)return err_failed;
	 p=contain_of(slist,ctx,t);
	 p->ctx=t;
	 return list_add(dest,p);
}
static err_t task_add(tasklist dest,TASKHANDLE t){
	 spnode  p=(spnode)ONEMALLOC(sizeof(slist));
	 if(p==NULL||dest==NULL||t==NULL)return err_failed;
	 p->ctx=t;
	 return list_add(dest,p);
}
//remove but not delete
static err_t task_remove(tasklist src,TASKHANDLE t){
	spnode  p=NULL;
	err_t  err;
	if(t==NULL||src==NULL)return err_failed;
	p=contain_of(slist,ctx,t);
	if(p==NULL)return err_failed;
	err=list_remove(src,p);
	return err;
}
static err_t task_change(tasklist src,tasklist dest,TASKHANDLE t){
	spnode  p=NULL;
	err_t  err;
	if(t==NULL||src==NULL||dest==NULL)return err_failed;
	if(src==dest)return err_ok;
	p=contain_of(slist,ctx,t);
	if(p==NULL)return err_failed;
	err=list_remove(src,p);
	if(err!=err_ok)return err_failed;
	err=list_add(dest,p);
	if(err!=err_ok)return err_failed;
	return err_ok;
}
static err_t task_find(tasklist src,TASKHANDLE t){
	spnode  p=NULL;
	if(t==NULL||src==NULL)return err_failed;
	p=contain_of(slist,ctx,t);
	if(p==NULL)return err_failed;
	if(p==list_find(src,p))
		return err_ok;
	else return err_failed; 
}
//返回指定tasklist中优先级最高的task,出错返回NULL
static TASKHANDLE task_max_priorty(tasklist src){
   spnode  pmax, p;
   uchar  max_priorty;
   pmax=p=src;
   if(p==NULL)return NULL;
   else{
     pmax=p=p->back;
	 max_priorty=((TASKHANDLE)(p->ctx))->priorty;
   }
   while(p!=src){
     if(max_priorty<((TASKHANDLE)(p->ctx))->priorty){
	   pmax=p;
	   max_priorty=((TASKHANDLE)(p->ctx))->priorty;
	 }
	 p=p->back;
   }
   if(p==src)return pmax->ctx;
   else return NULL;
}

//调度策略的实现，调度机制和体系结构相关，机制推迟在合适的时间再完成
//策略根据算法，在wait_task查找合适的task,设置为cur_task 任何时候都有一个idle task在运行
err_t schedule(){
   TASKHANDLE  task;
   err_t  err=err_ok;
#if(TIMESLICE==1)
   critical_area_enter();
   err=task_append(wait_task,cur_task);
   if(err!=err_ok)return err; //出错了
   cur_task->status=WAIT;
   task=wait_task->pre->ctx;	//add是头插，这里应该尾取
   err = task_remove(wait_task,task);
   if(err!=err_ok)return err; //出错了
   task->status=RUN;
   cur_task=task;
   critical_area_exit();
   return err;
#else
   critical_area_enter();
   err=task_append(wait_task,cur_task);
   if(err!=err_ok)return err; //出错了
   cur_task->status=WAIT;
   task=task_max_priorty(wait_task);
   if(task==NULL)return err_failed;//出错了
   err = task_remove(wait_task,task);
   if(err!=err_ok)return err; //出错了
   task->status=RUN;
   cur_task=task;
   critical_area_exit();
   return err;
#endif		
}
TASKHANDLE task_create(uchar task_id,uchar priorty,uchar stack_size,void(*task_fun)(void)){
	TASKHANDLE  t = (TASKHANDLE)ONEMALLOC(sizeof(TASK));
	if(t==NULL)return NULL;
	t->task_id=(task_id<0?0:(task_id>255?255:task_id));		//range 0~255
	t->priorty=(priorty<0?0:(priorty>255?255:priorty));	  //range 0~255
	t->stack_size = (stack_size<=16?16:(stack_size>255?16:stack_size)); //range 16~255 out off set to 16
	if(task_fun==NULL){t->task_fun=NULL;return NULL;}
	t->task_fun=task_fun;
	t->stack_addr=(void *)ONEMALLOC(stack_size);	//申请task栈ram，栈是向下生长的
	if(t->stack_addr==NULL)return NULL;
	t->stack_p=(void *)((uint)t->stack_addr+stack_size-1);//获取栈顶地址
	t->stack_p = (void *)(((uint)t->stack_p)&(~(uint)(0x0007)));//向下8字节对齐
	t->stack_p = arch_initTask_stack(t->stack_p,task_fun);//初始化并更新栈指针
	t->status=WAIT;
	if(task_add(wait_task,t)!=err_ok)return NULL;
	return t;
}

err_t task_suspend(TASKHANDLE task){
   if(task==NULL)return err_failed;
   if(task==cur_task){
     if(task_change(wait_task,block_task,task)!=err_ok)return err_failed; 
		 arch_task_yield();
   }
   if(task_find(wait_task,task)==err_ok)
     return task_change(wait_task,block_task,task);
   else return err_failed;
}
err_t task_resume(TASKHANDLE task){
   if(task==NULL)return err_failed;
   if(task==cur_task)
     return err_ok;
   if(task_find(wait_task,task)==err_ok)
     return err_ok;
   if(task_find(block_task,task)==err_ok)
   	 return task_change(block_task,wait_task,task);
   else return err_failed;
}

err_t task_delete(TASKHANDLE task){
   if(task==NULL)return err_failed;
   if(task==cur_task){
     if(task_change(wait_task,extinct_task,task)!=err_ok)return err_failed;
		 arch_task_yield();
   }
   if(task_find(wait_task,task)==err_ok)
     return task_change(wait_task,extinct_task,task);
   if(task_find(block_task,task)==err_ok)
   	 return task_change(block_task,extinct_task,task);
   else return err_failed;
}
err_t task_clean(){
	err_t  err=err_ok;
	tasklist p=extinct_task;
    if(extinct_task==NULL)
	  return err_failed;
	else p=p->back;
	while(p!=extinct_task){
	  err=list_delete(extinct_task,&p);
	  if(err!=err_ok)return err;
	}
	return err;
}


void idle_fun(){
  for(;;){
  	task_clean();
		task_suspend(idle_task);
  }
}
void task_err_fun(){
	
}
err_t oneos_init(){
	wait_task = list_create();
	block_task = list_create();
	extinct_task = list_create();
	if(wait_task==NULL||block_task==NULL||extinct_task==NULL)return err_failed;
	idle_task=task_create(0,0,254,idle_fun);
	cur_task=idle_task;
	return err_ok;	  
}
void oneos_start(){
   arch_oneos_start();
}