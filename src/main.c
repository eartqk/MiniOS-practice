// main.c -- Определяет точку входа Cи-кода ядра
#include "descriptor_tables.h"
#include "monitor.h"
#include "multiboot.h"
#include "paging.h"
#include "timer.h"

void proc1(u32int* n) {
    monitor_write("Unit 1 accessing variable:\n");
    monitor_write_hex(*n);
    monitor_write("\n");
}

void proc2(u32int* n) {
    monitor_write("Unit 2 accessing variable:\n");
    monitor_write_hex(*n);
    monitor_write("\n");
}

int main(struct multiboot* mboot_ptr) {
    init_descriptor_tables();

    monitor_clear();

    init_interuptions();

    asm volatile("int $0xf");
    asm volatile("sti");
	monitor_write("----------------\n");
    //Testing global variable allocation
    u32int* ptr = (u32int*)0xF0;
    ptr = 0xf;
    monitor_write("Initializing global variable with value of: ");
    monitor_write_hex(ptr);
    monitor_write(", at adress:");
    monitor_write_hex(&ptr);
    monitor_write("\n");
    proc1(&ptr);
    proc2(&ptr);

    monitor_write("----------------\n");

    //Test of choosing free pages
    monitor_write("Test of choosing free pages. \n Trying to allocate 2 variables:\n");
    u32int* ptr2 = smalloc();
    monitor_write("First allocated at: ");
    monitor_write_hex(&ptr2);
    monitor_write("\n");

    u32int* ptr3 = smalloc();
    monitor_write("Second allocated at: ");
    monitor_write_hex(&ptr3);
    monitor_write("\n");

    monitor_write("Done!");
    // init_timer(10);

    //u32int *ptr = (u32int*)0xA0000000;

    //u32int do_page_fault = *ptr;

    return 0;
}