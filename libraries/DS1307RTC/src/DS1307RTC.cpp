/*
  DS1307RTC.cpp - library for DS1307 RTC
    mudei mario
*/
#include <arduino.h>
#include <Wire.h>
#include <DS1307RTC.h>
//==========================================================
#define DS1307_ADD	0x68
#define inicioRAM	0x08
#define tamanhoRAM	56

// leap year calculator - years offset from 1970
#define Bissexto(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
// API starts months from 1
static  const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
const uint32_t seventyYears = 2208988800UL;

//==============================================================================
//	área de variáveis da RAM do DS1307  - 08h -> 3Fh  (56 x 8)
//==============================================================================
char ramArea[55];//56 BYTES	

//=========================================================
DS1307RTC::DS1307RTC(){}

// PUBLIC FUNCTIONS
//===================================================
void DS1307RTC::begin(void)
{
	Wire.begin();
}
//=================================================
// get -> read -> make
//retorna time_t (unix)
//=================================================
time_t DS1307RTC::get(){
  tmElements_t tmreg;
  if (read(tmreg) == false) return 0;
  return(makeTimeRTC(tmreg));
}
//================================================
// set - > break -> write
//retorna true se OK
//================================================
bool DS1307RTC::set(time_t t){
  tmElements_t tmreg;
  breakTimeRTC(t, tmreg);
  return write(tmreg); 
}
// ====================================================
// lê RTC - DS1307 e preenche estrutura tmElements_t
//retorna true se OK
//=====================================================
bool DS1307RTC::read(tmElements_t &tmreg){
	//offset 1970
  uint8_t sec;
  Wire.beginTransmission(DS1307_ADD);
  Wire.write((uint8_t)0x00); 
  if (Wire.endTransmission() != 0) return false;
  
   // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
  int tmNbrFields = 7;
  Wire.requestFrom(DS1307_ADD, tmNbrFields);
  
  if (Wire.available() < tmNbrFields) return false;
  sec = Wire.read();
  tmreg.Second = bcd2dec(sec & 0x7f);   
  tmreg.Minute = bcd2dec(Wire.read());
  tmreg.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  tmreg.Wday = bcd2dec(Wire.read());
  tmreg.Day = bcd2dec(Wire.read());
  tmreg.Month = bcd2dec(Wire.read());
  tmreg.Year = bcd2dec(Wire.read());

  if (sec & 0x80) return false; // clock is halted
  return true;
}
//==========================================================
// acerta o relógio RTC-DS1307 usando estrutura tmElements_t
//===========================================================
bool DS1307RTC::write(tmElements_t &tmreg)
{
// offset 1970
//não uso timezone no RTC
//	timezone tz;
//	int fuso = tz.tz_minuteswest = 0;
//	int verao = tz.tz_dsttime = 0;
int	fuso = 0;
int verao = 0;
//
  Wire.beginTransmission(DS1307_ADD);
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write((uint8_t)0x80); // Stop the clock. The seconds will be written last
  Wire.write(dec2bcd(tmreg.Minute));
  //bit 6 do "hour" deve ser 0 para relogio 24horas
  Wire.write(dec2bcd(tmreg.Hour + (fuso/3600) + (verao/3600)));//não uso timezone/fuso
  Wire.write(dec2bcd(tmreg.Wday));   
  Wire.write(dec2bcd(tmreg.Day));
  Wire.write(dec2bcd(tmreg.Month));
  Wire.write(dec2bcd(tmreg.Year));  // base (-1970) Unix
  if (Wire.endTransmission() != 0) {
      return false;
  }
  // Now go back and set the seconds, starting the clock back up as a side effect
  Wire.beginTransmission(DS1307_ADD);
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write(dec2bcd(tmreg.Second)); // write the seconds, with the stop bit clear to restart
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return true;
}
//========================================================
//vê se relogio funcionando - RTC DS1307
//retorna 1 se OK
//========================================================
unsigned char DS1307RTC::isRunning(){
  Wire.beginTransmission(DS1307_ADD);
  Wire.write((uint8_t)0x00); 
  Wire.endTransmission();
  // Just fetch the seconds register and check the top bit
  Wire.requestFrom(DS1307_ADD, 1);
  return !(Wire.read() & 0x80);
}
//================================================================
//RTC - pega unix time, monta estrutura tmElements_t  e retorna
//================================================================
void DS1307RTC::breakTimeRTC(time_t timeInput, tmElements_t &tmreg) {
	
	// offset 1970 !!!
	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t)timeInput;
	tmreg.Second = time % 60;
	time /= 60; //  minutes
	tmreg.Minute = time % 60;
	time /= 60; //  hours
	tmreg.Hour = time % 24;
	time /= 24; //  days
	tmreg.Wday = ((time + 4) % 7) + 1;  // domingo dia 1 

	year = 0;
	days = 0;
	while ((unsigned)(days += (Bissexto(year) ? 366 : 365)) <= time) {
		year++;
	}

	tmreg.Year = year; // offset from 1970 
	days -= Bissexto(year) ? 366 : 365;
	time -= days; // days in this year, starting at 0
	days = 0;
	month = 0;
	monthLength = 0;

	for (month = 0; month < 12; month++) {
		if (month == 1) { // february
			if (Bissexto(year)) {
				monthLength = 29;
			}else {
				monthLength = 28;
			}
		}else {
			monthLength = monthDays[month];
		}
		if (time >= monthLength) {
			time -= monthLength;
		}else {
			break;
		}
	}
	tmreg.Month = month + 1;  // jan is month 1  
	tmreg.Day = time + 1;     // day of month
}
//===================================================
// pega a estrutura tmElements_t e retorna tempo Unix
//===================================================
time_t DS1307RTC::makeTimeRTC(tmElements_t &tmreg) {
	
	//  offset 1970 
	int i;
	uint32_t seconds;
	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds = tmreg.Year*(SECS_PER_DAY * 365);
	for (i = 0; i < tmreg.Year; i++) {
		if (Bissexto(i)) {
			seconds += SECS_PER_DAY;   // add extra days for leap years
		}
	}
	// add days for this year, months start from 1
	for (i = 1; i < tmreg.Month; i++) {
		if ((i == 2) && Bissexto(tmreg.Year)) {
			seconds += SECS_PER_DAY * 29;
		}else {
			seconds += SECS_PER_DAY * monthDays[i - 1];  //monthDay array starts from 0
		}
	}
	seconds += (tmreg.Day - 1) * SECS_PER_DAY;
	seconds += tmreg.Hour * SECS_PER_HOUR;
	seconds += tmreg.Minute * SECS_PER_MIN;
	seconds += tmreg.Second;
	return ((time_t)seconds);
}
//============================================================
//		Lê e Escreve em RAM (08h-3Fh) - 56 bytes do relógio
//============================================================
bool DS1307RTC::readRam(uint8_t registro, uint8_t qtosBytes) {
	
	uint8_t registroHex;
	registroHex= dec2bcd(registro);
	Wire.beginTransmission(DS1307_ADD);
	Wire.write((uint8_t)registroHex);
	if (Wire.endTransmission() != 0) return false;
	int qtosLista = qtosBytes;
	Wire.requestFrom(DS1307_ADD, qtosLista);
	int i = 0;
	for (i = 0; i < qtosLista; i++){
		ramArea[i] = Wire.read();
	}
	return true;
}
//============================================================
bool DS1307RTC::writeRam(uint8_t registro, uint8_t qtosBytes) {

	uint8_t registroHex;
	registroHex = dec2bcd(registro);
	Wire.beginTransmission(DS1307_ADD);
	Wire.write((uint8_t)registroHex);
	if (Wire.endTransmission() != 0) return false;
	int qtosLista = qtosBytes;
	int i = 0;
	for (i = 0; i < qtosLista; i++) {
		 Wire.write(ramArea[i]);
	}
	return true;
}
//============================================================
// Funções Privadas
//=======================================================
// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS1307RTC::dec2bcd(uint8_t num) {
	return ((num / 10 * 16) + (num % 10));
}
//=======================================================
// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS1307RTC::bcd2dec(uint8_t num) {
	return ((num / 16 * 10) + (num % 16));
}
//=======================================================
//cria instância relógio
//=======================================================
DS1307RTC relogio;

