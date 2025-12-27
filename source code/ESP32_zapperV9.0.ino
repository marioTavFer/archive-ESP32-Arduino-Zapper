/**********************************************************************************************//**
 * @file	ESP32_zapperV9.0.ino
 *
 * @brief	Modulo Esp32-zapper v 9.0
 * 			funcoes de tratamento do PWM para zapper, clark, rife, zappicator.
 * 			funcoes da tela HMI - Nextion.
 * 			funcoes de leitura do INA219 - sensor tensao e corrente
 * @author	MLTF
 * @date	17/07/2018 - 10/12/2019
 **************************************************************************************************/
  
 /*********************************************************
  *                  Modulo ESP32-zapperV9.0              *
 **********************************************************/
 
/**
* @defgroup Sistema Sistema-hardware
* @brief rotinas-funcoes relacionadas ao hardware ESP32 WROOM-32
*/

/**
* @defgroup Protocolos Rife-Clark-Zappicator
* @brief rotinas-funcoes relacionadas aos protocolos-tratamentos Rife, Clark, Zappicator
* 			a)descricao linha da zappicator {n, x,y,x,y,x,y,.....}
* 			n - quantidade de grupos frequencia+tempo ligada,
* 			x-frequencia, (em Hz)
* 			y-tempo ligada a freq. (em segundos) -> 15min = 900seg
* 			o original e uma frequencia 1KHz e uns +/- 15min (tembem li 30m)
* 			aqui pode colocar mais freq igual a clark "pares"=freq+tempo
* 			
*			b)descricao linha da CLARK {n,x,y,x,y,x,y,.....}
* 			n - quantidade de grupos frequencia+tempo ligada,
* 			x-frequencia, (em Hz)
* 			y-tempo ligada a freq. (em segundos)
* 			Clark "oficial" (5 fases/etapas):
* 			30KHz->7min;descanco->20min;30KHz->7min;descanco->20min;30KHz->7min;FIM
* 			
*			c)CAFL-ETDFL-2018 RIFE: {x,y,t, zzz,zzz,zzz,.....}
*			x - numero do programa,
*			y - quantidade de frequencias,
*			t - tempo para cada frequencia em segundos-por enquanto,(minx60)
*			zzz- as frequencias em *** KHz ***
*			TRANSFERI TABELAS RIFE PARA => tabelas_rife-ESP32.h (esta nos "include")
*			olhar comentarios la e tambem "a fazer".
*/

/**
* @defgroup Ina219 Interface-com-Ina219
* @brief rotinas-funcoes de tratamento da leitura de tensao e corrente do Ina219
*/

/**
* @defgroup Nextion Nextion-HMI-Display
* @brief rotinas-funcoes relacionadas ao hardware
*/

//ESP32 #include <TimerOneML.h>
#include <Wire.h>
//ESP32 #include <AD9850SPI.h>
#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <Nextion.h>
#include <tabelas_rife-esp32.h>
#include <mario_nextion.h>



/** @var bool DEBUGANDO	 */
/** @brief	Variavel Global - boolean true indica que debug habilitado*/
bool DEBUGANDO = true;

//***************** DE HARDWARE

/**
* @addtogroup Ina219
* @{
*/

/**************** INA219 - SENSOR DE CORRENTE*/

/** @brief	cria instancia do INA219 - Adafruit_INA219 ina219 */
Adafruit_INA219 ina219;
/** @var	unsigned long tempoIna219Rasc */
/** @brief unsigned long, rascunho de tempo-timer do Ina219 */
unsigned long tempoIna219Rasc = 0;
/** @var	int16_t n_ina	*/
/** @brief rascunho de valor do ina219 */
int16_t n_ina = 0;
/** @var	float mediaVolts	*/
/** @brief	media de volts medidos/lidos */
float mediaVolts = 0;
/** @var	float	mediaAmperes*/
/** @brief	media de amperes medidos/lidos */
float mediaAmperes = 0;
/** @var	float accVolts	*/
/** @brief	acumulado de volts */
float accVolts = 0;
/** @var	float	accAmperes	*/
/** @brief	acumulado de  amperes */
float accAmperes = 0;
/** @var	bool	paraLeitura */
/** @brief	para temporariamente leitura */
bool paraLeitura = false;

/**
* @}
*/


/*
*rotinas que utilizava no Mega2560 - arduino para referencia de tensao
* --------------------------------------------------------------------
*referencia externa TL431 (no AREF) e
*entrada ADC para tensao (double check)
*valor do TL431 esta na area de constantes, abaixo
*float ADC_CONST = 0.0048828125;	// 5.00/1024 => v/step
*se for 5.01 => 0.004892578125
*float CONST_431 = 0.003955; //para AREF =>4.05V do TL431
*float CONST_431 = 0.00276562; //para AREF =>2.832V do TL431 c/R=12K+cap10uf
*float CONST_431 = 0.00274511; //para AREF =>2.811V do TL431 c/R=12K+cap10uf
*
*float Voltagem = 0;
*float minvolt = 9999;
*float maxvolt = 0;
*unsigned long tempoADCRasc = 0;
*
*filtro para tirar "spikes" no ADC/leitura
*ver parte de constantes, abaixo
*/

//***********Um buffer de rascunho *****************
// 
// 
/** @var int buffer_itens*/
/** @brief	numero de itens (tamanho) do buffer */
int buffer_itens = 50;
/** @var int buffer_v[50]*/
/** @brief	o buffer_v[ 50] buffer de 50 posicoes */
int buffer_v[50];

//************ Variaveis de relogio ****************

/** @var unsigned int segundos*/
/** @brief	para relogio, timer e temporizadores */
unsigned int segundos;
/** @var unsigned int minutos*/
/** @brief	para relogio, timer e temporizadores */
unsigned int minutos;
/** @var unsigned int horas*/
/** @brief	para relogio, timer e temporizadores */
unsigned int horas;
/** @var unsigned int n7*/
/** @brief	para relogio, timer e temporizadores */
unsigned int n7; 

/** @var unsigned long tempoAgora*/
/** @brief	tempo agora */
unsigned long tempoAgora = 0;
/** @var unsigned long tempoAntes*/
/** @brief	tempo antes */
unsigned long tempoAntes = 0;
/** @var unsigned long tempoInicial*/
/** @brief	rascunho de tempo no momento que foi ligado/desligado */
unsigned long tempoInicial;
/** @var unsigned long tempoSegundos*/
/** @brief	tempo em segundos */
unsigned long tempoSegundos;

/**
* @addtogroup Nextion
* @{
*/

//**************** Variaveis do Nextion ***********************

/** @var char bufferNextion[90]*/
/** @brief	buffer de comunicacao do-para NEXTION */
char bufferNextion[90];
/** @var uint32_t barraNextionRasc*/
/** @brief	variavel da barra (p/incre-decre-mentar) do Nextion */
uint32_t barraNextionRasc;
/** @var int k */
/** @brief	int K do processo - contador*/
int k;
/*TESTE PARA DELETAR APOS/NO FINAL*/
/** @var int tamComando*/
/** @brief	tamanho do telegrama-comando Nextion */
int tamComando;
/** @var char bufferNextionRasc[20] */
/** @brief	buffer rascunho/buffer de tx-rx Nextion */
char bufferNextionRasc[20];

/**
* @}
*/

/**
* @addtogroup Protocolos
* @{
*/

/***************** Variaveis de controle dos Protocolos*/
 
/** @var bool emPrograma */
/** @brief	*************** flag de em programa de Rife, Clark, Zappicator */
bool emPrograma = false;
/** @var int qualPrograma */
/** @brief	identifica programa, 1-clark, 2-zappic, 3-rife, etc) */
int qualPrograma = 0;
/** @var int estagio */
/** @brief	indica em que estagio esta - qual par de frequencias */
int estagio = 0;
/** @var int quantosEstagios */
/** @brief	indica quantos estagios-fases tem o protocolo */
int quantosEstagios = 0;
/** @var float pwmFrequencia */
/** @brief	indica a (pwm) frequencia */
float pwmFrequencia = 0;
/** @var pwmDuracao */
/** @brief	indica (pwm) a duracao  do protocolo-fase-estagio*/
float pwmDuracao = 0;
/** @var bool primeiraVez*/
/** @brief	indica se comecou(1 vez) CLARK */
bool primeiraVez = true;
/** @var unsigned long protocoloAntesRasc*/
/** @brief	do protocolo CLARK antes rascunho */
unsigned long protocoloAntesRasc;
/** @var unsigned long protocoloSegundosTotal*/
/** @brief	do protocolo CLARK contador segundos total */
unsigned long protocoloSegundosTotal = 0;
/** @var unsigned long protocoloSegundos*/
/** @brief	do protocolo CLARK contador segundos */
unsigned long protocoloSegundos = 0;

//**************** RIFE*/

/** @var int qualRife*/
/** @brief  indica qual protocolo Rife */
int qualRife = 1;


/** @var uint32_t frequenciaRasc[7]*/
/** @brief	Operacao Manual (Rife) tabela de rascunho das frequencias */
uint32_t frequenciaRasc[7];
/** @var uint32_t duracaoRasc[7]*/
/** @brief	Operacao Manual (Rife) tabela de duracao das frequencias */
uint32_t duracaoRasc[7];

/**
 * @}
 */

/*********************************************************
*                     Constantes                         *
**********************************************************/

//***************** INA219 e TL431*/
/** @brief	intervalo (de 10 seg). em milissegundos de leitura do ina219 */
const unsigned long intervaloLeituraIna219 = 10000;

//const float CONST_431 = 0.003955; //para AREF =>4.05V do TL431
//const float CONST_431 = 0.00390625; //para AREF =>4.00V do TL431
//const float CONST_431 = 0.00275879; //para AREF =>2.825V do TL431 c/R=12K+cap10uf
//const float V_ajuste_div = 0.201603;//mais do divisor de tensao ligado em A0 =>R1=22120 e R2=5570
//const float alpha = 0.98; //do filtro passa baixa


/*ESP32-nao usa (era do mega2560)
**************** PINOS DO ADC
const uint8_t leCanalA0 = 0; //entrada - canal analogico A0
const unsigned long intervaloLeituraADC = 10000; //10 seg. em milissegundos
*/

/*ESP32-nao usa
**************** DDS 9850 (gerador de frequencias)
const double ajusteFreq = 124985500;//teoricamente isto devera ser feito placa por placa
const int fase = 0; //depende do soft/aplicacao, fase talvez mude para variaveis
*/

/*ESP32-nao usa
**************** Pinos(do arduino/MEGA) para o DDS9850
const int W_CLK_PIN = 52;	// placa AD - pin - 2 =>SCK
const int FQ_UD_PIN = 50;	// placa AD - pin - 3 => PB0
const int DATA_PIN = 51;	// placa AD - pin - 4 => MSOI
const int RESET_PIN = 53;	// placa AD - pin - 5 => PD7
*/

//***************** para ligar bit(led) da placa ===sem uso
//const int bitLed = 13;

//***************** Liga/desliga*****/
/** @brief	constante de acao liga (HIGH -> liga) */
const int liga = HIGH;
/** @brief	constante de acao desliga (LOW -> desliga) */
const int desliga = LOW;

/*	***************** hardware-arduino == definicao dos pinos buz,teclas e pwm ******/
/*	constante bit buzina => PD6-arduino; **** NO MEGA (PH3) */
/* const int bitBuzina = 6;*/
/* constante bit PWM saida do PWM => PB1 (9)=> OC1A - arduino; *** no MEGA (11)PB5 */
/*const int pinoPWM = 11; //arduino	*/

/**@brief	constante bit buzina = 4  */
const int buzina = 4;

//***************** IDENTIFICACAO DOS PROTOCOLOS****/
/** @brief	Protocolo Clark 30KHz => 7m-20m-7m-20m-7m */
const int clark = 1;
/** @brief	Rife prca-definidos (3 exemplos) */
const int rifePre = 3;
/** @brief	ZAPPICATOR (padrao ca 1KHZ durante 30m) */
const int zapp = 2;
/** @brief	Rife entrando Numero do Protocolo */
const int rifeETDFL = 4;
/** @brief	digitando frequencias e duracao (atca 6 freq.) */
const int rifeManual = 5;
/** @brief	e o numero de protocolos existentes na tabela tabela_rife.h */
const int maximoRife = 268;
/** @brief	constantes no ESP32 */
const uint8_t freq_var_canal = 0;
/** @brief	uint8_t frequency 1 k canal */
const uint8_t freq_1K_canal = 1;
/** @brief	uint8_t  buz canal */
const uint8_t buz_canal = 2;
/** @brief	uint8_t 50% duty cycle para todos>1000Hz */
const uint8_t duty_todos = 1;
/** @brief	uint8_t 8 bits =>256 p/freq baixas */
const uint8_t duty_baixo = 8;

/** @brief	int clark rife */
const int clark_rife = 27;
/** @brief	int zappicator */
const int zappicator = 26;

/** @brief	freq. Inicial => 1KHz */
const uint32_t freq_inicial = 1000;

/** @brief	uint32_t frequencia variavel de rascunho */
uint32_t freq_var = 0;

//***************** Protocolos ***********************************

/**********************************************************************************************//**
 * @brief	linha da CLARK {n,x,y,x,y,x,y,.....}
 **************************************************************************************************/

const float tab_clark[11] = { 5,30000,420,0,1200,30000,420,0,1200,30000,420 };
/** @brief	TESTE CLARK - tempo reduzido para alguns segundos */
const float tab_clarkTT[11] = { 5,30000,7,0,20,30000,7,0,20,30000,7 };

/**********************************************************************************************//**
 * @brief	linha da zappicator {n, x,y,x,y,x,y,.....}
 **************************************************************************************************/

const float tab_zappic[3] = { 1,1000,900 };
/** @brief	TESTE ZAPPICATOR */
const float tab_zappicTT[3] = { 1,1000,10 };
const float tab_zappicTT1[3] = { 1,1000,10 };
const float tab_zappicTT2[3] = { 1,2500,10 };
const float tab_zappicTT3[3] = { 1,428,10 };
const float tab_zappicTT4[3] = { 1,529,10 };



///****************DEFINICOES/PROTOTIPOS *********************
void daBeep(int tempoBip);///< bipa por(miliseg) definido em "int"
void avisaXseg(int tempoBip, int numeroBips);
void tickRelogio();///< relogio feito por software no loop - nao vai usar
void iniciaDebug();///< envia msg ao monitor inicio do debug
void dbgMostraDados(String mensagem, int valor);///<  mostra msg e dados - monitor
void limpaStatus(int protocolo);///< limpa todos flags do protocolo em execucao
void tempoMarcaInicio();///< guarda hora que comecou protocolo
unsigned long tempoLeAgora();///< retorna false se estourou tempo

//esp32-nao usa (era chip que gerava pwm utilizado com mega2560-arduino)
//void iniciaDDS9850();

void iniciaPWMs();///< inicia ondas quadradas dos protocolos-tratamentos
//void terminaProgramaDDS9850(int protocolo);
void terminaPrograma(int protocolo);///< termina protocolo-tratamento
//void ligaFreqDDS9850(float pwmFreq);
void ligaPWM(float pwmFreq);///< liga pwm
//void desligaDDS9850();
void desligaPWM();///< desliga pwm

//void iniciaPWMT1();
void terminaProgramaZ(int protocolo);///< termina protocolo tipo zappicator
void ligaPWMZ(float entraFreq);///< liga pwm do tipo zappicator
void desligaPWMZ();///< termina pwm do tipo zappicator
//void ligaFreqPWMTZ(float pwmFreq);

void iniciaINA219();///< inicia chip sensores de tensao e corrente ina219
void leINA219();///< le sensores-entradas analogicas de tensao e corrente do ina219
void dbgMostraIna219(float voltagemCircuito, float voltagemShunt, float voltagemEntrada, float corrente_mA, float potencia_mW, float mediaVolts, float mediaAmperes);

//esp32-nao usa (era chip que gerava pwm utilizado com mega2560-arduino)
//void iniciaADC(uint8_t leCanalX);
//void leTensaoADC(uint8_t leCanalX);
//float leFiltraConverteADC(uint8_t leCanalX);
//void dbgMostraADC(float Vcalculo);
//uint16_t leADC(uint8_t leCanalX);
//void leTensaoADCantiga(uint8_t leCanalX);
//void dbgMostraADCAntiga(float voltagem, float minVolt, float maxVolt);

void comecaClark();
void proximoParClark();
void clark_7min();
void clark_20min();
void avancaBarraClark(int incremento);

void comecaZappic();
void avancaBarraZapp(float tempoDivisor);
void controleZappic();

void preparaRifeBichoX(int qualRife);
void comecaRifePre(int qualRife);
void proximoRifePre();
void avancaBarraRifePre(float tempoDivisor);
void controleRifePre();

void comecaRifeETDFL(int qualRife);
void proximoRifeETDFL();
void avancaBarraRifeETDFL(float tempoDivisor);
void controleRifeETDFL();

void comecaRifeManual();
void proximoRifeManual();
void avancaBarraRifeManual(float tempoDivisor);
void controleRifeManual();

/**
* @addtogroup Sistema
* @{
*/

/**********************************************************************************************//**
 * @fn	void setup()
 * @brief	inicializacao do zapper V9.0
 * @date	10/12/2019
 **************************************************************************************************/
void setup()
{
	Serial.begin(115200);
	delay(10);

	//iniciaDDS9850();
	iniciaPWMs();
	//iniciaADC(leCanalA0);
	iniciaINA219();
	initObjNextion();

	//buzina
	pinMode(buzina, OUTPUT_OPEN_DRAIN);
	digitalWrite(buzina, LOW);
	ledcSetup(buz_canal, freq_inicial, duty_todos);
	ledcAttachPin(buzina, buz_canal);
	daBeep(1000);

	iniciaDebug();
}

/**********************************************************************************************//**
 * @fn	void loop()
 * @brief	Loops this object
 * @date	10/12/2019
 **************************************************************************************************/
void loop()
{
	nexLoop(nex_listen_list);
	void tickRelogio();
	if (emPrograma) {
		switch (qualPrograma) {
		case clark: {  ///< controle do protocolo Clark
			if (estagio <= 5) {
				if (estagio == 1 || estagio == 3 || estagio == 5) {
					clark_7min(); ///< ---- controle do Clark - 7 min
				}
				if (estagio == 2 || estagio == 4) {
					clark_20min(); ///< ---- controle do Clark 20min 
				}
			}
			else {
				terminaPrograma(clark); ///< --- termina Clark se fim
			}
			break;
		}case zapp: {
			controleZappic();///< controle do protocolo zappicator
			break;
		}case rifePre: {
			controleRifePre();///< controle do protocolo Rife pre-definido
			break;
		}case rifeETDFL: {
			controleRifeETDFL();///< controle do protocolo Rife com N. do protocolo
			break;
		}case rifeManual: {
			controleRifeManual();///< controle da entrada manual de frequencias Rife (6 freq+duracao)
			break;
		}default: {
			break;
		}
		}
	}
	else {

		if (!paraLeitura) {
			leINA219(); ///< le sensor Ina219 - corrente e tensao
			//leFiltraConverteADC(leCanalA0);
		}
	}
}//-------------------------------- FIM LOOP------------------------------------

/**********************************************************************************************//**
 * @fn	void daBeep(int tempoBip)
 * @brief	da um BEEP (tempo em milisegundos)
 * @date	10/12/2019
 * @param 	tempoBip	int; The tempo bip em milisegundos.
 **************************************************************************************************/
void daBeep(int tempoBip) {
	//um bip
	ledcWrite(buz_canal, duty_todos);
	delay(tempoBip);
	ledcWrite(buz_canal, 0);///< bit, frequencia, tempo ms
}

/**********************************************************************************************//**
 * @fn	void avisaXseg(int tempoBip, int numeroBips)
 * @brief	******** AVISO DE N BEEPS POR X SEG *************
 * 			    1 segundo entre bips (1000 ms)
 * 			***cuidado com rotina para nao travar processamento
 * @date	10/12/2019
 * @param 	tempoBip  	int; The tempo bip (ms)
 * @param 	numeroBips	int; The numero bips.
 **************************************************************************************************/
void avisaXseg(int tempoBip, int numeroBips) {
	int x;

	for (x = 0; x < numeroBips; x++) {///< numeroBips - quantos bips
		daBeep(tempoBip); ///<um bip - tempoBip de um bip
		delay(500); ///< 0,5 segundo entre bips
	}
}

/**********************************************************************************************//**
 * @fn	void tickRelogio()
 * @brief	Tick relogio feito por software usando o loop (pessima ideia)
 * 			mudado para RTC-DS1307+relogio interno do sistema, isto, na versao/modulo
 * 			AsensoresRTC-BDV7.ino
 * @date	10/12/2019
 **************************************************************************************************/
void tickRelogio() {
	tempoAgora = millis();
	tempoAgora = tempoAgora / 1000;

	if (tempoAgora != tempoAntes) {
		tempoAntes = tempoAgora;
		segundos++;
		if (segundos > 59) {
			minutos++;
			segundos = 0;
			if (minutos > 59) {
				horas++;
				minutos = 0;
				segundos = 0;
				if (horas > 23) {
					horas = 0;
					minutos = 0;
					segundos = 0;
				}
			}
		}
	}
}

/**********************************************************************************************//**
 * @fn	void iniciaDebug()
 * @brief	Inicia debug, envia msg para monitor (pela USB)
 * @date	10/12/2019
 **************************************************************************************************/
void iniciaDebug() {
	//Serial.begin(9600); //para sair no PC
	//while (!Serial);// wait for serial port to connect. Needed for native USB
	Serial.println(" Inicio do ESP32-Zapper-V8 ");
	Serial.println("timers parados");
}

/**********************************************************************************************//**
 * @fn	void dbgMostraDados(String mensagem, int valor)
 * @brief	Debug mostra dados
 * @date	10/12/2019
 * @param 	mensagem	The mensagem (String).
 * @param 	valor   	The valor (int).
 **************************************************************************************************/
void dbgMostraDados(String mensagem, int valor) {
	Serial.print(mensagem);///< mensagem para monitor
	Serial.println(": ");
	Serial.println(valor);///< valor (int) para monitor
	Serial.println("=========");
}

/**
* @}
*/

/**
* @addtogroup Protocolos
* @{
*/

/**********************************************************************************************//**
 * @fn	void limpaStatus(int protocolo)
 * @brief	Limpa status: zera variaveis dos programas/protocoloes e frequencias. Identifica o
 * 			protocolo por causa do rife manual para limpar tabela rascunho.
 * @date	10/12/2019
 * @param 	protocolo	int; protocolo (identifica qual protocolo que tem que parar).
 **************************************************************************************************/
void limpaStatus(int protocolo) {
	emPrograma = false;
	pg0emPrograma.setValue(0);
	qualPrograma = 0;
	estagio = 0;

	if (protocolo == rifeManual) {
		for (int i = 0; i <= 6; i++) {
			frequenciaRasc[i] = 0;
			duracaoRasc[i] = 0;
		}
	}
	protocoloAntesRasc = 0;
	protocoloSegundos = 0;
	protocoloSegundosTotal = 0;
	primeiraVez = true;
	paraLeitura = false;
}

/**********************************************************************************************//**
 * @fn	void tempoMarcaInicio()
 * @brief	marca tempo inicial do comeco do protocolo e flags de primeira vez, antes, para
 * 			leitura do ina219, etc.
 * @date	10/12/2019
 **************************************************************************************************/
void tempoMarcaInicio() {
	tempoInicial = millis();///< marca qdo comecou em milissegundos
	protocoloSegundosTotal = 0;
	protocoloSegundos = 0;
	protocoloAntesRasc = 0;
	primeiraVez = true;
	paraLeitura = true;
}

/**********************************************************************************************//**
 * @fn	unsigned long tempoLeAgora()
 * @brief	Le tempo que passou desde inicio do programa
 * 			retorna false se estourou tempo. Verifica o tempo que timer-pwm esta ligado/desligado,
 * 			em funcao do programado em "tempoReferencia" - em segundos. O tempoInicial foi quando
 * 			comecou (na rotina tempoMarcaInicio). O rasc o tempo(relogio) e agora. O
 * 			tempoReferencia e quanto tem que demorar
 * @date	10/12/2019
 * @returns	unsigned long diferenca (retorna quanto tempo passou, em segundos)
 **************************************************************************************************/
unsigned long tempoLeAgora() {
	unsigned long diferenca = 0;
	unsigned long rasc = millis();

	diferenca = (rasc - tempoInicial) / 1000;///< para ter em segundos divide por 1000

	return diferenca; 		///< volta tempo decorrido em segundos
}

/**
* @}
*/

/**
* @addtogroup Sistema
* @{
*/

/**********************************************************************************************//**
 * @fn	void iniciaPWMs()
 * @brief	Inicia PWMs, clark-rife e zappicator sao dois canais (bits/pinos)
 * @date	10/12/2019
 **************************************************************************************************/
void iniciaPWMs() {

	//zappicator
	ledcSetup(freq_1K_canal, freq_inicial, duty_todos);
	ledcAttachPin(zappicator, freq_1K_canal);

	//clark-rife
	ledcSetup(freq_var_canal, freq_inicial, duty_todos);
	ledcAttachPin(clark_rife, freq_var_canal);

}

/**********************************************************************************************//**
 * @fn	void terminaPrograma(int protocolo)
 * @brief	Termina programa-protocolo, limpa status e da uns(5) beeps
 * @date	10/12/2019
 * @param 	protocolo	int; protocolo.
 **************************************************************************************************/
void terminaPrograma(int protocolo) {
	ledcWrite(freq_var_canal, 0);
	limpaStatus(protocolo); ///< deve faltar "limpar" mais flags e Ns
	avisaXseg(50, 5);
}

/**********************************************************************************************//**
 * @fn	void ligaPWM(float pwmFreq)
 * @brief	Liga pwm
 * @date	10/12/2019
 * @param 	pwmFreq	float; (frequencia de saida).
 **************************************************************************************************/
void ligaPWM(float pwmFreq) {
	uint32_t rasc = pwmFreq;
	if (pwmFreq > 0) {
		if (pwmFreq > 1000) {
			ledcSetup(freq_var_canal, rasc, duty_todos); ///< 1 bit duty
			ledcWrite(freq_var_canal, 1);	///< 0 ou 1 = 50% duty
		}
		else {
			ledcSetup(freq_var_canal, rasc, duty_baixo); ///< 8 bits duty
			ledcWrite(freq_var_canal, 127);	///< 127 = 50% duty
		}
	}
	else {
		ledcWrite(freq_var_canal, 0);
	}
}

/**********************************************************************************************//**
 * @fn	void desligaPWM()
 * @brief	Desliga pwm
 * @date	10/12/2019
 **************************************************************************************************/
void desligaPWM() {
	ledcWrite(freq_var_canal, 0);
}

//********************************** ROTINAS DO TIMER1 (para zappicator) ******************

/**********************************************************************************************//**
 * @fn	void terminaProgramaZ(int protocolo)
 * @brief	Termina programa zappicator (rotinas timer 1).
 * @date	10/12/2019
 * @param 	protocolo	int; qual protocolo.
 **************************************************************************************************/
void terminaProgramaZ(int protocolo) {
	ledcWrite(freq_1K_canal, 0);
	limpaStatus(protocolo);	///< deve faltar "limpar" mais flags e Ns
	avisaXseg(50, 5);
}

/**********************************************************************************************//**
 * @fn	void ligaPWMZ(float entraFreq)
 * @brief	Liga pwm do programa zappicator
 * @date	10/12/2019
 * @param 	entraFreq	float; frequencia de trabalho.
 **************************************************************************************************/
void ligaPWMZ(float entraFreq) {
	uint32_t rasc = entraFreq;
	if (entraFreq > 0) {
		if (entraFreq > 1000) {
			ledcSetup(freq_1K_canal, rasc, duty_todos);
			ledcWrite(freq_1K_canal, 1);
		}
		else {
			ledcSetup(freq_1K_canal, rasc, duty_baixo);
			ledcWrite(freq_1K_canal, 127);
		}

	}
	else {
		ledcWrite(freq_1K_canal, 0);
	}
}

/**********************************************************************************************//**
 * @fn	void desligaPWMZ()
 * @brief	Desliga pwm do programa zappicator
 * @date	10/12/2019
 **************************************************************************************************/

void desligaPWMZ() {
	ledcWrite(freq_1K_canal, 0);
}

/**
* @}
*/

/**
* @addtogroup Ina219
* @{
*/


//********************************** ROTINAS DO INA219 - SENSOR ****************************

/**********************************************************************************************//**
 * @fn	void iniciaINA219()
 * @brief	Inicia INA219, sensor de tensao e corrente, para saber carga do sistema
 * @date	10/12/2019
 **************************************************************************************************/
void iniciaINA219() {
	ina219.begin();
	ina219.setCalibration_16V_400mA();
}

/**********************************************************************************************//**
 * @fn	void leINA219()
 * @brief	Le ina 219
 * @date	10/12/2019
 **************************************************************************************************/
void leINA219() {
	float voltagemShunt = 0;
	float voltagemCircuito = 0;
	float corrente_mA = 0;
	float voltagemEntrada = 0;
	float potencia_mW = 0;


	voltagemShunt = ina219.getShuntVoltage_mV();
	voltagemCircuito = ina219.getBusVoltage_V();
	corrente_mA = ina219.getCurrent_mA();
	potencia_mW = ina219.getPower_mW();
	voltagemEntrada = voltagemCircuito + (voltagemShunt / 1000);

	if (n_ina < 500) {
		accVolts += voltagemCircuito;
		accAmperes += corrente_mA;
		mediaVolts = accVolts / (n_ina + 1);
		mediaAmperes = accAmperes / (n_ina + 1);
		n_ina += 1;
	}
	else {
		n_ina = 0;
		mediaVolts = 0;
		mediaAmperes = 0;
		accAmperes = 0;
		accVolts = 0;
	}
	if (DEBUGANDO) {
		dbgMostraIna219(voltagemCircuito, voltagemShunt, voltagemEntrada, corrente_mA, potencia_mW, mediaVolts, mediaAmperes);
	}
}

/**********************************************************************************************//**
 * @fn	void dbgMostraIna219(float voltagemCircuito, float voltagemShunt, float voltagemEntrada, float corrente_mA, float potencia_mW, float mediaVolts, float mediaAmperes)
 * @brief	Debug mostra valores ina 219 (mA, mV, mW, media Volt, media A)
 * @date	10/12/2019
 * @param 	voltagemCircuito	float;voltagem circuito.
 * @param 	voltagemShunt   	float;voltagem shunt.
 * @param 	voltagemEntrada 	float;voltagem entrada.
 * @param 	corrente_mA			float;corrente ma.
 * @param 	potencia_mW			float;potencia mw.
 * @param 	mediaVolts			float;media volts.
 * @param 	mediaAmperes		float;media amperes.
 **************************************************************************************************/
void dbgMostraIna219(float voltagemCircuito, float voltagemShunt, float voltagemEntrada, float corrente_mA, float potencia_mW, float mediaVolts, float mediaAmperes) {
	unsigned long agora = millis();
	unsigned long tempo = agora - tempoIna219Rasc;

	if (tempo > intervaloLeituraIna219) {
		Serial.print("Voltagem Circuito:   "); Serial.print(voltagemCircuito); Serial.println(" V");
		Serial.print("Voltagem SHUNT: "); Serial.print(voltagemShunt); Serial.println(" mV");
		Serial.print("Voltagem Entrada:  "); Serial.print(voltagemEntrada); Serial.println(" V");
		Serial.print("Corrente:       "); Serial.print(corrente_mA); Serial.println(" mA");
		Serial.print("Potencia:         "); Serial.print(potencia_mW); Serial.println(" mW");
		Serial.print("Media-V-circuito:"); Serial.print(mediaVolts); Serial.println("  V");
		Serial.print("Media-A-entrada:"); Serial.print(mediaAmperes); Serial.println(" mA");
		Serial.println(F("========================="));
		tempoIna219Rasc = millis();
	}
}

/**
* @}
*/



/*ESP32-nao usa leitura de canal ADC, mudou para ina219
********************************* Rotinas do ADC medicao tensao em A0

void iniciaADC(uint8_t leCanalX) {
	analogReference(EXTERNAL);
	delay(2);
	analogRead(leCanalX);
	delayMicroseconds(120);
	analogRead(leCanalX);
	delayMicroseconds(120);
}
*================= Le N vezes ADC; N=buffer_itens
void leTensaoADC(uint8_t leCanalX) {
	for (int v = 0; v<buffer_itens; v++) {
		buffer_v[v] = analogRead(leCanalX);
		delay(2);
	}
}
****************** outra possibilidade de leitura da ADC
*nao utilizada nesta versao
*================= le uma vez o ADC
uint16_t leADC(uint8_t leCanalX) {
	uint16_t ADC_rasc = 0;

	ADC_rasc = analogRead(leCanalX);
	return ADC_rasc;
}
//================= le uma vez ADC, converte e guarda max e min
void leTensaoADCantiga(uint8_t leCanalX) {
	float voltagem = 0;
	float maxVolt = 0;
	float minVolt = 0;
//	analogReference(EXTERNAL); //ja esta feita ref qdo inicializa ADC
//	delay(2);
	uint16_t rasc_leVcc = analogRead(leCanalX);
	delay(2);
	//CONST_431 => para AREF =>2.825V do TL431 c/R=12K+cap10uf
	//V_ajuste_div => divisor de tensao ligado em A0 =>R1=22120 e R2=5570
	voltagem = ((float)rasc_leVcc*CONST_431)/V_ajuste_div;
	maxVolt = max(voltagem, maxVolt);
	minVolt = min(minVolt, voltagem);
	if (DEBUGANDO) {
		dbgMostraADCAntiga(voltagem, minVolt, maxVolt);
	}
}
//=================
void dbgMostraADCAntiga(float voltagem, float minVolt, float maxVolt) {
	Serial.print("Volt-ADC = ");       //
	Serial.print(voltagem, 3);
	Serial.println(" V");
	Serial.print(" Min = ");
	Serial.print(minVolt, 3);
	Serial.print(" V");
	Serial.print(" Max = ");
	Serial.print(maxVolt, 3);
	Serial.println(" V");
	Serial.println("==============================");
}

//=================
//le N vezes canal ADC, filtro passa baixas e converte/ajusta ref do TL431
float leFiltraConverteADC(uint8_t leCanalX) {
	int novaleitura;
	float Vcalculo;
	float Vfiltrada;
	static float vfiltro = 0.0;

	leTensaoADC(leCanalX);
	for (int v = 0; v<buffer_itens; v++) {//aplica filtro passa baixa 0,98
		novaleitura = buffer_v[v];
		vfiltro = (alpha*vfiltro) + ((1 - alpha)*novaleitura);
	}
	Vfiltrada = vfiltro;
	Vcalculo = (Vfiltrada*CONST_431) / V_ajuste_div;

	if (DEBUGANDO) {
		dbgMostraADC(Vcalculo);
	}
	return Vcalculo;
}
//=================
void dbgMostraADC(float Vcalculo) {
	unsigned long agora = millis();
	unsigned long tempo = agora - tempoADCRasc;

	if (tempo > intervaloLeituraADC) {
		Serial.print(F("Volt-ADC = "));       //
		Serial.print(Vcalculo, 3);
		Serial.println(F(" V"));
		Serial.println(F("=============================="));
		tempoADCRasc = millis();
	}
}
*/

/**
* @addtogroup Nextion
* @{
*/


//============================================
//	ROTINAS DO NEXTION
//============================================

/**********************************************************************************************//**
 * @fn	void clarkProt_iniciarPopCallback(void *ptr)
 *
 * @brief	*******PAGINAS definidas no mario_nextion.h ***** NexPage page0    = NexPage(0, 0,
 * 			"page0");//menuPrinc-Menu Principal NexPage page1    = NexPage(1, 0, "page1");
 * 			//clarkProt-Protocolo CLARK NexPage page2    = NexPage(2, 0, "page2");//teste01-pag
 * 			de teste geral NexPage page3    = NexPage(3, 0, "page3");//rifePre-Protocolo RIFE pre-
 * 			definido NexPage page4    = NexPage(4, 0, "page4");//zappicProt-Protocolo Zappicator
 * 			NexPage page5    = NexPage(5, 0, "page5");//rifeETDFL-Protocolo Rife c/ETDFL 1017-
 * 			2018 NexPage page6    = NexPage(6, 0, "page6");//pagFig-pag com figura grande NexPage
 * 			page7    = NexPage(7, 0, "page7");//rifeManual- pag RIFE Manual 6 seq de freq+tempo
 *
 * @date	10/12/2019
 *
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void clarkProt_iniciarPopCallback(void* ptr) {
	daBeep(50);
	if (!emPrograma) {
		comecaClark();///< inicia protocolo CLARK
	}
	else {
		avisaXseg(100, 3);
	}
}

/**********************************************************************************************//**
 * @fn	void rifePre_iniciarPopCallback(void *ptr)
 * @brief	Callback, called when the rife pre qdo botao iniciar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifePre_iniciarPopCallback(void* ptr) {
	daBeep(50);
	if (!emPrograma) {
		comecaRifePre(qualRife);///< inicia protocolo RIFE (1)
	}
	else {
		avisaXseg(100, 3);
	}
}

/**********************************************************************************************//**
 * @fn	void zappicProt_iniciarPopCallback(void *ptr)
 * @brief	Callback, called when the zappic prot qdo botao iniciar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_iniciarPopCallback(void* ptr) {
	daBeep(50);
	if (!emPrograma) {
		comecaZappic();///< inicia protocolo ZAPPICATOR
	}
	else {
		avisaXseg(100, 3);
	}
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_iniciarPopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl qdo botao iniciar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_iniciarPopCallback(void* ptr) {
	daBeep(50);
	if (!emPrograma) {
		comecaRifeETDFL(qualRife);///< inicia protocolo RIFE (2)
	}
	else {
		avisaXseg(100, 3);
	}
}

/**********************************************************************************************//**
 * @fn	void rifeManual_iniciarPopCallback(void *ptr)
 * @brief	Callback, called when the rife manual qdo botao iniciar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_iniciarPopCallback(void* ptr) {

	daBeep(50);
	if (!emPrograma) {
		comecaRifeManual();///< inicia protocolo opManual
	}
	else {
		avisaXseg(100, 3);
	}
}

/**********************************************************************************************//**
 * @fn	void clarkProt_cancelarPopCallback(void *ptr)
 * @brief	Callback, called when the clark prot qdo botao cancelar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void clarkProt_cancelarPopCallback(void* ptr) {
	daBeep(50);
	terminaPrograma(clark);
}

/**********************************************************************************************//**
 * @fn	void rifePre_cancelarPopCallback(void *ptr)
 * @brief	Callback, called when the rife pre qdo botao cancelar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifePre_cancelarPopCallback(void* ptr) {
	daBeep(50);
	rifePre_barra.setValue(0);
	rifePre_frequencia.setText("");
	rifePre_tempo.setText("");
	rifePre_nomeProtocolo.setText("");
	terminaPrograma(rifePre);
}

/**********************************************************************************************//**
 * @fn	void zappicProt_cancelarPopCallback(void *ptr)
 * @brief	Callback, called when the zappic prot qdo botao cancelar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_cancelarPopCallback(void* ptr) {
	daBeep(50);
	zappicProt_barra.setValue(0);
	zappicProt_duracao.setValue(0);
	zappicProt_slicer.setValue(30);
	terminaProgramaZ(zapp);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_cancelarPopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl qdo botao cancelar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_cancelarPopCallback(void* ptr) {
	daBeep(50);
	rifeETDFL_barra.setValue(0);
	rifeETDFL_nomeProtocolo.setText("");
	rifeETDFL_etapa.setValue(0);
	rifeETDFL_numeroProtocolo.setValue(0);
	rifeETDFL_frequencia.setText("");
	terminaPrograma(rifeETDFL);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_cancelarPopCallback(void *ptr)
 * @brief	Callback, called when the rife manual qdo botao cancelar pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_cancelarPopCallback(void* ptr) {
	daBeep(50);
	paraLeitura = false;
	rifeManual_freq01.setValue(0);
	rifeManual_freq02.setValue(0);
	rifeManual_freq03.setValue(0);
	rifeManual_freq04.setValue(0);
	rifeManual_freq05.setValue(0);
	rifeManual_freq06.setValue(0);
	rifeManual_dura01.setValue(0);
	rifeManual_dura02.setValue(0);
	rifeManual_dura03.setValue(0);
	rifeManual_dura04.setValue(0);
	rifeManual_dura05.setValue(0);
	rifeManual_dura06.setValue(0);
	rifeManual_barra.setValue(0);
	rifeManual_numero.setText("");
	terminaPrograma(rifeManual);
}

/**********************************************************************************************//**
 * @fn	void rifePre_bicho1PopCallback(void *ptr)
 * @brief	==== RIFE Predefindo (bicho-1) botao
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifePre_bicho1PopCallback(void* ptr) {
	qualRife = 1;
	preparaRifeBichoX(qualRife);
}

/**********************************************************************************************//**
 * @fn	void rifePre_bicho2PopCallback(void *ptr)
 * @brief	Callback, called when the rife pre bicho 2 botao pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifePre_bicho2PopCallback(void* ptr) {
	qualRife = 2;
	preparaRifeBichoX(qualRife);
}

/**********************************************************************************************//**
 * @fn	void rifePre_bicho3PopCallback(void *ptr)
 * @brief	Callback, called when the rife pre bicho 3 botao pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifePre_bicho3PopCallback(void* ptr) {
	qualRife = 3;
	preparaRifeBichoX(qualRife);
}

/**********************************************************************************************//**
 * @fn	void preparaRifeBichoX(int qualRife)
 * @brief	Prepara rife bicho (le tabela de frequencias)
 * @date	10/12/2019
 * @param 	qualRife	The qual rife.
 **************************************************************************************************/
void preparaRifeBichoX(int qualRife) {
	char buffer[50];
	daBeep(50);
	//quantosEstagios=pgm_read_dword_near(&(tab_rife[qualRife-1][1]));
	pwmFrequencia = (tab_rife[qualRife - 1][3]) * 1000;
	pwmDuracao = (tab_rife[qualRife - 1][2]);
	rifePre_barra.setValue(0);

	ltoa(long(pwmFrequencia), buffer, 10);
	rifePre_frequencia.setText(buffer);
	ltoa(long(pwmDuracao), buffer, 10);
	rifePre_tempo.setText(buffer);
	//itoa (estagio,rasc_num,10);
	//rifePre_etapa.setText(rasc_num);

	strcpy_P(buffer, tab_nomerife[qualRife - 1]);
	rifePre_nomeProtocolo.setText(buffer);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_okPopCallback(void *ptr)
 * @brief	RIFE(2)-teclado botao OK, coloca no display o nome e etapas
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_okPopCallback(void* ptr) {
	char buffer[50];
	daBeep(50);
	uint32_t numero = 0;
	//coloca no display: nome e etapas
	rifeETDFL_numeroProtocolo.getValue(&numero);
	qualRife = int(numero);
	strcpy_P(buffer, tab_nomerife[qualRife - 1]);
	rifeETDFL_nomeProtocolo.setText(buffer);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n1PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 1 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n1PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n2PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 2 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n2PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n3PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 3 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n3PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n4PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 4 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n4PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n5PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 5 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n5PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n6PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 6 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n6PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n7PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 7 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n7PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n8PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 8 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n8PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n9PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 9 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n9PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_n0PopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n 0 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_n0PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_nCPopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl botao tecla n c pop (limpa/cancela)
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_nCPopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeETDFL_numeroProtocoloPopCallback(void *ptr)
 * @brief	Callback, called when the rife etdfl numero protocolo pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeETDFL_numeroProtocoloPopCallback(void* ptr) {
	daBeep(50);

}

/**********************************************************************************************//**
 * @fn	void zappicProt_1KPopCallback(void *ptr)
 * @brief	Callback, Zappicator de 1 KHz botao pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_1KPopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void zappicProt_2K5PopCallback(void *ptr)
 * @brief	Callback, called when the zappic prot 2.5 KHz botao pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_2K5PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void zappicProt_428PopCallback(void *ptr)
 * @brief	Callback, called when the zappic prot 428 Hz botao pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_428PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void zappicProt_529PopCallback(void *ptr)
 * @brief	Callback, called when the zappic prot 529 Hz botao pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void zappicProt_529PopCallback(void* ptr) {
	daBeep(50);
}

// **************OPERACAO MANUAL de Rife - Tela 7 ************************

/**********************************************************************************************//**
 * @fn	void rifeManual_n1PopCallback(void *ptr)
 * @brief	Callback, rife manual botao n1
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n1PopCallback(void* ptr) {//TECLADO - tela 7
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n2PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 2 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n2PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n3PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 3 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n3PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n4PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 4 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n4PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n5PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 5 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n5PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n6PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 6 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n6PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n7PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 7 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n7PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n8PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 8 pop
 * @date	10/12/2019
 * @param [in]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n8PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n9PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 9 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n9PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_n0PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n 0 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_n0PopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_nCPopCallback(void *ptr)
 * @brief	Callback, called when the rife manual botao n c pop (cancela/limpa)
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_nCPopCallback(void* ptr) {
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq01PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 01 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq01PopCallback(void* ptr) {

	int n_erro = 0;
	bool ok_erro = true;
	ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
	//MARIO
	while (ok_erro == false && n_erro <= 3) {

		if (!ok_erro && n_erro <= 3) {
			n_erro++;
			ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
		}
		else if (frequenciaRasc[1] > 2000000) {//confirm. que nao ha freq > 2MHz
			n_erro++;
			ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
		}
	}
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq02PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 02 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq02PopCallback(void* ptr) {
	rifeManual_freq02.getValue(&frequenciaRasc[2]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq03PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 03 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq03PopCallback(void* ptr) {
	rifeManual_freq03.getValue(&frequenciaRasc[3]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq04PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 04 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq04PopCallback(void* ptr) {
	rifeManual_freq04.getValue(&frequenciaRasc[4]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq05PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 05 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq05PopCallback(void* ptr) {
	rifeManual_freq05.getValue(&frequenciaRasc[5]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_freq06PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le frequency 06 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_freq06PopCallback(void* ptr) {
	rifeManual_freq06.getValue(&frequenciaRasc[6]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura01PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 01 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura01PopCallback(void* ptr) {
	rifeManual_dura01.getValue(&duracaoRasc[1]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura02PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 02 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura02PopCallback(void* ptr) {
	rifeManual_dura02.getValue(&duracaoRasc[2]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura03PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 03 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura03PopCallback(void* ptr) {
	rifeManual_dura03.getValue(&duracaoRasc[3]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura04PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 04 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura04PopCallback(void* ptr) {
	rifeManual_dura04.getValue(&duracaoRasc[4]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura05PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 05 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura05PopCallback(void* ptr) {
	rifeManual_dura05.getValue(&duracaoRasc[5]);
	daBeep(50);
}

/**********************************************************************************************//**
 * @fn	void rifeManual_dura06PopCallback(void *ptr)
 * @brief	Callback, called when the rife manual le duracao 06 pop
 * @date	10/12/2019
 * @param [in,out]	ptr	If non-null, the pointer.
 **************************************************************************************************/
void rifeManual_dura06PopCallback(void* ptr) {
	rifeManual_dura06.getValue(&duracaoRasc[6]);
	daBeep(50);
}

/**
* @}
*/

/**
* @addtogroup Protocolos
* @{
*/


//********************************************************
//**ROTINAS DOS PROTOCOLOS - CLARK - RIFE - ZAPPICATOR
//* ******************************************************

/**********************************************************************************************//**
 * @fn	void comecaClark()
 * @brief	Inicia protocolo CLARK
 * @date	10/12/2019
 **************************************************************************************************/
void comecaClark() {
	int k;
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = clark;	///< indica que programa Clark comeca
	estagio = 1;		///< indica que 1 estagio comeca
	k = estagio - 1;
	quantosEstagios = int(tab_clarkTT[k]);
	k = estagio * 2 - 1;
	pwmFrequencia = (tab_clarkTT[k]);
	k = estagio * 2;
	pwmDuracao = (tab_clarkTT[k]);///< tempo em segundos	

	tempoMarcaInicio();
	daBeep(50);
	clarkProt_barra.setValue(0);
	ligaPWM(pwmFrequencia);
}

/**********************************************************************************************//**
 * @fn	void proximoParClark()
 * @brief	Proximo par clark
 * @date	10/12/2019
 **************************************************************************************************/
void proximoParClark() {
	//pega par seguinte de frequencia + tempo, no array da tab_clark
	//estagio:1,2,3,4,5 => 
	//elementos do array => freq,tempo => 1,2; 3,4; 5,6; 7,8; 9,10
	//ser for para "dar um tempo" coloca 0Hz na frequencia e desliga timer
	if (estagio < 5) {
		estagio++;
		k = estagio * 2 - 1;
		pwmFrequencia = (tab_clarkTT[k]);
		k = estagio * 2;
		pwmDuracao = (tab_clarkTT[k]);
		primeiraVez = true;///< para controlar rotinas de clark7min e clark20m
		tempoMarcaInicio();
		daBeep(50);
		ligaPWM(pwmFrequencia);
	}
}

/**********************************************************************************************//**
 * @fn	void clark_7min()
 * @brief	Controle de tempo (7 min) - CLARK
 * @date	10/12/2019
 **************************************************************************************************/
void clark_7min() { // (6 x 3) + 2 para resultar "20")
	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	unsigned long rasc = millis() / 1000;
	if (rasc != protocoloAntesRasc) {
		protocoloAntesRasc = rasc;
		protocoloSegundos++;
		if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
			protocoloSegundos--;
			protocoloSegundosTotal++;
			if (protocoloSegundosTotal == (pwmDuracao - 1)) {
				avancaBarraClark(2);	///< + 2
			}
			else {
				avancaBarraClark(3);	///< (6 x 3) para resultar "20"
			}
		}
		if (estagio < 5 && protocoloSegundosTotal == pwmDuracao) {
			proximoParClark();
		}
		else if (estagio == 5 && protocoloSegundosTotal == pwmDuracao) {
			//clarkProt_barra.setValue(0);
			terminaPrograma(clark);
		}
	}
}

/**********************************************************************************************//**
 * @fn	void clark_20min()
 * @brief	Controle de tempo (20 min) - Clark
 * @date	10/12/2019
 **************************************************************************************************/
void clark_20min() {
	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	unsigned long rasc = millis() / 1000;
	if (rasc != protocoloAntesRasc) {
		protocoloAntesRasc = rasc;
		protocoloSegundos++;
		if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
			protocoloSegundos--;
			protocoloSegundosTotal++;
			avancaBarraClark(1);
		}
		if (protocoloSegundosTotal == pwmDuracao) {
			avancaBarraClark(1); ///< se terminou uma etapa avanca barra no display
			if (estagio < 5) { // se etapa <5 vai para proxima etapa
				protocoloSegundosTotal = 0;
				protocoloSegundos = 0;
				primeiraVez = true;
				proximoParClark();
			}
			else {
				//nada??? nunca passa aqui.
			}
		}
	}
}

/**********************************************************************************************//**
 * @fn	void avancaBarraClark(int incremento)
 * @brief	Avanca barra clark
 * @date	10/12/2019
 * @param 	incremento	int; incremento.
 **************************************************************************************************/
void avancaBarraClark(int incremento) {
	clarkProt_barra.getValue(&barraNextionRasc);
	barraNextionRasc += incremento;
	if (barraNextionRasc <= 100) {
		clarkProt_barra.setValue(barraNextionRasc);
	}
}

/**********************************************************************************************//**
 * @fn	void comecaZappic()
 * @brief	comeca ZAPPICATOR
 * @date	10/12/2019
 **************************************************************************************************/
void comecaZappic() {
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = zapp;	///< indica que programa Zapiccator comeca
	estagio = 1;		///< indica que 1 estagio comeca
	uint32_t flag_zapp;
	quantosEstagios = int(tab_zappicTT[0]); ///< CUIDADO-PROVISORIO-TT
	zappicProt_1K.getValue(&flag_zapp);///< ve se liga 1 KHz
	if (flag_zapp) {
		pwmFrequencia = tab_zappicTT1[estagio * 2 - 1];
	}
	else {
		zappicProt_2K5.getValue(&flag_zapp);///< ve se liga 2.5 KHz
		if (flag_zapp) {
			pwmFrequencia = tab_zappicTT2[estagio * 2 - 1];
		}
		else {
			zappicProt_428.getValue(&flag_zapp);///< ve se liga 428 Hz
			if (flag_zapp) {
				pwmFrequencia = tab_zappicTT3[estagio * 2 - 1];
			}
			else {
				zappicProt_529.getValue(&flag_zapp);///< ve se liga 529 Hz
				pwmFrequencia = tab_zappicTT4[estagio * 2 - 1];
			}
		}
	}

	zappicProt_duracao.getValue(&flag_zapp);///< ve qual tempo de duracao
	pwmDuracao = long(flag_zapp) * 60; ///< converte min para seg
	if (DEBUGANDO) {
		String mensagem = "pwmDuracao";
		dbgMostraDados(mensagem, pwmDuracao);
	}
	zappicProt_barra.setValue(0);///< zera barra no display
	tempoMarcaInicio();

	if (pwmFrequencia > 0) {
		ligaPWMZ(pwmFrequencia);///< liga pwm
	}
	else {
		desligaPWMZ();///< desliga pwm
	}
}

/**********************************************************************************************//**
 * @fn	void controleZappic()
 * @brief	Controle do zappicator
 * @date	10/12/2019
 **************************************************************************************************/
void controleZappic() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;	///< controla (escala) avanco de barras

	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	diferenca = tempoLeAgora();
	if (diferenca <= pwmDuracao) {
		rasc = millis();
		rasc = rasc / 1000;
		if (rasc != protocoloAntesRasc) {
			protocoloAntesRasc = rasc;
			protocoloSegundos++;
			if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraZapp(tempoDivisor);
			}
		}
		if (protocoloSegundosTotal == pwmDuracao) {
			zappicProt_barra.setValue(0);
			zappicProt_duracao.setValue(0);
			zappicProt_slicer.setValue(30);
			terminaProgramaZ(zapp);
		}
	}
	else {
		//nunca passa aqui:Serial.println("FIM-II ZAPP");
		zappicProt_barra.setValue(0);
		zappicProt_duracao.setValue(0);
		zappicProt_slicer.setValue(30);
		terminaProgramaZ(zapp);
	}
}

/**********************************************************************************************//**
 * @fn	void avancaBarraZapp(float tempoDivisor)
 * @brief	Avanca barra zapp
 * @date	10/12/2019
 * @param 	tempoDivisor	float; The tempo divisor.
 **************************************************************************************************/
void avancaBarraZapp(float tempoDivisor) {
	barraNextionRasc = int(tempoDivisor);
	if (barraNextionRasc <= 100) {
		zappicProt_barra.setValue(barraNextionRasc);
	}
}

/**********************************************************************************************//**
 * @fn	void comecaRifePre(int qualRife)
 * @brief	Comeca RIFE PRE
 * @date	10/12/2019
 * @param 	qualRife	int; qual programa rife.
 **************************************************************************************************/
void comecaRifePre(int qualRife) {
	char buffer[50];
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifePre;	///< indica que programa RIFE comeca
	estagio = 1;		///< indica que 1 estagio comeca

	if (qualRife > maximoRife) {
		terminaPrograma(rifePre);					///< ERRO
	}
	else {
		quantosEstagios = tab_rife[qualRife - 1][1];
		pwmDuracao = tab_rife[qualRife - 1][2];
		pwmFrequencia = tab_rife[qualRife - 1][3] * 1000;

		rifePre_barra.setValue(0);			///< inicia display do nextion
		ltoa(long(pwmFrequencia), buffer, 10);
		rifePre_frequencia.setText(buffer);
		itoa(int(pwmDuracao), buffer, 10);
		rifePre_tempo.setText(buffer);
		itoa(estagio, buffer, 10);
		rifePre_etapa.setText(buffer);
		strcpy_P(buffer, tab_nomerife[qualRife - 1]);
		rifePre_nomeProtocolo.setText(buffer);

		tempoMarcaInicio();
		if (pwmFrequencia > 0) {
			ligaPWM(pwmFrequencia);
		}
		else {
			desligaPWM();
		}
	}
}

/**********************************************************************************************//**
 * @fn	void proximoRifePre()
 * @brief	Proximo rife pre
 * @date	10/12/2019
 **************************************************************************************************/
void proximoRifePre() {
	char buffer[20];
	estagio++;
	if (estagio <= quantosEstagios) {
		pwmFrequencia = tab_rife[qualRife - 1][estagio + 2] * 1000;
		//pwmDuracao=(pgm_read_float_near(&(tab_rife[qualRife-1][2])));
		tempoMarcaInicio();

		ltoa(long(pwmFrequencia), buffer, 10); ///< inicia display do nextion
		rifePre_frequencia.setText(buffer);
		itoa(estagio, buffer, 10);
		rifePre_etapa.setText(buffer);

		daBeep(500);
		ligaPWM(pwmFrequencia);
	}
	else {
		rifePre_barra.setValue(0);
		rifePre_frequencia.setText("");
		rifePre_tempo.setText("");
		rifePre_nomeProtocolo.setText("");
		terminaPrograma(rifePre);
	}
}

/**********************************************************************************************//**
 * @fn	void avancaBarraRifePre(float tempoDivisor)
 * @brief	ALGO ESTRANHO COM ESTA AVANCA BARRAS DO RIFE
 * @date	10/12/2019
 * @param 	tempoDivisor	float ;divisor de tempo para inc da barra.
 **************************************************************************************************/
void avancaBarraRifePre(float tempoDivisor) {
	uint32_t rascBarra = 0;
	//rifePre_barra.getValue(&rascBarra);
	//rascBarra++;
	rascBarra = int(tempoDivisor);
	if (rascBarra < 100) {
		rifePre_barra.setValue(rascBarra);
	}
}

/**********************************************************************************************//**
 * @fn	void controleRifePre()
 * @brief	Controle rife pre
 * @date	10/12/2019
 **************************************************************************************************/
void controleRifePre() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor;

	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	diferenca = tempoLeAgora();
	//	if (DEBUGANDO) {
	//		String mensagem = "diferenca";
	//		dbgMostraDados(mensagem, diferenca);
	//	}
	if (diferenca <= pwmDuracao) {
		rasc = millis();
		rasc = rasc / 1000;
		if (rasc != protocoloAntesRasc) {
			protocoloAntesRasc = rasc;
			protocoloSegundos++;
			if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifePre(tempoDivisor);
			}
		}
		if (estagio < quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifePre();
		}
		else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifePre_barra.setValue(0);
			terminaPrograma(rifePre);
		}
	}
	else {
		if (estagio > quantosEstagios) {
			rifePre_barra.setValue(0);
			terminaPrograma(rifePre);
		}
		else {
			proximoRifePre();
		}
	}
}

/**********************************************************************************************//**
 * @fn	void comecaRifeETDFL(int qualRife)
 * @brief	RIFE ETDFL
 * @date	10/12/2019
 * @param 	qualRife	int ;qual o programa Rife.
 **************************************************************************************************/
void comecaRifeETDFL(int qualRife) {
	char buffer[50];
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifeETDFL;	///< indica que programa RIFE comeca
	estagio = 1;		///< indica que 1 estagio comeca

	if (qualRife > maximoRife) {
		terminaPrograma(rifeETDFL);					///<ERRO
	}
	else {
		quantosEstagios = tab_rife[qualRife - 1][1];
		pwmDuracao = tab_rife[qualRife - 1][2];
		pwmFrequencia = tab_rife[qualRife - 1][3] * 1000;

		rifeETDFL_barra.setValue(0);			///<inicia display do nextion
		ltoa(long(pwmFrequencia), buffer, 10);
		rifeETDFL_frequencia.setText(buffer);
		rifeETDFL_etapa.setValue(estagio);
		strcpy_P(buffer, tab_nomerife[qualRife - 1]);
		rifeETDFL_nomeProtocolo.setText(buffer);

		tempoMarcaInicio();
		if (pwmFrequencia > 0) {
			ligaPWM(pwmFrequencia);
		}
		else {
			desligaPWM();///< INCONSISTENTE LOGICA. 
			//se Freq=0 talvez "termina programa" (limpa tudo)
			// e nao "comeca nada"
		}
	}
}

/**********************************************************************************************//**
 * @fn	void proximoRifeETDFL()
 * @brief	Proximo rife etdfl
 * @date	10/12/2019
 **************************************************************************************************/
void proximoRifeETDFL() {
	char buffer[20];
	estagio++;
	if (estagio <= quantosEstagios) {
		pwmFrequencia = tab_rife[qualRife - 1][estagio + 2] * 1000;//*
		//pwmDuracao=(pgm_read_float_near(&(tab_rife[qualRife-1][2])));	//*
		tempoMarcaInicio();

		ltoa(long(pwmFrequencia), buffer, 10); ///<inicia display do nextion
		rifeETDFL_frequencia.setText(buffer);
		rifeETDFL_etapa.setValue(estagio);

		daBeep(500);
		ligaPWM(pwmFrequencia);
	}
	else {
		rifeETDFL_barra.setValue(0);
		rifeETDFL_frequencia.setText("");
		rifeETDFL_etapa.setValue(0);
		rifeETDFL_nomeProtocolo.setText("");
		rifeETDFL_numeroProtocolo.setValue(0);
		terminaPrograma(rifeETDFL);
	}
}

/**********************************************************************************************//**
 * @fn	void avancaBarraRifeETDFL(float tempoDivisor)
 * @brief	Avanca barra rife etdfl
 * @date	10/12/2019
 * @param 	tempoDivisor	float; tempo divisor.
 **************************************************************************************************/
void avancaBarraRifeETDFL(float tempoDivisor) {
	uint32_t rascBarra;
	rascBarra = int(tempoDivisor);
	if (rascBarra < 100) {
		rifeETDFL_barra.setValue(rascBarra);
	}
}

/**********************************************************************************************//**
 * @fn	void controleRifeETDFL()
 * @brief	Controle rife etdfl
 * @date	10/12/2019
 **************************************************************************************************/
void controleRifeETDFL() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;	///<controla (escala) avanco de barras

	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	diferenca = tempoLeAgora();
	//	if (DEBUGANDO) {
	//		String mensagem = "diferenca";
	//		dbgMostraDados(mensagem, diferenca);
	//	}
	if (diferenca <= pwmDuracao) {
		rasc = millis();
		rasc = rasc / 1000;
		if (rasc != protocoloAntesRasc) {
			protocoloAntesRasc = rasc;
			protocoloSegundos++;
			if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifeETDFL(tempoDivisor);
			}
		}
		if (estagio < quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifeETDFL();
		}
		else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifeETDFL_barra.setValue(0);
			terminaPrograma(rifeETDFL);
		}
	}
	else {
		if (estagio > quantosEstagios) {
			rifeETDFL_barra.setValue(0);
			terminaPrograma(rifeETDFL);
		}
		else {
			proximoRifeETDFL();
		}
	}
}

/**********************************************************************************************//**
 * @fn	void comecaRifeManual()
 * @brief	Comeca RIFE MANUAL
 * 			vai ler frequencias e tempos; numero lido - getvalue - usa little endian order 0x01,
 * 			0x02, 0x03,0x04 (4 bytes na msg de retorno)
 * 			0x01+(0x02*256)+(0x03*65536)+(0x04*16777216)
 * @date	10/12/2019
 **************************************************************************************************/
void comecaRifeManual() {
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifeManual;	///< indica que o programa RIFE comeca
	estagio = 1;		///< indica que 1 estagio comeca
	quantosEstagios = 0;

	/**  
	*@brief vai ler frequencias e tempos;
	*   numero lido - getvalue - usa little endian order
	*   0x01,0x02,0x03,0x04 (4 bytes na msg de retorno)
	*   0x01+(0x02*256)+(0x03*65536)+(0x04*16777216)     */

	if (DEBUGANDO) {
		Serial.println("++++++++++++++++++");
		Serial.print("frequenciaRasc1-a-6:");
		Serial.println(frequenciaRasc[1]);
		Serial.println(frequenciaRasc[2]);
		Serial.println(frequenciaRasc[3]);
		Serial.println(frequenciaRasc[4]);
		Serial.println(frequenciaRasc[5]);
		Serial.println(frequenciaRasc[6]);
		Serial.println("++++++++++++++++++");
		Serial.println("duracao de 1-6:");
		Serial.println(duracaoRasc[1]);
		Serial.println(duracaoRasc[2]);
		Serial.println(duracaoRasc[3]);
		Serial.println(duracaoRasc[4]);
		Serial.println(duracaoRasc[5]);
		Serial.println(duracaoRasc[6]);
		Serial.println("*******************");
	}

	if (frequenciaRasc[1] > 0 && duracaoRasc[1] > 0) {///< se nao houver freq1 ou dura1 cai fora
		quantosEstagios++;
		rifeManual_dura01.Set_background_color_bco(nxtYELLOW);
		if (frequenciaRasc[2] > 0 && duracaoRasc[2] > 0) {
			quantosEstagios++;
			rifeManual_dura02.Set_background_color_bco(nxtYELLOW);
			if (frequenciaRasc[3] > 0 && duracaoRasc[3] > 0) {
				quantosEstagios++;
				rifeManual_dura03.Set_background_color_bco(nxtYELLOW);
				if (frequenciaRasc[4] > 0 && duracaoRasc[4] > 0) {
					quantosEstagios++;
					rifeManual_dura04.Set_background_color_bco(nxtYELLOW);
					if (frequenciaRasc[5] > 0 && duracaoRasc[5] > 0) {
						quantosEstagios++;
						rifeManual_dura05.Set_background_color_bco(nxtYELLOW);
						if (frequenciaRasc[6] > 0 && duracaoRasc[6] > 0) {
							quantosEstagios++;
							rifeManual_dura06.Set_background_color_bco(nxtYELLOW);
						}
					}
				}
			}
		}
		pwmDuracao = duracaoRasc[1];
		pwmFrequencia = frequenciaRasc[1];

		rifeManual_barra.setValue(0);			///< inicia display do nextion
		tempoMarcaInicio();

		if (pwmFrequencia > 0) {
			ligaPWM(pwmFrequencia);
		}
		else {
			desligaPWM();
		}
	}
	else {
		terminaPrograma(rifeManual);
	}
}

/**********************************************************************************************//**
 * @fn	void proximoRifeManual()
 * @brief	Proximo rife manual
 * @date	10/12/2019
 **************************************************************************************************/
void proximoRifeManual() {
	estagio++;

	switch (estagio) {
	case 2: {
		rifeManual_dura01.Set_background_color_bco(nxtWHITE);
		duracaoRasc[estagio - 1] = 0;
		break;
	}case 3: {
		rifeManual_dura02.Set_background_color_bco(nxtWHITE);
		duracaoRasc[estagio - 1] = 0;
		break;
	}case 4: {
		rifeManual_dura03.Set_background_color_bco(nxtWHITE);
		duracaoRasc[estagio - 1] = 0;
		break;
	}case 5: {
		rifeManual_dura04.Set_background_color_bco(nxtWHITE);
		duracaoRasc[estagio - 1] = 0;
		break;
	}case 6: {
		rifeManual_dura05.Set_background_color_bco(nxtWHITE);
		duracaoRasc[estagio - 1] = 0;
		break;

	}default: {
		//nada
	}
	}

	rifeManual_barra.setValue(0);

	if (estagio <= quantosEstagios) {
		pwmDuracao = long(duracaoRasc[estagio]);
		pwmFrequencia = long(frequenciaRasc[estagio]);
		tempoMarcaInicio();
		daBeep(500);
		ligaPWM(pwmFrequencia);
	}
	else {
		switch (estagio) {
		case 2: {
			rifeManual_dura02.Set_background_color_bco(nxtWHITE);
			duracaoRasc[estagio] = 0;
			break;
		}case 3: {
			rifeManual_dura03.Set_background_color_bco(nxtWHITE);
			duracaoRasc[estagio] = 0;
			break;
		}case 4: {
			rifeManual_dura04.Set_background_color_bco(nxtWHITE);
			duracaoRasc[estagio] = 0;
			break;
		}case 5: {
			rifeManual_dura05.Set_background_color_bco(nxtWHITE);
			duracaoRasc[estagio] = 0;
			break;
		}case 6: {
			rifeManual_dura06.Set_background_color_bco(nxtWHITE);
			duracaoRasc[estagio] = 0;
			break;
		}default: {
			//nada
		}
		}
		terminaPrograma(rifeManual);
	}
}

/**********************************************************************************************//**
 * @fn	void avancaBarraRifeManual(float tempoDivisor)
 * @brief	Avanca barra rife manual
 * @date	10/12/2019
 * @param 	tempoDivisor	float; The tempo divisor.
 **************************************************************************************************/
void avancaBarraRifeManual(float tempoDivisor) {
	uint32_t rascBarra;
	rascBarra = int(tempoDivisor);
	if (rascBarra < 100 && rascBarra>0) {
		rifeManual_barra.setValue(rascBarra);
	}
}

/**********************************************************************************************//**
 * @fn	void controleRifeManual()
 * @brief	Controle rife manual
 * @date	10/12/2019
 **************************************************************************************************/
void controleRifeManual() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;///< controla (escala) avanco de barras
	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	diferenca = tempoLeAgora();
	//	if (DEBUGANDO) {
	//		String mensagem = "diferenca";
	//		dbgMostraDados(mensagem, diferenca);
	//	}
	if (diferenca <= pwmDuracao) {
		rasc = millis();
		rasc = rasc / 1000;
		if (rasc != protocoloAntesRasc) {
			protocoloAntesRasc = rasc;
			protocoloSegundos++;
			if (protocoloSegundos == 1 && protocoloSegundosTotal < pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifeManual(tempoDivisor);
			}
		}
		if (estagio < quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifeManual();
		}
		else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifeManual_barra.setValue(0);
			terminaPrograma(rifeManual);
		}
	}
	else {
		if (estagio > quantosEstagios) {
			rifeManual_barra.setValue(0);
			terminaPrograma(rifeManual);
		}
		else {
			proximoRifeManual();
		}
	}
}
/**
* @}
*/
