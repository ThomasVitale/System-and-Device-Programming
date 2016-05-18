// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "paging.h"

int main(struct multiboot *mboot_ptr) {

    int i=0;
    u32int *ptr = (u32int*)0x00000000;

    // Initialise all the ISRs and segmentation
    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();

    initialise_paging();
    monitor_write("Hello, paging world!\n");
		
    while(1) {
        monitor_write("Normal access at address ");
        monitor_write_hex((u32int)*ptr);
        monitor_write(" at page ");
        monitor_write_dec(i);
        monitor_write("\n");
        ptr+=0x1000;
        i += 1;
    }

    return 0;
}