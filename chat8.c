#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#define MAGICALNUMBER 243
#define ONLINE 1;
#define OFFLINE 2;
#define BUSY 3;
#define CHAT_CONFIG_STT _IOW(MAGICALNUMBER, 1,char *)
#define CHAT_CONFIG_USERNAME _IOW(MAGICALNUMBER, 2,char *)
#define CHAT_CONFIG_PID _IOW(MAGICALNUMBER, 3,char *)
#define PROCFS_NAME "Chat_Mess"
#define PROCFS_ADDPID_NAME "Chat_Add_Pid"

/*Send Mess*/
void send_mess(long pid, char *mess)
{
	char *file_link;
	file_link = kzalloc(100, GFP_KERNEL);
	sprintf(file_link, "/proc/%lu/fd/2",pid);
	printk(KERN_INFO" test: %s\n", file_link);
	struct file *fp=NULL;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(get_ds());
	loff_t pos;
	fp = filp_open(file_link, O_RDWR, 0777);
	printk(KERN_INFO "fp= %p\n", fp);
	if(IS_ERR(fp))
	{
		printk(KERN_INFO "Open file to send message error\n");
		return -1;
	}
	pos =0;
	vfs_write(fp,mess,strlen(mess), &pos);
	filp_close(fp,NULL);
	set_fs(oldfs);
	kfree(file_link);
}

/*Linked List*/
struct user_data
{
	int stt;	
	char user_name[50];
	long pid;
	struct list_head ulist;
};
struct list_head *data_ulist;
struct user_data* Node_create(long pid,const char *user_name)
{
	struct user_data *udata;
	udata = kzalloc(sizeof(struct user_data), GFP_KERNEL);
	if(udata == NULL)
	{
		printk(KERN_INFO "can't alloc user_data");
		return NULL;
	}
	udata->stt = ONLINE;
	strncpy(udata->user_name,user_name,strlen(user_name));
	udata->pid = pid;
	return udata;
}
void Node_add(struct list_head *head, long pid, char *user_name)
{
	struct user_data *tmpNode;
	tmpNode = Node_create(pid,user_name);
	list_add(&tmpNode->ulist, head);
}
void Node_show(struct list_head *head, char *mess)
{
	struct user_data *udata_tmp;
	struct list_head *tmp_head = head;
	list_for_each_entry(udata_tmp, tmp_head, ulist)
	{
		send_mess(udata_tmp->pid,mess);
	}
}
//struct user_data *Node_search()
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
static ssize_t procfs_read(struct file *file, char __user *buffer, size_t length, loff_t *offp)
{
	/*................................*/
	printk(KERN_INFO "Procfs_read is running");
	char *all_user;
	all_user = kzalloc(2000 * sizeof(char), GFP_KERNEL);
	struct user_data *udata_tmp;
	struct list_head *read_head = data_ulist;
	list_for_each_entry(udata_tmp, read_head, ulist)
	{
		char status[200];
		sprintf(status, "User: %s___PID: %lu___ STT: %d\n",udata_tmp->user_name, udata_tmp->pid,udata_tmp->stt);
		all_user=strcat(all_user,status);
	}
	//printk(KERN_INFO "User:\n %s", all_user);
	ssize_t cnt=strlen(all_user), ret;
	ret = copy_to_user(buffer, all_user, cnt);
	*offp += cnt - ret;
	if(*offp > cnt)
		return 0;
	else 
		return cnt;
}

static ssize_t procfs_addpid_write(struct file *file, const char __user *buffer, size_t length, loff_t *f_pos)
{
	/*................................*/
	char *msg;
	char *user_name;
	char *c_pid;
	char *msg_tmp;
	msg = kzalloc(100*sizeof(char),GFP_KERNEL);
	msg_tmp = msg;
	if(copy_from_user(msg,buffer,length))
	{
		return EFAULT;
	}
	const char *ct = "/";
	user_name = strsep(&msg,ct);
	c_pid = strsep(&msg,ct);
	long pid;
	int ret = kstrtol(c_pid,10, &pid);
	//printk(KERN_INFO "PID = %lu\n", pid);
	//printk(KERN_INFO "USERNAME = %s\n", user_name);
	Node_add(data_ulist,pid,user_name);
	msg = NULL;
	kfree(msg_tmp);
	return length;
}
static ssize_t procfs_mess_write(struct file *file, const char __user *buffer, size_t length, loff_t *f_pos)
{
	/*................................*/
	char *msg;
	char *char_pid_des;
	char *msg_tmp;
	char *mess;
	char *user_des;
	msg = kzalloc(200*sizeof(char),GFP_KERNEL);
	msg_tmp = msg;
	if(copy_from_user(msg,buffer,length))
	{
		return EFAULT;
	}
	const char *ct = "/";
	user_des = strsep(&msg,ct);
	char_pid_des = strsep(&msg,ct);
	mess = strsep(&msg,ct);
	if(strcmp(char_pid_des,"all")==0)
		Node_show(data_ulist, mess);
	else
	{
		long pid_des;
		int ret = kstrtol(char_pid_des,10,&pid_des);
		char mess2[200];
		sprintf(mess2,"Mess: > %s\n", user_des, mess);
		send_mess(pid_des,mess2);
	}
	kfree(msg_tmp);
	return length;
}
/*IOCTL FUNCTIONS*/
char *string_input;
char *string1, *string2;
char *change_name;
static void string_sep(char *input_ioctl)
{
	const char *ct = "/";
	string1 = strsep(&input_ioctl,ct);
	string2 = strsep(&input_ioctl,ct);
}
struct user_data* chat_cdev_search(struct list_head *head,char *name)
{
	struct user_data *udata_tmp;
	struct list_head *tmp_head = head;
	list_for_each_entry(udata_tmp, tmp_head, ulist)
	{
		if(strcmp(udata_tmp->user_name,name) == 0)
			break;
	}
	return udata_tmp;

}
static long chat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	 char string_input[100];
	 struct user_data *pointer_tmp;
         switch(cmd) 
	 {
	         case CHAT_CONFIG_PID:
                        copy_from_user(string_input ,(char *) arg,sizeof(string_input));
			string_sep(string_input);
			long pid_new;
			int ret1 = kstrtol(string2,10, &pid_new);
			pointer_tmp = chat_cdev_search(data_ulist,string1);
			pointer_tmp->pid = pid_new;
              		break;
		 case CHAT_CONFIG_USERNAME:
                        copy_from_user(string_input,(char *) arg, sizeof(string_input));
			string_sep(string_input);
			pointer_tmp = chat_cdev_search(data_ulist,string1);
			memset(pointer_tmp->user_name,0,sizeof(pointer_tmp->user_name));
			memcpy(pointer_tmp->user_name,string2,strlen(string2));
                        break;
		 case CHAT_CONFIG_STT:
                        copy_from_user(string_input ,(char *) arg, sizeof(string_input));
			string_sep(string_input);
			long stt_new;
			int ret2 = kstrtol(string2, 10, &stt_new);
			pointer_tmp = chat_cdev_search(data_ulist, string1);
			pointer_tmp->stt = stt_new;
                        break;

	 }
	string1 = NULL;
	string2 = NULL;
        return 0;
}

struct file_operations chat_mess_fops = 
{	
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_mess_write
};
struct file_operations chat_addpid_fops = 
{	
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_addpid_write,
//	.unlocked_ioctl = chat_ioctl
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
	struct list_head *read_head = data_ulist;
	INIT_LIST_HEAD(data_ulist);		
	/*create the procfs chat_mess*/
	struct proc_dir_entry *chat_proc_file;
	chat_proc_file = proc_create(PROCFS_NAME, 0777, NULL, &chat_mess_fops);
	if(!chat_proc_file)
	{
		printk(KERN_INFO "Failed create procfs");
		goto failed_create_procfs;
	}
	printk(KERN_INFO "Create proc file Chat_module successfully\n");
	/*Create the procfs chat_addpid*/
	struct proc_dir_entry *chat_addpid_proc_file;
	chat_addpid_proc_file = proc_create(PROCFS_ADDPID_NAME, 0777, NULL, &chat_addpid_fops);
	if(!chat_addpid_proc_file)
	{
		printk(KERN_INFO "Failed create procfs");
		goto failed_create_procfs;
	}
	printk(KERN_INFO "Create proc file Chat_add_pid successfully\n");
	return 0;
	/*Failed actions*/
	failed_create_procfs:
		return -1;
}

void chat_exit(void)
{
	Node_free(data_ulist);
	remove_proc_entry(PROCFS_NAME,NULL);
	remove_proc_entry(PROCFS_ADDPID_NAME,NULL);
	printk(KERN_INFO "Goodbye ! \n");
}

module_init(chat_init);
module_exit(chat_exit);
MODULE_AUTHOR("Nhat Anh");
MODULE_LICENSE("GPL");
