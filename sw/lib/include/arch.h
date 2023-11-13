#pragma once
#include <utils.h>

/**
 * @brief Get the hartid
 * @return int hartid
 */
static inline int get_hartid(){
    unsigned id;
	asm volatile("csrr %0, mhartid" : "=r" (id));
	return id;
}

/**
 * @brief enable global interrupts
 */
static inline void arch_en_int(){
    unsigned bits = (1 << 3);
	asm volatile("csrs mstatus, %0" : : "r" (bits));
}

/**
 * @brief Disable global interrupts
 */
static inline void arch_di_int(){
	unsigned bits = (1 << 3);
	asm volatile("csrc mstatus, %0" : : "r" (bits));
}

/**
 * @brief Enable machine software interrupts
 */
static inline void arch_en_msi()
{
	unsigned bits = (1 << 3);
	asm volatile("csrs mie, %0" : : "r" (bits));
}

/**
 * @brief Disable machine software interrupts
 */
static inline void arch_di_msi()
{
	unsigned bits = (1 << 3);
	asm volatile("csrc mie, %0" : : "r" (bits));
}

/**
 * @brief Enable machine timer interrupts
 */
static inline void arch_en_mti()
{
	unsigned bits = (1 << 7);
	asm volatile("csrs mie, %0" : : "r" (bits));
}

/**
 * @brief Disable machine timer interrupts
 */
static inline void arch_di_mti()
{
	unsigned bits = (1 << 7);
	asm volatile("csrc mie, %0" : : "r" (bits));
}

/**
 * @brief Enable machine external interrupts
 */
static inline void arch_en_mei()
{
	unsigned bits = (1 << 11);
	asm volatile("csrs mie, %0" : : "r" (bits));
}

/**
 * @brief Disable machine external interrupts
 */
static inline void arch_di_mei()
{
	unsigned bits = (1 << 11);
	asm volatile("csrc mie, %0" : : "r" (bits));
}

_WEAK void arch_unhandled_irq();
_WEAK void arch_unhandled_exception();

/**
 * @brief Register exception handler
 * @param id exception id
 * @param handler handler function
 */
void register_exception_handler(unsigned id, void (*handler)(void));

/**
 * @brief Deregister exception handler
 * @param id exception id
 */
void deregister_exception_handler(unsigned id);

/**
 * @brief Register interrupt handler
 * @param id Interrupt id
 * @param handler handler function
 */
void register_interrupt_handler(unsigned id, void (*handler)(void));

/**
 * @brief Deregister interrupt handler
 * @param id interrupt id
 */
void deregister_interrupt_handler(unsigned id);

