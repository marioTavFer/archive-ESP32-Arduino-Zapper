/*****************************************************//**
 * @file	C:\Users\mario\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\comRel\src\comRel.h.
 *
 * @brief	definicao da classe comRel que chamei (instancia) de WfifRelog
 *			tem rotinas do RTC e relogio-sistema e conecta com Internet-Wifi
 *********************************************************/
/*
comRel.h
Biblioteca de rotinas de comunicação Wifi e do relógio
chamar como "WifiRelog"
software para ESP32
Mario
*/

#ifndef comRel_h
#define comRel_h

#include <arduino.h>
#include <wiFi.h>
//#include "apps/sntp/sntp.h"//só no arduino-esp32 v1.0.0 ou <
#include "lwip/apps/sntp.h" 
#include <time.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <Wire.h>
#include <DS1307RTC.h>

/** @brief	The datahora */
extern tm datahora;
/** @brief	The dataemsegundos */
extern time_t dataemsegundos;

/*****************************************************//**
 * @class	comRel comRel.h
 * 			C:\Users\mario\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\comRel\src\comRel.h
 *
 * @brief	A com relative.
 *
 * @date	18/12/2019
 *********************************************************/
class comRel
{
public:

	/** @brief	The ultimo IP */
	String ultimoIP = "0.0.0.0";
	/** @brief	The qual IP */
	IPAddress qualIP;

	/** @brief	1900-1970 (setentaAnos) */
	const uint32_t setentaAnos = 2208988800UL;
	/** @brief	local se gmt  946684800 */
	const time_t epoch2000 = 946692000UL;
	/** @brief	The epoch 2000gmt */
	const time_t epoch2000gmt = 946684800UL;
	/** @brief	The epoch 2000 tz */
	const time_t epoch2000_TZ = 946699200UL;

	/*****************************************************//**
	 * @fn	comRel::comRel();
	 *
	 * @brief	Default constructor
	 *
	 * @date	18/12/2019
	 *********************************************************/
	comRel();

	/*****************************************************//**
	 * @fn	bool comRel::iniciaWIFI(String ssid, String password, bool DEBUGANDO);
	 *
	 * @brief	Inicia WiFi
	 *
	 * @date	18/12/2019
	 *
	 * @param 	ssid	 	The SSID.
	 * @param 	password 	The password.
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool iniciaWIFI(String ssid, String password, bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	bool comRel::conectaWIFI(String Qualrede, String Qualsenha, bool DEBUGANDO);
	 *
	 * @brief	Conecta WiFi
	 *
	 * @date	18/12/2019
	 *
	 * @param 	Qualrede 	The qualrede.
	 * @param 	Qualsenha	The qualsenha.
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool conectaWIFI(String Qualrede, String Qualsenha, bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	void comRel::desconectaWIFI(bool DEBUGANDO);
	 *
	 * @brief	Desconecta WiFi
	 *
	 * @date	18/12/2019
	 *
	 * @param 	DEBUGANDO	True to debugando.
	 *********************************************************/
	void desconectaWIFI(bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	bool comRel::pegaDataHoraNTP(bool DEBUGANDO);
	 *
	 * @brief	Pega data hora ntp
	 *
	 * @date	18/12/2019
	 *
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool pegaDataHoraNTP(bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	void comRel::AconfigTime(const char* server1, const char* server2, const char* server3);
	 *
	 * @brief	Aconfig time
	 *
	 * @date	18/12/2019
	 *
	 * @param 	server1	The first server.
	 * @param 	server2	The second server.
	 * @param 	server3	The third server.
	 *********************************************************/
	void AconfigTime(const char* server1, const char* server2, const char* server3);

	/*****************************************************//**
	 * @fn	bool comRel::AgetLocalTime(tm* datahora, uint32_t ms, bool DEBUGANDO);
	 *
	 * @brief	Aget local time
	 *
	 * @date	18/12/2019
	 *
	 * @param [in,out]	datahora 	If non-null, the datahora.
	 * @param 		  	ms		 	The milliseconds.
	 * @param 		  	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool AgetLocalTime(tm* datahora, uint32_t ms, bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	time_t comRel::fazTimeStamp(bool DEBUGANDO);
	 *
	 * @brief	Faz time stamp
	 *
	 * @date	18/12/2019
	 *
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	A time_t.
	 *********************************************************/
	time_t fazTimeStamp(bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	void comRel::iniciaRelogioRTCSys(bool okWiFi, bool DEBUGANDO);
	 *
	 * @brief	Inicia relogio RTC system
	 *
	 * @date	18/12/2019
	 *
	 * @param 	okWiFi   	True to ok WiFi.
	 * @param 	DEBUGANDO	True to debugando.
	 *********************************************************/
	void iniciaRelogioRTCSys(bool okWiFi, bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	bool comRel::iniciaRelogioSys(time_t epochData, tm* datahora, bool DEBUGANDO);
	 *
	 * @brief	Inicia relogio system
	 *
	 * @date	18/12/2019
	 *
	 * @param 		  	epochData	Information describing the epoch.
	 * @param [in,out]	datahora 	If non-null, the datahora.
	 * @param 		  	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool iniciaRelogioSys(time_t epochData, tm* datahora, bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	bool comRel::atualizaRTC(bool DEBUGANDO);
	 *
	 * @brief	Atualiza RTC
	 *
	 * @date	18/12/2019
	 *
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool atualizaRTC(bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	bool comRel::atualizaRTCEpoch(bool DEBUGANDO);
	 *
	 * @brief	Atualiza RTC epoch
	 *
	 * @date	18/12/2019
	 *
	 * @param 	DEBUGANDO	True to debugando.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 *********************************************************/
	bool atualizaRTCEpoch(bool DEBUGANDO);

	/*****************************************************//**
	 * @fn	void comRel::fazTimeStrNTP(tm *datahora, char* resultado);
	 *
	 * @brief	Faz time string ntp
	 *
	 * @date	18/12/2019
	 *
	 * @param [in,out]	datahora 	If non-null, the datahora.
	 * @param [in,out]	resultado	If non-null, the resultado.
	 *********************************************************/
	void fazTimeStrNTP(tm *datahora, char* resultado);

	/*****************************************************//**
	 * @fn	void comRel::fazTimeStrNextion(tm* datahora, char* resultado);
	 *
	 * @brief	Faz time string nextion
	 *
	 * @date	18/12/2019
	 *
	 * @param [in,out]	datahora 	If non-null, the datahora.
	 * @param [in,out]	resultado	If non-null, the resultado.
	 *********************************************************/
	void fazTimeStrNextion(tm* datahora, char* resultado);

	/*****************************************************//**
	 * @fn	void comRel::fazTimeStrRTC(tmElements_t *tmreg, char* resultado);
	 *
	 * @brief	Faz time string RTC
	 *
	 * @date	18/12/2019
	 *
	 * @param [in,out]	tmreg	 	If non-null, the tmreg.
	 * @param [in,out]	resultado	If non-null, the resultado.
	 *********************************************************/
	void fazTimeStrRTC(tmElements_t *tmreg, char* resultado);
	

private:
	/** @brief	The first ntp server */
	const char* _ntpServer1 = "c.ntp.br";
	/** @brief	The second ntp server */
	const char* _ntpServer2 = "b.ntp.br";
	/** @brief	The third ntp server */
	const char* _ntpServer3 = "a.ntp.br";

	/** @brief	The dia s nome[ 8][4] */
	const char _diaS_nome[8][4] = {
	"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"
	};

	/*****************************************************//**
	 * @fn	void comRel::_dbgMostraDados(String mensagem, int valor);
	 *
	 * @brief	Debug mostra dados
	 *
	 * @date	18/12/2019
	 *
	 * @param 	mensagem	The mensagem.
	 * @param 	valor   	The valor.
	 *********************************************************/
	void _dbgMostraDados(String mensagem, int valor);
};
/** @brief	The WiFirelog */
extern comRel WifiRelog;
#endif // !comRel_h