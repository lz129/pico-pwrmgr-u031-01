/*****************************************************************************
* Product: DPP example
* Last Updated for Version: 5.4.0
* Date of the Last Update:  2015-03-07
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, LLC. state-machine.com.
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
* Web  : www.state-machine.com
* <info@state-machine.com>
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

#define BSP_TICKS_PER_SEC    100U

enum Signals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,            // the last published signal

    TIMEOUT_SIG,            // Timeout

    BUTTON1_PRESS_SIG,       // Rotary encoder button press
    BUTTON1_RELEASE_SIG,     // Rotary encoder button release
    BUTTON1_CLICK_SIG,
    BUTTON1_LONG_PRESS_SIG,
    BUTTON1_LONG_PRESS_REPEAT_SIG,

    BUTTON2_PRESS_SIG,       // Rotary encoder button press
    BUTTON2_RELEASE_SIG,     // Rotary encoder button release
    BUTTON2_CLICK_SIG,
    BUTTON2_LONG_PRESS_SIG,
    BUTTON2_LONG_PRESS_REPEAT_SIG,

    NO_BUTTON_WAKEUP_SIG,
    SHUTDOWN_REQUEST_SIG,

    ENCODER_STEP_SIG,       // Rotary encoder step
    ONESECOND_SIG,          // One second tick
    BEEP_FINISHED_SIG,      // Beep signal finished
    MAX_SIG                 // the last signal
};



void BSP_init(void);
uint32_t BSP_getTicks(void);
uint32_t BSP_getTicksPowerOn(void);

void BSP_LED1_On(void);
void BSP_LED1_Off(void);
void BSP_LED2_On(void);
void BSP_LED2_Off(void);
uint32_t BSP_Button1Pressed(void);
uint32_t BSP_Button2Pressed(void);
void BSP_CheckButton1Wakeup(void);
void BSP_CheckButton2Wakeup(void);
void BSP_CheckCheckNoButtonWakeup(void);
void Shutdown(void);
void BSP_SetPower(bool on);

#endif /* BSP_H */
