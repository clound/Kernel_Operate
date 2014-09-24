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
static int atoi(char a[],int n){
	int ret = a[n-1]-'0';
	int i = n-2;
	int carry = 1;
	for(;i >= 0;i--){
		carry *= 10;
		ret += (a[i]-'0')*carry;
	}
	return ret;
}
static int print(void){
	/*search loadavg info*/
	struct file* fp;
	struct task_struct *tmp_p = NULL;
	mm_segment_t fs;
	loff_t pos;
	int i = 0,j = 0;
	int line = 1;
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
		printk("open fail");
		return -1;
	}
	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(fp,buf,sizeof(buf),&pos);
	while(i < 100){
		if(line == 3)
		  break;
		if(buf[i] == '\n'){
			line++;
			j = 0;
		}
		if(buf[i] >= '0' && buf[i] <= '9'){
			if(line == 1){
				total_buf[j] = buf[i];
			}
			if(line == 2){
				free_buf[j] = buf[i];
			}
			j++;
		}
		i++;
	}
	printk("total_buf: %s,free_buf: %s\n",total_buf,free_buf);
	printk("available memory is %d\n",atoi(total_buf,strlen(total_buf))-atoi(free_buf,strlen(free_buf)));
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
