/* Copyright (c) 2016, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 #include "bootloader.h"
 #include "spiffs-arch.h"
 #include "nrf_sdm.h"
 #include "nrf_mbr.h"
 #include "nrf.h"
 #include "app_util.h"

 #include "nrf_gpio.h"
 #include "app_util.h"
 #include "app_error.h"
 #include "softdevice_handler.h"
 #include "ble_stack_handler_types.h"
 #include "ble_advdata.h"
 #include "ble_l2cap.h"
 #include "ble_gap.h"
 #include "ble_gatt.h"
 #include "ble_hci.h"
 #include "app_util.h"

 #include "dev/watchdog.h"
 #include "libs/util/byteorder.h"
 #include "errno.h"

 #ifdef USE_FREERTOS
 #include "FreeRTOS.h"
 #include "task.h"
 #endif
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
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

#define NRF_UICR_BOOT_START_ADDRESS         (NRF_UICR_BASE + 0x14)      /**< Register where the bootloader start address is stored in the UICR register. */


//#define CODE_REGION_1_START                 SD_SIZE_GET(MBR_SIZE)
//#define SOFTDEVICE_REGION_START             MBR_SIZE

#define BOOTLOADER_REGION_START         0x0003C000
#define BOOTLOADER_REGION_SIZE          0x00004C00
#define BOOTLOADER_SETTINGS_ADDRESS     0x0003FC00
#define BOOTLOADER_SETTINGS_SIZE     		0x00000400


#define CODE_REGION_1_START						  0x1B000

//#define UICR_ADDRESS                    0x10001014

#define IRQ_ENABLED             			 0x01                    								/**< Field identifying if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS   			 32                      								/**< Maximum number of interrupts available. */


/*---------------------------------------------------------------------------*/
// Jump into bootloader
static void __attribute__ ((noinline))
nrf_bootloader_app_start_impl(uint32_t start_addr)
{
  __asm volatile(
      "ldr   r0, [%0]\t\n"            // Get App initial MSP for bootloader.
      "msr   msp, r0\t\n"             // Set the main stack pointer to the applications MSP.
      "ldr   r0, [%0, #0x04]\t\n"     // Load Reset handler into R0.

      "movs  r4, #0xFF\t\n"           // Move ones to R4.
      "sxtb  r4, r4\t\n"              // Sign extend R4 to obtain 0xFFFFFFFF instead of 0xFF.

      "mrs   r5, IPSR\t\n"            // Load IPSR to R5 to check for handler or thread mode.
      "cmp   r5, #0x00\t\n"           // Compare, if 0 then we are in thread mode and can continue to reset handler of bootloader.
      "bne   isr_abort\t\n"           // If not zero we need to exit current ISR and jump to reset handler of bootloader.

      "mov   lr, r4\t\n"              // Clear the link register and set to ones to ensure no return.
      "bx    r0\t\n"                  // Branch to reset handler of bootloader.

      "isr_abort:  \t\n"

      "mov   r5, r4\t\n"              // Fill with ones before jumping to reset handling. Will be popped as LR when exiting ISR. Ensures no return to application.
      "mov   r6, r0\t\n"              // Move address of reset handler to R6. Will be popped as PC when exiting ISR. Ensures the reset handler will be executed when exist ISR.
      "movs  r7, #0x21\t\n"           // Move MSB reset value of xPSR to R7. Will be popped as xPSR when exiting ISR. xPSR is 0x21000000 thus MSB is 0x21.
      "rev   r7, r7\t\n"              // Reverse byte order to put 0x21 as MSB.
      "push  {r4-r7}\t\n"             // Push everything to new stack to allow interrupt handler to fetch it on exiting the ISR.

      "movs  r4, #0x00\t\n"           // Fill with zeros before jumping to reset handling. We be popped as R0 when exiting ISR (Cleaning up of the registers).
      "movs  r5, #0x00\t\n"           // Fill with zeros before jumping to reset handling. We be popped as R1 when exiting ISR (Cleaning up of the registers).
      "movs  r6, #0x00\t\n"           // Fill with zeros before jumping to reset handling. We be popped as R2 when exiting ISR (Cleaning up of the registers).
      "movs  r7, #0x00\t\n"           // Fill with zeros before jumping to reset handling. We be popped as R3 when exiting ISR (Cleaning up of the registers).
      "push  {r4-r7}\t\n"             // Push zeros (R4-R7) to stack to prepare for exiting the interrupt routine.

      "movs  r0, #0xF9\t\n"           // Move the execution return command into register, 0xFFFFFFF9.
      "sxtb  r0, r0\t\n"              // Sign extend R0 to obtain 0xFFFFFFF9 instead of 0xF9.
      "bx    r0\t\n"                  // No return - Handler mode will be exited. Stack will be popped and execution will continue in reset handler initializing other application.
      ".align\t\n"
      :: "r" (start_addr)             // Argument list for the gcc assembly. start_addr is %0.
      :  "r0", "r4", "r5", "r6", "r7" // List of register maintained manually.
  );
}
/*---------------------------------------------------------------------------*/
static void
interrupts_disable(void)
{
	NVIC->ICER[0] = 0xffffffff;
#ifdef NRF52
	NVIC->ICER[1] = 0xffffffff;
#endif
}
/*---------------------------------------------------------------------------*/
static void
bootloader_util_reset(uint32_t start_addr)
{
	sd_softdevice_disable();
	sd_softdevice_vector_table_base_set(start_addr);
	NVIC_ClearPendingIRQ(SWI2_IRQn);
	interrupts_disable();
	PRINTF("[bootloader-arch] watchdog reboot\n");
	NVIC_SystemReset();
  // watchdog_reboot();
	while(1){};
}
/*---------------------------------------------------------------------------*/
uint32_t
bootloader_settings_get(void *p_settings, int size)
{
	uint32_t err_code = ENONE;
	nrf_spiffs_flash_read(BOOTLOADER_SETTINGS_ADDRESS, (uint8_t*)p_settings, size);
	return err_code;
}
/*---------------------------------------------------------------------------*/
uint32_t
bootloader_start(void)
{
	nrf_delay_ms(100);
	bootloader_util_reset(BOOTLOADER_REGION_START);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
