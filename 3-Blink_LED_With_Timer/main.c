/**
 * @brief This program sets up a button interrupt that when pushed
 * will toggle an LED off and on. It is a simple example of how to
 * use an interrupt.
 */
#include <msp430fr2433.h>

#define SMCLK 1000000 ///< Using a SMCLK set to 1MHz
#define TIMER_VAL 1000 ///< 1 second delay
#define LED1 BIT0 ///< P1.0
#define LED2 BIT1 ///< P1.1


// Function definitions
void init_gpio();
void init_timer();

// Global variable
unsigned int timero_ao_count = 2;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Initialize GPIO pins for LED
    init_gpio();

    // Initialize timer0_a0
    init_timer();

    // Enable system interrupts
    __bis_SR_register(GIE);

    while(1);

    return 0;
}

/**
 * @brief Timer A0 interrupt service routine
 */
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timer0_a0_isr(void)
{
    --timero_ao_count;

    if(timero_ao_count == 0)
    {
        // Toggle LED
        P1OUT ^= LED1;

        // Reset count
        timero_ao_count = 2;
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

    // 2) Clear LOCKLPM5
    PM5CTL0 &= ~LOCKLPM5;

    // 3) Clear PxIFGs to avoid erroneous port interrupts

    // 4) Enable port interrupts in PxIE
}

void init_timer()
{

    // Timer_0_A0 Capture/Compare Register
    // Timer value: (1,000,000 / 8) = 125,000
    // 16-bit register, can only count to 65,535
    // So: 125,000 / 2 = 62,500
    // Count twice to 62,500 for one second
    TA0CCR0 = 62500; // Count to this value and trigger interrupt

    TA0CTL = (TASSEL_2 | // Timer-A clock source select : SMCLK
             ID_3 |      // Input divider : /8
             MC__UP);    // Mode control : Up mode, timer counts to TA0CCR0

    TA0CCTL0 |= CCIE;    // Interrupt enabled
}


