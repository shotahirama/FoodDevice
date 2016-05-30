/*

    Copyright (C)  2016  Shota Hirama

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

MODULE_AUTHOR("Shota Hirama");
MODULE_DESCRIPTION("print food");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;

static volatile unsigned int food_c = 0;

static ssize_t food_write(struct file * filp, const char* buf, size_t count, loff_t* pos){
	char c;
	if(copy_from_user(&c, buf, sizeof(char))){
		return -EFAULT;
	}
	if(c >= '0' && c <= '9'){
		food_c = (int)c - 48;
	}

	return 1;
}

static ssize_t food_read(struct file* filp, char* buf, size_t count, loff_t* pos){
	
	int ret = 0;

	char sushi[] = {0xF0,0x9F,0x8D,0xA3,'\0'};
	char pudding[] = {0xF0,0x9F,0x8D,0xAE,'\0'};
	char riceball[] = {0xF0,0x9F,0x8D,0x99,'\0'};
	char hamburger[] = {0xF0,0x9F,0x8D,0x94,'\0'};
	char curryrice[] = {0xF0,0x9F,0x8D,0x9B,'\0'};
	char bread[] = {0xF0,0x9F,0x8D,0x9E,'\0'};
	char wineglass[] = {0xF0,0x9F,0x8D,0xB7,'\0'};
	char sunflower[] = {0xF0,0x9F,0x8C,0xBB,'\0'};
	char mapleleaf[] = {0xF0,0x9F,0x8D,0x81,'\0'};
	char aubergine[] = {0xF0,0x9F,0x8D,0x86,'\0'};
	char nl[] = {'\n','\0'};

	char *emoji[] = { sushi, pudding, riceball, hamburger, curryrice, bread, wineglass, sunflower, mapleleaf, aubergine};
	size_t sizeoflist[] = {sizeof(sushi), sizeof(pudding), sizeof(riceball), sizeof(hamburger), sizeof(curryrice), sizeof(bread), sizeof(wineglass), sizeof(sunflower), sizeof(mapleleaf), sizeof(aubergine)};

	int size = 0;
	ret = copy_to_user(buf+size, (const char *)emoji[food_c], sizeoflist[food_c]);

	if(ret ==  -EFAULT){
		return -EFAULT;
	}

	size += sizeoflist[food_c];

	if(copy_to_user(buf+size, (const char *)nl, sizeof(nl))){
		printk(KERN_INFO "sushi ; copy_to_user failed\n");
		return -EFAULT;
	}
	size += sizeof(nl);
	return size;
}

static struct file_operations food_fops = {
	.owner = THIS_MODULE,
	.write = food_write,
	.read = food_read
};

static int __init init_mod(void){
	int ret;
	ret = alloc_chrdev_region(&dev, 0, 1, "food");
	if(ret < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return ret;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));

	cdev_init(&cdv, &food_fops);
	ret = cdev_add(&cdv, dev, 1);
	if(ret < 0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d", MAJOR(dev), MINOR(dev));
		return ret;
	}

	cls = class_create(THIS_MODULE, "food");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}

	device_create(cls, NULL, dev, NULL, "food%d", MINOR(dev));
	return 0;
}

static void __exit cleanup_mod(void){
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
