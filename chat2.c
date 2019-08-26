#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/delay.h>
#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "Chat_procfs"

/*Send Mess*/
struct file *file_open(const char *path)
{
	struct file *fp=NULL;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());
	fp = filp_open(path, O_RDWR, 0777);
	printk(KERN_INFO "fp= %p\n", fp);
	if(IS_ERR(fp))
	{
		printk(KERN_INFO "Open file to send message error\n");
		return -1;
	}
	return fp;

}
int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(get_ds());
	ret = vfs_write(file,data,size, &offset);
	set_fs(oldfs);
	return ret;
}

void send_mess(long l_pid)
{
	char buf[]="Hello !!!!\n";
	char *file_link;
	file_link = kzalloc(100, GFP_KERNEL);
	sprintf(file_link,"/proc/%u/fd/2", l_pid);
	printk(KERN_INFO" test: %s\n", file_link);
	struct file *filp = file_open(file_link);
	file_write(filp,0,buf,sizeof(buf));
	kfree(file_link);
}

/*Doubly Linked List*/
struct user_data
{
	int pid;
	struct list_head ulist;
};
struct list_head *data_ulist;
struct user_data* Node_create(int pid)
{
	struct user_data *udata;
	udata = kzalloc(sizeof(struct user_data), GFP_KERNEL);
	if(udata == NULL)
	{
		printk(KERN_INFO "can't alloc user_data");
		return NULL;
	}
	udata->pid = pid;
	return udata;
}
void Node_add(struct list_head *head, int pid)
{
	struct user_data *tmpNode;
	tmpNode = Node_create(pid);
	list_add(&tmpNode->ulist, head);
}
void Node_show(struct list_head *head)
{
	struct user_data *udata_tmp;
	struct list_head *tmp_head = head;
	list_for_each_entry(udata_tmp, tmp_head, ulist)
	{
		send_mess(udata_tmp->pid);
	}
}

void Node_free(struct list_head *head)
{
	struct user_data *tmp, *obj_tmp;
	struct list_head *new_pos;

	tmp = list_entry(head->next, typeof(*tmp), ulist);
	do
	{
		new_pos = tmp->ulist.next;
		obj_tmp = list_entry(new_pos, typeof(*obj_tmp), ulist);
		list_del(&tmp->ulist);
		kfree(tmp);
		tmp = obj_tmp;
	}
	while(&tmp->ulist != head);
}

/*PROCFS_FPOS*/
static ssize_t procfs_read(struct file *file, char *buffer, size_t length, loff_t *offp)
{
	/*................................*/
	printk(KERN_INFO "Procfs_read is not finish yet");
}

static ssize_t procfs_write(struct file *file, const char __user *buffer, size_t length, loff_t *f_pos)
{
	/*................................*/
	char *msg;
	msg = kzalloc(10*sizeof(char),GFP_KERNEL);
	if(copy_from_user(msg,buffer,length))
	{
		return EFAULT;
	}
	int ret;
	long pid;
	ret = kstrtol_from_user(buffer, length, 10, &pid);
	printk(KERN_INFO "Pid = %u\n", pid);
	Node_add(data_ulist,pid);
	//Node_show(data_ulist);
	send_mess(pid);
	kfree(msg);
	return length;
}
struct file_operations proc_fops = 
{	
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_write
};
int chat_init(void)
{
	/*Create doubly linked list*/
	data_ulist = kzalloc(sizeof(struct list_head), GFP_KERNEL);
	if(data_ulist == NULL)
	{
		printk(KERN_INFO "Failed allocate data_ulist");
		return -ENOMEM;
	}
	INIT_LIST_HEAD(data_ulist);		
	/*create the procfs*/
	struct proc_dir_entry *chat_proc_file;
	chat_proc_file = proc_create(PROCFS_NAME, 0777, NULL, &proc_fops);
	if(!chat_proc_file)
	{
		printk(KERN_INFO "Failed create procfs");
		goto failed_create_procfs;
	}
	printk(KERN_INFO "Create proc file successfully\n");
	return 0;
	/*Failed actions*/
	failed_create_procfs:
		return -1;
}

void chat_exit(void)
{
	//procfs_remove(PROCFS_NAME,NULL);
	Node_free(data_ulist);
	remove_proc_entry(PROCFS_NAME,NULL);
	printk(KERN_INFO "Goodbye ! \n");
}

module_init(chat_init);
module_exit(chat_exit);
MODULE_AUTHOR("Nhat Anh");
MODULE_LICENSE("GPL");

