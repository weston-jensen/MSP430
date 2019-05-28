/**
 * @brief Keep track of time and output the time over
 * UART each time the button is pressed"
 */
#include <msp430fr2433.h>

#define SMCLK 1000000 ///< Using a SMCLK set to 1MHz
#define TIMER_VAL 1000 ///< 1 second delay
#define UART_Tx_PIN BIT4 ///< P1.4
#define UART_Rx_PIN BIT5 ///< P1.5
#define BTN1 BIT3 ///< P2.3

struct Time{
    unsigned int seconds;
    unsigned int minutes;
    unsigned int hours;
    unsigned int days;
};


// Function definitions
void init_gpio();
void init_uart();
void init_rtc();
void write_uart(char *str);
void itoa(unsigned int value, char *str);

// Global variable
volatile struct Time time;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Initialize RTC
    init_rtc();

    // Initialize GPIO pins for UART communication and button
    init_gpio();

    // Initialize UART
    init_uart();

    while(1);


    return 0;
}

/**
 * @brief Port 2 Interrupt Service Routine
 */
#pragma vector = PORT2_VECTOR
__interrupt void port2_isr()
{
    // BTN1 interrupt
    if (P2IFG & BTN1)
    {
        char days[10];
        char hours[10];
        char minutes[10];
        char seconds[10];

        itoa(time.days, days);
        itoa(time.hours, hours);
        itoa(time.minutes, minutes);
        itoa(time.seconds, seconds);

        write_uart(days);
        write_uart("_");
        write_uart(hours);
        write_uart(":");
        write_uart(minutes);
        write_uart(":");
        write_uart(seconds);


        // Clear interrupt flag
        P2IFG &= ~BTN1;
    }
}

#pragma vector = RTC_VECTOR
__interrupt void rts_isr(void)
{
    if(RTCIV & RTCIV_RTCIF)
    {
        time.seconds++;

        if(time.seconds >= 60)
        {
            // Reset seconds count
            time.seconds = 0;

            // Increment minutes count
            time.minutes++;

            if(time.minutes >= 60)
            {
                // Reset minutes count
                time.minutes = 0;

                // Increment hours count
                time.hours++;

                if(time.hours >= 24)
                {
                    // Reset hours count
                    time.hours = 0;

                    // Increment days count
                    time.days++;
                }
            }
        }

        // Clear interrupt flag
        //RTCCTL &= ~RTCIF;
    }
}

/**
 * @brief Method to initialize GPIO pins
 * UG Section 8.3.2: steps on setting up i/o pins
 *
 * Steps UG Section 8.3.1:
 * - 1. Initialize ports: PxDIR, PxREN, PxOUT, and PxIED
 * - 2. Clear LOCKLPM5
 * - 3. Clear PxIFGs to avoid erroneous port interrupts
 * - 4. Enable port interrupts in PxIE
 */
void init_gpio()
{
    // 1) Initialize ports: PxDIR, PxREN, PxOUT, and PxIED

    // Initialize LED on port 1
    P1DIR |= UART_Tx_PIN; // Set direction to output
    P1OUT &= ~UART_Tx_PIN; // Set output to low

    // Configure pin to be used for UART instead of GPIO
    P1SEL1 &= ~UART_Tx_PIN;
    P1SEL0 |= UART_Tx_PIN;

    // Initialize Button on port 2
    P2DIR &= ~BTN1; // Set direction to input
    P2REN |= BTN1; // Resistor enabled
    P2OUT |= BTN1; // Use pull-up resistor
    P2IES |= BTN1; // Configure to interrupt on a falling edge

    // 2) Clear LOCKLPM5
    PM5CTL0 &= ~LOCKLPM5;

    // 3) Clear PxIFGs to avoid erroneous port interrupts
    P2IFG = 0x0; // Clear all port 2 interrupt flags

    // 4) Enable port interrupts in PxIE
    P2IE |= BTN1; // Enable port 2 interrupts for selected pins
}

/**
 * @brief Initialize UCAO UART for 9600 baud
 */
void init_uart()
{
    // Place UART into reset
    UCA0CTLW0 |= UCSWRST;

    // Select SMCLK 1MHz
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Setup UART to use 9600 baud
    // See User Guide Page 586
    // 1) N = f_BRCLK/BAUDRATE = 1,000,000/9600 = 104, [if N>16 continue to step 3]
    // 2) OS16 = 0, UCBRx = INT(N) [continue with step 4]
    // 3) OS16 = 1
    //    UCBRx = INT(N/16) = INT(104/16) = INT(6.00) = 6
    //    UCBRFx = INT([(N/16) - INT(N/16)] x 16
    //           = INT([104/16) - INT(104/16)] x 16
    //           = INT[6 - 6] x 16
    //           = 0
    // 4) USBRSx = fraction part of N in table 22-4
    //           = 0x00
    UCA0BRW = 6;
    UCA0MCTLW = 0;

    // Re-enable UART
    UCA0CTLW0 &= ~UCSWRST;
}

void init_rtc()
{
    // RTCMOD = (SMCLK / RTC_PREDIVIDE) * (DELAY_TIME_MS / 1,000);
    // RTCMOD = (1,000,000 / 1,000) * (1,000 / 1,000) = 1,000
    RTCMOD = 1000;

    // RTC Counter Control Register
    RTCCTL = (RTCSS_1 | // Set clock source as SMCLK
             RTCPS__1000 | // RTC pre-divide value
             RTCSR | // Clear counter value
             RTCIE); // Enable interrupts
}

/**
 * @brief Blocking method to write out a string over
 * UART Tx
 *
 * @param str = Character string array
 */
void write_uart(char *str)
{
    while(*str != '\0')
    {
        while(!(UCA0IFG & UCTXIFG));

        UCA0TXBUF = *str;

        ++str;
    }
}

void itoa(unsigned int value, char *str)
{
    char *ptr1 = str, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= 10;
        *str++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * 10)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *str++ = '-';
    *str-- = '\0';
    while(ptr1 < str) {
            tmp_char = *str;
            *str--= *ptr1;
            *ptr1++ = tmp_char;
    }
}


