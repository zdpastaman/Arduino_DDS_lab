/*
 * Dual DDS
 *
 * Copyright 2019 Aaron P. Dahlen       APDahlen@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */


// AVR GCC libraries for more information see:
//     http://www.nongnu.org/avr-libc/user-manual/modules.html
//     https://www.gnu.org/software/libc/manual/

    #include <avr/io.h>
    #include <avr/interrupt.h>
    #include <stdint.h>
    #include <string.h>
    #include <ctype.h>
    #include <stdio.h>


// Arduino libraries: see http://arduino.cc/en/Reference/Libraries


// Project specific includes

    #include "configuration.h"
    #include "line_parser.h"
    #include "DDS.h"
    #include "USART.h"


// Global variables

    #define F_ISR 10000
    #define F_OUT 60
    #define PIR 25769803
//#define PIR (uint32_t) (F_OUT * 2^32) / F_ISR     //FIXME 

void setup(){

    init_timer_1_CTC(F_ISR); // Enable the timer ISR

    USART_init(F_CLK, BAUD_RATE);

    DDS_set_PIR(PIR);
    DDS_on( );

    // Use the ATMega328p "fast PWM" a.k.a. hardware PWM for best performance

        pinMode(3, OUTPUT);
        pinMode(11, OUTPUT);
        TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
        TCCR2B = _BV(CS20);  // No prescaling)                          // This register controls the PWM frequency
        OCR2A = 0;                                                      // Duty cycle for I/O pin D11
        OCR2B = 0;                                                      // Duty cycle for I/O pin D3

}


/*********************************************************************************
 *  ______  ____   _____   ______  _____  _____    ____   _    _  _   _  _____
 * |  ____|/ __ \ |  __ \ |  ____|/ ____||  __ \  / __ \ | |  | || \ | ||  __ \
 * | |__  | |  | || |__) || |__  | |  __ | |__) || |  | || |  | ||  \| || |  | |
 * |  __| | |  | ||  _  / |  __| | | |_ ||  _  / | |  | || |  | || . ` || |  | |
 * | |    | |__| || | \ \ | |____| |__| || | \ \ | |__| || |__| || |\  || |__| |
 * |_|     \____/ |_|  \_\|______|\_____||_|  \_\ \____/  \____/ |_| \_||_____/
 *
 ********************************************************************************/

ISR(USART_RX_vect){

 /**
 * @note This Interrupt Service Routine is called when a new character is received by the USART.
 * Ideally it would have been placed in the USART.cpp file but there is a error "multiple definition
 * of vector_18". Apparently Arduino detects when an ISR is in the main sketch. If you place it
 * somewhere else it is missed and replaced with the Arduino handler. This is the source of the
 * multiple definitions error -  * see discussion @ http://forum.arduino.cc/index.php?topic=42153.0
 */

    USART_handle_ISR();

}

ISR(TIMER1_COMPA_vect){

/** @brief This Interrupt Service Routine (ISR) serves as the heartbeat 
 * for the Arduino. See the companion function init_timer_1_CTC for additional 
 * information.
 *
 * @Note:
 *    1) Compiler generated code pushes status register and any used registers to stack.
 *    2) Calling a subroutine from the ISR causes compiler to save all 32 registers; a
 *       slow operation (FIXME fact check).
 *    3) Status and used registers are popped by compiler generated code.
 */

    uint16_t DDS_value_1 = DDS_service_1( );
    uint16_t DDS_value_2 = DDS_service_2(PIR*14);

    OCR2A = (DDS_value_1 >> 4);                    // The DDS returns a 12-bit value while the PWM register is only 8-bits
    OCR2B = (DDS_value_2 >> 4);

}


/*********************************************************************************
 *  ____            _____  _  __ _____  _____    ____   _    _  _   _  _____
 * |  _ \    /\    / ____|| |/ // ____||  __ \  / __ \ | |  | || \ | ||  __ \
 * | |_) |  /  \  | |     | ' /| |  __ | |__) || |  | || |  | ||  \| || |  | |
 * |  _ <  / /\ \ | |     |  < | | |_ ||  _  / | |  | || |  | || . ` || |  | |
 * | |_) |/ ____ \| |____ | . \| |__| || | \ \ | |__| || |__| || |\  || |__| |
 * |____//_/    \_\\_____||_|\_\\_____||_|  \_\ \____/  \____/ |_| \_||_____/
 *
 ********************************************************************************/

void loop(){

    char line[BUF_LEN];

    snprintf(line, BUF_LEN, "hello\n");

    USART_puts(line);

    while(1){

        ;

        //FIXME: Add code prompting the user to enter the desired phase shift...

    }
}



void init_timer_1_CTC(long desired_ISR_freq){
/**
 * @brief Configure timer #1 to operate in Clear Timer on Capture Match (CTC Mode)
 *
 *      desired_ISR_freq = (F_CLK / prescale value) /  Output Compare Registers
 *
 *   For example:
 *        Given an Arduino Uno: F_clk = 16 MHz
 *        let prescale                = 64
 *        let desired ISR heartbeat   = 100 Hz
 *
 *        if follows that OCR1A = 2500
 *
 * @param desired_ISR_freq is the desired operating frequency of the ISR
 * @param F_CLK must be defined globally e.g., #define F_CLK 16000000L
 *
 * @return void
 *
 * @note The prescale value is set manually in this function.  Refer to ATMEL ATmega48A/PA/88A/PA/168A/PA/328/P datasheet for specific settings.
 *
 * @warning There are no checks on the desired_ISR_freq parameter.  Use this function with caution.
 *
 * @warning Use of this code will break the Arduino Servo() library.
 */

    cli();                                          // Disable global
    TCCR1A = 0;                                     // Clear timer counter control registers.  The initial value is zero but it appears Arduino code modifies them on startup...
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);                         // Timer #1 using CTC mode
    TIMSK1 |= (1 << OCIE1A);                        // Enable CTC interrupt
    TCCR1B |= (1 << CS10)|(1 << CS11);              // Prescale: divide by F_CLK by 64.  Note SC12 already cleared
    OCR1A = (F_CLK / 64L) / desired_ISR_freq;       // Interrupt when TCNT1 equals the top value of the counter specified by OCR
    sei();                                          // Enable global
}


