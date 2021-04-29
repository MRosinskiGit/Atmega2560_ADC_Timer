#include <avr/io.h>
#include <avr/interrupt.h>
#define PRESCALER 1024
#define F_CPU 16000000
#define NB_OF_SAMPLES 10
#define SAMPLE_FREQ_HZ 10000

uint16_t tab[NB_OF_SAMPLES];
uint8_t wait=0, startadc=0;;

ISR (ADC_vect){
	wait = 0;
}

ISR(TIMER1_COMPA_vect) {
	startadc=1;
}

void InitADC()
{
	ADMUX |=(1<<REFS0);//zrodlo 
	ADCSRA |=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//przerwania prescaler nable
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	if (ADCchannel>7){		//jesli trzeba uzyc 2 rejestrów 
		ADCSRB|=(1<<MUX5);		//ustaw ten drugi
		ADCchannel=ADCchannel-8;	//oblicz dla pierwszego
		ADMUX &= 0b11100000;		//wyzeruj aktualne ustawienie
		ADMUX |= ADCchannel;		//ustaw kana³
	} else
	{
		ADCSRB &=!(1<<MUX5);		//wyzeruj aktualne
		ADMUX &= 0b11100000;		//wyzeruj aktualne
		ADMUX |= ADCchannel;		//ustaw kanal
	}
	wait=1;							//flaga czekania	
	ADCSRA |= (1<<ADSC);			//start konwersji
	while(wait){					//czekanie
		asm ("nop");
	}
	startadc=0;					//pomiar wykonany
	return ADC; //tutaj umiesc breakpoint
}
void InitTimer(uint16_t freq)
{
	if (freq>3000)				//czestotliwosci graniczne
		freq=3000;
	if (freq==0)
	 	freq=1;
	TCCR1A |=(1<<COM1A0);				//toggle mode
	TCCR1B |=(1<<WGM12);				//ctc
	TIMSK1 |= (1<<OCIE1A);				//interrupt enable A compare
	if (freq>=200)				//w zaleznosci od F wyzeruj aktualne ustawienia, ustaw nowy prescaler, wyzeruj aktualny licznik
		{
		OCR1A = (F_CPU/(2*1*(freq/2)))-1;		//oblicz OCR1A
		TCCR1A &= 0b11111000;				//wyzeruj prescaler
		TCCR1C |= (1<<FOC1A);				//wyzeruj licznik
		TCCR1B |=(1<<CS10);					//ustaw prescaler
		} 
	else if (freq>=30)
		{
		OCR1A = (F_CPU/(2*8*(freq/2)))-1;
		TCCR1A &= 0b11111000;
		TCCR1C |= (1<<FOC1A);
		TCCR1B |=(1<<CS11);
		}
	else if (freq>=3){
		OCR1A = (F_CPU/(2*64*(freq/2)))-1;
		TCCR1A &= 0b11111000;
		TCCR1C |= (1<<FOC1A);
		TCCR1B |=(1<<CS10)|(1<<CS11);
		}
	else 
	{
		OCR1A = (F_CPU/(2*1024*(freq/2)))-1;
		TCCR1A &= 0b11111000;
		TCCR1C |= (1<<FOC1A);
		TCCR1B |= (1<<CS12);
		}
}

int main(void)
{	
	sei();					//przerwania
	InitADC();				//wstepna konfiguracja adc
	InitTimer(3000);		//konfiguracja timera o f=3000Hz
	int loop_ctr=0;			//licznik pomiarow
	
	while (1)
	{
		if (startadc==1) {		//gdy timer1 ustawi flage rozpoczni pomiar
			tab[loop_ctr]=ReadADC(1);		//zapisz wynik
			loop_ctr++;						//inkrementacja licznika
		}
		if((loop_ctr)==NB_OF_SAMPLES)		// gdy limit pomiarow
			while (1) 
				asm ("nop");				//do nothing

	}
}

