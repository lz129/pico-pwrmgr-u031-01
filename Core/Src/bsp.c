/*****************************************************************************
* Product: DPP on STM32 NUCLEO-L053R8 board, cooperative QV kernel
** Last Updated for Version: 5.5.1
* Date of the Last Update:  2015-10-05
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses/>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/

#include "qpn.h"
#include "bsp.h"
#include "main.h"
#include "stm32u0xx_ll_gpio.h"
#include "stm32u0xx_ll_pwr.h"
#include "tasks.h"

//Q_DEFINE_THIS_FILE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
* DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
*/
enum KernelAwareISRs {
    GPIOPORTA_PRIO = QF_AWARE_ISR_CMSIS_PRI, /* see NOTE00 */
    SYSTICK_PRIO,
    /* ... */
    MAX_KERNEL_AWARE_CMSIS_PRI /* keep always last */
};
/* "kernel-aware" interrupts should not overlap the PendSV priority */
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));


// debounce button
static volatile uint16_t ticks;

static volatile uint8_t debounceReg = 0x02;
static volatile uint8_t pressed;
static volatile uint8_t released;
static volatile uint16_t pressedCounter;
static volatile uint16_t releasedCounter;
static volatile uint8_t clickCount;

void Shutdown(void)
{
   /* Setup pullup resistor for wakeup pin while in shutdown mode */
   LL_PWR_EnableGPIOPullUp(LL_PWR_GPIO_A, LL_PWR_GPIO_BIT_0);
   LL_PWR_EnablePUPDCfg();

   /* Enable wake up pin */ 
   LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
   LL_PWR_ClearFlag_WU1();

   /* Set Shutdown mode */
   LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);

   /* Set SLEEPDEEP bit of Cortex System Control Register */
   LL_LPM_EnableDeepSleep();

   /* Request Wait For Interrupt */
   __WFI();
}

void SysTick_Handler(void) {

    ++ticks;

    QF_tickXISR(0U); /* process time events for rate 0 */

    // debounce button
    if (pressed && (pressedCounter != 0xffff)) pressedCounter += pressed;
    if (released && (releasedCounter != 0xffff)) releasedCounter += released;

    uint32_t bp = BSP_ButtonPressed();
    debounceReg = (debounceReg << 1) + bp;
    if (debounceReg == 0x7f)
    {
        if (releasedCounter > 100) {
            clickCount = 1;
        }
        pressed = 1;
        released = 0;
        pressedCounter = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON_PRESS_SIG, 0U);
    }
    if (debounceReg == 0x80)
    {
        if (pressedCounter > 100) {
            clickCount = 0;
        }
        released = 1;
        pressed = 0;
        releasedCounter = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON_RELEASE_SIG, 0U);

        if (clickCount > 0) {
            QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON_CLICK_SIG, clickCount);
        }

        clickCount++;
    }

    if (pressedCounter == 256) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON_LONG_PRESS_SIG, 0U);
    }
    if ((pressedCounter > 256) && ((pressedCounter % 128) == 0)) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON_LONG_PRESS_REPEAT_SIG, 0U);
    }
}


void BSP_init(void) {
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();
}

void BSP_LED1_Off(void)
{
    LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
}

void BSP_LED1_On(void)
{
    LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
}

void BSP_LED2_Off(void)
{
    LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

void BSP_LED2_On(void)
{
    LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

uint32_t BSP_ButtonPressed(void)
{
    return ! LL_GPIO_IsInputPinSet(BUTTON1_GPIO_Port, BUTTON1_Pin);
}



void QF_onStartup(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE00
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    NVIC_SetPriority(SysTick_IRQn,   SYSTICK_PRIO);
    /* ... */

    /* enable IRQs... */
}

void QV_onIdle(void) {  /* called with interrupts disabled, see NOTE1 */


    /* toggle an LED on and then off (not enough LEDs, see NOTE2) */
    //GPIOA->BSRR |= (LED_LD2);        /* turn LED[n] on  */
    //GPIOA->BSRR |= (LED_LD2 << 16);  /* turn LED[n] off */

//#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    /* !!!CAUTION!!!
    * The WFI instruction stops the CPU clock, which unfortunately disables
    * the JTAG port, so the ST-Link debugger can no longer connect to the
    * board. For that reason, the call to __WFI() has to be used with CAUTION.
    *
    * NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    * reset the board, then connect with ST-Link Utilities and erase the part.
    * The trick with BOOT(0) is it gets the part to run the System Loader
    * instead of your broken code. When done disconnect BOOT0, and start over.
    */
    QV_CPU_SLEEP();  /* atomically go to sleep and enable interrupts */
    //QF_INT_ENABLE(); /* for now, just enable interrupts */
//#else
//    QF_INT_ENABLE(); /* just enable interrupts */
//#endif
}

/*..........................................................................*/
Q_NORETURN Q_onAssert(char const Q_ROM * const module, int loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;

    NVIC_SystemReset();
}

/*****************************************************************************
* NOTE1:
* The QV_onIdle() callback is called with interrupts disabled, because the
* determination of the idle condition might change by any interrupt posting
* an event. QV_onIdle() must internally enable interrupts, ideally
* atomically with putting the CPU to the power-saving mode.
*
* NOTE2:
* One of the LEDs is used to visualize the idle loop activity. The brightness
* of the LED is proportional to the frequency of invcations of the idle loop.
* Please note that the LED is toggled with interrupts locked, so no interrupt
* execution time contributes to the brightness of the User LED.
*/
