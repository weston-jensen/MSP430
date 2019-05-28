/**
 * @brief Setup UART to use 9600 baud and
 * write out "Hello World"
 */
#include <msp430fr2433.h>

#define SMCLK 1000000    ///< Using a SMCLK set to 1MHz
#define TIMER_VAL 1000   ///< 1 second delay
#define UART_Tx_PIN BIT4 ///< P1.4
#define UART_Rx_PIN BIT5 ///< P1.5


// Function definitions
void init_gpio();
void init_uart();
void write_uart(char *str);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Initialize GPIO pins for UART communication
    init_gpio();

    // Initialize UART
    init_uart();

    write_uart("Hello World\n\r");


    return 0;
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


    // 2) Clear LOCKLPM5
    PM5CTL0 &= ~LOCKLPM5;

    // 3) Clear PxIFGs to avoid erroneous port interrupts

    // 4) Enable port interrupts in PxIE
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

/**
 * @brief Blocking method to write out a string over
 * UART Tx
 *
 * @param str = Character string array
 */
void write_uart(char *str)
{
    // While not end of string
    while(*str != '\0')
    {
        // Wait for UART Tx buffer to be empty
        while(!(UCA0IFG & UCTXIFG));

        // Add to Tx Buffer
        UCA0TXBUF = *str;

        // Go to next char in string
        ++str;
    }
}


