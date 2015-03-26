/*
 * interrupt.h
 *
 *  Created on: 25.03.2015
 *      Author: Philip
 */

#ifndef SRC_SYSTEM_HAL_COMMON_INTERRUPT_H_
#define SRC_SYSTEM_HAL_COMMON_INTERRUPT_H_


typedef void (*interrupt_listener)(void);

/**
 * Initializes the IRQ environment.
 */
void interrupt_init();

/**
 * Enables the IRQ environment
 */
void interrupt_enable();

/**
 * Disables the IRQ environment
 */
void interrupt_disable();

/**
 * Registers a new listener to an IRQ
 *
 * @param irq The IQR to listen on
 * @param listener The listener function, which will be called when an interrupt occurs
 */
void interrupt_add_listener(uint32_t irq, interrupt_listener listener);

#endif /* SRC_SYSTEM_HAL_COMMON_INTERRUPT_H_ */
