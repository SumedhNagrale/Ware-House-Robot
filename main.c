#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <MoonDust/lcdmoondust.h>

volatile int RIGHT,LEFT,CENTER,Selection;
volatile int LIGHT;
volatile char Button;
volatile char Sensor;
volatile int output;
volatile int flag;
volatile int old_value;
volatile int direction;
volatile int Range;
volatile int janganamana;
volatile int m;
volatile int retrace[100];
volatile int ToOpen;
volatile int countLight;
volatile int ThresholdLight = 220;
volatile int TurnFlag = 0;

#define T_0 0
#define T_1 64
#define T_2 128
#define T_3 192
#define Launch 256

void SensorValue();
void Control();
void HallWayNavigation();
void updateMotors(signed int,signed int);


int main(void)
{
	countLight = 0;
	direction = 0;
	flag=3;
	old_value  = 0;
	Button = 'Y';
	Sensor = 'A';
	RIGHT,LEFT,CENTER,Selection = 0;
	LIGHT = 0;

	sei();
	DDRC = (1<<PORTC4)|(1<<PORTC5);	
	DDRB=(1<<PORTB1)|(1<<PORTB2)|(1<< PORTB0)|(1<< PORTB4)|(1<< PORTB5)|(1<< PORTB3);
    DDRD = (1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7)|(1<<PORTD3); 
	
	TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);//FAST PWM Mode, CLK/256
	TCCR0B = (0<<WGM02)|(1<<CS02)|(0<<CS01)|(1<<CS00);
	OCR0A = 128;
	OCR0B = 128;
	
	TCCR1A=(0<<COM1A1)|(1<<COM1A0)|(0<<WGM11)|(0<<WGM10);//CTC Mode, CLK/256
	TCCR1B=(0<<WGM13)|(1<<WGM12)|(1<<CS12)|(0<<CS11)|(0<<CS10);
	OCR1A=6250;//  (.1s)*(16 Mhz/256)
	TIMSK1=(1<<OCIE1A);
	
	
	//TCCR2A  =  (1<<  COM2A1  )|(0<<  COM2A0  )|(1<<  WGM21  )|(1<<  WGM20);
	//TCCR2B  =  (0<<  WGM22  )|(1<<  CS22  )|(1<<  CS21  )|(0<<  CS20  );
	////OCR0B  =  64;
	//OCR2A = 1.18 * (0) + 43.75;
	TCCR2A  =  (1<<  COM2A1  )|(0<<  COM2A0  )|(1<<  WGM21  )|(1<<  WGM20);
	TCCR2B  =  (0<<  WGM22  )|(1<<  CS22  )|(1<<  CS21  )|(0<<  CS20  );
	//OCR0B  =  64;
	OCR2A = 1.18 * (0) + 43.75;
	OCR2B = 125;
	TIMSK2=(1<<OCIE2B);
	
	
	ADMUX  =  (0<<  REFS1  )|(1<<  REFS0  )|(1<<  ADLAR  )|(0  <<  MUX3  )|(0<<  MUX2  )|(0<<  MUX1  )|(0<< MUX0);
	ADCSRA  =  (1<<  ADEN  )|(1<<  ADIE  )|(1<<  ADPS2  )|(1<<  ADPS1  )|(1<<  ADPS0  );
	DIDR0  =  (1<<ADC1D)|(1<<ADC0D)|(1<<ADC2D);
	ADCSRA  |=  (1<<  ADSC);
	
	
	EICRA=(0<<ISC01)|(1<<ISC00);//Configure interrupt for any pin change
	EIMSK=(1<<INT0);//Enable INT0
	//Enable all interrupts
	
	InitializeLCD();
	while(1)
    {
    }
}

ISR(ADC_vect)
{
	if (Sensor == 'A')
	{  
	RIGHT = ADCH;
	//OCR0A = ADCH;
	}
	else if(Sensor == 'B')
	{
	LEFT = ADCH;	
	//OCR0B = ADCH;
	}
	else if (Sensor == 'C')
	{
	CENTER = ADCH;	
	}
	else if (Sensor == 'D')
	{
		LIGHT = ADCH;
	}
	if (Sensor!= 'D')
	{
		Control();
	}
	else
	{
		//PositionReached(m);
	}
	
}

ISR(INT0_vect){
	//Check if button has been pushed
	//This checks whether the button is pushed or not.
	//N--> CW
	//Y-->CCW
	if(PIND&(1<<PIND2))
	{
	if(Button == 'N')
		{
		//CCW
		PORTD = (1<<PORTD7);
		PORTB = (0<<PORTB2)|(1<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		Button = 'Y';
		}
	else
		{
			//CW
			PORTD = (1<<PORTD7);
			PORTB = (1<<PORTB2)|(0<<PORTB0)|(1<<PORTB4)|(0<<PORTB5);
			Button = 'N';
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
		
	//This interrrupt is called for timer1, Since we have enabled bit in TIMSK1
	//Once we are here we will start the ADC
	SensorValue();
	ADCSRA|=(1<<ADSC);
	LCD_Command(CURSOR_HOME);	
	//Added some delay for display
	_delay_us(64*200);
	//Position from where the output on the LCD must be displayed, This can be found from the table given in the lecture 8
	LCD_Command(SET_ADDRESS|00);
	//***************************************
	//EXAMPLE: 125; 125/100 = 1 and OR the result with 0X30 IN ORDER TO DISPLAY IT ON THE LCD SCREEN.
	//FOLLOWING THE SAME PATTERN WE CAN IDENTIFY TENS AND UNITS PLACE OF A DECIMAL NUMBER.
	//RIGHT = (9.5*1024)/(5*RIGHT);
	char hundred = RIGHT/100 |0x30;
	char tens = ((RIGHT%100)/10)|0x30;
	char units = (RIGHT%10)|0x30;
	//LCD_Display function takes char data. hence we have to display the RIGHT in this fashion.
	LCD_Display('R');
	LCD_Display('N'|0x30);
	LCD_Display(hundred);
	LCD_Display(tens);
	LCD_Display(units);
	LCD_Command(SET_ADDRESS|6);
	LCD_Display('B');
	LCD_Display('N'|0x30);
	LCD_Display(Button);
	if(LEFT!= 0)
	{
	LCD_Command(SET_ADDRESS|60);
	char hundredB = LEFT/100 |0x30;
	char tensB = ((LEFT%100)/10)|0x30;
	char unitsB = (LEFT%10)|0x30;
	//LCD_Display function takes char data. hence we have to display the RIGHT in this fashion.
	LCD_Display('L');
	LCD_Display('N'|0x30);
	LCD_Display(hundredB);
	LCD_Display(tensB);
	LCD_Display(unitsB);
	
LCD_Command(SET_ADDRESS|70);
	//***************************************
	//EXAMPLE: 125; 125/100 = 1 and OR the result with 0X30 IN ORDER TO DISPLAY IT ON THE LCD SCREEN.
	//FOLLOWING THE SAME PATTERN WE CAN IDENTIFY TENS AND UNITS PLACE OF A DECIMAL NUMBER.
	char h6 = LIGHT/100 |0x30;
	char t6 = ((LIGHT%100)/10)|0x30;
	char u6 = (LIGHT%10)|0x30;
	//LCD_Display function takes char data. hence we have to display the RIGHT in this fashion.
	LCD_Display('P');
	LCD_Display('N'|0x30);
	LCD_Display(h6);
	LCD_Display(t6); 
	LCD_Display(u6);
	

	}
	if(CENTER!= 0)
	{
	LCD_Command(SET_ADDRESS|20);
	//***************************************
	//EXAMPLE: 125; 125/100 = 1 and OR the result with 0X30 IN ORDER TO DISPLAY IT ON THE LCD SCREEN.
	//FOLLOWING THE SAME PATTERN WE CAN IDENTIFY TENS AND UNITS PLACE OF A DECIMAL NUMBER.
	char h = CENTER/100 |0x30;
	char t = ((CENTER%100)/10)|0x30;
	char u = (CENTER%10)|0x30;
	//LCD_Display function takes char data. hence we have to display the RIGHT in this fashion.
	LCD_Display('C');
	LCD_Display('N'|0x30);
	LCD_Display(h);
	LCD_Display(t); 
	LCD_Display(u);
	}

	LCD_Command(SET_ADDRESS|84);
	LCD_Display('D');
	LCD_Display('N'|0x30);
	
	if(Button == 'Y')
	{
	LCD_Display('C');
	LCD_Display('W');	
	LCD_Display(' ');
	}
	else
	{
		LCD_Display('C');
		LCD_Display('C');
		LCD_Display('W');
	}
}

void SensorValue()
{
	if (Sensor == 'A')
	{
		Sensor = 'B';
	}
	else if (Sensor == 'B')
	{
		Sensor = 'C';
	}
	else if (Sensor == 'C')
	{
		Sensor = 'D';
	}
	else if (Sensor == 'D')
	{
		Sensor = 'A';
	}
	
	if(Sensor == 'A')
	{
		ADMUX  =  (0<<  REFS1  )|(1<<  REFS0  )|(1<<  ADLAR  )|(0  <<  MUX3  )|(0<<  MUX2  )|(0<<  MUX1  )|(0<< MUX0);
	}
	else  if (Sensor == 'B')
	{
		ADMUX  =  (0<<  REFS1  )|(1<<  REFS0  )|(1<<  ADLAR  )|(0  <<  MUX3  )|(0<<  MUX2  )|(0<<  MUX1  )|(1<< MUX0);
	}
	else  if (Sensor == 'C')
	{
		ADMUX  =  (0<<  REFS1  )|(1<<  REFS0  )|(1<<  ADLAR  )|(0  <<  MUX3  )|(0<<  MUX2  )|(1<<  MUX1  )|(0<< MUX0);
	}
	else  if (Sensor == 'D')
	{
		ADMUX  =  (0<<  REFS1  )|(1<<  REFS0  )|(1<<  ADLAR  )|(0  <<  MUX3  )|(0<<  MUX2  )|(1<<  MUX1  )|(1<< MUX0);
	}
}

void Control()
{
	int ThresholdR,ThresholdL,ThresholdC,R,L,C;
	ThresholdR = 80;
	ThresholdL = 80;
	ThresholdC = 140;
	if (RIGHT  > ThresholdR)
	R = 1;
	else 
	R = 0;
	if (LEFT  > ThresholdL)
	L = 1;
	else
	L = 0;
	if (CENTER  > ThresholdC)
	C = 1;
	else
	C = 0;
	
	Selection = 4*R + 2*L + C;
	HallWayNavigation();
}

void HallWayNavigation()
{
	if (LIGHT <ThresholdLight)
	{
switch (Selection)
{
	case 0x00:
	{
		PORTB = (1<<PORTB2)|(0<<PORTB0)|(1<<PORTB4)|(0<<PORTB5);
		updateMotors(110,110);
		//Nothing
		break;
	}
	case 0x01:
	{
		PORTB = (0<<PORTB2)|(1<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		updateMotors(T_2,T_2);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);			
		//Move back
		break;
	}
	case 0x02:
	{
		PORTB = (1<<PORTB2)|(0<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		updateMotors(T_1,T_3);
		//Move Right
		break;
	}
	case 0x03:
	{
		PORTB = (1<<PORTB2)|(0<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		updateMotors(T_0,Launch);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);	
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
		_delay_us(64*200);
		//Reverse and move Right
		break;
	}
	case 0x04:
	{
		PORTB = (0<<PORTB2)|(1<<PORTB0)|(1<<PORTB4)|(0<<PORTB5);
		updateMotors(T_3,T_1);
		//Move Left
		break;
	}
	case 0x05:
	{
		PORTB = (0<<PORTB2)|(1<<PORTB0)|(1<<PORTB4)|(0<<PORTB5);
		updateMotors(Launch,T_0);
		_delay_us(64*200);
		_delay_us(64*200);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
		//Reverse and move Left
		break;
	}
	case 0x06:
	{
		PORTB = (0<<PORTB2)|(1<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		updateMotors(T_2,T_2);
		//Reverse and move to a Left direction 
		break;
	}
	case 0x07:
	{
		PORTB = (1<<PORTB2)|(0<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
		updateMotors(Launch,Launch);
		_delay_us(64*200);
		_delay_us(64*200);
		_delay_us(64*200);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
		_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);		
		//Turn around from Right
		break;
	}
}
	}
	else
	{
		if(TurnFlag==0)
		{
			PORTB = (0<<PORTB2)|(1<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
			updateMotors(Launch,Launch);
			_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
			_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
			_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
			//_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
			//_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
			//Move back
			//TurnFlag = 1;
		//}
		//else
		//{
				//PORTB = (1<<PORTB2)|(0<<PORTB0)|(0<<PORTB4)|(1<<PORTB5);
				//updateMotors(Launch,Launch);
				//_delay_us(64*200);
				//_delay_us(64*200);
				//_delay_us(64*200);
				//_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
				//_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);_delay_us(64*200);
				////Turn around from Right
				//TurnFlag = 0;
		}
	}
}

void updateMotors(signed int stage1, signed int stage2)
{
		OCR0A = stage2;
		OCR0B = stage1;	
}

ISR(TIMER2_COMPB_vect)
{
	if(LIGHT > ThresholdLight)
	{
		Range = 162;
		OCR2A = Range;
}
	if(LIGHT < 150)
	{
		OCR2A = 43.75;
	}

}

