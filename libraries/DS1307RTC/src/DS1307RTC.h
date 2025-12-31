/*
 * DS1307RTC.h - library for DS1307 RTC
 * modificada por mim Mario
 */
#ifndef DS1307RTC_h
#define DS1307RTC_h
//mario
#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
//=================================================================================
//Estrutura de dados para o DS1307 (registros)
typedef struct {
	uint8_t Second; //00-59
	uint8_t Minute;	//00-59
	uint8_t Hour;	//00-23
	uint8_t Wday;   // domingo dia 1
	uint8_t Day;	//01-31
	uint8_t Month;	//01-12
	uint8_t Year;   //00-99 offset de 1970 - Unix (não pode ser de 1900-NTP)
} 	tmElements_t;
/*==============================================================================*/
/* constantes */
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_WEEK * 52UL))
#define SECS_YR_2000  ((time_t)(946684800UL)) // horário no ano 2000
//==============================================================================
/* variáveis do fuso e horário de verão*/
// tem que ser dentro de rotina
//timezone tz;
//tz.tz_minuteswest = 0;
//tz.tz_dsttime = 0;





// library interface description
class DS1307RTC
{
private:
	static uint8_t dec2bcd(uint8_t num);
	static uint8_t bcd2dec(uint8_t num);
  public:
	DS1307RTC();
    void begin(void);
    static time_t get();					//pega do DS1307 (time_t)
    static bool set(time_t t);				//escreve no DS1307 (time_t)
    static bool read(tmElements_t &tmreg);	//pega struct DS1307 (tmElements_t)
    static bool write(tmElements_t &tmreg);	//escreve struct DS1307(tmElements)
	static unsigned char isRunning();		// ve se está rodando relógio
	bool readRam(uint8_t registro, uint8_t qtosBytes);//lê ram 
	bool writeRam(uint8_t registro, uint8_t qtosBytes);

	/* ========== funções de conversão ==================*/
	// quebra time_t em tmElements_t
	static void breakTimeRTC(time_t time, tmElements_t &tmreg);
	// converte tmElements_t em time_t
	static time_t makeTimeRTC(tmElements_t &tmreg);  
 };
extern DS1307RTC relogio;
#endif
 

