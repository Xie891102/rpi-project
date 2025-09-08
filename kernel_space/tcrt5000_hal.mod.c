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
	{ 0xab6a9a6c, "gpio_to_desc" },
	{ 0x3a9a7864, "gpiod_direction_input" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x92997ed8, "_printk" },
	{ 0xfe990052, "gpio_free" },
	{ 0x8a3dbe73, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1145D445C3B09EFC131C208");
