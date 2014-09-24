#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
char buf[100] = {'\0'};
char buf1[10] = {'\0'};
char total_buf[10] = {'\0'};
char free_buf[10] = {'\0'};
static struct task_struct* MyThread = NULL;
//将数字字符串转换成数字
static int atoi(char a[])
{
	int i,n = 0;
	for(i = 0; a[i] != '\0'; i++){
		a[i] = a[i] - '0';
		n = n*10 + a[i];
	}
	return n;
}
static int print(void){
	/*search loadavg info*/
	struct file* fp;
	struct task_struct *tmp_p = NULL;
	mm_segment_t fs;
	loff_t pos;
	int i = 0,j = 0;
	int line = 1;
	int buf_use;
	char ch;
	printk("hello kernel\n");
	fp = filp_open("/proc/loadavg",O_RDWR,0);
	if(IS_ERR(fp)){
		printk("open fail");
		return -1;
	}
	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(fp,buf1,4,&pos);
	printk("read loadavg is: %s\n",buf1);
	filp_close(fp,NULL);
	set_fs(fs);
	/*search meminfo info*/
	fp = filp_open("/proc/meminfo",O_RDWR,0);
	if(IS_ERR(fp)){
		printk("open file error!\n");
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;

	vfs_read(fp,&ch,sizeof(char),&pos);
	while(ch != '\n'){
		vfs_read(fp,&ch,sizeof(char),&pos);
		if(ch >='0' && ch <= '9')
			total_buf[i++] = ch;	
	}
	
	i = 0;
	vfs_read(fp,&ch,sizeof(char),&pos);
	while(ch != '\n'){
		vfs_read(fp,&ch,sizeof(char),&pos);
		if(ch >='0' && ch <= '9')
			free_buf[i++] = ch;	
	}

	printk("mem_all:%s kB\nmem_free:%s kB\n",total_buf,free_buf);

	buf_use = atoi(total_buf) - atoi(free_buf);
	printk("mem_use:%d kB\n",buf_use);

	filp_close(fp,NULL);
	set_fs(fs);
	/*current pid's name pid*/
	for_each_process(tmp_p){
		if(tmp_p->state == 0)
		printk("p name: %s,pid is: %d\n",tmp_p->comm,tmp_p->pid);
	}
}
static int MyPrintk(void *date){
	while(!kthread_should_stop()){
		//printk("hello kernel thread\n");
		print();		
		ssleep(3);
	}
	return 0;
}
static int __init
init_kthread(void)
{
	MyThread = kthread_run(MyPrintk,NULL,"mythread");
	 return 0;
}
module_init(init_kthread);

static void __exit
exit_kthread(void)
{
	if(MyThread){
		printk("stop MyThread\n");
		kthread_stop(MyThread);
	}
}
module_exit(exit_kthread);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valerie Henson <val@nmt.edu>");
MODULE_DESCRIPTION("\"Hello, world!\" minimal module");
MODULE_VERSION("printk");
