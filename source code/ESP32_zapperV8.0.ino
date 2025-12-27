//ESP32 #include <TimerOneML.h>
#include <Wire.h>
//ESP32 #include <AD9850SPI.h>
#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <Nextion.h>
#include <tabelas_rife-esp32.h>
#include <mario_nextion.h>
/*********************************************************
 *                  Variáveis Globais                    *
**********************************************************/

//MARCAR TRUE SE QUISER SAIDA NO SERIAL MONITOR PARA DEBUGAR
bool DEBUGANDO = true;

//***************** DE HARDWARE

//**************** INA219 - SENSOR DE CORRENTE
//cria instância do INA219
Adafruit_INA219 ina219;
unsigned long tempoIna219Rasc = 0;
int16_t n_ina = 0;
float mediaVolts = 0;
float mediaAmperes = 0;
float accVolts = 0;
float accAmperes = 0;

bool paraLeitura = false; // para parar temporariament leitura


//referência externa TL431 (no AREF) e
//entrada ADC para tensão (double check)
//valor do TL431 está na área de constantes, abaixo
//float ADC_CONST = 0.0048828125;	// 5.00/1024 => v/step
// se for 5.01 => 0.004892578125
//float CONST_431 = 0.003955; //para AREF =>4.05V do TL431
//float CONST_431 = 0.00276562; //para AREF =>2.832V do TL431 c/R=12K+cap10uf
//float CONST_431 = 0.00274511; //para AREF =>2.811V do TL431 c/R=12K+cap10uf
			  
//float Voltagem = 0;
//float minvolt = 9999;
//float maxvolt = 0;

unsigned long tempoADCRasc = 0;

//filtro para tirar "spikes" no ADC/leitura
//ver parte de constantes, abaixo
int buffer_itens = 50;
int buffer_v[50];

//***************** DE TEMPORIZAÇÃO E RELÓGIO
unsigned int segundos, minutos, horas, n7; //relógio
unsigned long tempoAgora = 0;
unsigned long tempoAntes = 0;
unsigned long tempoInicial;	//rascunho no momento que foi ligado/desligado
unsigned long tempoSegundos; //tempo em segundos

//***************** Variáveis do NEXTION
char bufferNextion[90]; //
uint32_t barraNextionRasc;
int k;
//TESTE PARA DELETAR APÓS/NO FINAL
int tamComando;
char bufferNextionRasc[20];

//***************** Variáveis de controle dos Protocolos
//***************** e Menus Rife, Clark, Zappicator
bool emPrograma = false;	//se true esta executando algum protocolo
int qualPrograma = 0;	//identifica programa, 1-clark, 2-zappic, 3-rife, etc)
int estagio = 0; //indica em que estágio está - qual par de frequencias
int quantosEstagios = 0;//indica quantos estágios tem o protocolo
float pwmFrequencia = 0;
float pwmDuracao = 0;

//***************** CLARK
bool primeiraVez = true; 
unsigned long protocoloAntesRasc;
unsigned long protocoloSegundosTotal = 0;
unsigned long protocoloSegundos = 0;

//***************** RIFE
int qualRife = 1;

//***************** Zappicator

//***************** Operação Manual (Rife)
uint32_t frequenciaRasc[7];
uint32_t duracaoRasc[7];

/*********************************************************
*                     Constantes                         *
**********************************************************/

//***************** INA219 e TL431
const unsigned long intervaloLeituraIna219 = 10000; //10 seg. em milissegundos
//const float CONST_431 = 0.003955; //para AREF =>4.05V do TL431
//const float CONST_431 = 0.00390625; //para AREF =>4.00V do TL431
//const float CONST_431 = 0.00275879; //para AREF =>2.825V do TL431 c/R=12K+cap10uf
//const float V_ajuste_div = 0.201603;//mais do divisor de tensão ligado em A0 =>R1=22120 e R2=5570
//const float alpha = 0.98; //do filtro passa baixa


/*ESP32
**************** PINOS DO ADC
const uint8_t leCanalA0 = 0; //entrada - canal analogico A0
const unsigned long intervaloLeituraADC = 10000; //10 seg. em milissegundos
*/

/*ESP32
**************** DDS 9850 (gerador de frequencias)
const double ajusteFreq = 124985500;//teoricamente isto deverá ser feito placa por placa
const int fase = 0; //depende do soft/aplicação, fase talvez mude para variáveis
*/

/*ESP32
**************** Pinos(do arduino/MEGA) para o DDS9850
const int W_CLK_PIN = 52;	// placa AD - pin - 2 =>SCK
const int FQ_UD_PIN = 50;	// placa AD - pin - 3 => PB0
const int DATA_PIN = 51;	// placa AD - pin - 4 => MSOI
const int RESET_PIN = 53;	// placa AD - pin - 5 => PD7
*/

//***************** para ligar bit(led) da placa ===sem uso
//const int bitLed = 13;

//***************** Liga/desliga
const int liga = HIGH;
const int desliga = LOW;

//***************** hardware == definicao dos pinos buz,teclas e pwm
//const int bitBuzina = 6; //buzina => PD6; **** NO MEGA (PH3)
//const int pinoPWM = 11;	//saida do PWM => PB1 (9)=> OC1A; *** no MEGA (11)PB5

const int buzina = 4;

//***************** IDENTIFICAÇÃO DOS PROTOCOLOS
const int clark = 1; // Protocolo Clark 30KHz => 7m-20m-7m-20m-7m
const int rifePre = 3; //Rife pré-definidos (3 exemplos)
const int zapp = 2;//ZAPPICATOR (padrão é 1KHZ durante 30m)
const int rifeETDFL = 4; //Rife entrando Numero do Protocolo
const int rifeManual = 5; // digitando frequencias e duração (até 6 freq.)
const int maximoRife = 268; // é o número de protocolos existentes na tabela
							//tabela_rife.h
//ESP32
const uint8_t freq_var_canal = 0;
const uint8_t freq_1K_canal = 1;
const uint8_t buz_canal = 2;
const uint8_t duty_todos = 1;	//50% duty cycle para todos>1000Hz
const uint8_t duty_baixo = 8;	//8 bits =>256 p/freq baixas

const int clark_rife = 27;
const int zappicator = 26;

const uint32_t freq_inicial = 1000;	//1KHz

uint32_t freq_var = 0;

//***************** Protocolos **********************************
/***************** descricao linha da CLARK {n,x,y,x,y,x,y,.....}
n - quantidade de grupos frequencia+tempo ligada,
x-frequencia, (em Hz)
y-tempo ligada a freq. (em segundos)
Clark "oficial" (5 fases/etapas):
30KHz->7min;descanco->20min;30KHz->7min;descanco->20min;30KHz->7min;FIM
*/
const float tab_clark[11] = { 5,30000,420,0,1200,30000,420,0,1200,30000,420 };
//TESTE CLARK - tempo reduzido para alguns segundos
const float tab_clarkTT[11] = { 5,30000,7,0,20,30000,7,0,20,30000,7 };

/***************** descricao linha da zappicator {n, x,y,x,y,x,y,.....}
n - quantidade de grupos frequencia+tempo ligada,
x-frequencia, (em Hz)
y-tempo ligada a freq. (em segundos) -> 15min = 900seg
o original é uma frequencia 1KHz e uns +/- 15min (tembém li 30m)
aqui pode colocar mais freq igual a clark "pares"=freq+tempo
*/
const float tab_zappic[3] = { 1,1000,900 };
//TESTE ZAPPICATOR
const float tab_zappicTT[3] = { 1,1000,10 };
const float tab_zappicTT1[3] = { 1,1000,10 };
const float tab_zappicTT2[3] = { 1,2500,10 };
const float tab_zappicTT3[3] = { 1,428,10 };
const float tab_zappicTT4[3] = { 1,529,10 };

/***************** CAFL-ETDFL-2018 RIFE: {x,y,t, zzz,zzz,zzz,.....}
x - numero do programa,
y - quantidade de frequencias,
t - tempo para cada frequencia em segundos-por enquanto,(minx60)
zzz- as frequencias em *** KHz ***
TRANSFERI TABELAS RIFE PARA => tabelas_rife.h (está nos "include")
olhar comentários lá e também "a fazer".
*/

//***************** DEFINIÇÕES DO NEXTION ********************
//***********no arquivo mario_nextion.h (nos "include")

//****************DEFINIÇÕES/PROTOTIPOS *********************
void daBeep(int tempoBip);//bipa por(miliseg) definido em "int"
void avisaXseg(int tempoBip, int numeroBips);
void tickRelogio();
void iniciaDebug();
void dbgMostraDados(String mensagem, int valor);
void limpaStatus(int protocolo);
void tempoMarcaInicio();//guarda hora que começou protocolo
unsigned long tempoLeAgora(); //retorna false se estourou tempo

//esp32
//void iniciaDDS9850();
void iniciaPWMs();
//void terminaProgramaDDS9850(int protocolo);
void terminaPrograma(int protocolo);
//void ligaFreqDDS9850(float pwmFreq);
void ligaPWM(float pwmFreq);
//void desligaDDS9850();
void desligaPWM();

//void iniciaPWMT1();
void terminaProgramaZ(int protocolo);
void ligaPWMZ(float entraFreq);
void desligaPWMZ();
//void ligaFreqPWMTZ(float pwmFreq);

void iniciaINA219();
void leINA219();
void dbgMostraIna219(float voltagemCircuito, float voltagemShunt, float voltagemEntrada, float corrente_mA, float potencia_mW, float mediaVolts, float mediaAmperes);

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

//*************************************SETUP
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
//************************************* LOOP
void loop()
{
	nexLoop(nex_listen_list);
	void tickRelogio();
	if (emPrograma) {
		switch (qualPrograma) {
		case clark: {  //***** protocolo Clark
			if (estagio <= 5) {
				if (estagio == 1 || estagio == 3 || estagio == 5) {
					clark_7min();
				}
				if (estagio == 2 || estagio == 4) {
					clark_20min();
				}
			}else {
				terminaPrograma(clark);
			}
			break;
		}case zapp: {//***** protocolo zappicator
			controleZappic();
			break;
		}case rifePre: {//***** protocolo Rife pré-definido
			controleRifePre();
			break;
		}case rifeETDFL: {//***** protocolo Rife com N. do protocolo
			controleRifeETDFL();
			break;
		}case rifeManual: {//***** entrada manual de frequências (6 freq+duração)
			controleRifeManual();
			break;
		}default: {
			break;
		}
		}
	}else {
		
		if (!paraLeitura) {
			leINA219();
			//leFiltraConverteADC(leCanalA0);
		}
	}
}
//************************************************** FIM LOOP

//***************** rotinas de hardware
//************************************* BEEP (tempo em milisegundos)
void daBeep(int tempoBip) {
	//um bip
	ledcWrite(buz_canal, duty_todos);
	delay(tempoBip);
	ledcWrite(buz_canal, 0);//bit, frequencia, tempo ms
}
//******************************************* AVISO DE N BEEPS POR X SEG
//tempo_bip em milissegundos de buzina acionada
//n_bips - o numero de bips
//1 segundo entre bips (1000 ms)
//***cuidado com rotina para nao travar processamento
void avisaXseg(int tempoBip, int numeroBips) {
	int x;

	for (x = 0; x<numeroBips; x++) {
		//outra opção, em vez de tone é => daBeep(tempoBip);
		daBeep(tempoBip);
		delay(500); // 0,5 segundo entre bips
	}
}
//***************************** TICK DE RELOGIO SIMPLES (por software)
void tickRelogio() {
	tempoAgora = millis();
	tempoAgora = tempoAgora / 1000;

	if (tempoAgora != tempoAntes) {
		tempoAntes = tempoAgora;
		segundos++;
		if (segundos>59) {
			minutos++;
			segundos = 0;
			if (minutos>59) {
				horas++;
				minutos = 0;
				segundos = 0;
				if (horas>23) {
					horas = 0;
					minutos = 0;
					segundos = 0;
				}
			}
		}
	}
}
//**********************************  inicia DEBUG no terminal serial/Micro
void iniciaDebug() {
	//Serial.begin(9600); //para sair no PC
	//while (!Serial);// wait for serial port to connect. Needed for native USB
	Serial.println(" Inicio do ESP32-Zapper-V8 ");
	Serial.println("timers parados");
}
//***********************************
void dbgMostraDados(String mensagem, int valor) {
	Serial.print(mensagem);
	Serial.println(": ");
	Serial.println(valor);
	Serial.println("=========");
}
//************************************* LIMPA STATUS
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
//****************************** GUARDA TEMPO INICIAL DO PROTOCOLO
void tempoMarcaInicio() {
	tempoInicial = millis();//marca qdo comecou em milissegundos
	protocoloSegundosTotal = 0;
	protocoloSegundos = 0;
	protocoloAntesRasc = 0;
	primeiraVez = true;
	paraLeitura = true;
}
//***************** LE TEMPO QUE PASSOU DESDE INICIO PROGRAMA
// retorna false se estourou tempo
//verifica o tempo que timer-pwm está ligado/desligado
//em funcao do programado em "tempoReferencia" - em segundos
//tempoInicial foi quando comecou (na rotina tempoMarcaInicio)
//rasc é agora
//tempoReferencia é quanto tem que demorar
unsigned long tempoLeAgora() {
	unsigned long diferenca = 0;
	unsigned long rasc = millis();
	
	diferenca = (rasc - tempoInicial)/1000;//para ter em segundos divide por 1000
	
	return diferenca; 		//volta tempo decorrido em segundos
}
//********************************************** ROTINAS DO ESP32/AD9850
void iniciaPWMs() {
	
	//zappicator
	ledcSetup(freq_1K_canal, freq_inicial, duty_todos);
	ledcAttachPin(zappicator, freq_1K_canal);

	//clark-rife
	ledcSetup(freq_var_canal, freq_inicial, duty_todos);
	ledcAttachPin(clark_rife, freq_var_canal);

}
//*****************
void terminaPrograma(int protocolo) {
	ledcWrite(freq_var_canal, 0);
	limpaStatus(protocolo); //deve faltar "limpar" mais flags e Ns
	avisaXseg(50, 5);
}

//*****************
void ligaPWM(float pwmFreq) {
	uint32_t rasc = pwmFreq;
	if (pwmFreq>0) {
		if (pwmFreq > 1000) {
			ledcSetup(freq_var_canal, rasc, duty_todos); // 1 bit duty
			ledcWrite(freq_var_canal, 1);	//0 ou 1 = 50% duty
		}
		else {
			ledcSetup(freq_var_canal, rasc, duty_baixo); // 8 bits duty
			ledcWrite(freq_var_canal, 127);	//127 = 50% duty
		}
	}else {
		ledcWrite(freq_var_canal, 0);
	}
}

//*******************
void desligaPWM() {
	ledcWrite(freq_var_canal, 0);
}

//********************************** ROTINAS DO TIMER1 (para zappicator)

//=================
void terminaProgramaZ(int protocolo) {
	ledcWrite(freq_1K_canal, 0);
	limpaStatus(protocolo);	//deve faltar "limpar" mais flags e Ns
	avisaXseg(50, 5);
}
//=================
void ligaPWMZ(float entraFreq) {
	uint32_t rasc = entraFreq;
	if (entraFreq>0) {
		if (entraFreq > 1000) {
			ledcSetup(freq_1K_canal, rasc, duty_todos);
			ledcWrite(freq_1K_canal, 1);
		}
		else {
			ledcSetup(freq_1K_canal, rasc, duty_baixo);
			ledcWrite(freq_1K_canal, 127);
		}
		
	}else {
		ledcWrite(freq_1K_canal, 0);
	}
}
//=================
void desligaPWMZ() {
	ledcWrite(freq_1K_canal, 0);
}


//********************************** ROTINAS DO INA219 - SENSOR

void iniciaINA219() {
	ina219.begin();
	ina219.setCalibration_16V_400mA();
}
//********************************************
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
//************************************
void dbgMostraIna219(float voltagemCircuito, float voltagemShunt, float voltagemEntrada, float corrente_mA, float potencia_mW, float mediaVolts, float mediaAmperes) {
	unsigned long agora = millis();
	unsigned long tempo = agora - tempoIna219Rasc;
	
	if (tempo>intervaloLeituraIna219) {
		Serial.print("Voltagem Circuito:   "); Serial.print(voltagemCircuito); Serial.println(" V");
		Serial.print("Voltagem SHUNT: "); Serial.print(voltagemShunt); Serial.println(" mV");
		Serial.print("Voltagem Entrada:  "); Serial.print(voltagemEntrada); Serial.println(" V");
		Serial.print("Corrente:       "); Serial.print(corrente_mA); Serial.println(" mA");
		Serial.print("Potência:         "); Serial.print(potencia_mW); Serial.println(" mW");
		Serial.print("Media-V-circuito:"); Serial.print(mediaVolts); Serial.println("  V");
		Serial.print("Media-A-entrada:"); Serial.print(mediaAmperes); Serial.println(" mA");
		Serial.println(F("========================="));
		tempoIna219Rasc = millis();
	}
}
/*ESP32
//********************************* Rotinas do ADC medição tensão em A0

void iniciaADC(uint8_t leCanalX) {
	analogReference(EXTERNAL);
	delay(2);
	analogRead(leCanalX);
	delayMicroseconds(120);
	analogRead(leCanalX);
	delayMicroseconds(120);
}
//================= Le N vezes ADC; N=buffer_itens
void leTensaoADC(uint8_t leCanalX) {
	for (int v = 0; v<buffer_itens; v++) {
		buffer_v[v] = analogRead(leCanalX);
		delay(2);
	}
}
//****************** outra possibilidade de leitura da ADC
//não utilizada nesta versão
//================= le uma vez o ADC
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
//	analogReference(EXTERNAL); //já está feita ref qdo inicializa ADC
//	delay(2);
	uint16_t rasc_leVcc = analogRead(leCanalX);
	delay(2);
	//CONST_431 => para AREF =>2.825V do TL431 c/R=12K+cap10uf
	//V_ajuste_div => divisor de tensão ligado em A0 =>R1=22120 e R2=5570
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

//============================================
//	ROTINAS DO NEXTION
//============================================
/***********PAGINAS definidas no mario_nextion.h *****
NexPage page0    = NexPage(0, 0, "page0");//menuPrinc-Menu Principal
NexPage page1    = NexPage(1, 0, "page1");//clarkProt-Protocolo CLARK
NexPage page2    = NexPage(2, 0, "page2");//teste01-pag de teste geral
NexPage page3    = NexPage(3, 0, "page3");//rifePre-Protocolo RIFE pré-definido
NexPage page4    = NexPage(4, 0, "page4");//zappicProt-Protocolo Zappicator
NexPage page5    = NexPage(5, 0, "page5");//rifeETDFL-Protocolo Rife c/ETDFL 1017-2018
NexPage page6    = NexPage(6, 0, "page6");//pagFig-pag com figura grande
NexPage page7    = NexPage(7, 0, "page7");//rifeManual- pag RIFE Manual 6 seq de freq+tempo
*/

void clarkProt_iniciarPopCallback(void *ptr) {
	daBeep(50); //inicia protocolo CLARK
	if (!emPrograma) {
		comecaClark();
	}else {
		avisaXseg(100, 3);
	}
}
//=================================================
void rifePre_iniciarPopCallback(void *ptr) {
	daBeep(50); 	//inicia protocolo RIFE (1)
	if (!emPrograma) {
		comecaRifePre(qualRife);
	}
	else {
		avisaXseg(100, 3);
	}
}
//=================================================
void zappicProt_iniciarPopCallback(void *ptr) {
	daBeep(50); //inicia protocolo ZAPPICATOR
	if(!emPrograma) {
		comecaZappic();
	}else{
		avisaXseg(100, 3);
	}
}
//=================================================
void rifeETDFL_iniciarPopCallback(void *ptr) {
	daBeep(50); //inicia protocolo RIFE (2)
	if(!emPrograma) {
		comecaRifeETDFL(qualRife);
	}else{
		avisaXseg(100, 3);
	}
}
//====================================================
void rifeManual_iniciarPopCallback(void *ptr) {
	//inicia protocolo opManual
	daBeep(50);
	if (!emPrograma) {
		comecaRifeManual();
	}else{
		avisaXseg(100, 3);
	}
}
//=================================================
void clarkProt_cancelarPopCallback(void *ptr) {
	daBeep(50);
	terminaPrograma(clark);
}
//=================================================
void rifePre_cancelarPopCallback(void *ptr) {
	daBeep(50);
	rifePre_barra.setValue(0);
	rifePre_frequencia.setText("");
	rifePre_tempo.setText("");
	rifePre_nomeProtocolo.setText("");
	terminaPrograma(rifePre);
}
//=================================================
void zappicProt_cancelarPopCallback(void *ptr) {
	daBeep(50);
	zappicProt_barra.setValue(0);
	zappicProt_duracao.setValue(0);
	zappicProt_slicer.setValue(30);
	terminaProgramaZ(zapp);
}
//=================================================
void rifeETDFL_cancelarPopCallback(void *ptr) {
	daBeep(50);
	rifeETDFL_barra.setValue(0);
	rifeETDFL_nomeProtocolo.setText("");
	rifeETDFL_etapa.setValue(0);
	rifeETDFL_numeroProtocolo.setValue(0);
	rifeETDFL_frequencia.setText("");
	terminaPrograma(rifeETDFL);
}
//=====================================================
void rifeManual_cancelarPopCallback(void *ptr) {
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
//=================================================
//RIFE Pré(bicho-1)
void rifePre_bicho1PopCallback(void *ptr) {
	qualRife = 1;
	preparaRifeBichoX(qualRife);
}
//================================================
void rifePre_bicho2PopCallback(void *ptr) {
	qualRife = 2;
	preparaRifeBichoX(qualRife);
}
//=================================================
void rifePre_bicho3PopCallback(void *ptr) {
	qualRife = 3;
	preparaRifeBichoX (qualRife);
}
//=================================================
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
//=================================================
//RIFE(2)-teclado
void rifeETDFL_okPopCallback(void *ptr) {
	char buffer[50];
	daBeep(50);
	uint32_t numero = 0;
	rifeETDFL_numeroProtocolo.getValue(&numero);
	qualRife = int(numero);
	strcpy_P(buffer, tab_nomerife[qualRife - 1]);
	rifeETDFL_nomeProtocolo.setText(buffer);
}
void rifeETDFL_n1PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n2PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n3PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n4PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n5PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n6PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n7PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n8PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n9PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_n0PopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_nCPopCallback(void *ptr) {
	daBeep(50);
}
void rifeETDFL_numeroProtocoloPopCallback(void *ptr) {
	daBeep(50);
	//coloca do display: nome e etapas
}
//===================================================
//Zappicator
void zappicProt_1KPopCallback(void *ptr) {
	daBeep(50);
}
void zappicProt_2K5PopCallback(void *ptr) {
	daBeep(50);
}
void zappicProt_428PopCallback(void *ptr) {
	daBeep(50);
}
void zappicProt_529PopCallback(void *ptr) {
	daBeep(50);
}
//=============================================================
//OPERACAO MANUAL
//
void rifeManual_n1PopCallback(void *ptr) {//TECLADO - tela 7
	daBeep(50);
}
void rifeManual_n2PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n3PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n4PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n5PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n6PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n7PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n8PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n9PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_n0PopCallback(void *ptr) {
	daBeep(50);
}
void rifeManual_nCPopCallback(void *ptr) {
	daBeep(50);
}
//====================================================================
void rifeManual_freq01PopCallback(void *ptr) {//TECLADO - tela 7
	
	int n_erro = 0;
	bool ok_erro = true;
	ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
	//MARIO
	while (ok_erro == false && n_erro <= 3) {

		if (!ok_erro && n_erro <= 3) {
			n_erro++;
			ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
		}else if (frequenciaRasc[1]>2000000) {//confirm. que não há freq > 2MHz
			n_erro++;
			ok_erro = rifeManual_freq01.getValue(&frequenciaRasc[1]);
		}
	}
	daBeep(50);
}
void rifeManual_freq02PopCallback(void *ptr) {
	rifeManual_freq02.getValue(&frequenciaRasc[2]);
	daBeep(50);
}
void rifeManual_freq03PopCallback(void *ptr) {
	rifeManual_freq03.getValue(&frequenciaRasc[3]);
	daBeep(50);
}
void rifeManual_freq04PopCallback(void *ptr) {
	rifeManual_freq04.getValue(&frequenciaRasc[4]);
	daBeep(50);
}
void rifeManual_freq05PopCallback(void *ptr) {
	rifeManual_freq05.getValue(&frequenciaRasc[5]);
	daBeep(50);
}
void rifeManual_freq06PopCallback(void *ptr) {
	rifeManual_freq06.getValue(&frequenciaRasc[6]);
	daBeep(50);
}
void rifeManual_dura01PopCallback(void *ptr) {
	rifeManual_dura01.getValue(&duracaoRasc[1]);
	daBeep(50);
}
void rifeManual_dura02PopCallback(void *ptr) {
	rifeManual_dura02.getValue(&duracaoRasc[2]);
	daBeep(50);
}
void rifeManual_dura03PopCallback(void *ptr) {
	rifeManual_dura03.getValue(&duracaoRasc[3]);
	daBeep(50);
}
void rifeManual_dura04PopCallback(void *ptr) {
	rifeManual_dura04.getValue(&duracaoRasc[4]);
	daBeep(50);
}
void rifeManual_dura05PopCallback(void *ptr) {
	rifeManual_dura05.getValue(&duracaoRasc[5]);
	daBeep(50);
}
void rifeManual_dura06PopCallback(void *ptr) {
	rifeManual_dura06.getValue(&duracaoRasc[6]);
	daBeep(50);
}
//********************************************************
//*		ROTINAS DOS PROTOCOLOS - CLARK - RIFE - ZAPPICATOR
//********************************************************
//***************************** CLARK
void comecaClark() {
	int k;
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = clark;	//seta que é o programa Clark que começa
	estagio = 1;		//seta que é 1 estagio que começa
	k = estagio - 1;
	quantosEstagios = int(tab_clarkTT [k]);
	k = estagio * 2 - 1;
	pwmFrequencia = (tab_clarkTT [k]);
	k = estagio * 2;
	pwmDuracao = (tab_clarkTT [k]);//tempo em segundos	

	tempoMarcaInicio();
	daBeep(50);
	clarkProt_barra.setValue(0);
	ligaPWM(pwmFrequencia);
}
//***************** proximo estágio clark
void proximoParClark() {
	//pega par seguinte de frequencia + tempo, no array da tab_clark
	//estagio:1,2,3,4,5 => 
	//elementos do array => freq,tempo => 1,2; 3,4; 5,6; 7,8; 9,10
	//ser for para "dar um tempo" coloca 0Hz na frequencia e desliga timer
	if (estagio<5) {
		estagio++;
		k = estagio * 2 - 1;
		pwmFrequencia = (tab_clarkTT [k]);
		k = estagio * 2;
		pwmDuracao = (tab_clarkTT [k]);
		primeiraVez = true;//para controlar rotinas de clark7min e clark20m
		tempoMarcaInicio();
		daBeep(50);
		ligaPWM(pwmFrequencia);
	}
}
//***************** Controle tempo - CLARK
//=================
void clark_7min() { // (6 x 3) + 2 para resultar "20")
	if (primeiraVez) {
		protocoloAntesRasc = millis()/1000;
		primeiraVez = false;
	}
	unsigned long rasc = millis()/1000;
	if (rasc != protocoloAntesRasc) {
		protocoloAntesRasc = rasc;
		protocoloSegundos++;
		if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
			protocoloSegundos--;
			protocoloSegundosTotal++;
			if (protocoloSegundosTotal == (pwmDuracao - 1)) {
				avancaBarraClark(2);	//+ 2
			}else {
				avancaBarraClark(3);	//(6 x 3)
			}							//para resultar "20"
		}
		if (estagio<5 && protocoloSegundosTotal == pwmDuracao) {
			proximoParClark();
		}else if (estagio == 5 && protocoloSegundosTotal == pwmDuracao) {
			//clarkProt_barra.setValue(0);
			terminaPrograma(clark);
		}
	}
}
//=================
void clark_20min() {
	if (primeiraVez) {
		protocoloAntesRasc = millis()/1000;
		primeiraVez = false;
	}
	unsigned long rasc = millis()/1000;
	if (rasc != protocoloAntesRasc) {
		protocoloAntesRasc = rasc;
		protocoloSegundos++;
		if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
			protocoloSegundos--;
			protocoloSegundosTotal++;
			avancaBarraClark(1);
		}
		if (protocoloSegundosTotal == pwmDuracao) {
			avancaBarraClark(1);
			if (estagio<5) {
				protocoloSegundosTotal = 0;
				protocoloSegundos = 0;
				primeiraVez = true;
				proximoParClark();
			}else {
				//nada??? nunca passa aqui.
			}
		}
	}
}
//=========================================================	
void avancaBarraClark(int incremento) {
	clarkProt_barra.getValue(&barraNextionRasc);
	barraNextionRasc += incremento;
	if (barraNextionRasc<=100) {
		clarkProt_barra.setValue(barraNextionRasc);
	}
}
//
//******************************************* ZAPPICATOR
//
void comecaZappic() {
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = zapp;	//seta que é o programa Zapiccator que começa
	estagio = 1;		//seta que é 1 estagio que começa
	uint32_t flag_zapp;
	quantosEstagios = int(tab_zappicTT [0]); //CUIDADO-PROVISORIO-TT
	zappicProt_1K.getValue(&flag_zapp);
	if (flag_zapp) {
		pwmFrequencia = tab_zappicTT1 [estagio * 2 - 1];
	}else {
		zappicProt_2K5.getValue(&flag_zapp);
		if (flag_zapp) {
			pwmFrequencia = tab_zappicTT2 [estagio * 2 - 1];
		}else {
			zappicProt_428.getValue(&flag_zapp);
			if (flag_zapp) {
				pwmFrequencia = tab_zappicTT3 [estagio * 2 - 1];
			}else {
				zappicProt_529.getValue(&flag_zapp);
				pwmFrequencia = tab_zappicTT4 [estagio * 2 - 1];
			}
		}
	}

	zappicProt_duracao.getValue(&flag_zapp);	//converte min para seg
	pwmDuracao = long(flag_zapp) * 60;
	if (DEBUGANDO) {
		String mensagem = "pwmDuracao";
		dbgMostraDados(mensagem, pwmDuracao);
	}
	zappicProt_barra.setValue(0);
	tempoMarcaInicio();
	
	if (pwmFrequencia>0) {
		ligaPWMZ(pwmFrequencia);
	}else {
		desligaPWMZ();
	}
}
//****************************************** Controle Zappicator
void controleZappic() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;	//controla (escala) avanço de barras

	if (primeiraVez) {
		protocoloAntesRasc = millis() / 1000;
		primeiraVez = false;
	}
	diferenca = tempoLeAgora();
	if (diferenca<=pwmDuracao) {
		rasc = millis();
		rasc = rasc / 1000;
		if (rasc != protocoloAntesRasc) {
			protocoloAntesRasc = rasc;
			protocoloSegundos++;
			if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
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
	}else {
		//nunca passa aqui:Serial.println("FIM-II ZAPP");
		zappicProt_barra.setValue(0);
		zappicProt_duracao.setValue(0);
		zappicProt_slicer.setValue(30);
		terminaProgramaZ(zapp);
	}
}
//********************************************* Avança Barra Zappicator
void avancaBarraZapp(float tempoDivisor) {
	barraNextionRasc = int(tempoDivisor);
	if (barraNextionRasc <= 100) {
		zappicProt_barra.setValue(barraNextionRasc);
	}
}
//******************************
//			RIFE PRE
//******************************

void comecaRifePre(int qualRife) {
	char buffer[50];
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifePre;	//seta que é o programa RIFE que começa
	estagio = 1;		//seta que é 1 estagio que começa

	if (qualRife > maximoRife) {
		terminaPrograma(rifePre);					//ERRO
	}else {
			quantosEstagios = tab_rife[qualRife - 1][1];
			pwmDuracao = tab_rife[qualRife - 1][2];
			pwmFrequencia = tab_rife[qualRife - 1][3] * 1000;

			rifePre_barra.setValue(0);			//inicia display do nextion
			ltoa(long(pwmFrequencia), buffer, 10);
			rifePre_frequencia.setText(buffer);
			itoa(int(pwmDuracao), buffer, 10);
			rifePre_tempo.setText(buffer);
			itoa(estagio, buffer, 10);
			rifePre_etapa.setText(buffer);
			strcpy_P(buffer, tab_nomerife[qualRife - 1]);
			rifePre_nomeProtocolo.setText(buffer);
			
			tempoMarcaInicio();
			if (pwmFrequencia>0) {
				ligaPWM(pwmFrequencia);
			}else {
				desligaPWM();
			}
	}
}
//***************************************************** proximo rifePre
void proximoRifePre() {
	char buffer[20];
	estagio++;
	if (estagio <= quantosEstagios) {
		pwmFrequencia = tab_rife[qualRife - 1][estagio + 2] * 1000;
		//pwmDuracao=(pgm_read_float_near(&(tab_rife[qualRife-1][2])));
		tempoMarcaInicio();
				
		ltoa(long(pwmFrequencia), buffer, 10); //inicia display do nextion
		rifePre_frequencia.setText(buffer);
		itoa(estagio, buffer, 10);
		rifePre_etapa.setText(buffer);

		daBeep(500);
		ligaPWM(pwmFrequencia);
	}else {
		rifePre_barra.setValue(0);
		rifePre_frequencia.setText("");
		rifePre_tempo.setText("");
		rifePre_nomeProtocolo.setText("");
		terminaPrograma(rifePre);
	}
}
//************************************************************
//ALGO ESTRANHO COM ESTA AVANCA BARRAS DO RIFE
//
void avancaBarraRifePre(float tempoDivisor) {
	uint32_t rascBarra = 0;
	//rifePre_barra.getValue(&rascBarra);
	//rascBarra++;
	rascBarra = int(tempoDivisor);
	if (rascBarra < 100) {
		rifePre_barra.setValue(rascBarra);
	}
}
//*********************************************************
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
			if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifePre(tempoDivisor);
			}
		}
		if (estagio<quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifePre();
		}else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifePre_barra.setValue(0);
			terminaPrograma(rifePre);
		}
	}else {
		if (estagio>quantosEstagios) {
			rifePre_barra.setValue(0);
			terminaPrograma(rifePre);
		}else {
			proximoRifePre();
		}
	}
}
//*************************************************
//					RIFE ETDFL
//************************************************
void comecaRifeETDFL(int qualRife) {
	char buffer[50];
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifeETDFL;	//seta que é o programa RIFE que começa
	estagio = 1;		//seta que é 1 estagio que começa

	if (qualRife > maximoRife) {
		terminaPrograma(rifeETDFL);					//ERRO
	}else {
		quantosEstagios = tab_rife[qualRife - 1][1];
		pwmDuracao = tab_rife[qualRife - 1][2];
		pwmFrequencia = tab_rife[qualRife - 1][3] * 1000;

		rifeETDFL_barra.setValue(0);			//inicia display do nextion
		ltoa(long(pwmFrequencia), buffer, 10);
		rifeETDFL_frequencia.setText(buffer);
		rifeETDFL_etapa.setValue(estagio);
		strcpy_P(buffer, tab_nomerife[qualRife - 1]);
		rifeETDFL_nomeProtocolo.setText(buffer);

		tempoMarcaInicio();
		if (pwmFrequencia>0) {
			ligaPWM(pwmFrequencia);
		}else {
			desligaPWM();//INCONSISTENTE LÓGICA. 
			//se Freq=0 talvez "termina programa" (limpa tudo)
			// e não "começa nada"
		}
	}
}
//**************************************************************
void proximoRifeETDFL() {
	char buffer[20];
	estagio++;
	if (estagio <= quantosEstagios) {
		pwmFrequencia = tab_rife[qualRife - 1][estagio + 2] * 1000;//*
		//pwmDuracao=(pgm_read_float_near(&(tab_rife[qualRife-1][2])));	//*
		tempoMarcaInicio();

		ltoa(long(pwmFrequencia), buffer, 10); //inicia display do nextion
		rifeETDFL_frequencia.setText(buffer);
		rifeETDFL_etapa.setValue(estagio);

		daBeep(500);
		ligaPWM(pwmFrequencia);
	}else {
		rifeETDFL_barra.setValue(0);
		rifeETDFL_frequencia.setText("");
		rifeETDFL_etapa.setValue(0);
		rifeETDFL_nomeProtocolo.setText("");
		rifeETDFL_numeroProtocolo.setValue(0);
		terminaPrograma(rifeETDFL);
	}
}
//************************************************************
void avancaBarraRifeETDFL(float tempoDivisor) {
	uint32_t rascBarra;
	rascBarra = int(tempoDivisor);
	if (rascBarra<100) {
		rifeETDFL_barra.setValue(rascBarra);
	}
}
//************************************************************
void controleRifeETDFL() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;	//controla (escala) avanço de barras

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
			if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifeETDFL(tempoDivisor);
			}
		}
		if (estagio<quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifeETDFL();
		}else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifeETDFL_barra.setValue(0);
			terminaPrograma(rifeETDFL);
		}
	}else {
		if (estagio>quantosEstagios) {
			rifeETDFL_barra.setValue(0);
			terminaPrograma(rifeETDFL);
		}else {
			proximoRifeETDFL();
		}
	}
}
//**********************************************************
//					RIFE MANUAL
//**********************************************************
void comecaRifeManual() {
	avisaXseg(50, 5);
	emPrograma = true;
	pg0emPrograma.setValue(1);
	qualPrograma = rifeManual;	//seta que é o programa RIFE que começa
	estagio = 1;		//seta que é 1 estagio que começa
	quantosEstagios = 0;
	
	//vai ler frequencias e tempos;
	//numero lido - getvalue - usa little endian order
	//0x01,0x02,0x03,0x04 (4 bytes na msg de retorno)
	//0x01+(0x02*256)+(0x03*65536)+(0x04*16777216)

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
	
	if (frequenciaRasc[1]>0 && duracaoRasc[1]>0) {//se não houver freq1 ou dura1 cai fora
		quantosEstagios++;
		rifeManual_dura01.Set_background_color_bco(nxtYELLOW);
		if (frequenciaRasc[2]>0 && duracaoRasc[2]>0) {
			quantosEstagios++;
			rifeManual_dura02.Set_background_color_bco(nxtYELLOW);
			if (frequenciaRasc[3]>0 && duracaoRasc[3]>0) {
				quantosEstagios++;
				rifeManual_dura03.Set_background_color_bco(nxtYELLOW);
				if (frequenciaRasc[4]>0 && duracaoRasc[4]>0) {
					quantosEstagios++;
					rifeManual_dura04.Set_background_color_bco(nxtYELLOW);
					if (frequenciaRasc[5]>0 && duracaoRasc[5]>0) {
						quantosEstagios++;
						rifeManual_dura05.Set_background_color_bco(nxtYELLOW);
						if (frequenciaRasc[6]>0 && duracaoRasc[6]>0) {
							quantosEstagios++;
							rifeManual_dura06.Set_background_color_bco(nxtYELLOW);
						}
					}
				}
			}
		}
		pwmDuracao = duracaoRasc[1];
		pwmFrequencia = frequenciaRasc[1];

		rifeManual_barra.setValue(0);			//inicia display do nextion
		tempoMarcaInicio();
		
		if (pwmFrequencia>0) {
			ligaPWM(pwmFrequencia);
		}else {
			desligaPWM();
		}
	}else {
		terminaPrograma(rifeManual);
	}
}
//*******************************************************************
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
	}else {
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
//******************************************************
void avancaBarraRifeManual(float tempoDivisor) {
	uint32_t rascBarra;
	rascBarra = int(tempoDivisor);
	if (rascBarra < 100 && rascBarra>0) {
		rifeManual_barra.setValue(rascBarra);
	}
}
//******************************************************
void controleRifeManual() {
	unsigned long rasc;
	unsigned long diferenca;
	float tempoDivisor = 1;//controla (escala) avanço de barras
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
			if (protocoloSegundos == 1 && protocoloSegundosTotal<pwmDuracao) {
				protocoloSegundos--;
				protocoloSegundosTotal++;
				tempoDivisor = (float(protocoloSegundosTotal) / pwmDuracao) * 100;
				avancaBarraRifeManual(tempoDivisor);
			}
		}
		if (estagio<quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			proximoRifeManual();
		}else if (estagio == quantosEstagios && protocoloSegundosTotal == pwmDuracao) {
			rifeManual_barra.setValue(0);
			terminaPrograma(rifeManual);
		}
	}else {
		if (estagio>quantosEstagios) {
			rifeManual_barra.setValue(0);
			terminaPrograma(rifeManual);
		}else {
			proximoRifeManual();
		}
	}
}