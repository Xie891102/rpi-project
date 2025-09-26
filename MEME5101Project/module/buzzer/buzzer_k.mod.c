#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x92997ed8, "_printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x5f754e5a, "memset" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x97255bdf, "strlen" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x6b522821, "cdev_init" },
	{ 0x9c161ff0, "cdev_add" },
	{ 0x2c9676c6, "class_create" },
	{ 0x9b880ba9, "device_create" },
	{ 0x1d37eeed, "ioremap" },
	{ 0x84369d1, "device_destroy" },
	{ 0x8616ca1d, "class_destroy" },
	{ 0xedc03953, "iounmap" },
	{ 0x8a86c590, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xdcffdfcd, "param_ops_int" },
	{ 0x8a3dbe73, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DE45E0B768FFAA75F8C718A");
