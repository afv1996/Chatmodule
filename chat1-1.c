#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "Chat_procfs"

/*Doubly Linked List*/
struct user_data
{
	char *pid;
	struct list_head ulist;
};
struct list_head *data_ulist;
struct user_data* Node_create(void)
{
	struct user_data *udata;
	udata = kzalloc(sizeof(struct user_data), GFP_KERNEL);
	udata->pid = kzalloc(5*sizeof(char), GFP_KERNEL);
	if(udata == NULL)
	{
		printk(KERN_INFO "can't alloc user_data");
		return NULL;
	}
	return udata;
}
void Node_add(struct list_head *head)
{
	struct user_data *tmpNode;
	tmpNode = Node_create();
	list_add(&tmpNode->ulist, head);
}
void Node_show(struct list_head *head)
{
	struct user_data *udata_tmp;
	struct list_head *tmp_head = head;
	list_for_each_entry(udata_tmp, tmp_head, ulist)
	{
		printk(KERN_INFO "Pid %s\n", udata_tmp->pid);
		printk(KERN_INFO "Ulist %p, tmp_head %p\n",&udata_tmp->ulist,tmp_head);
	}
}

/*PROCFS_FPOS*/
static ssize_t procfs_read(struct file *file, char *buffer, size_t length, loff_t *offp)
{
	/*................................*/
	printk(KERN_INFO "Procfs_read is not finish yet");
	Node_show(data_ulist);
}

static ssize_t procfs_write(struct file *file, const char __user *buffer, size_t length, loff_t *f_pos)
{
	/*................................*/
	char *msg;
	msg = kzalloc(1024*sizeof(char),GFP_KERNEL);
	if(copy_from_user(msg,buffer,length))
	{
		return EFAULT;
	}
	printk(KERN_INFO "MSG: %s\n", msg);
	//Node_add(data_ulist,msg);
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
	Node_add(data_ulist);
	Node_show(data_ulist);	
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
	remove_proc_entry(PROCFS_NAME,NULL);
	printk(KERN_INFO "Goodbye ! \n");
}

module_init(chat_init);
module_exit(chat_exit);
MODULE_AUTHOR("Nhat Anh");
MODULE_LICENSE("GPL");

