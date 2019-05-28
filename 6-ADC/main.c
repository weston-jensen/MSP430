/**
 * @brief Setup ADC to sample channels 1-2
 * every millisecond with an RTC timer
 */
#include <msp430fr2433.h>

void init_adc();
void init_rtc();

unsigned int Channel_1_ADCMEM0;
unsigned int Channel_2_ADCMEM0;

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init_adc();
	init_rtc();

	return 0;
}

#pragma vector=ADC_VECTOR
__interrupt void adc_isr(void)
{
    static channel_sampled = 2;
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_ADCIFG:
            if(channel_sampled == 2)
            {
                channel_sampled--;
                Channel_2_ADCMEM0 = ADCMEM0;
            }
            else
            {
                channel_sampled  = 2;
                Channel_1_ADCMEM0 = ADCMEM0;
            }
            break;
        default:
            break;
    }
}

void init_adc()
{
    ADCCTL0 |= (ADCSHT_8 | // Sample and hold time
                ADCON);    // Turn on ADC, not yet enabled though

    ADCCTL1 |= (ADCSSEL_2 | // Select SMCLK
                ADCSHP);    // Set sampling signal to sampling timer

    ADCCTL2 &= ~ADCRES;  // Clear ADCRES in ADCCTL

    ADCMCTL0 |= ADCINCH_2; // Select channel 2
}

void init_rtc()
{
    // RTCMOD = (SMCLK / RTC_PREDIVIDE) * (DELAY_TIME_MS / 1,000);
    // RTCMOD = (1,000,000 / 1,000) * (1 / 1,000) = 1
    RTCMOD = 1;

    // RTC Counter Control Register
    RTCCTL = (RTCSS_1 | // Set clock source as SMCLK
             RTCPS__1000 | // RTC pre-divide value
             RTCSR | // Clear counter value
             RTCIE); // Enable interrupts
}
