/*-------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - projekt
					Projekt Gra w kosci
					autor: Mateusz Furgala
					wersja: 17.01.2024r.
----------------------------------------------------------------------------*/

#include "frdm_bsp.h"
#include "MKL05Z4.h"                    // Device header
#include "i2c.h"
#include "ADC.h"
#include "lcd1602.h"
#include "klaw.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
volatile uint8_t S2_press=0;
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;

char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
uint8_t wynik_ok=0;
uint16_t temp;
float wynik;

//Funkcja obslugujaca przerwania przetwornika AC
void ADC0_IRQHandler()
{	
	temp = ADC0->R[0];										// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)													// Sprawdz, czy wynik skonsumowany przez petle glówna
	{
		wynik = temp;												// Wyslij nowa dana do petli glównej
		wynik_ok=1;
	}
		ADC0->SC1[0] |= ADC_SC1_ADCH(8);		// Wyzwolenie programowe przetwornika ADC0 w kanale 8
}

// Podprogram obslugi przerwania od klawiszy S2, S3 i S4
void PORTA_IRQHandler(void)	
{
	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK | S4_MASK);

	
	switch(buf)
	{
									
		case S2_MASK:	DELAY(10)
									if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgan zestykó·Š		
										{
										if(!(PTA->PDIR&S2_MASK))	// Minimalizacja drgan zestykó· ¨c.d.)
										{
											if(!S2_press)
											{
												S2_press=1;
												S3_press=0;
												S4_press=0;
											}
										}
									}
									break;
	
		case S3_MASK:	DELAY(10)
									if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgan zestykó·Š					
										{
										if(!(PTA->PDIR&S3_MASK))	// Minimalizacja drgan zestykó· ¨c.d.)
										{
											if(!S3_press)
											{
												S3_press=1;
												S2_press=0;
												S4_press=0;
											}
										}
									}
									break;
	
		case S4_MASK:	DELAY(10)
									if(!(PTA->PDIR&S4_MASK))		// Minimalizacja drgan zestykó·Š						
										{
										if(!(PTA->PDIR&S4_MASK))	// Minimalizacja drgan zestykó· ¨c.d.)
										{
											if(!S4_press)
											{
												S4_press=1;
												S3_press=0;
												S2_press=0;
											}
										}
									}
									break;
		default:			break;
	}	
	PORTA->ISFR |=  S2_MASK | S3_MASK | S4_MASK;	// Kasowanie wszystkich bitó· ‰SF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}


//Wyswietlania kosci z wskaznikiem na konkretna kosc
void displayDiceCursor(uint8_t dice[], uint8_t reroll[], uint8_t c_pos){
	char cur_table[5];
	
	for(uint8_t i = 0; i < 5; i++){  //Sprawdzanie pozycji wskaznika i ustawianie parametrów do wyswietlenia
		if(reroll[i] == 1){
			if(c_pos != i){
				cur_table[i] = 0x21;
			}
			else{
				cur_table[i] = 0x24; 
			}
		}
		else if(c_pos == i){
			cur_table[i] = 0x2A;
		}
		else{
			cur_table[i] = ' ';
		}
	}
	
	LCD1602_ClearAll();	
	sprintf(display,"Dice: %d %d %d %d %d ", dice[0], dice[1], dice[2], dice[3], dice[4]);		//Wyswietlenie stanu kosci
	LCD1602_Print(display);
	LCD1602_SetCursor(0,1);

    sprintf(display,"      %c %c %c %c %c", cur_table[0], cur_table[1], cur_table[2], cur_table[3], cur_table[4]);		//Wyswietlenie odpowiednich wskazników które zostaly wczesniej ustawione

	LCD1602_Print(display);
	DELAY(5);
}
//Wyswietlenie koncowego rzutu i wynik
void displayDiceEND(uint8_t dice[]) {
	LCD1602_ClearAll();
	sprintf(display,"WYNIK: %d %d %d %d %d", dice[0], dice[1], dice[2], dice[3], dice[4]);		//Wyswietlenie koncowego stanu kosci
	LCD1602_Print(display);
	LCD1602_SetCursor(0,1);
	LCD1602_Print("RESTART with S4");
	LCD1602_SetCursor(0,0);
}

//Podprogram obslugi rzutu koscmi
void rolDice(uint8_t dice[], uint8_t reroll[]){
	//pobranie seed z czujnika AC mierzacego szumy "powietrza"
	uint16_t seed = (int)wynik;
	
	srand(seed);
	
	for (uint8_t i = 0; i < 5; i++){
		if(reroll[i] == 1){
			dice[i] = (rand() % 6) + 1; 		//Rzucanie konkretynmi koscmi
		}
	}
	
	//Zerowanie tablicy
	for (uint8_t i = 0; i < 5; i++) {
        reroll[i] = 0;
    }
}


int main() {
	uint8_t	kal_error;
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{
		while(1);									// Klaibracja sie nie powiodla
	}
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);

	//Petla glówna programu
	while(1){
		Klaw_Init();
		Klaw_S2_4_Int();
		LCD1602_Init();	
		LCD1602_Backlight(TRUE);  	
		LCD1602_ClearAll();	
		LCD1602_Print("  Press S1 to");		//Wyswietlanie startowego okna
		LCD1602_SetCursor(0,1);			
		LCD1602_Print("      START ");	
		LCD1602_SetCursor(0,0);
		
		uint8_t dice[5] = {1, 1, 1, 1, 1};	  //Tablica kosci
		uint8_t reroll[5] = {1, 1, 1, 1, 1};  //Flaga do rzutu koscia (tak/nie)
		uint8_t selectedDie = 0;
		uint8_t rollCount = 0;
		
		while(PTA->PDIR&S1_MASK){};  	// Czy klawisz S1 wcisniety? (oczekiwanie na wcisniecie klawisza)

		//Po kliknieciu S1 kosci maja sie rzucic
		rolDice(dice, reroll);
		rollCount++;
			
		//Petla konkretnej rozgrywki
		while(1) {
			
			displayDiceCursor(dice, reroll, selectedDie);
			
			if(rollCount == 3){
				displayDiceEND(dice);
				break;
			}
			
			while(1){
				
				if(S2_press)							
				{

					if(selectedDie < 4){
						selectedDie++;
					}
					else if(selectedDie == 4){
						selectedDie = 0;
					}
					
					S2_press=0;
					break;
				}
				if(S3_press)							
				{
					if(reroll[selectedDie] == 1){
						reroll[selectedDie] = 0;
					}
					else{
						reroll[selectedDie] = 1;
					}
					
					S3_press=0;
					break;
				}
				if(S4_press)							
				{
					rolDice(dice, reroll);
					
					rollCount++;
					S4_press=0;
					break;
				}
			}

		}
		//Restart rozgrywki
		while(1){
			if(S4_press)							
			{
				rollCount = 0;
				S4_press=0;
				break;
			}
		}
	}
}