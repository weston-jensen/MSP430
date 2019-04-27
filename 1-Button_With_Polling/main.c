#include <msp430fr2433.h>

#define LED1 BIT0 // P1.0
#define LED2 BIT1 // P1.1

#define BTN1 BIT3 // P2.3
#define BTN2 BIT7 // P2.7

/**
 * toggle a led by pressing a button
 */

void init_gpio();

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    init_gpio();

    while(1)
    {
        // Check for BTN1 to be pressed
        if(P2IN & BTN1)
        {
            // Debounce BTN1 press
            while(P2IN & BTN1);

            // Toggle LED
            P1OUT ^= LED1;
        }
    }

    return 0;
}

void init_gpio()
{
    // Initialize LED on port 1
    P1DIR |= LED1; // Set direction to output
    P1OUT &= ~LED1; // Set output to low
    P1DIR |= LED1; // Set pin direction to output

    // Initialize Button on port 2
    P2DIR &= ~BTN1; // Set direction to input
    P2OUT |= BTN1; // Use pull-up resistor
    P2REN |= BTN1; // Resistor enabled

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}
