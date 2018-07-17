/*
    FreeRTOS V8.2.1 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/
 #include "loader/lunchr.h"
 #include "contiki.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#include "app_util.h"
#include "app_util_platform.h"
#endif
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/

extern TaskHandle_t g_contiki_thread;

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void ) __attribute__ (( naked ));

/*
 * Exception handlers.
 */
void vPortSVCHandler( void );
void xPortPendSVHandler( void ) __attribute__ (( naked ));


static hardfault_dump_t dump;

void hard_fault_handler_c(unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;

  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);

  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);


#if DEBUG_MODULE > 1
  PRINTF ("\n\n[Hard fault handler - all numbers in hex]\n");
  PRINTF ("R0 = %x\n", stacked_r0);
  PRINTF ("R1 = %x\n", stacked_r1);
  PRINTF ("R2 = %x\n", stacked_r2);
  PRINTF ("R3 = %x\n", stacked_r3);
  PRINTF ("R12 = %x\n", stacked_r12);
  PRINTF ("LR [R14] = %x  subroutine call return address\n", stacked_lr);
  PRINTF ("PC [R15] = %x  program counter\n", stacked_pc);
  PRINTF ("PSR = %x\n", stacked_psr);
  PRINTF ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
  PRINTF ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
  PRINTF ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
  PRINTF ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
  PRINTF ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
  PRINTF ("SCB_SHCSR = %x\n", SCB->SHCSR);
#endif

  dump.type = HADRFAULT_NONE_DEFINE;
  dump.r0 = stacked_r0;
  dump.r1 = stacked_r1;
  dump.r2 = stacked_r2;
  dump.r3 = stacked_r3;
  dump.r12 = stacked_r12;
  dump.lr = stacked_lr;
  dump.pc = stacked_pc;
  dump.psr = stacked_psr;

  BaseType_t yield_req = pdFALSE;

  if ((SCB->HFSR & (1 << 30)) != 0) {
    // UsageError
    if((SCB->CFSR & 0xFFFF0000) != 0) {
      PRINTF("[freertos port] Usage fault: ");
      uint32_t CFSRValue = SCB->CFSR >> 16;
      if((CFSRValue & (1 << 9)) != 0) {
        PRINTF("Divide by zero\n");
        dump.type = HADRFAULT_DIV_0;
      }
      if((CFSRValue & (1 << 8)) != 0) {
        PRINTF("Unaligned access\n");
        dump.type = HADRFAULT_UNALIGNED_ACCESS;
      }
      if((CFSRValue & (1 << 3)) != 0) {
        PRINTF("No coprocessor UsageFault\n");
        dump.type = HADRFAULT_NO_COPROCESSOR;
      }
      if((CFSRValue & (1 << 2)) != 0) {
        PRINTF("Invalid PC load UsageFault\n");
        dump.type = HADRFAULT_INVALID_PC_LOAD;
      }
      if((CFSRValue & (1 << 1)) != 0) {
        PRINTF("Invalid state\n");
        dump.type = HADRFAULT_INVALID_STATE;
      }
      if((CFSRValue & (1 << 0)) != 0) {
        PRINTF("Undefined instruction\n");
        dump.type = HADRFAULT_UNDEFINED_INSTRUCTION;
      }
    }
    // BusError
    if((SCB->CFSR & 0xFF00) != 0) {
      /*
      If is in theory possible to design handler to skip one instruction,
      but the handler will need to check if the bus fault is precise or imprecise first,
      and also use software to decode if the faulting instruction is 16-bit or 32-bit
      so that the return program address can be calculated.

      The handler will need to extract the faulting program counter address from the stack frame
      (see HardFault handler section).

      However, if the fault is caused by a stack corruption you might not be able to do that.
      Then finally I would question if skipping a faulting instruction is right. If a bus fault is reported, skipping the faulting instruction doesn't mean the remaining part of the program can continue correctly. As you say, all read operations on Cortex-M3/M4 are precise so you can use bus fault to handle parity / ECC error.
      */
      PRINTF("[freertos port] Bus fault\n");
      dump.type = HADRFAULT_BUS;
    }
    // MemoryManagementError
    if((SCB->CFSR & 0xFF) != 0) {
      PRINTF("[freertos port] MemoryManagement fault\n");
      dump.type = HADRFAULT_MEMORT_MANAGEMENT;
    }
  }

#if SYS_HARDFAULT_DIRECT_REBOOT == 1
  watchdog_reboot();
#else
  process_post(&hardfault_process, PROCESS_EVENT_CONTINUE, &dump);
  vTaskNotifyGiveFromISR(g_contiki_thread, &yield_req);
  portYIELD_FROM_ISR(yield_req);
  taskYIELD();
#endif
}

void xPortHardFaultHandler( void )
{
  __asm volatile
  (
  "   TST LR, #4                         \n"
  "   ITE EQ                             \n"
  "   MRSEQ R0, MSP                      \n"
  "   MRSNE R0, PSP                      \n"
  "   B hard_fault_handler_c             \n"
  );
}

/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
    __asm volatile(
    "   .syntax unified           \n"
    "                             \n"
    "   ldr r0, =__Vectors     \n" /* Locate the stack using __isr_vector table. */
    "   ldr r0, [r0]              \n"
    "   msr msp, r0               \n" /* Set the msp back to the start of the stack. */
    "                             \n"
    "   ldr r3, =pxCurrentTCB \n" /* Obtain location of pxCurrentTCB. */
    "   ldr r1, [r3]              \n"
    "   ldr r0, [r1]              \n" /* The first item in pxCurrentTCB is the task top of stack. */
    "   adds r0, #32              \n" /* Discard everything up to r0 */
    "   msr psp, r0               \n" /* This is now the new top of stack to use in the task. */
    "   movs r0, #2               \n" /* Switch to the psp stack. */
    "   msr CONTROL, r0           \n"
    "   pop {r0-r5}               \n" /* Pop the registers that are saved automatically. */
    "   mov lr, r5                \n" /* lr is now in r5. */
    "   cpsie i                   \n" /* The first task has its context and interrupts can be enabled. */
    "   pop {pc}                  \n" /* Finally, pop the PC to jump to the user defined task code. */
    "                             \n"
    "   .align 2                  \n"
    );
}

/*-----------------------------------------------------------*/

/*void vPortSVCHandler( void )
{
    // This function is no longer used, but retained for backward
    // compatibility.
}*/
void vPortSVCHandler( void )
{
  __asm volatile (
      "   ldr r3, =pxCurrentTCB           \n" /* Restore the context. */
      "   ldr r1, [r3]                    \n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
      "   ldr r0, [r1]                    \n" /* The first item in pxCurrentTCB is the task top of stack. */
      "   ldmia r0!, {r4-r11, r14}        \n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
      "   msr psp, r0                     \n" /* Restore the task stack pointer. */
      "   isb                             \n"
      "   mov r0, #0                      \n"
      "   msr basepri, r0                 \n"
      "   bx r14                          \n"
      "                                   \n"
      "   .align 2                        \n"
  );
}
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
    /* This is a naked function. */

    __asm volatile
    (
    "   .syntax unified                     \n"
    "                                       \n"
    "   mrs r0, psp                         \n"
    "                                       \n"
    "   ldr r3, =pxCurrentTCB               \n" /* Get the location of the current TCB. */
    "   ldr r2, [r3]                        \n"
    "                                       \n"
    "   subs r0, #32                        \n" /* Make space for the remaining low registers. */
    "   str r0, [r2]                        \n" /* Save the new top of stack. */
    "   stmia r0!, {r4-r7}                  \n" /* Store the low registers that are not saved automatically. */
    "   mov r4, r8                          \n" /* Store the high registers. */
    "   mov r5, r9                          \n"
    "   mov r6, r10                         \n"
    "   mov r7, r11                         \n"
    "   stmia r0!, {r4-r7}                  \n"
    "                                       \n"
    "   push {r3, r14}                      \n"
    "   bl vPortSafeTaskSwitchContext       \n"
    "   pop {r2, r3}                        \n" /* lr goes in r3. r2 now holds tcb pointer. */
    "                                       \n"
    "   ldr r1, [r2]                        \n"
    "   ldr r0, [r1]                        \n" /* The first item in pxCurrentTCB is the task top of stack. */
    "   adds r0, #16                        \n" /* Move to the high registers. */
    "   ldmia r0!, {r4-r7}                  \n" /* Pop the high registers. */
    "   mov r8, r4                          \n"
    "   mov r9, r5                          \n"
    "   mov r10, r6                         \n"
    "   mov r11, r7                         \n"
    "                                       \n"
    "   msr psp, r0                         \n" /* Remember the new top of stack for the task. */
    "                                       \n"
    "   subs r0, #32                        \n" /* Go back for the low registers that are not automatically restored. */
    "   ldmia r0!, {r4-r7}                  \n" /* Pop low registers.  */
    "                                       \n"
    "   bx r3                               \n"
    "                                       \n"
    "   .align 2                            \n"
    );
}
