/*===================================================
---teste do pulse sensor, RTC, ds1307, Banco de dados, etc.
Signal :    entrada analogica leitura a cada 2mS.
IBI  :      "interbeat interval" intervalo entre batidas resolução de 2 ms.
BPM  :      média de batidas por minuto calculada sobre as 10 ultimas.
QS  :       TRUE qdo um  Pulso é identificado e a BPM é atualizada. tem que zerar na aplicação.
Pulse :     TRUE qdo detetado uma batida.
pegando data e hora na rede, servidores:
a.ntp.br 200.160.0.8 e 2001:12ff::8
b.ntp.br 200.189.40.8
c.ntp.br 200.192.232.8
=======================================================*/

#include <arduino.h>
#include <Nextion.h>
#include <WiFi.h>
//#include "apps/sntp/sntp.h" //só no arduino-esp32 v1.0.0 ou <
#include "lwip/apps/sntp.h" 
#include <time.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <EEPROM.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <comRel.h>
//======== SQLite3
#include <shox96_0_2.h>
#include <config_ext.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

//--------------- PARA DEBUGAR -------------------------------
bool DEBUGANDO = true;
//dimensoes area util do display nextion - waveform
const int WIDTH = 301;
const int HEIGHT = 140;
//uso qdo debugando msgs no nexHardware.cpp
//char buffer_rasc[20];
//int tam_cmd;

//-----constantes e variaveis referentes a wifi e relógio-data ------

typedef struct {
	byte temRede; //0xAA tem rede
	char ssid[20];
	char senha[20];
} redeWifi;

String ssid_00 = "2casa2008";				//<= no C++ converter
String password_00 = "yedachecchia220958";	//string para char* dá warning
char leSsid[20];						//
char leSenha[20];
IPAddress ultimoIP;

/*static char mes_nome[13][4] = {
	"Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
	"Jul", "Ago", "Set", "Out", "Nov", "Dez"
};*/
//===================================================================
/*Estrutura do relógio do sistema
struct tm
{
  int  tm_sec; //0-60
  int tm_min;//0-59
  int tm_hour;//0-23
  int tm_mday;//1-31
  int tm_mon;//0-11
  int tm_year;//desde 1900
  int tm_wday;//0-6 domingo=0
  int tm_yday;//0-365
  int tm_isdst;//horário verão
#ifdef __TM_GMTOFF
  long  __TM_GMTOFF;
#endif
#ifdef __TM_ZONE
  const char *__TM_ZONE;
#endif
}
//================================
//Estrutura de dados para o DS1307 (registros)
typedef struct {
	uint8_t Second; //00-59
	uint8_t Minute;	//00-59
	uint8_t Hour;	//00-23
	uint8_t Wday;   // domingo dia 1
	uint8_t Day;	//01-31
	uint8_t Month;	//01-12
	uint8_t Year;   //00-99 offset de 1970-Unix(não pode ser de 1900-NTP)
} 	tmElements_t;
//===================================================================
*/
tmElements_t tmreg; //estrutura de 8 bits para o DS1307
struct tm datahora; //estrutura 32 bits do esp32
time_t dataemsegundos; // int de 32 bits esp32

long  gmtOffset_sec = 0; //-10800 menos 3 horas de greenwich/londres(usando "TZ")
int   daylightOffset_sec = 0; //3600 horário de verão (usando "TZ")
bool HorarioVerao = true;

// *** cria uma instância das funções wifiRelog (objeto) ***
//==================================================================
comRel wifiRelog;

//==================================================================
//variáveis do pulse sensor e outras temporizações
int X = 0;
long tempoantes = 0;
long tempoagora = 0;
long tempoantes2 = 0;
long tempoantes3 = 0;
long tempoantes4 = 0;

//==================================================================
//variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile uint8_t Signal2;
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

//==================================================================
//variável para ver ID do chip - basicamente end.MAC(6bytes)
uint64_t chipid;

//==================================================================
//colocado aqui include do pulse porque usa varáveis definidas acima

#include <PulseSensor_Interrup_ML.h>

//==================================================================
// variáveis de telas, usuários e operação

bool emPrograma = false; // vem do zapper V8.0
bool devTimeout = false;
int	operacaoAtiva = 0;

bool usuarioLogado = false;
int usuarioAtivo = 0;
int	usuarioNivel = 0;
String usuarioNome;

int pacienteAtivo = 0;
String pacienteNome;

int telaAnterior = 0;
int telaAtual = 0;

// variaveis para montar listas/rel na tela, paginadas X/5
typedef struct {
	bool pagTem;
	int pagAtual;
	int pagTotal;
	int Resto;
	int Offset;
	int nLinhas;
}stsRelatorio;


stsRelatorio logOperacao;

bool mostraMais = false;
bool mostraMenos = false;

//===================================================================
//			 variaveis do SQLite3 e SPIFFS
//===================================================================
//Só deve formatar SPIFFS uma vez (na primeira vez) eu não uso/formato

#define FORMAT_SPIFFS_IF_FAILED true
sqlite3			*banco01 = NULL;
sqlite3_stmt	*stmt = NULL;
char			*zErrMsg = 0;
char			*sql_query = (char*)"";
int rc = 0;
File root;
File arquivo;
bool retorno_bd = true;
bool ok_spiffs = true;

//==================================================================
//mensagens sqlite3 - operação

String buf_mostra;//mensagem na tela menu manutenção

enum stsMsg {nao_tem, tem_nova, ja_mandou, limpa_msg};
stsMsg temMsgManut;
//int temMsgManut = 0; //0-não(nada),1-tem(manda),2-já mandou(nada),3-limpa msg

bool voltaTelaAcesso = false;

//==================================================================
//constantes sqlite3 - operacao

//lista-numeracao das telas
const int nr_telaLogin = 0;
const int nr_menuManut = 1;
const int nr_telaPulso = 2;
const int nr_telaSensores = 3;
const int nr_telaTabUsu = 6;
const int nr_telaTabPac = 7;
const int nr_telaTabOpe = 8;
const int nr_telaTabTpTra = 9;
const int nr_telaArqLog = 10;
const int nr_telaWifi = 11;
const int nr_telaRelogioRTC = 12;
const int nr_menuTestes = 15;
const int nr_telaArqOp = 16;
const int nr_telaStatus = 17;

//Registro sqlite3 - Tabela de operacoes
const int regLogin = 1;
const int regLogoff = 2;
const int regAcertoRelogio = 3;
const int regTratamento = 4;
const int regCadPaciente = 5;
const int regEdPaciente = 6;
const int regDelPaciente = 7;
const int regTratManual = 8;
const int regErroLogin = 10;
bool sts_regOp = true;

//==================================================================
//Definição das Telas do Nextion

NexPage tela_login = NexPage(0, 0, "login");// tela de login da maquina
NexPage menu_Manut = NexPage(1, 0, "menuManut");// tela de Manutenção
NexPage tela_pulso = NexPage(2, 0, "pulso00");//gráfico - batidas
NexPage tela_sensores = NexPage(3, 0, "sensores");//campos para olhar variáveis e RTC
NexPage tela_tabUsu = NexPage(6, 0, "usuarios");// tabela de usuarios
NexPage tela_tabPac = NexPage(7, 0, "pacientes");// tabela de pacientes
NexPage tela_tabOpe = NexPage(8, 0, "tabOperacoes");// tabela de operações
NexPage tela_tabTpTra = NexPage(9, 0, "tiposTratament");// tabela Tipos tratamenot
NexPage tela_arqLog = NexPage(10, 0, "arqLog");// registro/Log dos tratamentos
NexPage tela_wifi = NexPage(11, 0, "wifi");// acertar manual/te ssid-senha
NexPage tela_relogioRTC = NexPage(12, 0, "relogio");// acertar manual/te relógios
NexPage menu_testes = NexPage(15, 0, "testes");// tela do Menu de Testes
NexPage tela_arqOp = NexPage(16, 0, "arqOp");// tela registro/log operações
NexPage tela_status = NexPage(17, 0, "status");// tela de status da máquina
// tela de login
NexButton bt_login_ok = NexButton(0, 6, "login.b0");//botão ok
NexText usuario_login = NexText(0, 4, "login.t3");//campo do usuário
NexText senha_login = NexText(0, 5, "login.t4");//campo da senha
NexText hora_login = NexText(0, 8, "login.t5");//campo de hora data
//da pagina 1 - menu manutencao
NexText hora_manut = NexText(1, 14, "menuManut.t9");//campo de hora e data
NexText msg_manut = NexText(1, 15, "menuManut.t10");//campo mensagem olá
NexVariable nivelUs_manut = NexVariable(1, 10, "menuManut.nivelUs"); //indica se >5 ADM
NexVariable emEd_manut = NexVariable(1, 11, "menuManut.emEd"); //indica em edicao
NexVariable permiteEd_manut = NexVariable(1, 9, "menuManut.permiteEd"); //indica permite edicao
NexText logoff_manut = NexText(1, 17, "menuManut.t12"); //click logoff e vai p/ tela login
//da página 2 - tela do Pulso
NexWaveform grafico_pulso = NexWaveform(2, 1, "pulso00.s0");
NexNumber bpm_pulso = NexNumber(2, 3, "pulso00.n0");//BPM-batidas por minuto
NexNumber ibi_pulso = NexNumber(2, 4, "pulso00.n1");//IBI-miliseg entre batidas
NexNumber amp_pulso = NexNumber(2, 5, "pulso00.n2");//amp-amplitude da batida
//da página 3 - Sensores
//chip - RGB (conferir ******)
NexNumber rgb_lux = NexNumber(3, 6, "sensores.n0"); //rgb-lux
NexNumber rgb_chn0 = NexNumber(3, 12, "sensores.n1");//lux-ch0
NexNumber rgb_chn1 = NexNumber(3, 13, "sensores.n2");//lux-ch1
NexNumber rgb_temp = NexNumber(3, 20, "sensores.n3");//rgb-temperatura
NexNumber rgb_R = NexNumber(3, 21, "sensores.n4");//rgb-R
NexNumber rgb_G = NexNumber(3, 22, "sensores.n5");//rgb-G
NexNumber rgb_B = NexNumber(3, 23, "sensores.n6");//rgb-B
//chip RTC e sistema
NexText rtc_local = NexText(3, 7, "sensores.t5");//rtc-local
NexText rtc_DS1307 = NexText(3, 8, "sensores.t6");//rtc-ds1307
NexText rtc_rede = NexText(3, 9, "sensores.t7");//rtc-rede
//chip - LUX
NexText lux_lux = NexText(3, 10, "sensores.t8");//lux-lux
NexText lux_watt = NexText(3, 11, "sensores.t9");//lux-w/cm2
//página 12 - Relógio - entrada manual data e hora
NexText hora_relog = NexText(12, 26, "relogio.t17");//campo->mostra hora/sistema
NexNumber rtcHoras = NexNumber(12, 9, "relogio.n0"); //horas 
NexNumber rtcMinutos = NexNumber(12, 10, "relogio.n1"); //minutos
NexNumber rtcSegundos = NexNumber(12, 11, "relogio.n2"); //segundos
NexNumber rtcDia = NexNumber(12, 12, "relogio.n3"); //Dia
NexNumber rtcMes = NexNumber(12, 13, "relogio.n4"); //Mês
NexNumber rtcAno = NexNumber(12, 14, "relogio.n5"); //Ano (4)
NexButton acerta_rtc = NexButton(12, 4, "relogio.b0");//botão manda acertar data-hora
//página 11 - wifi: SSID SENHA e Geral
NexText ssid_wifi = NexText(11, 60, "wifi.t1");//ssid
NexText senha_wifi = NexText(11, 61, "wifi.t2");//senha
NexButton acerta_Ssid = NexButton(11, 75, "wifi.b61");//botão manda acertar ssid senha
NexText msg_wifi = NexText(11, 59, "wifi.t0");//campo para mensagens
//página 16 - Registros/log de Operação
NexText infoLog_arqOp = NexText(16, 24, "arqOp.t20");
NexButton btMais_arqOp = NexButton(16, 16, "arqOp.b3");
NexButton btMenos_arqOp = NexButton(16, 15, "arqOp.b2");
NexText data01_arqOp = NexText(16, 3, "arqOp.t13");
NexText data02_arqOp = NexText(16, 4, "arqOp.t14");
NexText data03_arqOp = NexText(16, 5, "arqOp.t15");
NexText data04_arqOp = NexText(16, 6, "arqOp.t16");
NexText data05_arqOp = NexText(16, 7, "arqOp.t17");

NexText usu01_arqOp = NexText(16, 19, "arqOp.t1");
NexText usu02_arqOp = NexText(16, 20, "arqOp.t9");
NexText usu03_arqOp = NexText(16, 21, "arqOp.t10");
NexText usu04_arqOp = NexText(16, 22, "arqOp.t18");
NexText usu05_arqOp = NexText(16, 23, "arqOp.t19");

NexText op01_arqOp = NexText(16, 11, "arqOp.t2");
NexText op02_arqOp = NexText(16, 10, "arqOp.t4");
NexText op03_arqOp = NexText(16, 12, "arqOp.t5");
NexText op04_arqOp = NexText(16, 13, "arqOp.t6");
NexText op05_arqOp = NexText(16, 14, "arqOp.t7");
// pagina 17 - Tela de Status da maquina
NexText pacientes_status = NexText(17, 11, "status.t10");
NexText usuarios_status = NexText(17, 12, "status.t11");
NexText tpTratam_status = NexText(17, 13, "status.t12");
NexText numTratam_status = NexText(17, 14, "status.t13");
NexText logTratam_status = NexText(17, 15, "status.t14");
NexText logOperacoes_status = NexText(17, 16, "status.t15");
NexText tamBancosDados_status = NexText(17, 17, "status.t16");
NexText wifiSSID_status = NexText(17, 18, "status.t17");
NexText IPmaquina_status = NexText(17, 19, "status.t18");


//NexText p10t8 = NexText(3, 78, "page10.t8");//geral 1
//NexText p10t9 = NexText(3, 79, "page10.t9");//geral 2
//NexText p10t10 = NexText(3, 80, "page10.t10");//geral 3
//NexButton p10BtAcertaGeral = NexButton(3, 81, "page10.b62");//botão manda acertar geral

NexTouch *nex_listen_list[] =
{
//botões
	&acerta_rtc,
	&acerta_Ssid,
	&bt_login_ok,
	&logoff_manut,
	&btMais_arqOp,
	&btMenos_arqOp,
// telas
	&tela_login,
	&menu_Manut,
	&tela_pulso,
	&tela_sensores,
	&tela_tabUsu,
	&tela_tabPac,
	&tela_tabOpe,
	&tela_tabTpTra,
	&tela_arqLog,
	&tela_wifi,
	&tela_relogioRTC,
	&menu_testes,
	&tela_arqOp,
	&tela_status,
//	&p10BtAcertaGeral,
	NULL
};

//=====================================================
//definição dos construtores
//=====================================================

//==== Nextion ======================================================

void iniciaNextion();

// === operacao =====================================================

void LePrintDataHoraTela();
void veQualTela();
void ativaTela(int antes, int atual);
void limpaTelaAcesso();
void pegaSsidSenha();
bool veSeTemIDwifi();

// === botoes - clicks ==============================================

void acerta_rtcPopCallback(void *ptr);
void acerta_SsidPopCallback(void *ptr);
void bt_login_okPopCallback(void *ptr);
void logoff_manutPopCallback(void *ptr);
void btMais_arqOpPopCallback(void *ptr);
void btMenos_arqOpPopCallback(void *ptr);

// === Nextion - telas ==============================================

void tela_loginPopCallback(void *ptr);
void menu_ManutPopCallback(void *ptr);
void tela_pulsoPopCallback(void *ptr);
void tela_sensoresPopCallback(void *ptr);
void tela_tabUsuPopCallback(void *ptr);
void tela_tabPacPopCallback(void *ptr);
void tela_tabOpePopCallback(void *ptr);
void tela_tabTpTraPopCallback(void *ptr);
void tela_arqLogPopCallback(void *ptr);
void tela_wifiPopCallback(void *ptr);
void tela_relogioRTCPopCallback(void *ptr);
void menu_testesPopCallback(void *ptr);
void tela_arqOpPopCallback(void *ptr);
void tela_statusPopCallback(void* ptr);

// === Debug/auxiliar para imprimir data-hora (string)===============

void LePrintDataHoraNTP();
void LePrintDataHoraRTC();

// ==== debug, pulso, harware, chip ID ==============================

void dbgMostraDados(String mensagem, int valor);
void dbgMostraStr(String mensagem, String qualString);
void LeSensorPulso();
uint64_t leChipID();

// ==== SQLite3 =====================================================

void iniciaTesteBD();
bool iniciaSpiffs();
bool abreDiretorio();
bool iniciaSQLite3();
bool fechaSQLite3();
void msgSqlStatus(int posicao, int stsSQL);

// ==== SQLite3 - Operação =========================================

bool verificaUsuario();		//LOGIN
void logoffUsuario();		//LOGOFF
bool registraOperacao(int qual);
void erroLogin();
void vaiMostraLogOp();
bool listaLogOperacao();
bool lista_arqOp();
void display_linhaROp(int linha_tela, String data_rasc, String usuario_rasc, String operacao_rasc);
void zeraVariaveisListaROp();
void mostraStatus();
bool RegistrosN(String QualSelect, NexText QualCampo);

// ==== FIM construtores ===========================================

//===============================================
//		SETUP DO SISTEMA
//===============================================
void setup() {

	nexInit();//mudei nexSerial para 57600
	delay(10);
	iniciaNextion();
//inicializa serial para debug-terminal
	if (DEBUGANDO) {
		Serial.begin(115200);
		delay(50);
	}

// zera variaveis da geracao de relatorios
	zeraVariaveisListaROp();

// iniciar-testar os bancos de dados -----------------------------
	if (DEBUGANDO) {
		iniciaTesteBD();
	}else {
		ok_spiffs = iniciaSpiffs();
	}
	//falta fazer o tratamento de ocorrer ERRO ao abrir SPIFFS
	//ai não vai ter banco de dados......
	
//---------------------------------------------------------
// verifica EEPROM se tem ssid e senha lá - executa uma vez

	bool okRedeID;
	okRedeID = veSeTemIDwifi(); //CUIDADO falta limitar strings a 20 chars
	if (!okRedeID) Serial.println("ERRO Begin EEPROM");
	
//-------------------------------------------------------	
//conecta WiFi-pega data-NTP e desconecta
	bool okWiFi;
	okWiFi = wifiRelog.iniciaWIFI(ssid_00, password_00, DEBUGANDO);
	
	//**** debugando FORÇADO - duas variáveis a ZERO
	//if(DEBUGANDO){		//imaginando sem wifi-sem NTP
	//	okWiFi = false;
	//	pegouHorarioNTP = false;
	//}
//--------------------------------------------------------
//inicia o RTC se pegou data NTP
	wifiRelog.iniciaRelogioRTCSys(okWiFi, DEBUGANDO);
	if (DEBUGANDO) LePrintDataHoraNTP();
	if (DEBUGANDO) LePrintDataHoraRTC();
	if (DEBUGANDO) Serial.println("*** SETUP DONE ***");			//debug
//-------------------------------------------------------	
//Le identificacao do chip - o MAC address
	leChipID();

	//================ FIM SETUP===========================
}
//==============================================================
//						 INICIO DO LOOP 
//==============================================================

void loop() {
	nexLoop(nex_listen_list);
	tempoagora = millis();

	//LeSensorPulso();

	if (tempoagora - tempoantes3 > 30000) { //printa relogio cada 30seg
		tempoantes3 = tempoagora;			
		if (DEBUGANDO) {
			LePrintDataHoraNTP();//mostra hora do sistema
			LePrintDataHoraRTC();//mostra hora do RTC
		}
	}
	if (tempoagora - tempoantes4 > 1000) { // mostra hora/data por seg
		tempoantes4 = tempoagora;			//em telas específicas
		veQualTela();
	}
	if (logOperacao.pagTem){ //se tela Log Operacao
		vaiMostraLogOp();	//vai mostrar +5 registros de log
	}


	//delay(10);                             //  take a break
}
// ================== Fim do LOOP ===================


//==============================================
void LePrintDataHoraNTP(){ // só utilizada em DEBUG "manual" c/ if DEBUGANDO

	char* mostraData = new char[30];
	//char *mostraData[30];
	bool retHora = false;
	retHora = wifiRelog.AgetLocalTime(&datahora, 1000, DEBUGANDO);
	if (retHora) {
		wifiRelog.fazTimeStrNTP(&datahora, mostraData);
		Serial.print("Hora NTP: ");
		Serial.println(mostraData);
		//rtc_local.setText(mostraData);
	}else {
		if (DEBUGANDO) dbgMostraDados("Loop-não peguei hora-noSistema", 0);
	}
	delete[] mostraData;
	mostraData = NULL;
}
//===============================================
void LePrintDataHoraRTC(){ // só utilizada em DEBUG "manual" c/ if DEBUGANDO
	
	char* mostraData = new char[30];
	tmElements_t tmreg;
	if (relogio.read(tmreg)) {
		wifiRelog.fazTimeStrRTC(&tmreg, mostraData);
		Serial.print("Hora RTC: ");
		Serial.println(mostraData);
		//rtc_DS1307.setText(mostraData);
	}else {
		if (DEBUGANDO) 	dbgMostraDados(" ERRO DS1307 - de leitura. ", 0);
	}
	delete[] mostraData;
	mostraData = NULL;
}
//==============================================
void dbgMostraDados (String mensagem, int valor){
	Serial.print (mensagem);
	Serial.print (": ");
	Serial.println(valor);
	Serial.println("=====");
}
//==============================================
void dbgMostraStr(String mensagem, String qualString) {
	Serial.print(mensagem);
	Serial.print(": ");
	Serial.println(qualString);
	Serial.println("=====");
}
//====================================================================
// Lê ID do chip, na realidade lê end. MAC
uint64_t leChipID() {
	chipid = ESP.getEfuseMac();
	if (DEBUGANDO) {
		Serial.println("ID do chip:");
		Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));//High 2 bytes
		Serial.printf("%08X\n", (uint32_t)chipid);//Low 4bytes
		Serial.println(" ");
	}
	return chipid;
}
//====================================================================
//inicializa, prepara lê EEPROM
bool veSeTemIDwifi() {
  
	redeWifi olhaWifiID;
	if (!EEPROM.begin(900)) return false;
	EEPROM.get(10, olhaWifiID);
	if (olhaWifiID.temRede == 0xAA) {
		ssid_00 = olhaWifiID.ssid;
		password_00 = olhaWifiID.senha;
		if (DEBUGANDO) Serial.println("PEGOU da EEPROM-ssid-senha");
		return true;
	}else {
		
		strcpy(olhaWifiID.ssid, ssid_00.c_str());
		strcpy(olhaWifiID.senha, password_00.c_str());
		olhaWifiID.temRede = 0xAA;
		EEPROM.put(10, olhaWifiID);
		EEPROM.commit();
		if (DEBUGANDO) Serial.println("Gravado na EEPROM-ssid-senha");
		return true;
	}
 
}
//====================================================================
bool gravaWifiEEPROM(String rede_nome, String rede_pwd) {
  
  
	redeWifi olhaWifiID;
	strcpy(olhaWifiID.ssid, rede_nome.c_str());
	strcpy(olhaWifiID.senha, rede_pwd.c_str());
	olhaWifiID.temRede = 0xAA;
	EEPROM.put(10, olhaWifiID);
	EEPROM.commit();
	if (DEBUGANDO) Serial.println("Gravada NOVA REDE na EEPROM");
	return true;

}
//====================================================================
// Lê sensor de pulsação cardiaca
void LeSensorPulso(){
	if ((tempoagora - tempoantes) >= 2) {	//milisec
		tempoantes = tempoagora;
		lepulso2mili();
	}
	if (tempoagora - tempoantes2 > 10) {	//milisec
		tempoantes2 = tempoagora;
		grafico_pulso.addValue(0, Signal2);
		if (QS) {
			uint32_t valor = long(BPM);
			bpm_pulso.setValue(valor);
			ibi_pulso.setValue(IBI);
			amp_pulso.setValue(amp);
			QS = false;
		}
	}
}
//=======================================================================
//
//			ROTINAS BANCOS DE DADOS E OPERAÇÃO
//			COM TABELAS, USUÁRIOS, BANCOS DE DADOS, ETC.
//
//=======================================================================

//--------------------------------------------------
//inicia-teste do spiffs e BD-sqlite3
void iniciaTesteBD() {
	if (iniciaSpiffs()) {
		if (DEBUGANDO) Serial.println("Abriu do SPIFFS");
	}else {
		if (DEBUGANDO) Serial.println("erro na abertura do SPIFFS");
	}
	// testa spiffs (abrindo arquivo)
	if (DEBUGANDO) retorno_bd = abreDiretorio();

	//teste sqlite3
	retorno_bd = iniciaSQLite3();
	if (!retorno_bd) {
		if (DEBUGANDO) Serial.println("erro na abertura inicia_sql3");
	}else {
		if (DEBUGANDO) Serial.println("OK na abertura inicia_sql3");
		retorno_bd = fechaSQLite3();
		if (!retorno_bd) {
			if (DEBUGANDO) Serial.println("erro no fechamento fecha_sql3");
		}else {
			if (DEBUGANDO) Serial.println("OK no fechamento fecha_sql3");
		}
	}
}
//-------------------------------------------------
//inicia SPIFFS
bool iniciaSpiffs() {
	//if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
	if (!SPIFFS.begin()) {
		if (DEBUGANDO) Serial.println("Falhou montar sistema arquivos");
		return false;
	}
	if (DEBUGANDO) Serial.println("Montou sistema arquivos");
	return true;
}
//-----------------------------------------------------
// abre diretorio e lista arquivos (só quando DEBUGANDO)
bool abreDiretorio() {
	root = SPIFFS.open("/");
	if (!root) {
		Serial.println("- falhou em abrir diretorio");
		return false;
	}
	if (!root.isDirectory()) {
		Serial.println(" -não é diretorio");
		return false;
	}
	arquivo = root.openNextFile();
	while (arquivo) {
		if (arquivo.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(arquivo.name());
		}else {
			Serial.print("  Arquivo: ");
			Serial.print(arquivo.name());
			Serial.print("\tSIZE: ");
			Serial.println(arquivo.size());
		}
		arquivo = root.openNextFile();
	}
	return true;
}
//------------------------------------------------
//		Inicia SQLite3 (teste tbem fecha sqllite3)
bool iniciaSQLite3() {

	sqlite3_initialize();
	rc = sqlite3_open_v2("/spiffs/bd01.db", &banco01, SQLITE_OPEN_READWRITE,NULL);
	if (rc != SQLITE_OK) {
		sqlite3_close_v2(banco01);
		if (DEBUGANDO) Serial.println("  Erro ao abrir bd01.db"); //erro de sql
		return false;
	}else {
		if (DEBUGANDO) Serial.println("  Abriu bd01.db"); //abriu BD
		return true;
	}
}
//----------------------------------------
//     Fecha SQLite3
bool fechaSQLite3() {

	rc = sqlite3_close_v2(banco01);
	if (rc == SQLITE_OK) {
		if (DEBUGANDO) Serial.println("  Fechou bd01.db"); //Fechou BD
	}else {
		if (rc == SQLITE_BUSY) {
			if (DEBUGANDO) Serial.println("  Deu Sqlite3/bd01 - BUSY");
			return false;
		}else {
			if (DEBUGANDO) Serial.println("  Não fechou Sqlite3/bd01");
			return false;
		}
	}
	//rc = sqlite3_shutdown();
	//if ((rc == SQLITE_OK) && DEBUGANDO) Serial.println(" Fez Shutdown do Sqlite3"); //Fechou BD
	return true;
}
//================================================================
//rotina de verificacao de usuario-LOGIN
bool verificaUsuario() {

	char usuarioTxtLogin[30] = { 0 };
	char usuarioTxtSenha[30] = { 0 };
	uint16_t tamanho_campo = 30;

	int usuarioID_bco;
	String usuarioLogin_bco;
	String usuarioSenha_bco;
	String usuarioNome_bco;
	int usuarioNivel_bco = 0;
	bool ret_bd = true;//status-retorno do inicia-sqlite3 é bool
	
	usuario_login.getText(usuarioTxtLogin, tamanho_campo);
	senha_login.getText(usuarioTxtSenha, tamanho_campo);
	String SusuarioTxtLogin = usuarioTxtLogin;
	String SusuarioTxtSenha = usuarioTxtSenha;
	
	if (DEBUGANDO) {
		Serial.print("Login Usuario:");
		Serial.println(SusuarioTxtLogin);
		Serial.print("Senha Usuario:");
		Serial.println(SusuarioTxtSenha);
	}

	ret_bd = iniciaSQLite3();
	if (!ret_bd) {
		if (DEBUGANDO) msgSqlStatus(1, !ret_bd);				//msg-sql 01
		return false; //erro ao abrir BD, caiu fora
	}
		
	const char *sql = "SELECT * FROM usuarios";
	rc = sqlite3_prepare_v2(banco01, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		if (DEBUGANDO) msgSqlStatus(2, rc);			//msg-sql 02- erro prepare_v2
		sqlite3_finalize(stmt);
		fechaSQLite3();
		return false;
	}
	
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		usuarioID_bco = sqlite3_column_int(stmt, 0);
		usuarioLogin_bco = (const char*)sqlite3_column_text(stmt, 1);
		usuarioSenha_bco = (const char*)sqlite3_column_text(stmt, 2);
		usuarioNome_bco = (const char*)sqlite3_column_text(stmt, 3);
		usuarioNivel_bco = sqlite3_column_int(stmt, 4);
		if (DEBUGANDO) {
			Serial.println("ID,Login,Senha,Nome:");
			char buff_prv[10];
			itoa(usuarioID_bco, buff_prv, 10);
			Serial.println(buff_prv);
			Serial.println(usuarioLogin_bco);
			Serial.println(usuarioSenha_bco);
			Serial.println(usuarioNome_bco);
			itoa(usuarioNivel_bco, buff_prv, 10);
			Serial.println(buff_prv);
			Serial.println();
		}
		if (usuarioLogin_bco == SusuarioTxtLogin) {
			if (DEBUGANDO) Serial.println("OK - BATEU LOGIN");
			if (usuarioSenha_bco == SusuarioTxtSenha) {
				usuarioNivel = usuarioNivel_bco;
				usuarioAtivo = usuarioID_bco;
				usuarioNome = usuarioNome_bco;
				usuarioLogado = true;
				if (DEBUGANDO) Serial.println("OK - BATEU Senha");
				break;
			}else {
				if (DEBUGANDO) Serial.println("ERRO - NAO BATEU Senha");
				usuarioAtivo = 0;
				usuarioLogado = false;
			}
		}else {
			if (DEBUGANDO) Serial.println("ERRO - NAO BATEU Login");
			usuarioAtivo = 0;
			usuarioLogado = false;
			
		}
	}
	sqlite3_finalize(stmt);
	ret_bd = fechaSQLite3();
	//não esquecer fechar BD e testar flag

	if (DEBUGANDO) dbgMostraDados("usuarioAtivo: ", usuarioAtivo);
		
	if (usuarioAtivo > 0) {
		buf_mostra = "Bom dia, ";
		buf_mostra = buf_mostra + usuarioNome;
		temMsgManut = tem_nova;//tem msg
		if (DEBUGANDO) Serial.println(buf_mostra);
		limpaTelaAcesso();
		return true;
	}else {
		limpaTelaAcesso();
		char msgerro[] = "erro de Login";
		senha_login.setText(msgerro);
		return false;
	}
}
//====================================================================
//		LOGOFF - de Usuário
void logoffUsuario() {
	
	if (DEBUGANDO) dbgMostraDados("TELA LOGOFF, Operador: ", usuarioAtivo);
	
	sts_regOp = registraOperacao(regLogoff);
	usuarioNivel = 0;
	usuarioAtivo = 0;
	usuarioNome = "";
	usuarioLogado = false;
	
	ativaTela(telaAtual, nr_telaLogin); //vai p/tela login
	limpaTelaAcesso();
}
//====================================================================
//rotina de registro de operacao- data-operador-operacao
bool registraOperacao(int qual) {
	
	bool ini_bd = 0;
	sqlite3_stmt* regop = NULL;
	time(&dataemsegundos);

	ini_bd = iniciaSQLite3();
	if (!ini_bd) return false;

	const char logOpUsu[] = "INSERT INTO log_usuario (data, usuario, operacao) VALUES (?,?,?)";
	rc = sqlite3_prepare_v2(banco01, logOpUsu, -1, &regop, NULL);
	if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(3, rc);					//msg-sql 03
	if (rc != SQLITE_OK) {
		sqlite3_finalize(regop);
		fechaSQLite3();
		return false;
	}
// agora fazer: bind(s) e step, depois finalize e chama fechaSQLite3()
// não esquecer: 
// itens/variáveis do bind, o index inicia em 1
// colunas das tabelas, o index inicia em 0
										//tabela log_usuario
	time_t agoraReg = dataemsegundos;	//coluna 0 (data - datetime);
	int usuarioReg = usuarioAtivo;		//coluna 1 (usuario - integer);
	int operacaoReg = qual;				//coluna 2 (operacao - integer);

	rc=sqlite3_bind_int(regop, 1, agoraReg);
	if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(4, rc);
	if (rc != SQLITE_OK) {
		sqlite3_finalize(regop);
		fechaSQLite3();
		return false;
	}
	rc=sqlite3_bind_int(regop, 2, usuarioReg);
	if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(5, rc);
	if (rc != SQLITE_OK) {
		sqlite3_finalize(regop);
		fechaSQLite3();
		return false;
	}
	rc=sqlite3_bind_int(regop, 3, operacaoReg);
	if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(6, rc);
	if (rc != SQLITE_OK) {
		sqlite3_finalize(regop);
		fechaSQLite3();
		return false;
	}
	rc=sqlite3_step(regop);
	if ((rc != SQLITE_DONE) && DEBUGANDO) msgSqlStatus(7, rc);
	if (rc != SQLITE_DONE) {
		sqlite3_finalize(regop);
		fechaSQLite3();
		return false;
	}
	rc=sqlite3_finalize(regop);
	if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(8, rc);
	if (rc != SQLITE_OK) {
		fechaSQLite3();
		return false;
	}
	//rc=sqlite3_close(banco01);
	//if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(9, rc);
	fechaSQLite3();
	if (DEBUGANDO) Serial.println("Gravou registro de LogOperação");
	return true;
}
//=====================================================================
//mostra log de operacao
void vaiMostraLogOp() {
	if (mostraMais) {
		if ((logOperacao.pagAtual == (logOperacao.pagTotal + 1)) && (logOperacao.Resto == 0)) {
			//não faz nada
		}
		else if (logOperacao.pagAtual == (logOperacao.pagTotal + 2)) {
			//nao faz nada
		}
		else {
			lista_arqOp();
		}
		mostraMais = false;
	}
	else if (mostraMenos) {
		if (logOperacao.pagAtual >= 3) {
			logOperacao.pagAtual -= 2;
			lista_arqOp();
		}
		mostraMenos = false;
	}
}
//=====================================================================
//usando variaveis globais: 
//bool pagRelTem;int pagRelAtual;int pagRelTotal;int pagRelResto;
//lista registros de log operacao, login, logout, etc
bool listaLogOperacao() {

	bool ini_bd = 0;
	sqlite3_stmt* regop = NULL;
	time(&dataemsegundos);

	ini_bd = iniciaSQLite3();

	if (!ini_bd) return false;

//vamos contar numero de linhas
	const char logOpUsu[] = "SELECT count(*) FROM log_usuario";
	rc = sqlite3_prepare_v2(banco01, logOpUsu, -1, &regop, NULL);
	if (rc != SQLITE_OK){
		if(DEBUGANDO) msgSqlStatus(10, rc);
		rc = sqlite3_finalize(regop);
		if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(11, rc);
		rc = sqlite3_close(banco01);							//fecha sqlite3
		if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(11, rc);
		return false;
	}else {
		rc = sqlite3_step(regop);
		if (SQLITE_ROW != rc){								
			if (DEBUGANDO) msgSqlStatus(12, rc);//pode não ser erro, talvez ñ tem registro 
			rc = sqlite3_finalize(regop);
			if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(13, rc);
			fechaSQLite3();
			return false;
		}else {
			logOperacao.nLinhas = sqlite3_column_int(regop, 0);
			if (DEBUGANDO) {
				Serial.print("numero_linhas: ");		// tem N registros no arq.
				Serial.println(logOperacao.nLinhas);
			}
			rc = sqlite3_finalize(regop);
			if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(14, rc);
			fechaSQLite3();
			
			//mostra no nextion quantos registro tem.
			String numeroRegLog = "Numero registros: ";
			char rasStr [10];
			itoa(logOperacao.nLinhas, rasStr,10);
			String ras2Str = String(rasStr);
			numeroRegLog = numeroRegLog + ras2Str;
			char* buffNumReg = new char[numeroRegLog.length() + 1];
			strcpy(buffNumReg, numeroRegLog.c_str());
			infoLog_arqOp.setText(buffNumReg);

			//prepara p/no loop listar linhas
			if (logOperacao.nLinhas == 0) {//não pode ou não deve ser
				zeraVariaveisListaROp();
			}
			if (logOperacao.nLinhas > 5) { //tem mais de 1 página para controlar
				logOperacao.pagTotal = logOperacao.nLinhas / 5;
				logOperacao.Resto = logOperacao.nLinhas % 5;
				logOperacao.pagAtual = 1;
				logOperacao.pagTem = true;
				mostraMais = true;
			}else { // só tem alguns registros (5<=) logo, 1 pag
				logOperacao.pagTotal = 1;
				logOperacao.Resto = logOperacao.nLinhas;
				logOperacao.pagAtual = 0;
				logOperacao.pagTem = true;
				mostraMais = true;
			}
			
			delete[] buffNumReg;
			buffNumReg = NULL;
			return true;
		}
	}
}
//======================================================================
// controla linhas e paginas do registro de Operação para display/nextion
bool lista_arqOp() {
	
	bool ini_bd = 0;
	sqlite3_stmt* regop = NULL;
	time(&dataemsegundos);

	ini_bd = iniciaSQLite3();
	if (!ini_bd) {
		zeraVariaveisListaROp();
		return false;
	}
	
	if ((logOperacao.pagAtual == 0) && (logOperacao.Resto > 0)) { //se zero é porque n.linhas <=5
		const char listaOpUsu[] = "SELECT log_usuario.data, usuarios.nome, tab_operacoes.operacao FROM log_usuario JOIN usuarios ON log_usuario.usuario = usu_id JOIN tab_operacoes ON log_usuario.operacao = ope_id ORDER BY log_usuario.data DESC LIMIT 5";
		rc = sqlite3_prepare_v2(banco01, listaOpUsu, -1, &regop, NULL);
		if (rc != SQLITE_OK) {
			if (DEBUGANDO) msgSqlStatus(15, rc);
			rc = sqlite3_finalize(regop);
			fechaSQLite3();
			zeraVariaveisListaROp();
			return false;
		}else {
			time_t data_rasc;
			char* data_rasc2 = new char[50];
			String data_rasc3;
			tm datahora_r;
			String usuario_rasc;
			String operacao_rasc;
			int linha_tela = 1;
			if (DEBUGANDO) Serial.println("Pega os 5 primeiros regs");
			rc = sqlite3_step(regop);

			do {						//imrime todas linhas até fim do arquivo
				data_rasc = sqlite3_column_int(regop, 0);
				
				localtime_r(&data_rasc, &datahora_r);
				wifiRelog.fazTimeStrNextion(&datahora_r, data_rasc2);
				data_rasc3 = data_rasc2; 
				
				usuario_rasc = (const char*)sqlite3_column_text(regop, 1);
				operacao_rasc = (const char*)sqlite3_column_text(regop, 2);
				display_linhaROp(linha_tela, data_rasc3, usuario_rasc, operacao_rasc);
				
				if (DEBUGANDO) {
					Serial.print("manda linha: ");
					Serial.println(linha_tela);
				}
				linha_tela += 1;
				rc = sqlite3_step(regop);
			} while (rc == SQLITE_ROW);

			delete[] data_rasc2;
			data_rasc2 = NULL;
			rc = sqlite3_finalize(regop);
			fechaSQLite3();
			return true;
		}
	}
	if ((logOperacao.pagAtual > 0) && (logOperacao.pagAtual <= logOperacao.pagTotal+1)) {	//considerando sempre que paginas>1 se chegou aqui
					// lista mais uma página

		if (DEBUGANDO) {
			dbgMostraDados("Offset: ", logOperacao.Offset);
			dbgMostraDados("pagAtual: ", logOperacao.pagAtual);
			dbgMostraDados("pagTotal: ", logOperacao.pagTotal);
		}

		logOperacao.Offset = (logOperacao.pagAtual - 1) * 5;
		
		char rascstr00 [10];
		sprintf(rascstr00, "%d", logOperacao.Offset);
		String rascstr01 = rascstr00;
		String rascstr = "SELECT log_usuario.data, usuarios.nome, tab_operacoes.operacao FROM log_usuario JOIN usuarios ON log_usuario.usuario = usu_id JOIN tab_operacoes ON log_usuario.operacao = ope_id ORDER BY log_usuario.data DESC LIMIT 5 OFFSET ";
		rascstr = rascstr + rascstr01;
		if (DEBUGANDO) {
			Serial.println(rascstr);
			dbgMostraDados("Offset: ", logOperacao.Offset);
		}
		const char* sql = rascstr.c_str();
				
		rc = sqlite3_prepare_v2(banco01, sql, -1, &regop, NULL);
		if (rc != SQLITE_OK) {
			if (DEBUGANDO) msgSqlStatus(16, rc);//
			rc = sqlite3_finalize(regop);
			fechaSQLite3();
			zeraVariaveisListaROp();
			return false;
		}

		//mostra info de pagina mostrada: pag X de N pags
		char buffmsg[30];
		int pagTotal_rasc = 0;
		if (logOperacao.Resto > 0) pagTotal_rasc = logOperacao.pagTotal + 1;
		sprintf(buffmsg, "mostra pagina %d de %d", logOperacao.pagAtual, pagTotal_rasc);
		infoLog_arqOp.setText(buffmsg);

		time_t data_rasc;
		char* data_rasc2 = new char[50];
		String data_rasc3;
		tm datahora_r;
		String usuario_rasc;
		String operacao_rasc;
		int linha_tela = 1;
		if (DEBUGANDO) Serial.println("Pega  5 regs");
		rc = sqlite3_step(regop);

		do {						//imprime todas linhas até fim do arquivo
			data_rasc = sqlite3_column_int(regop, 0);
			localtime_r(&data_rasc, &datahora_r);
			wifiRelog.fazTimeStrNextion(&datahora_r, data_rasc2);

			data_rasc3 = data_rasc2;
			usuario_rasc = (const char*)sqlite3_column_text(regop, 1);
			operacao_rasc = (const char*)sqlite3_column_text(regop, 2);
			display_linhaROp(linha_tela, data_rasc3, usuario_rasc, operacao_rasc);
			linha_tela += 1;
			rc = sqlite3_step(regop);
		} while (rc == SQLITE_ROW);

		delete[] data_rasc2;
		data_rasc2 = NULL;
		rc = sqlite3_finalize(regop);
		fechaSQLite3();
				
		logOperacao.pagAtual += 1;
		
		if ((logOperacao.pagAtual == (logOperacao.pagTotal+1)) && (logOperacao.Resto == 0)) { //ñ tem resto, acabou
			if (DEBUGANDO) Serial.println("nao tem resto - acabou");
			return true;
		}
		if ((logOperacao.pagAtual == (logOperacao.pagTotal+1)) && (logOperacao.Resto != 0)) { //tem resto, ñ acabou
			if (DEBUGANDO) Serial.println("tem resto - nao acabou");
			return true;
		}
		if (logOperacao.pagAtual == (logOperacao.pagTotal + 2)) {//imprimiu resto, acabou
			if (DEBUGANDO) Serial.println("imprimiu resto - acabou");
			return true;
		}
		//qquer outra coisa continua paginando
		if (DEBUGANDO) {
			Serial.println("qquer outra coisa continua paginando");
			Serial.println(logOperacao.pagAtual);
			return true;
		}
	}
	// não é para chegar aqui
	if (DEBUGANDO) Serial.println("nao deve chegar aqui....");
	zeraVariaveisListaROp();
	return true;
}
//======================================================================
//mostra do display/tela uma linha do reg operacao
void display_linhaROp(int linha_tela, String data_rasc, String usuario_rasc, String operacao_rasc) {
	char buffrasc[50];
	switch (linha_tela)
	{
	case 1: {
		strcpy(buffrasc, data_rasc.c_str());
		data01_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, usuario_rasc.c_str());
		usu01_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, operacao_rasc.c_str());
		op01_arqOp.setText(buffrasc);
		
		break;
	}case 2: {
		strcpy(buffrasc, data_rasc.c_str());
		data02_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, usuario_rasc.c_str());
		usu02_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, operacao_rasc.c_str());
		op02_arqOp.setText(buffrasc);
		break;
	}case 3: {
		strcpy(buffrasc, data_rasc.c_str());
		data03_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, usuario_rasc.c_str());
		usu03_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, operacao_rasc.c_str());
		op03_arqOp.setText(buffrasc);
		break;
	}case 4: {
		strcpy(buffrasc, data_rasc.c_str());
		data04_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, usuario_rasc.c_str());
		usu04_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, operacao_rasc.c_str());
		op04_arqOp.setText(buffrasc);
		break;
	}case 5: {
		strcpy(buffrasc, data_rasc.c_str());
		data05_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, usuario_rasc.c_str());
		usu05_arqOp.setText(buffrasc);
		
		strcpy(buffrasc, operacao_rasc.c_str());
		op05_arqOp.setText(buffrasc);
		break;
	}default:
		break;
	}
}
//======================================================================
// zera variáveis globais do listaLogOperacao
void zeraVariaveisListaROp() {
	logOperacao.pagTem = false;
	logOperacao.pagTotal = 0;
	logOperacao.pagAtual = 0;
	logOperacao.Resto = 0;
	logOperacao.Offset = 0;
	logOperacao.nLinhas = 0;
	mostraMais = false;
	mostraMenos = false;
}
//======================================================================
// rotina de mensagens do sql de retorno de operacao para debug
void msgSqlStatus(int posicao, int stsSQL) {
	const int qualMsg = posicao;
	switch (qualMsg){
		case 1: {
			Serial.println("ret-bd-verificaUsu:Erro abrindo BD");
			break;
		}case 2: {
			Serial.println("Erro no prepare-V2: usuarios");
			break;
		}case 3: {
			Serial.println("Erro no prepare-V2: logUsu");
			break;
		}case 4: {
			Serial.println("Erro bind-agora: logUsu");
			break;
		}case 5: {
			Serial.println("Erro bind-usuario: logUsu");
			break;
		}case 6: {
			Serial.println("Erro bind-operacao: logUsu");
			break;
		}case 7: {
			Serial.println("Erro step: logUsu");
			break;
		}case 8: {
			Serial.println("Erro finalize: logUsu");
			break;
		}case 9: {
			Serial.println("Erro close: logUsu");
			break;
		}case 10: {
			Serial.println("Erro no prepare-V2: logOperacao");
			break;
		}case 11: {
			Serial.println("Erro prepv2 finaliz-close: logOperacao");
			break;
		}case 12: {
			Serial.println("Erro step: logOperacao");
			break;
		}case 13: {
			Serial.println("Erro step finaliz-close: logOperacao");
			break;
		}case 14: {
			Serial.println("Erro numero-finaliz-close: logOperacao");
			break;
		}case 15: {
			Serial.println("Erro no prepare-V2: lista_arqOp");
			break;
		}case 16: {
			Serial.println("Erro no prepare-V2/2: lista_arqOp");
			break;
		}case 17: {
			Serial.println("Erro no bind: lista_arqOp");
			break;
		}case 18: {
			Serial.println("Erro no BD Tela de Status");
			break;
		}default: {
			break;
		}
	}
	Serial.println(sqlite3_errmsg(banco01));
}
//==================================================================
//	registra erro login e volta tela
void erroLogin() {
	usuarioAtivo = 0;
	registraOperacao(regErroLogin);
}
//==========================================================================
//
//mostra Numero de registros por tabela na Tela de Status
void mostraStatus() {
	//tamanho tabelas
	String sqlPacientes = "SELECT count(*) FROM pacientes";
	RegistrosN(sqlPacientes, pacientes_status);
	String sqlUsuarios = "SELECT count(*) FROM usuarios";
	RegistrosN(sqlUsuarios, usuarios_status);
	String sqlTipos_tratamento = "SELECT count(*) FROM tipos_tratamento";
	RegistrosN(sqlTipos_tratamento, tpTratam_status);
	String sqlTratamentos = "SELECT count(*) FROM tratamentos";
	RegistrosN(sqlTratamentos, numTratam_status);
	String sqlLogTratamentos = "SELECT count(*) FROM log_tratamento";
	RegistrosN(sqlLogTratamentos, logTratam_status);
	String sqllogUsuario = "SELECT count(*) FROM log_usuario";
	RegistrosN(sqllogUsuario, logOperacoes_status);

	//tamanho do BD - arquivo
	int tamanhoArq;
	root = SPIFFS.open("/");
	arquivo = root.openNextFile();
	tamanhoArq = arquivo.size();
	
	char rasStr[10];
	itoa(tamanhoArq, rasStr, 10);
	tamBancosDados_status.setText(rasStr);

	//rede wifi
	redeWifi olhaWifiID;
	if (!EEPROM.begin(900)) return;
	EEPROM.get(10, olhaWifiID);
	if (olhaWifiID.temRede == 0xAA) {
		
		ssid_00 = olhaWifiID.ssid;
		if (DEBUGANDO) Serial.println("PEGOU da EEPROM-ssid");
		char* rasc = new char[ssid_00.length() + 1];
		strcpy(rasc, ssid_00.c_str());
		wifiSSID_status.setText(rasc);
		//ultimo IP da maquina	
		IPAddress rasc2 = wifiRelog.qualIP;
		char rascIP[16];
		sprintf(rascIP, "%d.%d.%d.%d", rasc2[0], rasc2[1], rasc2[2], rasc2[3]);
		IPmaquina_status.setText(rascIP);
			
		return;
	}
	EEPROM.commit();
	if (DEBUGANDO) Serial.println("Nao leu na EEPROM-ssid");
	return;
}
//=====================================================================
//
//lista N. de registros de arquivo e display Nextion
bool RegistrosN(String QualSelect, NexText QualCampo) {

	bool ini_bd = 0;
	sqlite3_stmt* regop = NULL;
	time(&dataemsegundos);

	ini_bd = iniciaSQLite3();

	if (!ini_bd) return false;

	//vamos contar numero de linhas
	
	const char* sql = QualSelect.c_str();
	rc = sqlite3_prepare_v2(banco01, sql, -1, &regop, NULL);
	if (rc != SQLITE_OK) {
		if (DEBUGANDO) msgSqlStatus(18, rc);
		rc = sqlite3_finalize(regop);
		if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(18, rc);
		fechaSQLite3();
		return false;
	}else {
		rc = sqlite3_step(regop);
		if (SQLITE_ROW != rc) {
			if (DEBUGANDO) msgSqlStatus(18, rc);//pode não ser erro, talvez ñ tem registro 
			rc = sqlite3_finalize(regop);
			if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(18, rc);
			fechaSQLite3();
			return false;
		}else {
			int numeroLinhas = sqlite3_column_int(regop, 0);
			if (DEBUGANDO) {
				Serial.print("numero_linhas: ");		// tem N registros no arq.
				Serial.println(numeroLinhas);
			}
			rc = sqlite3_finalize(regop);
			if ((rc != SQLITE_OK) && DEBUGANDO) msgSqlStatus(18, rc);
			fechaSQLite3();

			//mostra no nextion quantos registro tem.
			//String numeroRegLog = "Numero registros: ";
			char rasStr[10];
			itoa(numeroLinhas, rasStr, 10);
			//String ras2Str = String(rasStr);
			//numeroRegLog = numeroRegLog + ras2Str;
			//char* buffNumReg = new char[numeroRegLog.length() + 1];
			//strcpy(buffNumReg, numeroRegLog.c_str());
			QualCampo.setText(rasStr);

			//delete[] buffNumReg;
			//buffNumReg = NULL;
			return true;
		}
	}
}
//====================================================================
//
//							NEXTION (display - Touch Screen)
//
//====================================================================

//============================================
//		Inicia Nextion
void iniciaNextion() {

	// interface botoes para relogio e wifi ok/programar -----------
	acerta_rtc.attachPop(acerta_rtcPopCallback);
	acerta_Ssid.attachPop(acerta_SsidPopCallback);
	bt_login_ok.attachPop(bt_login_okPopCallback);
	logoff_manut.attachPop(logoff_manutPopCallback);
	btMais_arqOp.attachPop(btMais_arqOpPopCallback);
	btMenos_arqOp.attachPop(btMenos_arqOpPopCallback);
	// interface de telas
	tela_login.attachPop(tela_loginPopCallback);
	menu_Manut.attachPop(menu_ManutPopCallback);
	tela_pulso.attachPop(tela_pulsoPopCallback);
	tela_sensores.attachPop(tela_sensoresPopCallback);
	tela_tabUsu.attachPop(tela_tabUsuPopCallback);
	tela_tabPac.attachPop(tela_tabPacPopCallback);
	tela_tabOpe.attachPop(tela_tabOpePopCallback);
	tela_tabTpTra.attachPop(tela_tabTpTraPopCallback);
	tela_arqLog.attachPop(tela_arqLogPopCallback);
	tela_wifi.attachPop(tela_wifiPopCallback);
	tela_relogioRTC.attachPop(tela_relogioRTCPopCallback);
	menu_testes.attachPop(menu_testesPopCallback);
	tela_arqOp.attachPop(tela_arqOpPopCallback);
	tela_status.attachPop(tela_statusPopCallback);
	// limpa campos da tela de acesso ao iniciar maquina
	//por segurança
	limpaTelaAcesso();
}
//============================================
//vê qual tela para mostrar data hora
void veQualTela() {
	if ((telaAtual == nr_telaLogin) || (telaAtual == nr_menuManut) || (telaAtual == nr_telaRelogioRTC)) {
		LePrintDataHoraTela();
	}
	if ((telaAtual == nr_menuManut) && (temMsgManut != nao_tem)) {
		switch (temMsgManut) {
			case tem_nova: {
				temMsgManut = nao_tem;
				char* buf_mostra_chr = new char[buf_mostra.length() + 1];
				strcpy(buf_mostra_chr, buf_mostra.c_str());
				msg_manut.setText(buf_mostra_chr);
				delete[] buf_mostra_chr;
				buf_mostra_chr = NULL;
				break;
			} case ja_mandou:{
				//não faz nada
				break;
			}case limpa_msg: {
				temMsgManut = nao_tem;
				msg_manut.setText("");
				break;
			} default: {
				break;
			}
		}
	}
}
//==============================================
//mostra relógio em telas específicas
void LePrintDataHoraTela() {

	char* mostraData = new char[30];
	tm datahora;

	bool voltaTempo = wifiRelog.AgetLocalTime(&datahora, 1000, DEBUGANDO);
	if (voltaTempo) {
		wifiRelog.fazTimeStrNTP(&datahora, mostraData);
		switch (telaAtual) {
		case nr_telaLogin: {
			hora_login.setText(mostraData);
			break;
		}case nr_menuManut: {
			hora_manut.setText(mostraData);
			break;
		}case nr_telaRelogioRTC: {
			hora_relog.setText(mostraData);
			break;
		}default: {
			break;
		}
		}
	}else {
		if (DEBUGANDO) 	dbgMostraDados("Loop-não peguei hora-noSistema", 0);
	}
	delete[] mostraData;
	mostraData = NULL;
}

//============================================
// rotina de ativa/altera tela do Nextion
void ativaTela(int antes, int atual) {
	telaAnterior = antes;
	telaAtual = atual;
	switch (atual) {
		case 0: {
			tela_login.show();
			break;
		}case 1: {
			menu_Manut.show();
			break;
		}case 2: {
			tela_pulso.show();
			break;
		}case 3: {
			tela_sensores.show();
			break;
		//}case 4:{
			//	break;
		//}case 5:{
			//	break;
		}case 6: {
			tela_tabUsu.show();
			break;
		}case 7: {
			tela_tabPac.show();
			break;
		}case 8: {
			tela_tabOpe.show();
			break;
		}case 9: {
			tela_tabTpTra.show();
			break;
		}case 10: {
			tela_arqLog.show();
			break;
		}case 11: {
			tela_wifi.show();
			break;
		}case 12: {
			tela_relogioRTC.show();
			break;
			//}case 13:{
			//	break;
			//}case 14:{
			//	break;
		}case 15: {
			menu_testes.show();
			break;
		}case 16: {
			tela_arqOp.show();
			break;
		}default: {
			break;
		}
	}
}
//=====================================================
//		Rotina limpa tela de Acesso
void limpaTelaAcesso() {
	usuario_login.setText("");
	senha_login.setText("");
}
//=======================================================================
//
//			ROTINAS DE EVENTOS DO NEXTION, BOTÕES, TELAS, ETC.
//
//=======================================================================

//==================================================
//	BOTAO DE OK DA TELA DE RELOGIO MANUAL
// acerta manual/te no RTC e no relógio do sistema
//
void acerta_rtcPopCallback(void *ptr) {

	tmElements_t rascRTC;
	time_t horaDataRTC;
	uint32_t rascvalor;

	rtcHoras.getValue(&rascvalor);
	rascRTC.Hour = byte(rascvalor);
	if (DEBUGANDO) 	dbgMostraDados("Mostra HORA DS1307-manual:", rascRTC.Hour);

	rtcMinutos.getValue(&rascvalor);
	rascRTC.Minute = byte(rascvalor);

	rtcSegundos.getValue(&rascvalor);
	rascRTC.Second = byte(rascvalor);

	rtcDia.getValue(&rascvalor);
	rascRTC.Day = byte(rascvalor);

	rtcMes.getValue(&rascvalor);
	rascRTC.Month = byte(rascvalor);

	rtcAno.getValue(&rascvalor);
	rascRTC.Year = byte(rascvalor - 1970);//offset unix - 1970

	horaDataRTC = relogio.makeTimeRTC(rascRTC); // con referência Unix-Epoch 1970
	relogio.set(horaDataRTC);

	wifiRelog.iniciaRelogioSys(horaDataRTC,&datahora,DEBUGANDO);

	if (DEBUGANDO) 	dbgMostraDados("Mostra dados DS1307-manual:", 0);
	//LePrintDataHoraRTC();
	if (DEBUGANDO) 	dbgMostraDados("Mostra dados Relsistema-manual:", 0);
	//LePrintDataHoraNTP();
}
//========================================================================
//	PEGA SSID E SENHA - MANUAL
void pegaSsidSenha() {
	ssid_wifi.getText(leSsid, 20);
	senha_wifi.getText(leSenha, 20);

	if (DEBUGANDO) {
		Serial.println("SSID-SENHA");

		String s = leSenha;
		Serial.print("<");
		Serial.print(s);
		Serial.println(">");

		s = leSsid;
		Serial.print("<");
		Serial.print(s);
		Serial.println(">");
	}
	//faltam as rotinas de iniciar manual/te WiFI (e pegar NTP)
	String rasc1, rasc2;
	rasc1 = leSsid;
	rasc2 = leSenha;
	gravaWifiEEPROM(rasc1, rasc2);
	msg_wifi.setText("Gravou SSID, Rede Wifi");
	if (DEBUGANDO) Serial.println("Gravou SSID, Rede Wifi");
}
//=======================================================================
// BOTAO DE OK DA TELA DE WIFI MANUAL
//
void acerta_SsidPopCallback(void *ptr) {
	pegaSsidSenha();
}
//=======================================================================
// BOTAO DE LOGIN OK
//
void bt_login_okPopCallback(void *ptr) {
	if (emPrograma) {
		//algo errado, em programa não acessa login
		return;
	}
	if (usuarioLogado) {
		//algo errado, usuario logado não acessa login
		//posso fazer botão logout, aí sim.
		return;
	}else {
		bool sts_prv = verificaUsuario();
		if (sts_prv) {//achou usuario e senha ok-tela
			sts_regOp = registraOperacao(regLogin);
			//versão final vai p/tela menu operação
			ativaTela(nr_telaLogin, nr_menuManut);
			return;
		}else {//senha ou usuario errado
			//fazer um contador/tentativas login
			sts_regOp = registraOperacao(regErroLogin);
			//ativaTela(nr_telaLogin, nr_telaLogin);
		}
	}
}
//=====================================================================
//BOTAO-TEXTO menu manut para fazer logoff
void logoff_manutPopCallback(void *ptr) {
	if (DEBUGANDO) Serial.println("Entrou no Logoff - botao/texto acionado");
	logoffUsuario();
}
//======================================================================
//		ROTINAS DE IDENTIFICAÇÃO DE TELA ATIVA
//======================================================================
void tela_loginPopCallback(void *ptr) {
	telaAtual = 0;
	if (DEBUGANDO) {
		Serial.println("TELA LOGIN");
		dbgMostraDados("tela atual:", telaAtual);
	}
}
//======================================================================
void menu_ManutPopCallback(void *ptr) {
	telaAtual = 1;
	if (DEBUGANDO) Serial.println("Menu Manutenção");
}
//======================================================================
void tela_pulsoPopCallback(void *ptr) {
	telaAtual = 2;
	if (DEBUGANDO) Serial.println("Tela do Pulso");
}
//======================================================================
void tela_sensoresPopCallback(void *ptr) {
	telaAtual = 3;
	if (DEBUGANDO) Serial.println("Tela dos Sensores");
}
//======================================================================
void tela_tabUsuPopCallback(void *ptr) {
	telaAtual = 6;
	if (DEBUGANDO) Serial.println("Tabela de Usuários");
}
//======================================================================
void tela_tabPacPopCallback(void *ptr) {
	telaAtual = 7;
	if (DEBUGANDO) Serial.println("Tela dos Pacientes");
}
//======================================================================
void tela_tabOpePopCallback(void *ptr) {
	telaAtual = 8;
	if (DEBUGANDO) Serial.println("Tabela de Operações");
}
//======================================================================
void tela_tabTpTraPopCallback(void *ptr) {
	telaAtual = 9;
	if (DEBUGANDO) Serial.println("Tipos Tratamento");
}
//======================================================================
void tela_arqLogPopCallback(void *ptr) {
	telaAtual = 10;
	if (DEBUGANDO) Serial.println("Arquivo de Log");
}
//======================================================================
void tela_wifiPopCallback(void *ptr) {
	telaAtual = 11;
	if (DEBUGANDO) Serial.println("Tela do Wifi");
}
//======================================================================
void tela_relogioRTCPopCallback(void *ptr) {
	telaAtual = 12;
	if (DEBUGANDO) Serial.println("Tela do Relogio");
}
//======================================================================
void menu_testesPopCallback(void *ptr) {
	telaAtual = 15;
	if (DEBUGANDO) Serial.println("Menu de Testes");
}
//======================================================================
void tela_arqOpPopCallback(void *ptr) {
	telaAtual = 16;
	if (DEBUGANDO) Serial.println("Tela Registro Operação");
	bool sts_op = listaLogOperacao();
	if (!sts_op && DEBUGANDO) Serial.println("erro leitura Log Operação");
}
//======================================================================
void tela_statusPopCallback(void* ptr) {
	telaAtual = 17;
	if (DEBUGANDO) Serial.println("Tela Status Maquina");
	mostraStatus();
}
//======================================================================
//			FIM DE ROTINAS DE TELA ATIVA
//======================================================================
//
//======================================================================
//	botao de proxima pagina da tela de registros de operacao
void btMais_arqOpPopCallback(void *ptr) {
	mostraMais = true;
}
//=============================================================
//	botao de pagina anterior da tela de registros de operacao
void btMenos_arqOpPopCallback(void *ptr) {
	mostraMenos = true;
}
