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

extern tm datahora;
extern time_t dataemsegundos;

class comRel
{
public:

	String ultimoIP = "0.0.0.0";
	IPAddress qualIP;

	const uint32_t setentaAnos = 2208988800UL; //1900-1970 (setentaAnos)
	const time_t epoch2000 = 946692000UL; //local se gmt  946684800
	const time_t epoch2000gmt = 946684800UL;
	const time_t epoch2000_TZ = 946699200UL;

	comRel();
		
	bool iniciaWIFI(String ssid, String password, bool DEBUGANDO);
	bool conectaWIFI(String Qualrede, String Qualsenha, bool DEBUGANDO);
	void desconectaWIFI(bool DEBUGANDO);
	bool pegaDataHoraNTP(bool DEBUGANDO);
	void AconfigTime(const char* server1, const char* server2, const char* server3);
	bool AgetLocalTime(tm* datahora, uint32_t ms, bool DEBUGANDO);
	time_t fazTimeStamp(bool DEBUGANDO);
	void iniciaRelogioRTCSys(bool okWiFi, bool DEBUGANDO);
	bool iniciaRelogioSys(time_t epochData, tm* datahora, bool DEBUGANDO);
	bool atualizaRTC(bool DEBUGANDO);
	bool atualizaRTCEpoch(bool DEBUGANDO);
	void fazTimeStrNTP(tm *datahora, char* resultado);
	void fazTimeStrNextion(tm* datahora, char* resultado);
	void fazTimeStrRTC(tmElements_t *tmreg, char* resultado);
	

private:
	const char* _ntpServer1 = "c.ntp.br";
	const char* _ntpServer2 = "b.ntp.br";
	const char* _ntpServer3 = "a.ntp.br";

	const char _diaS_nome[8][4] = {
	"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"
	};

	void _dbgMostraDados(String mensagem, int valor);
};
extern comRel WifiRelog;
#endif // !comRel_h