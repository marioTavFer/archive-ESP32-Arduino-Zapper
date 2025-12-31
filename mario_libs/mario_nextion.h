//*****************************************************
//DEFINIÇÕES DO NEXTION
//*****************************************************

//***********PAGINAS****************
NexPage page0    = NexPage(0, 0, "page0");//menuPrinc-Menu Principal
NexPage page1    = NexPage(1, 0, "page1");//clarkProt-Protocolo CLARK
NexPage page2    = NexPage(2, 0, "page2");//teste01-pag de teste geral
NexPage page3    = NexPage(3, 0, "page3");//rifePre-Protocolo RIFE pré-definido
NexPage page4    = NexPage(4, 0, "page4");//zappicProt-Protocolo Zappicator
NexPage page5    = NexPage(5, 0, "page5");//rifeETDFL-Protocolo Rife c/ETDFL 1017-2018
NexPage page6    = NexPage(6, 0, "page6");//pagFig-pag com figura grande
NexPage page7    = NexPage(7, 0, "page7");//rifeManual- pag RIFE Manual 6 seq de freq+tempo

//****** Objetos PAGINA 0 **********
NexVariable pg0emPrograma = NexVariable(0, 9, "page0.emPrograma");//variavel global do nextion
										  
//****** Objetos PAGINA 1 **********
//NexButton clarkProt_b3 = NexButton(1, 6, "page1.b3");// HOME
NexButton clarkProt_iniciar = NexButton(1, 3, "page1.b2");
NexButton clarkProt_cancelar = NexButton(1, 5, "page1.b4");
NexProgressBar clarkProt_barra  = NexProgressBar(1, 2, "page1.j0");

//****** Objetos PAGINA 2 **********
//pagina teste
//

//****** Objetos PAGINA 3 **********
//NexButton rifePre_b0 = NexButton(0, 2, "page3.b0");//HOME
NexButton rifePre_bicho1 = NexButton(3, 5, "page3.b1");
NexButton rifePre_bicho2 = NexButton(3, 6, "page3.b2");
NexButton rifePre_bicho3 = NexButton(3, 7, "page3.b3");
NexButton rifePre_iniciar = NexButton(3, 8, "page3.b6");
NexButton rifePre_cancelar = NexButton(3, 9, "page3.b7");
NexProgressBar rifePre_barra  = NexProgressBar(3, 3, "page3.j0");
NexText rifePre_frequencia = NexText(3, 12, "page3.t4");
NexText rifePre_tempo = NexText(3, 13, "page3.t5");
NexText rifePre_etapa = NexText(3, 14, "page3.t6");
NexText rifePre_nomeProtocolo = NexText(3, 11, "page3.t3");

//****** Objetos PAGINA 4 **********
//NexButton zappicProt_b0 = NexButton(0, 2, "page4.b0");//HOME
NexButton zappicProt_iniciar = NexButton(4, 4, "page4.b1");
NexButton zappicProt_cancelar = NexButton(4, 5, "page4.b2");
NexProgressBar zappicProt_barra  = NexProgressBar(4, 3, "page4.j0");
NexNumber zappicProt_duracao = NexNumber(4, 16, "page4.n0");
NexRadio zappicProt_1K = NexRadio(4, 6, "page4.r0");
NexRadio zappicProt_2K5 = NexRadio(4, 7, "page4.r1");
NexRadio zappicProt_428 = NexRadio(4, 8, "page4.r2");
NexRadio zappicProt_529 = NexRadio(4, 9, "page4.r3");
NexSlider zappicProt_slicer = NexSlider(4,15,"page4.h0");

//****** Objetos PAGINA 5 **********
//NexButton rifeETDFL_b11 = NexButton(5, 17, "page5.b11");//HOME
NexButton rifeETDFL_ok = NexButton(5, 4, "page5.b0");
NexButton rifeETDFL_iniciar = NexButton(5, 18, "page5.b12");
NexButton rifeETDFL_cancelar = NexButton(5, 19, "page5.b13");
//teclado
NexButton rifeETDFL_n1 = NexButton(5, 6, "page5.b1");
NexButton rifeETDFL_n2 = NexButton(5, 8, "page5.b2");
NexButton rifeETDFL_n3 = NexButton(5, 9, "page5.b3");
NexButton rifeETDFL_n4 = NexButton(5, 10, "page5.b4");
NexButton rifeETDFL_n5 = NexButton(5, 11, "page5.b5");
NexButton rifeETDFL_n6 = NexButton(5, 12, "page5.b6");
NexButton rifeETDFL_n7 = NexButton(5, 13, "page5.b7");
NexButton rifeETDFL_n8 = NexButton(5, 14, "page5.b8");
NexButton rifeETDFL_n9 = NexButton(5, 15, "page5.b9");
NexButton rifeETDFL_n0 = NexButton(5, 16, "page5.b10");
NexButton rifeETDFL_nC = NexButton(5, 21, "page5.b14");
//
NexProgressBar rifeETDFL_barra  = NexProgressBar(5, 5, "page5.j0");
NexNumber rifeETDFL_numeroProtocolo = NexNumber(5, 2, "page5.n1");
NexNumber rifeETDFL_etapa = NexNumber(5, 22, "page5.n0");
NexText rifeETDFL_nomeProtocolo = NexText(5, 20, "page5.t3");
NexText rifeETDFL_frequencia = NexText(5, 24, "page5.t5");


//****** Objetos PAGINA 7 **********

NexButton rifeManual_iniciar = NexButton(7,29, "page7.b0");
NexButton rifeManual_cancelar = NexButton(7,30, "page7.b11");
//teclado
NexText rifeManual_numero = NexText(7,18, "page7.t3");
NexButton rifeManual_n1 = NexButton(7,17, "page7.b1");
NexButton rifeManual_n2 = NexButton(7,19, "page7.b2");
NexButton rifeManual_n3 = NexButton(7,20, "page7.b3");
NexButton rifeManual_n4 = NexButton(7,21, "page7.b4");
NexButton rifeManual_n5 = NexButton(7,22, "page7.b5");
NexButton rifeManual_n6 = NexButton(7,23, "page7.b6");
NexButton rifeManual_n7 = NexButton(7,24, "page7.b7");
NexButton rifeManual_n8 = NexButton(7,25, "page7.b8");
NexButton rifeManual_n9 = NexButton(7,26, "page7.b9");
NexButton rifeManual_n0 = NexButton(7,27, "page7.b10");
NexButton rifeManual_nC = NexButton(7, 28, "page7.b14");
//
NexProgressBar rifeManual_barra  = NexProgressBar(7,31, "page7.j0");
// frequencias e duracao
NexNumber rifeManual_freq01 = NexNumber(7,5, "page7.n0");
NexNumber rifeManual_freq02 = NexNumber(7,7, "page7.n2");
NexNumber rifeManual_freq03 = NexNumber(7,9, "page7.n4");
NexNumber rifeManual_freq04 = NexNumber(7,11, "page7.n6");
NexNumber rifeManual_freq05 = NexNumber(7,13, "page7.n8");
NexNumber rifeManual_freq06 = NexNumber(7,15, "page7.n10");
//
NexNumber rifeManual_dura01 = NexNumber(7,6, "page7.n1");
NexNumber rifeManual_dura02 = NexNumber(7,8, "page7.n3");
NexNumber rifeManual_dura03 = NexNumber(7,10, "page7.n5");
NexNumber rifeManual_dura04 = NexNumber(7,12, "page7.n7");
NexNumber rifeManual_dura05 = NexNumber(7,14, "page7.n9");
NexNumber rifeManual_dura06 = NexNumber(7,16, "page7.n11");


//******definição da lista de Objetos *** monitoramento/Registro******

NexTouch *nex_listen_list[] =
{
	&page0,	&page1,	&page2,	&page3,	&page4,	&page5, &page6, &page7,
	//pagina01
	&clarkProt_iniciar,&clarkProt_cancelar,//&clarkProt_barra,
	//pagina03
	&rifePre_bicho1,	&rifePre_bicho2,	&rifePre_bicho3,	&rifePre_iniciar,&rifePre_cancelar,
	//&rifePre_barra,//&rifePre_frequencia,//&rifePre_tempo,//&rifePre_nomeProtocolo,
	//pagina04
	&zappicProt_iniciar,&zappicProt_cancelar,//&zappicProt_barra,//&zappicProt_duracao,
	&zappicProt_1K,&zappicProt_2K5,&zappicProt_428,&zappicProt_529,
	//pagina05
	&rifeETDFL_iniciar,&rifeETDFL_cancelar,&rifeETDFL_ok,
	//pagina05-teclado
	&rifeETDFL_n1,&rifeETDFL_n2,&rifeETDFL_n3,&rifeETDFL_n4,&rifeETDFL_n5,
	&rifeETDFL_n6,&rifeETDFL_n7,&rifeETDFL_n8,&rifeETDFL_n9,&rifeETDFL_n0,&rifeETDFL_nC,
	//&rifeETDFL_barra,//&rifeETDFL_nomeProtocolo,//
	&rifeETDFL_numeroProtocolo,
	//pagina07
	&rifeManual_iniciar,&rifeManual_cancelar,
	//pagina07-teclado
	&rifeManual_n1,&rifeManual_n2,&rifeManual_n3,&rifeManual_n4,&rifeManual_n5,
	&rifeManual_n6,&rifeManual_n7,&rifeManual_n8,&rifeManual_n9,&rifeManual_n0,&rifeManual_nC,
	//pagina07-campos numéricos
	&rifeManual_freq01, &rifeManual_freq02, &rifeManual_freq03,
	&rifeManual_freq04, &rifeManual_freq05, &rifeManual_freq06,
	&rifeManual_dura01,&rifeManual_dura02, &rifeManual_dura03,
	&rifeManual_dura04, &rifeManual_dura05,&rifeManual_dura06,
	NULL
};

//***************definição cores básicas do nextion
//565 color value

const unsigned long nxtWHITE=65535;
const unsigned long nxtBLACK=65535;
const unsigned long nxtBLUE=31;
const unsigned long nxtBROWN=48192;
const unsigned long nxtGREEN=2016;
const unsigned long nxtYELLOW=65504;
const unsigned long nxtRED=63488;
const unsigned long nxtGRAY=33840;


void initObjNextion();

//DEFINIDO NAS BIBLIOTECAS DO NEXTION
void clarkProt_iniciarPopCallback(void *ptr);//Clark
void rifePre_iniciarPopCallback(void *ptr);//RIFE(1)
void zappicProt_iniciarPopCallback(void *ptr);//Zappicator
void rifeETDFL_iniciarPopCallback(void *ptr);//RIFE(2)
void rifeManual_iniciarPopCallback(void *ptr);//TECLADO OP.Manual
//
void clarkProt_cancelarPopCallback(void *ptr);
void rifePre_cancelarPopCallback(void *ptr);
void zappicProt_cancelarPopCallback(void *ptr);
void rifeETDFL_cancelarPopCallback(void *ptr);
void rifeManual_cancelarPopCallback(void *ptr);
//
void rifePre_bicho1PopCallback(void *ptr);//RIFE(1)
void rifePre_bicho2PopCallback(void *ptr);
void rifePre_bicho3PopCallback(void *ptr);
//
void zappicProt_1KPopCallback(void *ptr);//Zappicator
void zappicProt_2K5PopCallback(void *ptr);
void zappicProt_428PopCallback(void *ptr);
void zappicProt_529PopCallback(void *ptr);
//
void rifeETDFL_n0PopCallback(void *ptr);//teclado rife2
void rifeETDFL_n1PopCallback(void *ptr);
void rifeETDFL_n2PopCallback(void *ptr);
void rifeETDFL_n3PopCallback(void *ptr);
void rifeETDFL_n4PopCallback(void *ptr);
void rifeETDFL_n5PopCallback(void *ptr);
void rifeETDFL_n6PopCallback(void *ptr);
void rifeETDFL_n7PopCallback(void *ptr);
void rifeETDFL_n8PopCallback(void *ptr);
void rifeETDFL_n9PopCallback(void *ptr);
void rifeETDFL_nCPopCallback(void *ptr);
void rifeETDFL_okPopCallback(void *ptr);
void rifeETDFL_numeroProtocoloPopCallback(void *ptr);
//
void rifeManual_n0PopCallback(void *ptr);//teclado op.manual
void rifeManual_n1PopCallback(void *ptr);
void rifeManual_n2PopCallback(void *ptr);
void rifeManual_n3PopCallback(void *ptr);
void rifeManual_n4PopCallback(void *ptr);
void rifeManual_n5PopCallback(void *ptr);
void rifeManual_n6PopCallback(void *ptr);
void rifeManual_n7PopCallback(void *ptr);
void rifeManual_n8PopCallback(void *ptr);
void rifeManual_n9PopCallback(void *ptr);
void rifeManual_nCPopCallback(void *ptr);
//
void rifeManual_dura01PopCallback(void *ptr);//teclado op.manual
void rifeManual_dura02PopCallback(void *ptr);
void rifeManual_dura03PopCallback(void *ptr);
void rifeManual_dura04PopCallback(void *ptr);
void rifeManual_dura05PopCallback(void *ptr);
void rifeManual_dura06PopCallback(void *ptr);
void rifeManual_freq01PopCallback(void *ptr);
void rifeManual_freq02PopCallback(void *ptr);
void rifeManual_freq03PopCallback(void *ptr);
void rifeManual_freq04PopCallback(void *ptr);
void rifeManual_freq05PopCallback(void *ptr);
void rifeManual_freq06PopCallback(void *ptr);

void initObjNextion(){
	nexInit();
	clarkProt_iniciar.attachPop(clarkProt_iniciarPopCallback);
	rifePre_iniciar.attachPop(rifePre_iniciarPopCallback);
	zappicProt_iniciar.attachPop(zappicProt_iniciarPopCallback);
	rifeETDFL_iniciar.attachPop(rifeETDFL_iniciarPopCallback);
	//
	clarkProt_cancelar.attachPop(clarkProt_cancelarPopCallback);
	rifePre_cancelar.attachPop(rifePre_cancelarPopCallback);
	zappicProt_cancelar.attachPop(zappicProt_cancelarPopCallback);
	rifeETDFL_cancelar.attachPop(rifeETDFL_cancelarPopCallback);
	//
	rifePre_bicho1.attachPop(rifePre_bicho1PopCallback);
	rifePre_bicho2.attachPop(rifePre_bicho2PopCallback);
	rifePre_bicho3.attachPop(rifePre_bicho3PopCallback);
	//
	zappicProt_1K.attachPop(zappicProt_1KPopCallback);
	zappicProt_2K5.attachPop(zappicProt_2K5PopCallback);
	zappicProt_428.attachPop(zappicProt_428PopCallback);
	zappicProt_529.attachPop(zappicProt_529PopCallback);
	//teclado da P5
	rifeETDFL_ok.attachPop(rifeETDFL_okPopCallback);
	rifeETDFL_n1.attachPop(rifeETDFL_n1PopCallback);
	rifeETDFL_n2.attachPop(rifeETDFL_n2PopCallback);
	rifeETDFL_n3.attachPop(rifeETDFL_n3PopCallback);
	rifeETDFL_n4.attachPop(rifeETDFL_n4PopCallback);
	rifeETDFL_n5.attachPop(rifeETDFL_n5PopCallback);
	rifeETDFL_n6.attachPop(rifeETDFL_n6PopCallback);
	rifeETDFL_n7.attachPop(rifeETDFL_n7PopCallback);
	rifeETDFL_n8.attachPop(rifeETDFL_n8PopCallback);
	rifeETDFL_n9.attachPop(rifeETDFL_n9PopCallback);
	rifeETDFL_n0.attachPop(rifeETDFL_n0PopCallback);
	rifeETDFL_nC.attachPop(rifeETDFL_nCPopCallback);
	//P7 - Operacao Manual
	rifeManual_iniciar.attachPop(rifeManual_iniciarPopCallback);
	rifeManual_cancelar.attachPop(rifeManual_cancelarPopCallback);
	//teclado da P7
	rifeManual_n1.attachPop(rifeManual_n1PopCallback);
	rifeManual_n2.attachPop(rifeManual_n2PopCallback);
	rifeManual_n3.attachPop(rifeManual_n3PopCallback);
	rifeManual_n4.attachPop(rifeManual_n4PopCallback);
	rifeManual_n5.attachPop(rifeManual_n5PopCallback);
	rifeManual_n6.attachPop(rifeManual_n6PopCallback);
	rifeManual_n7.attachPop(rifeManual_n7PopCallback);
	rifeManual_n8.attachPop(rifeManual_n8PopCallback);
	rifeManual_n9.attachPop(rifeManual_n9PopCallback);
	rifeManual_n0.attachPop(rifeManual_n0PopCallback);
	rifeManual_nC.attachPop(rifeManual_nCPopCallback);
	//
	rifeManual_dura01.attachPop(rifeManual_dura01PopCallback);
	rifeManual_dura02.attachPop(rifeManual_dura02PopCallback);
	rifeManual_dura03.attachPop(rifeManual_dura03PopCallback);
	rifeManual_dura04.attachPop(rifeManual_dura04PopCallback);
	rifeManual_dura05.attachPop(rifeManual_dura05PopCallback);
	rifeManual_dura06.attachPop(rifeManual_dura06PopCallback);
	rifeManual_freq01.attachPop(rifeManual_freq01PopCallback);
	rifeManual_freq02.attachPop(rifeManual_freq02PopCallback);
	rifeManual_freq03.attachPop(rifeManual_freq03PopCallback);
	rifeManual_freq04.attachPop(rifeManual_freq04PopCallback);
	rifeManual_freq05.attachPop(rifeManual_freq05PopCallback);
	rifeManual_freq06.attachPop(rifeManual_freq06PopCallback);
}