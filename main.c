/*
 * main.c
 */

#include <stdio.h>
#include "src/system/driver/uart/UartDriver.h"
#include "src/system/driver/button/ButtonDriver.h"
#include "src/system/driver/gpio/GPIODriver.h"
#include "src/system/hal/common/interrupt_sw.h"
#include "src/system/hal/omap3530/interrupt/interrupt.h"
#include "src/system/hal/omap3530/timer/timer.h"
#include "src/system/hal/omap3530/clock/clock.h"
#include "src/system/scheduler/scheduler.h"
#include "src/system/hal/omap3530/mmu/mmu.h"
#include "src/system/hal/omap3530/tps65950/tps65950.h"
#include "src/applications/audio/audio.h"
#include "src/system/hal/omap3530/mcbsp/mcbsp.h"
#include "src/system/hal/omap3530/i2c/i2c.h"

interrupt_callback timer_irq;

void test(void);
void test2(void);
void uart_process(void);

#pragma TASK(main)
void main(void) {
	// Set up interrupts
	interrupt_init();

	// initialise onboard button
	button_driver_init();

	// initialise LED
	gpio_driver_init();

	// Enable sound
	mcbsp2_enable();
	mcbsp_init_master2(MCBSP2);
	//i2c1_enable();
	//i2c_init(I2C1);
	tps_led_init();
	tps_init();

	// initialise MMU
	mmu_init();

	// Add IRQ handler
	interrupt_add_listener(40, &timer_irq);

	gpt_timer_init(GPT_TIMER4, 500);
	gpt_timer_start(GPT_TIMER4);

	//scheduler_addProcess(test);
	//scheduler_addProcess(test2);
	//scheduler_addProcess(uart_process);
	//uart_driver_init(9600);

	// Enable interrupts globally
	//interrupt_enable();

	// call software interrupt
	//syscall(SYS_DEBUG, 0);

	test();

	// Execute
	while(1) {
		printf("..idle\n");
	}
}

void test(void) {
	int a = 1;
	a++;

	//printf("%i\n", a);
	syscall(SYS_DEBUG, 0);

	a++;
	//printf("%i\n", a);

	while(1) {
		printf("[1] Test sound\n");
		play_sample();
		printf("[1] Sound test finished\n");
		int x = 0;
		x++;
	}
}

void test2(void) {
	while(1) {
		printf("[2] task test\n");
		int y = 1;
		y--;
	}
}

void uart_process(void) {
	while (1) {
		int count = uart_driver_count();
		if ( count > 0 ) {
			char buffer[8];
			uart_driver_read(buffer, 8);

			int i;
			for ( i = 0; i < 8 && i < count; i++ ) {
				printf("%c", buffer[i]);
			}
			uart_driver_write(buffer, count < 8 ? count : 8);
		} else {
			//printf("No Data to process\n");
		}
	}
}

void timer_irq(Registers_t* context) {
	// This method will never return
	scheduler_run(context);

	gpt_timer_reset(GPT_TIMER4);
	gpt_timer_start(GPT_TIMER4);
}
