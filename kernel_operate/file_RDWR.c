//内核文件读写
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
static char *buf = "hello";
static char buf1[10];
static int __init
hello_init(void)
{
	struct file* fp;
	mm_segment_t fs;
	loff_t pos;
	printk("hello kernel\n");
	fp = filp_open("/home/kernel_file",O_CREAT|O_RDWR,0);
	if(IS_ERR(fp)){
		printk("create the file fail");
		return -1;
	}
	fs = get_fs();
	set_fs(KERNEL_DS);//内核模式
	pos = 0;
	vfs_write(fp,buf,sizeof(buf),&pos);
	pos = 0;
	vfs_read(fp,buf1,sizeof(buf),&pos);
	printk("read: %s\n",buf1);
	filp_close(fp,NULL);
	set_fs(fs);
    return 0;
}
module_init(hello_init);

static void __exit
 hello_exit(void)
{
	printk("Goodbye, world!\n");
}
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valerie Henson <val@nmt.edu>");
MODULE_DESCRIPTION("\"Hello, world!\" minimal module");
MODULE_VERSION("printk");
