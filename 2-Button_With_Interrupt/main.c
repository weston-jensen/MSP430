/**
 * @brief This program sets up a button interrupt that when pushed
 * will toggle an LED off and on. It is a simple example of how to
 * use an interrupt.
 */
#include <msp430fr2433.h>

#define LED1 BIT0 ///< P1.0
#define LED2 BIT1 ///< P1.1

#define BTN1 BIT3 ///< P2.3
#define BTN2 BIT7 ///< P2.7

// Function definitions
void init_gpio();

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Initialize gpio pins for LED and BTN
    init_gpio();

    // Enable system interrupts
    __bis_SR_register(GIE);

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
        // Debounce BTN1 press
        while(P2IN & BTN1);
        __delay_cycles(1000);

        // Toggle LED
        P1OUT ^= LED1;

        // Clear interrupt flag
        P2IFG &= ~LED1;
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
    P1DIR |= LED1; // Set direction to output
    P1OUT &= ~LED1; // Set output to low
    P1DIR |= LED1; // Set pin direction to output

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
