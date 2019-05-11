//atxmega256A3U
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define Plus 0
#define Sub  1
#define Did  2
#define Mul  3
#define Equ  4
#define i 0b11111011
#define n 0b10101011
#define f 0b10001110
#define p 0b10001100
#define e 0b10000110
#define q 0b10011000
#define u 0b11000001
#define m 0b11001000
#define l 0b11000111
#define d 0b10100001
#define s 0b10010010
#define b 0b10000011
#define minus 0b10111111
#define NumOne true
#define NumTwo false
#define Point 0b01111111

// кол-во итераций в ISR которое нужно для срабатывания нажатия клавиши (работает совместно с keyCounter)
const uint8_t keyDelay = 70;   

// 1/195 => 0.0051 сек * 40к = 205 сек (работает совместно с powerCounter)
const uint16_t powerStop = 40000;	

// задержка на индикацию нажатого действия (* / - + =)
const uint8_t delaySign = 50;	

const uint8_t numb[10] = {
	0b11000000, // 0
	0b11111001, // 1
	0b10100100, // 2
	0b10110000, // 3
	0b10011001, // 4
	0b10010010, // 5 
	0b10000010, // 6
	0b11011000, // 7
	0b10000000, // 8
	0b10010000  // 9
};
	
// разрешает обработку кнопки
bool isCanPressKey = true;

// показывает событие нажатия кнопки
bool pressKey = false; 

//писать в переменную numberOne
bool writeTo = NumOne; 
bool inf = false;

// счётчик кол-ва нажатия в ISR
uint8_t keyCounter = 0;

// счётчик для переключения режима питания МК  
uint16_t powerCounter = 0;
	
// хранит в себе данные для вывода на дисплей
uint8_t dispVal[4] = {255,255,255,numb[0]}; 

// переменные для хранения/записи/расчетов вычислений
float numberOne_f = 0.0;
float numberTwo = 0.0;

// кодирует 10-е число в 2-е
uint8_t coder(uint8_t code)
{
	code -=1;
	code = code -((code >> 1) & 0b01010101);
	return (code & 0b00110011) + ((code >> 2) & 0b00110011);
}

void DrawZnakomesto()
{
	PORTD.DIR = PORTD.DIR << 1;
	if(PORTD.DIR > 0b00001000)
	{
	PORTD.DIR = 0b00000001;
	}
	PORTE.OUT = PORTD.DIR;
	PORTC_OUT = dispVal[coder(PORTD.DIR)];
}

bool isKeyPres()
{
	return (PORTE.IN > 0b00001111);
}

bool isKeyPres(uint8_t key)
{
	return (PORTE.IN == key);
}

uint8_t getColumn()
{
	return coder(PORTE.IN & 0b00001111);
}

uint8_t getRow()
{
	return coder((PORTE.IN & 0b11110000) >> 4);
}

void key0(uint8_t* sign,float* number)
{
	*number *= 10;
}

void key1(uint8_t* sign,float* number)
{
	*number = *number * 10 + 1;
}

void key2(uint8_t* sign,float* number)
{
	*number = *number * 10 + 2;
}

void key3(uint8_t* sign,float* number)
{
	*number = *number * 10 + 3;
}

void key4(uint8_t* sign,float* number)
{
	*number = *number * 10 + 4;
}

void key5(uint8_t* sign,float* number)
{
	*number = *number * 10 + 5;
}

void key6(uint8_t* sign,float* number)
{
	*number = *number * 10 + 6;
}

void key7(uint8_t* sign,float* number)
{
	*number = *number * 10 + 7;
}

void key8(uint8_t* sign,float* number)
{
	*number = *number * 10 + 8;
}

void key9(uint8_t* sign,float* number)
{
	*number = *number * 10 + 9;
}

void On_C_Board(uint8_t* sign,float* number)
{
	if((*sign) == Equ)
	{
		inf	= false;
		numberTwo = 0.0;
		numberOne_f = 0.0;
		writeTo = NumOne;
		(*sign) = 0;
	}
	
	int32_t num_i= *number;
	
	if(num_i == 0)
	{
		writeTo = NumOne;
	}	
	
	num_i /= 10;
	*number = num_i;
}

void Plus_Board(uint8_t* sign,float* number)
{
	writeTo = NumTwo;
	*sign = Plus;
	dispVal[0] = p;
	dispVal[1] = l;
	dispVal[2] = u;
	dispVal[3] = s;
	_delay_ms(delaySign);
}

void Sub_Board(uint8_t* sign,float* number)
{
	writeTo = NumTwo;
	*sign = Sub;
	dispVal[0] = 255;
	dispVal[1] = s;
	dispVal[2] = u;
	dispVal[3] = b;
	_delay_ms(delaySign);
}

void Did_Board(uint8_t* sign,float* number)
{
	writeTo = NumTwo;
	*sign = Did;
	dispVal[0] = 255;
	dispVal[1] = d;
	dispVal[2] = i;
	dispVal[3] = d;
	_delay_ms(delaySign);
}

void Mul_Board(uint8_t* sign,float* number)
{
	writeTo = NumTwo;
	*sign = Mul;
	dispVal[0] = m;
	dispVal[1] = m;
	dispVal[2] = u;
	dispVal[3] = l;
	_delay_ms(delaySign);
}

void Equ_Board(uint8_t* sign,float* number)
{
	writeTo = NumOne;
	
	dispVal[0] = 255;
	dispVal[1] = e;
	dispVal[2] = q;
	dispVal[3] = u;
	_delay_ms(delaySign);
	
	masSign[*sign]();
	
	*sign = Equ;
	
	numberTwo = 0.0;
}

typedef void ( *VoidFunPtrBoard )(uint8_t* ,float* );

const VoidFunPtrBoard keyBoard[4][4] = {
	{Did_Board,key9,key8,key7},
	{Mul_Board,key6,key5,key4},
	{Sub_Board,key3,key2,key1},
	{Plus_Board,Equ_Board,key0,On_C_Board}};
	


void Plus_Sign()
{
	numberOne_f += numberTwo;
}

void Sub_Sign()
{
	numberOne_f -= numberTwo;
}

void Did_Sign()
{
	if(numberTwo == 0.0)
		{
			inf = true;
			numberOne_f = 0.0;
			numberTwo = 0.0;
		}
		else
		{
			numberOne_f /= numberTwo;
		}
}

void Mul_Sign()
{
	numberOne_f *= numberTwo;
}

void Equ_Sign()
{
	
}

typedef void ( *VoidFunPtrSign )(uint8_t* ,float* );

const VoidFunPtrSign masSign[5] = {Plus_Sign, Sub_Sign, Did_Sign, Mul_Sign, Equ_Sign};
	
uint8_t makeDispVal(uint8_t num)
{
	return( numb[ num ] );
}

uint8_t getLength(int32_t num)
{
	if(num == 0) return 1;
	
	uint8_t result = 0;
	
	while( num != 0)
	{
		num /= 10;
		result++;
	}
	
	return result;
}

// модуль числа
int32_t mod(int32_t num)
{
	return num < 0 ? num * (-1) : num;
}

// Возводит 10 в степень pw
int32_t pow_dec(uint8_t pw)
{
	uint8_t counter = 1;
	int32_t num = 10;
	while(pw != counter)
	{
		num *= 10;
		counter++;
	}
	
	return pw == 0 ? 1 : num;
}

// заполняет массив dispVal[]
void makeDisplayValue()
{	
	int32_t numberOne_i = numberOne_f;
	uint16_t digitsAfterPoint = mod((numberOne_f - numberOne_i) * 1000);
	
	if(writeTo == NumOne)
	{
		// при делении на 0
		if(inf)
		{
			dispVal[0] = 255;
			dispVal[1] = i;
			dispVal[2] = n;
			dispVal[3] = f;
		}
		else
		{
			uint8_t lenNumberOne = getLength(numberOne_i);
			
			if(numberOne_f < 0)
			{
				if(lenNumberOne < 4)
				{
					if(digitsAfterPoint == 0)
					{
						for(uint8_t j = 0; j < lenNumberOne; j++)
						{
							dispVal[3-j] = makeDispVal(mod(numberOne_i/pow_dec(j)%10));
						}
						
						dispVal[3-lenNumberOne] = minus;
					}
					else
					{
						for(uint8_t j = 0; j < lenNumberOne; j++)
						{
							dispVal[lenNumberOne-j] = makeDispVal(mod(numberOne_i/pow_dec(j)%10));
						}
						
						dispVal[lenNumberOne] &= Point;
						
						for(uint8_t j = 0; j < 4-lenNumberOne; j++)
						{
							dispVal[lenNumberOne+j+1] = makeDispVal(digitsAfterPoint/pow_dec(getLength(digitsAfterPoint)-j-1)%10);
						}
						
						dispVal[0] = minus;
					}
				}
				else
				{
					dispVal[3] = makeDispVal(lenNumberOne-1);
					dispVal[2] = e;
					dispVal[1] = makeDispVal(mod(numberOne_i/pow_dec(lenNumberOne-1)%10));
					dispVal[0] = minus;
				}
			}
			else
			{
				if(lenNumberOne <= 4)
				{
					if(digitsAfterPoint == 0)
					{
						for(uint8_t j = 0; j < lenNumberOne; j++)
						{
							dispVal[3-j] = makeDispVal(numberOne_i/pow_dec(j)%10);
						}
					}
					else
					{
						for(uint8_t j = 0; j < lenNumberOne; j++)
						{
							dispVal[lenNumberOne-j-1] = makeDispVal(numberOne_i/pow_dec(j)%10);
						}
						
						dispVal[lenNumberOne-1] &= Point;
						
						for(uint8_t j = 0; j < 4-lenNumberOne; j++)
						{
							dispVal[lenNumberOne+j] = makeDispVal(digitsAfterPoint/pow_dec(getLength(digitsAfterPoint)-j-1)%10);
						}
					}
				}
				else
				{	
					dispVal[3] = makeDispVal(lenNumberOne-1);
					dispVal[2] = e;
					dispVal[1] = makeDispVal(numberOne_i/pow_dec(lenNumberOne-1)%10);
					dispVal[0] = 255;
				}
			}
		}
	}
	// отображается число из numberTwo
	else
	{
		uint16_t numberTwo_i = numberTwo;
		uint8_t lenNumberTwo = getLength(numberTwo_i);

		for(uint8_t j = 0; j < lenNumberTwo; j++)
		{
			dispVal[3-j] = makeDispVal(numberTwo_i/pow_dec(j)%10);
		}
	}
}

// прерывания по переполнению таймера ТСС0
ISR(TCC0_OVF_vect)
{
		DrawZnakomesto();

		keyCounter++;
		if(keyCounter > keyDelay)
		{
			isCanPressKey = true;
			keyCounter = 0;
		}
			
		if(isKeyPres())
		{
			if(isCanPressKey)
			{
				isCanPressKey = false;
				pressKey = true;
				keyCounter = 0;
			}
	
			powerCounter = 0;
		}
		else
		{
			powerCounter++;
			if(powerCounter > powerStop)
			{
				powerCounter = 0;
				keyCounter = 0;
				//power = false; включить мод сна
			}
		}
}

//работает по умолчанию от генератора 2 MHz 
void init_timer() 
{
	cli();

	TCC0.PER = 40;
	TCC0.CTRLA |= TC_CLKSEL_DIV256_gc; 
	// 2 MHz / 255 / 40 ~= 195 Hz
	
	// установили низкий уровень прерывания
	TCC0.INTCTRLA |=  TC_OVFINTLVL_LO_gc;
	
	// Разрешили низкий уровень прерывания	
	PMIC.CTRL |=  PMIC_LOLVLEN_bm; 
	
	sei();
}

int main(void)
{
	PORTD.DIR = 0b00000001; 
	PORTD.OUT = 0b00000000;	
	
	PORTE.DIR = 0b00001111; 
	PORTE.OUT = 0b00000001;	
	
	PORTE.PIN4CTRL = PORT_OPC_PULLDOWN_gc;
	PORTE.PIN5CTRL = PORT_OPC_PULLDOWN_gc;
	PORTE.PIN6CTRL = PORT_OPC_PULLDOWN_gc;
	PORTE.PIN7CTRL = PORT_OPC_PULLDOWN_gc;

    PORTC.DIR = 0b11111111; 
	
	init_timer();

	// запоминает нужное действие (+-=:*)
	uint8_t sign = 0; 

	while (1) 
    {
		if(pressKey)
		{
			uint8_t col = getColumn();
			uint8_t row = getRow();
			
			// отсеяли помехи  
			if(row < 4 && col < 4) 
			{
				if(writeTo == NumOne)
				{
					keyBoard[col][row]( &sign, &numberOne_f);
				}
				else
				{
					keyBoard[col][row]( &sign, &numberTwo);
				}
				
				makeDisplayValue();
			}
			pressKey = false;
		}//if pressKey
	}
}

