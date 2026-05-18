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

static volatile uint8_t debounceReg1 = 0x02;
static volatile uint8_t pressed1;
static volatile uint8_t released1;
static volatile uint16_t pressedCounter1;
static volatile uint16_t releasedCounter1;
static volatile uint8_t clickCount1;

static volatile uint8_t debounceReg2 = 0x02;
static volatile uint8_t pressed2;
static volatile uint8_t released2;
static volatile uint16_t pressedCounter2;
static volatile uint16_t releasedCounter2;
static volatile uint8_t clickCount2;

void Shutdown(void)
{
   /* Setup pullup resistor for wakeup pin while in shutdown mode */
   LL_PWR_EnableGPIOPullUp(LL_PWR_GPIO_A, LL_PWR_GPIO_BIT_0);
   LL_PWR_EnableGPIOPullUp(LL_PWR_GPIO_A, LL_PWR_GPIO_BIT_1);
   LL_PWR_EnablePUPDCfg();

   /* Enable wake up pin 1 */ 
   LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
   LL_PWR_SetWakeUpPinPolarityLow(LL_PWR_WAKEUP_PIN1);
   LL_PWR_ClearFlag_WU1();

   /* Enable wake up pin 3 */ 
   LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN3);
   LL_PWR_SetWakeUpPinPolarityLow(LL_PWR_WAKEUP_PIN3);
   LL_PWR_ClearFlag_WU3();

   /* Set Shutdown mode */
   LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
   LL_PWR_EnableBORPVD_ULP();

   /* Set SLEEPDEEP bit of Cortex System Control Register */
   LL_LPM_EnableDeepSleep();

   /* Request Wait For Interrupt */
   __WFI();
}

void SysTick_Handler(void) {

    ++ticks;

    QF_tickXISR(0U); /* process time events for rate 0 */

    // debounce button 1
    if (pressed1 && (pressedCounter1 != 0xffff)) pressedCounter1 += pressed1;
    if (released1 && (releasedCounter1 != 0xffff)) releasedCounter1 += released1;

    debounceReg1 = (debounceReg1 << 1) + BSP_Button1Pressed();
    if (debounceReg1 == 0x7f)
    {
        if (releasedCounter1 > 100) {
            clickCount1 = 1;
        }
        pressed1 = 1;
        released1 = 0;
        pressedCounter1 = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_PRESS_SIG, 0U);
    }
    if (debounceReg1 == 0x80)
    {
        if (pressedCounter1 > 100) {
            clickCount1 = 0;
        }
        released1 = 1;
        pressed1 = 0;
        releasedCounter1 = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_RELEASE_SIG, 0U);

        if (clickCount1 > 0) {
            QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_CLICK_SIG, clickCount1);
        }

        clickCount1++;
    }

    if (pressedCounter1 == 256) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_LONG_PRESS_SIG, 0U);
    }
    if ((pressedCounter1 > 256) && ((pressedCounter1 % 128) == 0)) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_LONG_PRESS_REPEAT_SIG, 0U);
    }


    // debounce button 2
    if (pressed2 && (pressedCounter2 != 0xffff)) pressedCounter2 += pressed2;
    if (released2 && (releasedCounter2 != 0xffff)) releasedCounter2 += released2;

    debounceReg2 = (debounceReg2 << 1) + BSP_Button2Pressed();
    if (debounceReg2 == 0x7f)
    {
        if (releasedCounter2 > 100) {
            clickCount2 = 1;
        }
        pressed2 = 1;
        released2 = 0;
        pressedCounter2 = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_PRESS_SIG, 0U);
    }
    if (debounceReg2 == 0x80)
    {
        if (pressedCounter2 > 100) {
            clickCount2 = 0;
        }
        released2 = 1;
        pressed2 = 0;
        releasedCounter2 = 0;
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_RELEASE_SIG, 0U);

        if (clickCount2 > 0) {
            QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_CLICK_SIG, clickCount2);
        }

        clickCount2++;
    }

    if (pressedCounter2 == 256) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_LONG_PRESS_SIG, 0U);
    }
    if ((pressedCounter2 > 256) && ((pressedCounter2 % 128) == 0)) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_LONG_PRESS_REPEAT_SIG, 0U);
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

uint32_t BSP_Button1Pressed(void)
{
    return ! LL_GPIO_IsInputPinSet(BUTTON1_GPIO_Port, BUTTON1_Pin);
}

uint32_t BSP_Button2Pressed(void)
{
    return ! LL_GPIO_IsInputPinSet(BUTTON2_GPIO_Port, BUTTON2_Pin);
}

void BSP_CheckButton1Wakeup(void) 
{
    if (LL_PWR_IsActiveFlag_WU1()) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON1_CLICK_SIG, 1);
    }
    LL_PWR_ClearFlag_WU1();

}

void BSP_SetPower(bool on)
{
    if (on) {
        LL_GPIO_SetOutputPin(PWRON_OUT_GPIO_Port, PWRON_OUT_Pin);
    }
    else {
        LL_GPIO_ResetOutputPin(PWRON_OUT_GPIO_Port, PWRON_OUT_Pin);
    }
}

void BSP_CheckButton2Wakeup(void) 
{
    if (LL_PWR_IsActiveFlag_WU3()) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, BUTTON2_CLICK_SIG, 1);
    }
    LL_PWR_ClearFlag_WU3();

}

void BSP_CheckCheckNoButtonWakeup(void) 
{
    if (!(LL_PWR_IsActiveFlag_WU3() || LL_PWR_IsActiveFlag_WU1())) {
        QACTIVE_POST_ISR((QActive *)&AO_Test, NO_BUTTON_WAKEUP_SIG, 1);
    }
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
