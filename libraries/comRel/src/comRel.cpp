/*
comRel.cpp
Biblioteca de rotinas de comunicação Wifi e do relógio
chamar como "WifiRelog"
software para ESP32
mario
*/

#include "comRel.h"

comRel::comRel() {}

bool comRel::iniciaWIFI(String ssid, String password, bool DEBUGANDO) {
	
	bool stsWIFI = false;
	bool pegouHorarioNTP = false;
	stsWIFI = conectaWIFI(ssid, password, DEBUGANDO);
	if (stsWIFI) {
		qualIP = WiFi.localIP();
		if (DEBUGANDO) {
			Serial.print("QualIP: ");
			Serial.println(qualIP);
		}
		pegouHorarioNTP = pegaDataHoraNTP(DEBUGANDO);
		desconectaWIFI(DEBUGANDO);
		stsWIFI = false;
		if (pegouHorarioNTP) {
			return true;
		}else {
			return false;
		}
	}else {
		if (DEBUGANDO) Serial.println("NAO TEM WIFI P/PEGAR DATA");	//DEBUG
		return false;
	}
}

bool comRel::conectaWIFI(String ssid, String password, bool DEBUGANDO) {

	int espera = 0;
	char* ssid_char = new char[ssid.length() + 1];
	char* password_char = new char[password.length() + 1];
	strcpy(ssid_char, ssid.c_str());
	strcpy(password_char, password.c_str());
	if (DEBUGANDO) Serial.printf("Conectando com %s ", ssid_char); //para debug
	WiFi.begin(ssid_char, password_char);
	while ((WiFi.status() != WL_CONNECTED) && (espera < 10)) {
		delay(500);
		if (DEBUGANDO) Serial.print(".");				//para debug
		espera += 1;
	}
	if (espera < 10) {
		if (DEBUGANDO) Serial.println(" CONECTADO");	//para debug
		return true;
	}else {
		if (DEBUGANDO) Serial.println(" NAO TEM WIFI");	//para debug
		return false;
	}
	delete[] ssid_char;
	delete[] password_char;
	ssid_char = NULL;
	password_char = NULL;
}

void comRel::desconectaWIFI(bool DEBUGANDO) {			//desconecta WiFi
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	if (DEBUGANDO) Serial.println(" DESCONECTADO");	//para debug
}

//================================================
//conecta a um servidor NTP, lê data-hora (ref 1900)
// seta time zone (fuso horário)
//e atualiza relogio do sistema, 
//e depois lê horário do sistema (no debug)
//
bool comRel::pegaDataHoraNTP(bool DEBUGANDO) {

	//INICIA E PEGA HORA-DATA formato NTP - 1900
	AconfigTime(_ntpServer1, _ntpServer2, _ntpServer3);
	setenv("TZ", "BRST+3BRDT+2,M10.3.0,M2.3.0", 1);
	tzset();

	if (AgetLocalTime(&datahora, 1000, DEBUGANDO)) {
		if (DEBUGANDO) {
			time(&dataemsegundos);
			_dbgMostraDados("Ano: ", datahora.tm_year);
			_dbgMostraDados("dataemsegundos: ", dataemsegundos);
			Serial.println(" PEGOU DATA CERTA DO NTP");	//para debug
		}
		return true;
	}else {
		if (DEBUGANDO) _dbgMostraDados("NÃO PEGOU CERTO DO NTP. ", 0);//para debug
		return false;
	}
}

//=================================================================
//define os servidores NTP (relógio global), conecta na rede
// e lê horário global
//
void comRel::AconfigTime(const char* server1, const char* server2, const char* server3){
	
	if (sntp_enabled()) {
		sntp_stop();
	}
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, (char*)server1);
	sntp_setservername(1, (char*)server2);
	sntp_setservername(2, (char*)server3);
	sntp_init();
}

//==============================================================
// lê data-hora do sistema (ref 1900 - NTP)
//
bool comRel::AgetLocalTime(tm* datahora, uint32_t ms, bool DEBUGANDO) {

	uint32_t count = ms / 10;
	time(&dataemsegundos);
	localtime_r(&dataemsegundos, datahora);

	if (datahora->tm_year > (1999 - 1900)) { //porque meu default é 01/01/2000
		
		return true;
	}
	while (count--) {
		delay(10);
		time(&dataemsegundos);
		localtime_r(&dataemsegundos, datahora);
		if (datahora->tm_year > (1999 - 1900)) {//porque meu default é 01/01/2000
			
			return true;
		}
	}
	if (DEBUGANDO) _dbgMostraDados("Erro-AgetLocalTime: ", (datahora->tm_year + 1900));
	return false;
}

//==========================================================
time_t comRel::fazTimeStamp(bool DEBUGANDO) {
	bool flgLocal;
	tm datalocal;
	flgLocal = AgetLocalTime(&datalocal, 10, DEBUGANDO);
	return mktime(&datalocal);

}

//====================================
//inicia relogio do RTC e do Sistema
//
void comRel::iniciaRelogioRTCSys(bool okWiFi, bool DEBUGANDO) {

	tmElements_t registros;

	bool _TemDataRTC = false;
	bool _stsAtualizaRTC = false;
	bool _stsAtualizaSysEpoch = false;
	bool _stsAtualizaRTCEpoch = false;

	if (okWiFi) {								// pegou data-hora NTP-> acerta RTC
		_stsAtualizaRTC = atualizaRTC(DEBUGANDO);
		_TemDataRTC = relogio.read(registros);
		if (DEBUGANDO) _dbgMostraDados("atualizaRTC-registros.Year +1970: ", (registros.Year + 1970));
	}else {										//não pegou data-hora rede-NTP-sem wifi
		if (DEBUGANDO) _dbgMostraDados("Não pegou data NTP ou não tem rede", 0);
		unsigned char estaRodando = 0;
		int contaseg = 4;
		relogio.begin();

		while (contaseg && !estaRodando) {	//vê se rtc está rodando
			estaRodando = relogio.isRunning();
			delay(500);
			contaseg--;
		}
		
		_TemDataRTC = relogio.read(registros);//verifica se tem hora no RTC DS1307
		
		if (estaRodando) {
			if (DEBUGANDO) _dbgMostraDados("RTCRodando-registros.Year+1970: ", (registros.Year + 1970));
		}else {
			if (DEBUGANDO) _dbgMostraDados("RTC NaoEstáRODANDO: ", 0);
		}

		if (!_TemDataRTC) {						//não tem wifi e RTC não tem hora				
			_stsAtualizaRTCEpoch = atualizaRTCEpoch(DEBUGANDO); //vai olhar RTC se OK, se sim 2000
			_stsAtualizaSysEpoch = iniciaRelogioSys(epoch2000gmt, &datahora, DEBUGANDO); // seta sistema p/2000
		}else {									//RTC tem hora então só programa sistema
			time_t horaDataRTC;
			horaDataRTC = relogio.makeTimeRTC(registros);
			//horaDataRTC = horaDataRTC + setentaAnos; // muda de unix - NTP -> 1970 - 1900
			_stsAtualizaSysEpoch = iniciaRelogioSys(horaDataRTC, &datahora, DEBUGANDO);
		}
	}
}

//==============================================
//inicia manualmente relogio do sistema e 
//seta time zone.
//
bool comRel::iniciaRelogioSys(time_t epochData, tm* datahora, bool DEBUGANDO) {
	
	time_t now;
	time_t provEpoch = epochData; // é NTP 1900
	//inicia manualmente com timezone em greenwich (zero)
	struct timeval tms = { provEpoch, 0 };
	settimeofday(&tms, NULL);
	setenv("TZ", "GMT0", 1);
	tzset();
	delay(1000);

	if (DEBUGANDO) _dbgMostraDados(" Atualizou Relogio Sistema ", 0);

	if (DEBUGANDO) {
		time(&now);
		localtime_r(&now, datahora); // mostra hora local
		char mostra [30];
		sprintf(mostra, "%.2d/%.2d/%.2d %.2d:%.2d:%.2d",
			datahora->tm_mday,
			((datahora->tm_mon) + 1),
			datahora->tm_year + 1900,		//NTP
			datahora->tm_hour,
			datahora->tm_min,
			datahora->tm_sec
		);
		Serial.println(mostra);
		_dbgMostraDados("MostraRelogSistema-EPOCH-LOCAL", 0);
	}

	if (DEBUGANDO) {
		time(&now);
		gmtime_r(&now, datahora); //mostra hora GMT - sem timezone (zero)
		char mostra [30];
		sprintf(mostra, "%.2d/%.2d/%.2d %.2d:%.2d:%.2d",
			datahora->tm_mday,
			((datahora->tm_mon) + 1),
			datahora->tm_year + 1900,		//NTP
			datahora->tm_hour,
			datahora->tm_min,
			datahora->tm_sec
		);
		Serial.println(mostra);
		_dbgMostraDados("MostraRelogSistema-EPOCH-GMT", 0);
	}

	if (datahora->tm_year > (1999 - 1900)) { //NTP
		return true;
	}else {
		return false;
	}
}

//===============================================
bool comRel::atualizaRTC(bool DEBUGANDO) {

	if (DEBUGANDO) _dbgMostraDados(" VAI atualizar DS1307 ", 0);

	tmElements_t rascrtc;
	struct tm rascsys;
	time_t dataemsegundos;

	relogio.begin();
	bool leu = false;
	
	time(&dataemsegundos);
	localtime_r(&dataemsegundos, &rascsys); // data em NTP 1900
	rascrtc.Day = rascsys.tm_mday;
	rascrtc.Hour = rascsys.tm_hour;
	rascrtc.Minute = rascsys.tm_min;
	rascrtc.Month = rascsys.tm_mon + 1;
	rascrtc.Second = rascsys.tm_sec;
	rascrtc.Year = rascsys.tm_year - 70; // Muda diferença de 1900 para 1970
	dataemsegundos = relogio.makeTimeRTC(rascrtc); // data em UnixEpoch 1970
	
	leu = relogio.set(dataemsegundos);
	delay(100);
	
	if (!leu) {
		if (DEBUGANDO) Serial.println("ERRO ESCRITA-DS1307");
		return false;
	}
	if (DEBUGANDO) Serial.println("ATUALIZOU-DS1307");
	return true;
}

//==============================================
bool comRel::atualizaRTCEpoch(bool DEBUGANDO) {
	_dbgMostraDados(" VAI atualizar DS1307-Epoch2000gmt ", 0);
	relogio.begin();
	bool leu = false;
	leu = relogio.set(epoch2000gmt); //epoch2000gmt em Unix-epoch-1970
	delay(100);
	if (!leu) {
		if (DEBUGANDO) Serial.println("ERRO ESCRITA-DS1307-Epoch2000gmt");
		return false;
	}
	if (DEBUGANDO) Serial.println("ATUALIZOU-DS1307-Epoch2000gmt");
	return true;
}


//===============================================
void comRel::fazTimeStrNextion(tm* datahora, char* resultado) {

	sprintf(resultado, "%4d-%02d-%02d %02d:%02d",
		1900 + datahora->tm_year,	//NTP
		datahora->tm_mon + 1,
		datahora->tm_mday,
		datahora->tm_hour,
		datahora->tm_min
	);
}

//===============================================
void comRel::fazTimeStrNTP(tm* datahora, char* resultado) {

	sprintf(resultado, "%4d-%02d-%02d %3s %02d:%02d:%02d",
		1900 + datahora->tm_year,	//NTP
		datahora->tm_mon + 1,
		datahora->tm_mday,
		_diaS_nome[datahora->tm_wday],
		datahora->tm_hour,
		datahora->tm_min,
		datahora->tm_sec
	);
}

//===============================================
void comRel::fazTimeStrRTC(tmElements_t *tmreg, char* resultado) {

	sprintf(resultado, "%4d-%02d-%02d  %3s %02d:%02d:%02d",
		1970 + tmreg->Year,		//UNIX - Epoch
		tmreg->Month,
		tmreg->Day,
		_diaS_nome[tmreg->Wday - 1],
		tmreg->Hour,
		tmreg->Minute,
		tmreg->Second
	);
}

//========= private
void comRel::_dbgMostraDados(String mensagem, int valor) {
	Serial.print(mensagem);
	Serial.print(": ");
	Serial.println(valor);
	Serial.println("=====");
}

//=======================================================
//não cria instância
//=======================================================
//comRel wifiRelog;