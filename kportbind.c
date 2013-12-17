#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/socket.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_bind)(int, struct sockaddr __user *, int);

asmlinkage long new_sys_bind(int sockfd, struct sockaddr __user *myaddr, int addrlen) {
	unsigned short int port;
	unsigned short int uid;
	struct sockaddr_in *s;
	
	uid = current_uid();

	if ((myaddr->sa_family == AF_INET) && (uid > 1000)) {
		s = (struct sockaddr_in *)myaddr;
		port = ntohs(s->sin_port);

		if (!((port >= uid*10) && (port <= uid*10+9))) {
			printk(KERN_INFO "kportbind: DENIED: uid: %u port: %u\n", uid, port);
			return -EACCES;
		}

		//printk(KERN_INFO "OK: uid: %u port: %u\n", uid, port);
	}
	return ref_sys_bind(sockfd, myaddr, addrlen);
}

static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if (sct[__NR_close] == (unsigned long *) sys_close) 
			return sct;

		offset += sizeof(void *);
	}

	return NULL;
}

static void disable_page_protection(void) 
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if(!(value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value & ~0x00010000));
}

static void enable_page_protection(void) 
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if((value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value | 0x00010000));
}

static int __init interceptor_start(void) 
{
	if(!(sys_call_table = aquire_sys_call_table()))
		return -1;

	disable_page_protection();
	ref_sys_bind = (void *)sys_call_table[__NR_bind];
	sys_call_table[__NR_bind] = (unsigned long *)new_sys_bind;
	enable_page_protection();
	printk(KERN_INFO "kbindport: module enabled\n");

	return 0;
}

static void __exit interceptor_end(void) 
{
	if(!sys_call_table)
		return;

	disable_page_protection();
	sys_call_table[__NR_bind] = (unsigned long *)ref_sys_bind;
	enable_page_protection();
	printk(KERN_INFO "kbindport: module disabled\n");
}

module_init(interceptor_start);
module_exit(interceptor_end);
MODULE_LICENSE("GPL");
