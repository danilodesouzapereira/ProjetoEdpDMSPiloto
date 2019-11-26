/***
 * Classe para opera��es com os objetos de redes do Sinap
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TFuncoesDeRede.h"
#include "FuncoesFL.h"
#include "..\TCluster.h"
#include "..\AlgoFasorial\TAlgoFasorial.h"
#include <System.StrUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Bloco.h>
#include <PlataformaSinap\DLL_Inc\CalcIndCont.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Ordena.h>
#include <PlataformaSinap\DLL_Inc\Rede.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Arranjo\VTArranjo.h>
#include <PlataformaSinap\Fontes\Arranjo\VTArranjos.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Bloco\VTElo.h>
#include <PlataformaSinap\Fontes\CalcIndCont\VTBuscaProt.h>
#include <PlataformaSinap\Fontes\CalcIndCont\VTBlocoRad.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTEqbar.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTSuprimento.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
__fastcall TFuncoesDeRede::TFuncoesDeRede(VTApl* apl)
{
	// Par�metros b�sicos
	this->apl = apl;
	this->path = (VTPath*)apl->GetObject(__classid(VTPath));
	this->redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
   this->ordena = NULL;
   this->redeRef = NULL;
   this->blocos = NULL;
	listaBlocosRede = NULL;

   listaBlocosRede_RompCabo = new TList();
	rede_RompCabo = NULL;
}
//---------------------------------------------------------------------------
__fastcall TFuncoesDeRede::~TFuncoesDeRede(void)
{
	// Destroi objetos
   if(listaBlocosRede) {delete listaBlocosRede; listaBlocosRede = NULL;}
   if(listaBlocosRede_RompCabo) {delete listaBlocosRede_RompCabo; listaBlocosRede_RompCabo = NULL;}
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::AreaTemBarra(TList* lisBlocos, VTBarra* barra)
{
   VTBloco* bloco;

	if(lisBlocos == NULL || barra == NULL)
   	return false;

   for(int i=0; i<lisBlocos->Count; i++)
   {
		bloco = (VTBloco*) lisBlocos->Items[i];

      if(bloco->ExisteBarra(barra))
      	return true;
   }

   return false;
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::AreaTemLigacao(TList* lisBlocos, VTLigacao* ligacao)
{
	TList* lisAux;
	TList* lisLiga;
	TList* listaTodasLigacoes;
	VTBloco* bloco;
   VTElo* elo;
   VTLigacao* liga;
   bool resp;

	if(lisBlocos == NULL || ligacao == NULL)
   	return false;

	listaTodasLigacoes = new TList();
   lisAux = new TList();
   for(int i=0; i<lisBlocos->Count; i++)
   {
   	// Salva as liga��es do bloco
		bloco = (VTBloco*) lisBlocos->Items[i];
      lisLiga = bloco->LisLigacao();
      listaTodasLigacoes->Assign(lisLiga, laOr);

      lisAux->Clear();
      blocos->LisElo(bloco, lisAux);

      // Salva tamb�m as chaves em torno do bloco
      for(int j=0; j<lisAux->Count; j++)
      {
       	elo = (VTElo*) lisAux->Items[j];
         liga = (VTLigacao*) elo->Chave;
         if(listaTodasLigacoes->IndexOf(liga) < 0)
         	listaTodasLigacoes->Add(liga);
      }
   }

	if(listaTodasLigacoes->IndexOf(ligacao) >= 0)
   	resp = true;
   else
      resp = false;

   delete listaTodasLigacoes; listaTodasLigacoes = NULL;
   return(resp);
}
//---------------------------------------------------------------------------
double __fastcall TFuncoesDeRede::CalcImpedanciaTrechos(VTBarra* barra1, VTBarra* barra2, double &r0Total, double &r1Total, double &x0Total, double &x1Total)
{
	double r0, x0, c0;
	double r1, x1, c1;
   TList* lisLigacoes;
   VTRede* rede;

	if(!barra1 || !barra2) return 0.;

   // Procura a rede em quest�o
   rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];

      if(rede->TipoRede->Segmento != redePRI)
      	continue;

      if(rede->ExisteBarra(barra1) && rede->ExisteBarra(barra2))
			break;
      else
      	rede = NULL;
   }
   if(rede == NULL)
   	return 0.;

	// Pega as liga��es entre as barras 1 e 2
	lisLigacoes = new TList();
   GetCaminhoLigacoes_BarraRef_Barra(rede, barra1, barra2, lisLigacoes);

	r0Total = 0.;
   x0Total = 0.;
   r1Total = 0.;
   x1Total = 0.;
	for(int i=0; i<lisLigacoes->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) lisLigacoes->Items[i];
      if(liga->Tipo() != eqptoTRECHO)
      	continue;

      VTTrecho* trecho = (VTTrecho*) liga;

      trecho->Z0_ohm(r0, x0, c0);
      trecho->Z1_ohm(r1, x1, c1);

		r0Total += r0;
      x0Total += x0;
      r1Total += r1;
      x1Total += x1;
   }

   delete lisLigacoes; lisLigacoes = NULL;
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde uma barra de refer�ncia at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatancia2FT(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;

	if(!lisEXT || !barraRef || !redeMT) return;

	lisEXT->Clear();
	for(int i=0; i<redeMT->LisBarra()->Count; i++)
	{
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
		CalcImpedanciaTrechos(barraRef, barra, r0Total, r1Total, x0Total, x1Total);

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(x1Total - Xtotal_medido_seq1) / Xtotal_medido_seq1;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
         barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Rtotal_0 = r0Total;
         barraAlgoFasorial->Xtotal_0 = x0Total;
         barraAlgoFasorial->Rtotal_1 = r1Total;
         barraAlgoFasorial->Xtotal_1 = x1Total;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde o in�cio do alimentador at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatancia2FT(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;
	VTBarra* barraInicial;

	if(!lisEXT || !redeMT) return;

   lisEXT->Clear();

   // Tens�o de base (n�vel MT)
	barraInicial = redeMT->BarraInicial();
	vbase = barraInicial->vnom;

   for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
      CalcImpedanciaTrechos(barraInicial, barra, r0Total, r1Total, x0Total, x1Total);

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(x1Total - Xtotal_medido_seq1) / Xtotal_medido_seq1;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
         barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Rtotal_0 = r0Total;
         barraAlgoFasorial->Xtotal_0 = x0Total;
         barraAlgoFasorial->Rtotal_1 = r1Total;
         barraAlgoFasorial->Xtotal_1 = x1Total;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde uma barra de refer�ncia at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatancia2F(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;

	if(!lisEXT || !barraRef || !redeMT) return;

	lisEXT->Clear();
	for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
		CalcImpedanciaTrechos(barraRef, barra, r0Total, r1Total, x0Total, x1Total);

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(x1Total - Xtotal_medido_seq1) / Xtotal_medido_seq1;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
         barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Rtotal_0 = r0Total;
         barraAlgoFasorial->Xtotal_0 = x0Total;
         barraAlgoFasorial->Rtotal_1 = r1Total;
         barraAlgoFasorial->Xtotal_1 = x1Total;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde o in�cio do alimentador at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatancia2F(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;
	VTBarra* barraInicial;

	if(!lisEXT || !redeMT) return;

   lisEXT->Clear();

   // Tens�o de base (n�vel MT)
	barraInicial = redeMT->BarraInicial();
	vbase = barraInicial->vnom;

   for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
      CalcImpedanciaTrechos(barraInicial, barra, r0Total, r1Total, x0Total, x1Total);

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(x1Total - Xtotal_medido_seq1) / Xtotal_medido_seq1;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
         barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Rtotal_0 = r0Total;
         barraAlgoFasorial->Xtotal_0 = x0Total;
         barraAlgoFasorial->Rtotal_1 = r1Total;
         barraAlgoFasorial->Xtotal_1 = x1Total;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde uma barra de refer�ncia at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X0 + 2X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatanciaFT(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
	double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;

	if(!barraRef || !redeMT || !lisEXT) return;

	lisEXT->Clear();
	for(int i=0; i<redeMT->LisBarra()->Count; i++)
	{
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
		CalcImpedanciaTrechos(barraRef, barra, r0Total, r1Total, x0Total, x1Total);

		// Comp�e resist�ncia e reat�ncia totais
		Xtotal_calculado = x0Total + 2. * x1Total;
		Rtotal_calculado = r0Total + 2. * r1Total;

		// Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
		erroX = 100. * fabs(Xtotal_calculado - Xtotal_medido) / Xtotal_medido;

		// Adiciona a barra se o desvio estiver dentro da toler�ncia
		if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
			barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Xtotal = Xtotal_calculado;
         barraAlgoFasorial->Rtotal = Rtotal_calculado;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde o in�cio do alimentador at�
// um ponto com reat�ncia total = Xtotal.   Obs: Xtotal = X0 + 2X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatanciaFT(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;
	VTBarra* barraInicial;

	if(!lisEXT || !redeMT) return;

   lisEXT->Clear();

   // Tens�o de base (n�vel MT)
	barraInicial = redeMT->BarraInicial();
	vbase = barraInicial->vnom;

//   VTTrafo* trafoSE = NULL;
//   for(int i=0; i<redeSE->LisLigacao()->Count; i++)
//   {
//   	VTLigacao* liga = (VTLigacao*) redeSE->LisLigacao()->Items[i];
//      if(liga->Barra(0) == barraInicial || liga->Barra(1) == barraInicial)
//      {
//         if(liga->Tipo() == eqptoTRAFO)
//         {
//            trafoSE = (VTTrafo*) liga;
//            break;
//         }
//      }
//   }
//   if(!trafoSE) return;
//
//   // Reat�ncia seq. 1 do trafo SE
//	r1Trafo = trafoSE->z1.r * (vbase*vbase/trafoSE->snom);
//	x1Trafo = trafoSE->z1.x * (vbase*vbase / trafoSE->snom);
//
//   // Reat�ncia seq. 1 do suprimento
//   TList* lisSup = new TList();
//   redeSE->LisEqbar(lisSup, eqptoSUPRIMENTO);
//   VTSuprimento* sup = (VTSuprimento*) lisSup->Items[0];
//   r1Sup = sup->zeq1.r * (vbase*vbase/100.);
//   x1Sup = sup->zeq1.x * (vbase*vbase/100.);
//   delete lisSup; lisSup = NULL;

   for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
      CalcImpedanciaTrechos(barraInicial, barra, r0Total, r1Total, x0Total, x1Total);

      // Comp�e a reat�ncia total
		Xtotal_calculado = x0Total + 2. * x1Total;
//      Xtotal_calculado += 2. * x1Sup;
//      Xtotal_calculado += 2. * x1Trafo;

      // Comp�e a resist�ncia total
      Rtotal_calculado = r0Total + 2. * r1Total;
//      Rtotal_calculado += 2. * r1Sup;
//      Rtotal_calculado += 2. * r1Trafo;

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(Xtotal_calculado - Xtotal_medido) / Xtotal_medido;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
			barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Xtotal = Xtotal_calculado;
         barraAlgoFasorial->Rtotal = Rtotal_calculado;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
// Gera lista com as barras que geram caminho desde uma barra de refer�ncia at�
// um ponto com reat�ncia total = Xtotal.
// Obs: Xtotal = X1 (ohms)
void __fastcall TFuncoesDeRede::GetBarras_CaminhoReatancia3F(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido, double maxErroX, TList* lisEXT)
{
	double r0Total, r1Total, x0Total, x1Total;
   double vbase, Rtotal_calculado, Xtotal_calculado, erroX;
	double r1Trafo, x1Trafo, r1Sup, x1Sup;
	VTBarra* barraInicial;

	if(!lisEXT || !redeMT) return;

   lisEXT->Clear();

   // Tens�o de base (n�vel MT)
	barraInicial = redeMT->BarraInicial();
	vbase = barraInicial->vnom;

   VTTrafo* trafoSE = NULL;
   for(int i=0; i<redeSE->LisLigacao()->Count; i++)
   {
   	VTLigacao* liga = (VTLigacao*) redeSE->LisLigacao()->Items[i];
      if(liga->Barra(0) == barraInicial || liga->Barra(1) == barraInicial)
      {
         if(liga->Tipo() == eqptoTRAFO)
         {
            trafoSE = (VTTrafo*) liga;
            break;
         }
      }
   }
   if(!trafoSE) return;

   // Reat�ncia seq. 1 do trafo SE
	r1Trafo = trafoSE->z1.r * (vbase*vbase/trafoSE->snom);
	x1Trafo = trafoSE->z1.x * (vbase*vbase / trafoSE->snom);

   // Reat�ncia seq. 1 do suprimento
   TList* lisSup = new TList();
   redeSE->LisEqbar(lisSup, eqptoSUPRIMENTO);
   VTSuprimento* sup = (VTSuprimento*) lisSup->Items[0];
   r1Sup = sup->zeq1.r * (vbase*vbase/100.);
   x1Sup = sup->zeq1.x * (vbase*vbase/100.);
   delete lisSup; lisSup = NULL;

   for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
		VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];

		// Imped�ncia seq 0 e seq 1 dos trechos desde a SE at� a barra
      CalcImpedanciaTrechos(barraInicial, barra, r0Total, r1Total, x0Total, x1Total);

      // Comp�e a reat�ncia total
      Xtotal_calculado = x0Total + 2. * x1Total;
      Xtotal_calculado += 2. * x1Sup;
      Xtotal_calculado += 2. * x1Trafo;

      // Comp�e a resist�ncia total
      Rtotal_calculado = r0Total + 2. * r1Total;
      Rtotal_calculado += 2. * r1Sup;
      Rtotal_calculado += 2. * r1Trafo;

      // Calcula o erro/desvio entre a reat. total calculada e a reat. total medida
      erroX = 100. * fabs(Xtotal_calculado - Xtotal_medido) / Xtotal_medido;

      // Adiciona a barra se o desvio estiver dentro da toler�ncia
      if(erroX < maxErroX)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = new strBarraAlgoFasorial();
         barraAlgoFasorial->barra = barra;
         barraAlgoFasorial->Xtotal = Xtotal_calculado;
         barraAlgoFasorial->Rtotal = Rtotal_calculado;
      	lisEXT->Add(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::ExisteBifurcacao(TList* lisFilhas, double PorcMinLigacoes, TList* lisCaminhos)
{
	int NumTotLigacoes = 0;
   TList* ligaJus;
	VTLigacao* liga;

	if(lisFilhas == NULL || lisCaminhos == NULL)
   	return false;

   // N�mero total de liga��es na rede de refer�ncia
  	NumTotLigacoes = redeRef->LisLigacao()->Count;

   // Se pelo menos um caminho tiver n�m. de lig. menor que a porcentagem m�nima,
   // n�o � bifurca��o
   for(int i=0; i<lisFilhas->Count; i++)
   {
    	liga = (VTLigacao*) lisFilhas->Items[i];

	   ligaJus = new TList();
      GetLigacoesJusanteLigacao(liga, ligaJus);

      if(100. * ligaJus->Count / NumTotLigacoes < PorcMinLigacoes)
      	return false;

   	lisCaminhos->Add(ligaJus);
   }

   return true;
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::ExisteCaminhoLista(TList* listaExt, TList* caminho)
{
	VTLigacao* ultimaLiga = (VTLigacao*) caminho->Items[caminho->Count-1];

   for(int i=0; i<listaExt->Count; i++)
	{
    	TList* caminhoAux = (TList*) listaExt->Items[i];
      VTLigacao* ligaAux = (VTLigacao*) caminhoAux->Items[caminhoAux->Count-1];
      if(ligaAux == ultimaLiga) return true;
   }

   return false;
}
//---------------------------------------------------------------------------
//lisClusteresOrdenados: lista ordenada de clusteres (do maior para o menor)
void __fastcall TFuncoesDeRede::FiltraClusteres(TList* lisBlocosAreaBusca, TList* lisClusteresOrdenados)
{
	TCluster *cluster, *clusterAux;
   TList *listaMaiorLigacoes, *listaMenorLigacoes;
   TList *listaMaior_Blocos, *listaMenor_Blocos;
   VTBloco* bloco;

	if(!lisBlocosAreaBusca || !lisClusteresOrdenados) return;

   // Remove da lista de blocos maior (do cluster maior) os blocos que j�
   // est�o em um cluster menor
   for(int i=0; i<lisClusteresOrdenados->Count-1; i++)
   {
   	cluster = (TCluster*) lisClusteresOrdenados->Items[i];
      listaMaior_Blocos = cluster->GetBlocos();

      for(int j=i+1; j<lisClusteresOrdenados->Count; j++)
      {
         clusterAux = (TCluster*) lisClusteresOrdenados->Items[j];
         listaMenor_Blocos = clusterAux->GetBlocos();

         for(int k=0; k<listaMenor_Blocos->Count; k++)
         {
         	bloco = (VTBloco*) listaMenor_Blocos->Items[k];
            if(listaMaior_Blocos->IndexOf(bloco) >= 0)
               listaMaior_Blocos->Remove(bloco);
         }
      }
   }

   // Remove blocos fora da �rea de busca
   for(int i=lisClusteresOrdenados->Count-1; i>=0; i--)
   {
   	cluster = (TCluster*) lisClusteresOrdenados->Items[i];
      TList* lisBlocosCluster = cluster->GetBlocos();
      for(int i=lisBlocosCluster->Count-1; i>=0; i--)
      {
			VTBloco* bloco = (VTBloco*) lisBlocosCluster->Items[i];
         if(lisBlocosAreaBusca->IndexOf(bloco) < 0)
         	lisBlocosCluster->Remove(bloco);
      }

      // Se o cluster ficou sem blocos, remove-o
      if(lisBlocosCluster->Count == 0)
      {
      	lisClusteresOrdenados->Remove(cluster);
         delete cluster;
      }
   }
}
//---------------------------------------------------------------------------
/**
 * Este m�todo fornece uma lista de listas de barras dos blocos.
 * Entrada: lista de blocos
 * Sa�da: lista com listas de barras
 */
void __fastcall TFuncoesDeRede::GetBarrasBlocos(TList* listaBlocos, TList* listaExt)
{
	if(listaExt == NULL) return;

	TList* listaBarras = NULL;
	//Para cada bloco, gera uma lista de barras
	for(int i=0; i<listaBlocos->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) listaBlocos->Items[i];
		TList* listaBarras = bloco->LisBarra();

		//adiciona lista de barras � lista externa
		listaExt->Add(listaBarras);
	}
}
//---------------------------------------------------------------------------
/**
 * Este m�todo fornece uma lista de barras que distam L=Dist da barra de refer�ncia.
 * Entrada: c�digo da barra de refer�ncia (codBarraRef), dist�ncia (Dist)
 * Sa�da: lista com as barras (listaExt)
 */
void __fastcall TFuncoesDeRede::GetBarrasDistancia(VTRede* redeMT, String codBarraRef, double Dist, TList* listaExt)
{
	VTLigacao* ligaTerm;
   VTBarra* barraTerm;

	// Verifica��o da lista externa
	if(listaExt == NULL) return;

   // Lista de caminhos que se iniciam em barra ref e t�m dist�ncia = Dist
   TList* listaCaminhos = new TList();

	// Executa o algoritmo
   GetCaminhosDistancia(redeMT, codBarraRef, Dist, listaCaminhos);

   // Verifica��o
   if(listaCaminhos->Count == 0)
   {
      // Destroi objetos
      for(int i=listaCaminhos->Count-1; i>=0; i--) delete(listaCaminhos->Items[i]);
      delete listaCaminhos; listaCaminhos = NULL;
      return;
   }

	// Salva as barras terminais
   for(int i=0; i<listaCaminhos->Count; i++)
   {
   	// Pega um caminho
    	TList* caminho = (TList*) listaCaminhos->Items[i];
      // Pega a barra terminal deste caminho
		ligaTerm = (VTLigacao*) caminho->Items[caminho->Count-1];
      barraTerm = ligaTerm->Barra(1);
      // Salva a barra terminal na lista externa
      listaExt->Add(barraTerm);
   }

   // Destroi objetos
   for(int i=listaCaminhos->Count-1; i>=0; i--) delete(listaCaminhos->Items[i]);
   delete listaCaminhos; listaCaminhos = NULL;
}
//---------------------------------------------------------------------------
/**
 * Este m�todo fornece uma lista de barras que distam L=Dist da barra de refer�ncia.
 * Entrada: c�digo da barra de refer�ncia (codBarraRef), dist�ncia (Dist)
 * Sa�da: lista com as barras (listaExt)
 */
void __fastcall TFuncoesDeRede::GetBarrasDistancia(String codBarraRef, double Dist, TList* listaExt)
{
	VTLigacao* ligaTerm;
   VTBarra* barraTerm;

	// Verifica��o da lista externa
	if(listaExt == NULL) return;

   // Lista de caminhos que se iniciam em barra ref e t�m dist�ncia = Dist
   TList* listaCaminhos = new TList();

	// Executa o algoritmo
   GetCaminhosDistancia(codBarraRef, Dist, listaCaminhos);
   if(listaCaminhos->Count == 0)
   {
      // Destroi objetos
      for(int i=listaCaminhos->Count-1; i>=0; i--) delete(listaCaminhos->Items[i]);
      delete listaCaminhos; listaCaminhos = NULL;
      return;
   }

	// Salva as barras terminais
   for(int i=0; i<listaCaminhos->Count; i++)
   {
   	// Pega um caminho
    	TList* caminho = (TList*) listaCaminhos->Items[i];
      // Pega a barra terminal deste caminho
		ligaTerm = (VTLigacao*) caminho->Items[caminho->Count-1];
      barraTerm = ligaTerm->Barra(1);
      // Salva a barra terminal na lista externa
      listaExt->Add(barraTerm);
   }

   // Destroi objetos
   for(int i=listaCaminhos->Count-1; i>=0; i--) delete(listaCaminhos->Items[i]);
   delete listaCaminhos; listaCaminhos = NULL;
}
//---------------------------------------------------------------------------
// lisBlocos: lista de blocos da �rea de interesse
// listaExt: lista externa que ser� preenchida com as bifurca��es
void __fastcall TFuncoesDeRede::GetBifurcacoesBlocos(TList* lisBlocos, TList* listaExt)
{
	TBifurcacao* bifurc;

   if(lisBlocos == NULL || listaExt == NULL)
   	return;

   // Lista de chaves da �rea de interesse
   TList* listaChaves = new TList();
   GetChavesBlocos(lisBlocos, listaChaves);

   // Obt�m as bifurca��e da rede
   TList* lisBifurcacoes = new TList();
   for(int i=0; i<listaChaves->Count; i++)
   {
    	VTChave* chave = (VTChave*) listaChaves->Items[i];

      // Verifica se � bifurca��o
      bifurc = Bifurcacao(((VTLigacao*)chave)->ligaPai);
      if(bifurc == NULL) continue;

      listaExt->Add(bifurc);
   }

   // Destroi listas
   delete listaChaves; listaChaves = NULL;
}
//---------------------------------------------------------------------------
/***
 * Retorna a liga��o mais � montante de um conjunto de blocos
 *
 * Obs: setar a rede de refer�ncia (redeRef)
 */
VTLigacao* __fastcall TFuncoesDeRede::GetLigacaoMontanteArea(TList* lisBlocos)
{
	TList* lisLigacoes;
	VTLigacao* liga;

   if(lisBlocos == NULL) return NULL;

   // Obt�m as liga��es que pertencem � �rea dos blocos
   lisLigacoes = new TList();
	for(int i=0; i<redeRef->LisLigacao()->Count; i++)
   {
   	liga = (VTLigacao*) redeRef->LisLigacao()->Items[i];

      if(AreaTemLigacao(lisBlocos, liga))
      {
         lisLigacoes->Add(liga);
      }
   }

	// A liga��o mais � montante n�o tem liga pai dentro da �rea
   for(int i=0; i<lisLigacoes->Count; i++)
   {
      liga = (VTLigacao*) lisLigacoes->Items[i];

      if(lisLigacoes->IndexOf(liga->ligaPai) < 0)
      	break;
      else
      	liga = NULL;
   }

   delete lisLigacoes; lisLigacoes = NULL;

	if(liga == NULL)
   	return NULL;

   return liga;
}
//---------------------------------------------------------------------------
/***
 * Retorna a liga��o mais � montante de um conjunto de blocos
 */
VTLigacao* __fastcall TFuncoesDeRede::GetLigacaoMontanteArea(VTRede* redeRef, TList* lisBlocos)
{
	TList* lisLigacoes;
	VTLigacao* liga;

   if(!redeRef || !blocos) return NULL;

   // Obt�m as liga��es que pertencem � �rea dos blocos
   lisLigacoes = new TList();
	for(int i=0; i<redeRef->LisLigacao()->Count; i++)
   {
   	liga = (VTLigacao*) redeRef->LisLigacao()->Items[i];

      if(liga->Tipo() == eqptoCHAVE)
      {
         if(((VTChave*)liga)->Aberta)
         	continue;
      }

      if(AreaTemLigacao(lisBlocos, liga))
      {
         lisLigacoes->Add(liga);
      }
   }

	// A liga��o mais � montante n�o tem liga pai dentro da �rea
   for(int i=0; i<lisLigacoes->Count; i++)
   {
      liga = (VTLigacao*) lisLigacoes->Items[i];

      if(lisLigacoes->IndexOf(liga->ligaPai) < 0)
      	break;
      else
      	liga = NULL;
   }

	if(liga == NULL)
   {
      // Destroi lista de liga��es
      delete lisLigacoes; lisLigacoes = NULL;

   	return NULL;
   }

   // Se a liga��o montante encontrada for chave, pega uma liga��o filha
   if(liga->Tipo() == eqptoCHAVE)
   {
      for(int i=0; i<lisLigacoes->Count; i++)
      {
      	VTLigacao* ligaAux = (VTLigacao*) lisLigacoes->Items[i];
         if(ligaAux->ligaPai == liga)
			{
            liga = ligaAux;
            break;
         }
      }
   }

   // Destroi lista de liga��es
   delete lisLigacoes; lisLigacoes = NULL;

   return liga;
}
//---------------------------------------------------------------------------
/***
 * Retorna a barra mais � montante de um conjunto de blocos
 *
 * Obs: setar a rede de refer�ncia (redeRef)
 */
VTBarra* __fastcall TFuncoesDeRede::GetBarraMontanteArea(TList* lisBlocos)
{
	TList* lisLigacoes;
	VTLigacao* liga;

   if(lisBlocos == NULL) return NULL;

   // Obt�m as liga��es que pertencem � �rea dos blocos
   lisLigacoes = new TList();
	for(int i=0; i<redeRef->LisLigacao()->Count; i++)
   {
   	liga = (VTLigacao*) redeRef->LisLigacao()->Items[i];

      if(AreaTemLigacao(lisBlocos, liga))
      {
         lisLigacoes->Add(liga);
      }
   }

	// A liga��o mais � montante n�o tem liga pai dentro da �rea
   for(int i=0; i<lisLigacoes->Count; i++)
   {
      liga = (VTLigacao*) lisLigacoes->Items[i];

      if(lisLigacoes->IndexOf(liga->ligaPai) < 0)
      	break;
      else
      	liga = NULL;
   }

	if(liga == NULL)
   {
      // Destroi lista de liga��es
      delete lisLigacoes; lisLigacoes = NULL;
   	return NULL;
   }

   // Destroi lista de liga��es
   delete lisLigacoes; lisLigacoes = NULL;

   return liga->Barra(0);
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetCargasMT(TStringList* lisCodEqptosCampo, int numCargasPorBloco, TStringList* lisCodCargasMT)
{
	String codEqptoRef;
   TList* lisLigacao;
   VTLigacao* liga;
	VTRede* rede;

	// Verifica��es
   if(lisCodEqptosCampo == NULL || lisCodCargasMT == NULL) return;
   if(lisCodEqptosCampo->Count == 0) return;

   // Pega um eqpto da lista de eqptos de campo
   codEqptoRef = lisCodEqptosCampo->Strings[0];

   // Decide redeRef (rede de refer�ncia)
   redeRef = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      lisLigacao = rede->LisLigacao();

      liga = NULL;
      for(int j=0; j<lisLigacao->Count; j++)
      {
			liga = (VTLigacao*) lisLigacao->Items[j];

       	if(liga->Codigo == codEqptoRef)
         	break;
         else
         	liga = NULL;
      }

		if(liga != NULL)
      {
         redeRef = rede;
         break;
      }
   }
   if(redeRef == NULL) return;

   // numCargasPorBloco = -1  ==>  todas as cargas
   GetCargasBlocosRede(redeRef, numCargasPorBloco, lisCodCargasMT);
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetCargasBlocosRede(VTRede* redeRef, int numCargasPorBloco, TStringList* lisCodCargasMT)
{
	int limite;
	TList *lisBlocos, *lisAux;
   VTBloco* bloco;
   VTCarga* carga;

	// Verifica��es
   if(lisCodCargasMT == NULL) return;
   if(blocos == NULL)
   {
   	//Cria objeto de blocos
		blocos = DLL_NewObjBlocos();
      apl->Add(blocos);
   }
	//Executa para a lista de redes
	blocos->Executa(redes);

	lisAux = new TList();
   lisBlocos = new TList();
   blocos->LisBlocoRede(redeRef, lisBlocos);

   for(int i=0; i<lisBlocos->Count; i++)
   {
      bloco = (VTBloco*) lisBlocos->Items[i];
      lisAux->Clear();
      bloco->LisEqbar(lisAux, eqptoCARGA);

      if(numCargasPorBloco == -1)
	      limite = lisAux->Count;
      else if(numCargasPorBloco > 0)
      {
         if(lisAux->Count < numCargasPorBloco)
         	limite = lisAux->Count;
         else
         	limite = numCargasPorBloco;
      }
      else
      	limite = 0;

      for(int j=0; j<limite; j++)
      {
         carga = (VTCarga*) lisAux->Items[j];
         lisCodCargasMT->Add(carga->Codigo);
      }
   }

   // Destroi listas
   delete lisBlocos; lisBlocos = NULL;
   delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetChavesBlocos(TList* listaBlocos, TList* listaExt)
{
	TList* lisElos;
	VTBloco* bloco;
   VTElo* elo;
   VTRede* rede;

	// Verifica��es iniciais
	if((listaBlocos == NULL) || (listaExt == NULL)) return;

//   // Cria objeto de ordena��o
//   if(this->ordena == NULL)
//   {
//      this->ordena = DLL_NewObjOrdena(this->apl);
//      // Ordena a rede investigada
//      this->ordena->Executa(this->redes);
//   }

   lisElos = new TList();

	for(int i=0; i<listaBlocos->Count; i++)
   {
		bloco = (VTBloco*) listaBlocos->Items[i];

      lisElos->Clear();
      blocos->LisElo(bloco, lisElos);

      for(int j=0; j<lisElos->Count; j++)
      {
			elo = (VTElo*) lisElos->Items[j];

         if(listaExt->IndexOf(elo->Chave) < 0)
         	listaExt->Add(elo->Chave);
      }
   }

   // Destroi listas
   delete lisElos; lisElos = NULL;
}
//---------------------------------------------------------------------------
String __fastcall TFuncoesDeRede::GetCodChaveSinap(String codEqptoSage, TStringList* lisCadDisjuntores)
{
	if(!lisCadDisjuntores || codEqptoSage == "")
		return "";

   for(int i=0; i<lisCadDisjuntores->Count; i++)
   {
		String linha = lisCadDisjuntores->Strings[i];

      if(codEqptoSage == GetCampoCSV(linha, 1, ";"))
      {
         return (GetCampoCSV(linha, 2, ";"));
      }
   }

   return "";
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de blocos � jusante de uma barra
 *
 * entrada: barra de refer�ncia
 * sa�da: lista ordenada de blocos (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetBlocosJusanteBarra(VTRede* redeMT, VTBarra* barraRef, TList* lisExt)
{
	if(redeMT == NULL || barraRef == NULL || lisExt == NULL)
		return;

	TList* lisLigacoes = redeMT->LisLigacao();
	VTLigacao* ligacao = NULL;

	// Obt�m uma liga��o que tenha a barra passada por par�metro
	for(int i=0; i<lisLigacoes->Count; i++)
	{
		ligacao = (VTLigacao*) lisLigacoes->Items[i];
		if(ligacao->Barra(0) == barraRef || ligacao->Barra(1) == barraRef)
			break;
		else
			ligacao = NULL;
	}
	if(ligacao == NULL)
		return;

	// Chama m�todo para obter os blocos � jusante da liga��o determinada
	GetBlocosJusanteLigacao(ligacao, lisExt);
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de blocos � jusante de uma liga��o
 *
 * entrada: liga��o de refer�ncia
 * sa�da: lista ordenada de blocos (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetBlocosJusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt)
{
   TList* listaBarrasCaminho;
   TList* listaLigacoesCaminho;

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligacaoRef == NULL) || (lisExt == NULL)) return;

	// Cria objeto de ordena��o
	ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena));
	if(!ordena)
	{
		ordena = DLL_NewObjOrdena(this->apl);
		apl->Add(ordena);
		ordena->Executa(redes);
	}

	// Procura a rede que tem a liga��o de refer�ncia
	VTRede* rede = NULL;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      if(rede->ExisteLigacao(ligacaoRef)) break;
      else rede = NULL;
   }

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
	if(rede == NULL) return;

	if(redeRef != rede)
	{
      redeRef = rede;
      // Obt�m os blocos da rede
		listaBlocosRede = new TList();
		GetBlocosRede(redeRef, listaBlocosRede);
	}
//	// Obt�m os blocos da rede
//	if(listaBlocosRede) {delete listaBlocosRede; listaBlocosRede = NULL;}
//	listaBlocosRede = new TList();
//	GetBlocosRede(rede, listaBlocosRede);

   // Cria listas para os caminhos
   listaBarrasCaminho = new TList();
	listaLigacoesCaminho = new TList();
	// Para um bloco, pega a �ltima barra e obt�m o caminho da barra � SE
	for(int i=0; i<listaBlocosRede->Count; i++)
   {
   	// Informa��es de um bloco
      VTBloco* bloco = (VTBloco*) listaBlocosRede->Items[i];
      TList* listaBarrasBloco = bloco->LisBarra();
      TList* listaLigacoesBloco = bloco->LisLigacao();

      // Se o bloco cont�m a liga��o de refer�ncia,
      // adiciona o bloco e passa para o pr�ximo
      if(listaLigacoesBloco->IndexOf(ligacaoRef) >= 0)
      {
         if(lisExt->IndexOf(bloco) < 0) lisExt->Add(bloco);
         continue;
      }

      // Pega a �ltima barra do bloco
      VTBarra* barraRef = (VTBarra*) listaBarrasBloco->Items[listaBarrasBloco->Count-1];

      // Perfaz o caminho desde a barra de refer�ncia at� a SE
      listaBarrasCaminho->Clear();
      listaLigacoesCaminho->Clear();
		GetCaminhoBarrasSE_Barra(redeRef, barraRef, listaBarrasCaminho);
      GetCaminhoLigacoesSE_Barra(redeRef, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
      {
      	if(lisExt->IndexOf(bloco) < 0) lisExt->Add(bloco);
      }

      // ====== DEBUG:
//      TStringList* listaBarras = new TStringList();
//      for(int j=0; j<listaBarrasCaminho->Count; j++)
//      {
//         VTBarra* barra = (VTBarra*) listaBarrasCaminho->Items[j];
//         listaBarras->Add(barra->Codigo);
//      }
//      TStringList* listaLigacoes = new TStringList();
//      for(int j=0; j<listaLigacoesCaminho->Count; j++)
//      {
//         VTLigacao* liga = (VTLigacao*) listaLigacoesCaminho->Items[j];
//         listaLigacoes->Add(liga->Codigo);
//      }
//      String fileName = "caminho" + String(i + 1) + ".txt";
//      listaBarras->SaveToFile("c:\\users\\user\\desktop\\caminhosBarras\\" + fileName);
//      listaLigacoes->SaveToFile("c:\\users\\user\\desktop\\caminhosLigacoes\\" + fileName);
		// ====== FIM DEBUG
   }

   // Destroi listas
   if(listaBarrasCaminho != NULL) {delete listaBarrasCaminho; listaBarrasCaminho = NULL;}
	if(listaLigacoesCaminho != NULL) {delete listaLigacoesCaminho; listaLigacoesCaminho = NULL;}
}
//---------------------------------------------------------------------------
// M�todo que verifica se "ligaJus" � uma liga��o � jusante de "ligaMont", considerando
// as liga��es da lista "lisLigacoes".
bool __fastcall TFuncoesDeRede::LigacaoJusante(VTLigacao* ligaMont, VTLigacao* ligaJus)
{
	if(ligaMont == NULL || ligaJus == NULL) return false;

	// Cria objeto de ordena��o
	if(this->ordena == NULL)
	{
		this->ordena = DLL_NewObjOrdena(this->apl);
		// Ordena a rede investigada
		this->ordena->Executa(this->redes);
	}

	VTLigacao* ligaPai = ligaJus;
	while(ligaPai != NULL)
	{
		if(ligaPai->Codigo == ligaMont->Codigo)
			return(true);

		ligaPai = ligaPai->ligaPai;
	}
	return(false);
}
//---------------------------------------------------------------------------
VTBloco* __fastcall TFuncoesDeRede::GetBlocoAssociadoChave(VTChave* chave)
{
	if(!chave) return(NULL);

	if(!redes) redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos));
	if(!blocos)
	{
		//Cria objeto de blocos
		blocos = DLL_NewObjBlocos();
		apl->Add(blocos);
		blocos->Executa(redes);
	}
	VTRede* rede = chave->rede;
	if(!rede) return(NULL);

	// Pega a liga��o ap�s a chave de refer�ncia
	VTLigacao* ligacaoJusanteChave = GetLigacaoFilha(chave, rede);
	// Encontra o bloco procurado
	VTBloco* blocoLigacao = blocos->ExisteBloco(ligacaoJusanteChave);

	return(blocoLigacao);
}
//---------------------------------------------------------------------------
VTBloco* __fastcall TFuncoesDeRede::GetBlocoAssociadoCarga(VTCarga* carga)
{
	if(!carga) return(NULL);

	if(!redes) redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos));
	if(!blocos)
	{
		//Cria objeto de blocos
		blocos = DLL_NewObjBlocos();
		apl->Add(blocos);
		blocos->Executa(redes);
	}
	VTBloco* blocoCarga = blocos->ExisteBloco(carga->pbarra);

   return(blocoCarga);
}
//---------------------------------------------------------------------------
/**
 * Este m�todo fornece uma lista dos blocos de uma rede.
 * Entrada: rede
 * Sa�da: lista de blocos da rede
 */
void __fastcall TFuncoesDeRede::GetBlocosRede(VTRede* rede, TList* listaExt)
{
	if(listaExt == NULL) return;

	blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos));
	if(!blocos)
	{
		//Cria objeto de blocos
		blocos = DLL_NewObjBlocos();
		apl->Add(blocos);
		blocos->Executa(redes);
	}
	//Limpa lista externa
	listaExt->Clear();
	//Obt�m a lista de blocos de uma rede
	blocos->LisBlocoRede(rede, listaExt);
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetBlocosRede(String CodigoAlimentador, TList* listaExt)
{
	VTRede* rede;

	// Verifica��es
	if(listaExt == NULL) return;

   // Procura a rede em quest�o
   rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];

      if(rede->TipoRede->Segmento != redePRI)
      	continue;

      if(rede->Codigo == CodigoAlimentador)
      	break;
      else
      	rede = NULL;
   }

   if(rede == NULL)
   	return;

   // Pega lista de blocos da rede
   GetBlocosRede(rede, listaExt);
}
//---------------------------------------------------------------------------
/***
 * M�todo que retorna lista (lisExt) com as liga��es ordenadas, desde uma barra 0 at� uma barra de refer�ncia.
 *
 * Entrada: rede, barra 0 e barra de refer�ncia
 * Sa�da: lista ordenada de barras (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetCaminhoLigacoes_BarraRef_Barra(VTRede* rede, VTBarra* barra0, VTBarra* barraRef, TList* lisExt)
{
	TList* listaLigacoes;
	VTLigacao *ligaRef, *ligaPai;

	// Se a lista externa for nula ou a barra for nula, sai do m�todo
	if((lisExt == NULL)|| (barraRef == NULL)) return;

   // Cria objeto de ordena��o
   if(ordena == NULL)
   {
      ordena = DLL_NewObjOrdena(apl);
      // Ordena a rede investigada
      ordena->Executa(redes);
   }

   // Lista de liga��es da rede
	listaLigacoes = rede->LisLigacao();

   // Procura a liga��o de refer�ncia
   ligaRef = NULL;
   for(int i=0; i<listaLigacoes->Count; i++)
   {
      ligaRef = (VTLigacao*) listaLigacoes->Items[i];
      if((ligaRef->Barra(0) == barraRef) || (ligaRef->Barra(1) == barraRef))
			break;
      else
      	ligaRef = NULL;
   }

   // Se n�o achou a liga��o, sai do m�todo
   if(ligaRef == NULL) return;

   // Percorre as liga��es at� chegar � barra 0
   while(true)
   {
      // Adiciona as barras � lista externa
      if(lisExt->IndexOf(ligaRef) < 0)
      {
	      lisExt->Insert(0, ligaRef);
      }

      // Verifica se j� chegou � liga��o inicial
      if((ligaRef->Barra(0) == barra0) || (ligaRef->Barra(1) == barra0))
      {
         break;
      }
      else
      {
      	// Seta a liga��o de refer�ncia
    		ligaRef = ligaRef->ligaPai;
         // Verifica liga��o
         if(ligaRef == NULL)
         {
            lisExt->Clear();
            break;
         }
      }
   }
}
//---------------------------------------------------------------------------
/***
 * M�todo que retorna lista (lisExt) com as barras ordenadas, desde a subesta��o
 * at� a barra de refer�ncia.
 *
 * Entrada: rede e barra de refer�ncia
 * Sa�da: lista ordenada de barras (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetCaminhoBarrasSE_Barra(VTRede* rede, VTBarra* barraRef, TList* lisExt)
{
	VTLigacao *ligaRef, *ligaPai;

	// Se a lista externa for nula ou a barra for nula, sai do m�todo
	if((lisExt == NULL)|| (barraRef == NULL)) return;

   // Lista de liga��es da rede
	TList* listaLigacoes = rede->LisLigacao();

   // Procura a liga��o de refer�ncia
   ligaRef = NULL;
   for(int i=0; i<listaLigacoes->Count; i++)
   {
      ligaRef = (VTLigacao*) listaLigacoes->Items[i];
      if((ligaRef->Barra(0) == barraRef) || (ligaRef->Barra(1) == barraRef))
			break;
      else
      	ligaRef = NULL;
   }

   // Se n�o achou a liga��o, sai do m�todo
   if(ligaRef == NULL) return;

   // Percorre as liga��es at� chegar � liga��o inicial (n�o tem ligaPai)
   while(true)
   {
      // Adiciona as barras � lista externa
      if(lisExt->IndexOf(ligaRef->Barra(1)) < 0)
      {
	      lisExt->Insert(0, ligaRef->Barra(1));
      }
		if(lisExt->IndexOf(ligaRef->Barra(0)) < 0)
      {
         lisExt->Insert(0, ligaRef->Barra(0));
      }

      // Verifica se j� chegou � liga��o inicial
      if(ligaRef->ligaPai == NULL)
      {
         break;
      }
      else
      {
      	// Seta a liga��o de refer�ncia
    		ligaRef = ligaRef->ligaPai;
      }
   }
}
//---------------------------------------------------------------------------
// lisBlocos: lista com todos os blocos da �rea de interesse
void __fastcall TFuncoesDeRede::GetClusteres(TList* lisBlocos, TList* lisEXT)
{
	TBifurcacao* bifurc;
   TCluster* cluster;
   TList* lisBifurcacoes;
   TList* lisBlocosJusLiga;
	TList* listaChavesRede;
   VTChave* chave;
   VTLigacao* liga;
   VTLigacao* ligacaoMontanteArea;

	if(lisBlocos == NULL || lisEXT == NULL) return;

//   //debug
//   for(int i=0; i<lisBlocos->Count; i++)
//   {
//   	VTBloco* bloco = (VTBloco*) lisBlocos->Items[i];
//      for(int j=0; j<bloco->LisBarra()->Count; j++)
//      {
//      	VTBarra* barra = (VTBarra*) bloco->LisBarra()->Items[j];
//         int a = 0;
//      }
//   }

//   // Verifica se VTBuscaProt j� foi inicializado
//   if((buscaProt = (VTBuscaProt*) apl->GetObject(__classid(VTBuscaProt))) == NULL)
//   {
//      // Verifica VTBlocos para iniciar VTBuscaProt
//      if((blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos))) == NULL)
//      {
//	      blocos = DLL_NewObjBlocos();
//         apl->Add(blocos);
//         blocos->Executa(redes);
//      }
//      buscaProt = DLL_NewBuscaProt(apl, true);
//      apl->Add(buscaProt);
//      buscaProt->Inicia(false);
//   }


   // Chaves iniciais dos clusteres
   TList* lisChavesIniClusteres = new TList();
   GetChavesIniciaisClusteres(lisChavesIniClusteres);

   for(int i=0; i<lisChavesIniClusteres->Count; i++)
   {
   	VTChave* chvMontante =  (VTChave*) lisChavesIniClusteres->Items[i];

      cluster = new TCluster(chvMontante);
      lisBlocosJusLiga = new TList();
      GetBlocosJusanteLigacao((VTLigacao*) chvMontante, lisBlocosJusLiga);
      cluster->SetBlocos(lisBlocosJusLiga);
      lisEXT->Add(cluster);
   }
   // Ordena com base no n�mero de liga��es
   OrdenaClusteres(lisEXT);

	FiltraClusteres(lisBlocos, lisEXT);

   OrdenaBlocosClusteres(lisEXT);

   // Destroi lista
   delete lisChavesIniClusteres; lisChavesIniClusteres = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::OrdenaBlocosClusteres(TList* lisClusteres)
{
	TCluster* cluster;

   for(int i=0; i<lisClusteres->Count; i++)
   {
   	cluster = (TCluster*) lisClusteres->Items[i];
      OrdenaBlocosCluster(cluster);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::OrdenaBlocosCluster(TCluster* cluster)
{
   VTChave* chvMont;

   // Transfere os blocos temporariamente para lista auxiliar
   TList* lisAux = new TList();
   for(int i=cluster->GetBlocos()->Count-1; i>=0; i--)
   {
		VTBloco* bloco = (VTBloco*) cluster->GetBlocos()->Items[i];
      cluster->GetBlocos()->Remove(bloco);
      lisAux->Add(bloco);
   }

   // Determina o bloco mais � montante
   chvMont = cluster->chaveMontante;
   VTBloco* blocoMont = NULL;
   for(int i=0; i<lisAux->Count; i++)
   {
		blocoMont = (VTBloco*) lisAux->Items[i];
      if(blocoMont->ExisteBarra(chvMont->Barra(0)) || blocoMont->ExisteBarra(chvMont->Barra(1)))
      {
      	lisAux->Remove(blocoMont);
       	cluster->GetBlocos()->Add(blocoMont);
         break;
      }
      else
      {
         blocoMont = NULL;
      }
   }
   if(!blocoMont)
   	return;

   // Transfere os blocos para sua lista original, observando uma certa ordem
   while(lisAux->Count > 0)
   {
      for(int i=0; i<lisAux->Count; i++)
      {
         VTBloco* bloco = (VTBloco*) lisAux->Items[i];
         if(ExisteEloComBlocoExistente(cluster->GetBlocos(), bloco))
         {
            lisAux->Remove(bloco);
            cluster->GetBlocos()->Add(bloco);
            break;
         }
      }
   }

   // Destroi lista
   delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::ExisteEloComBlocoExistente(TList* lisBloExistentes, VTBloco* bloco)
{
	if(!lisBloExistentes || !bloco)
   	return false;

   for(int i=0; i<lisBloExistentes->Count; i++)
   {
   	VTBloco* blocoExistente = (VTBloco*) lisBloExistentes->Items[i];
      if(blocos->ExisteElo(blocoExistente, bloco))
      	return true;
   }

   return false;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetChavesIniciaisClusteres(TList* lisExt)
{
	if(lisExt == NULL)
   	return;

   TStringList* LisCodChvIniCluster = new TStringList();
   String pathIniCluster = path->DirDat() + "\\FaultLocation\\IniCluster.txt";
	LisCodChvIniCluster->LoadFromFile(pathIniCluster);

   lisExt->Clear();
   redeRef->LisChave(lisExt, chvFECHADA);

   for(int i=lisExt->Count-1; i>=0; i--)
   {
		VTChave* chave = (VTChave*) lisExt->Items[i];
      if(LisCodChvIniCluster->IndexOf(chave->Codigo) < 0)
      	lisExt->Remove(chave);
   }

   // Destroi lista
   delete LisCodChvIniCluster; LisCodChvIniCluster = NULL;
}
//---------------------------------------------------------------------------
/***
 * Conta o n�mero de liga��es filha, para definir se d� origem a uma bifurca��o
 */
TBifurcacao* __fastcall TFuncoesDeRede::Bifurcacao(VTLigacao* ligaRef)
{
	int cont;
   TBifurcacao* bifurc;
   TList* lisLigaIni;
   TList* lisBlocosJus = NULL;
   VTLigacao *liga, *ligaAux;

	if(ligaRef == NULL || redeRef == NULL)
   	return NULL;

   cont = 0;
   lisLigaIni = new TList();
   for(int i=0; i<redeRef->LisLigacao()->Count; i++)
   {
   	liga = (VTLigacao*) redeRef->LisLigacao()->Items[i];

		if(liga->ligaPai == ligaRef)
      {
      	lisLigaIni->Add(liga);
			cont += 1;
      }
   }

   bifurc = NULL;
   if(cont > 1)
   {
      lisBlocosJus = new TList();
      GetBlocosJusanteLigacao((VTLigacao*) lisLigaIni->Items[0], lisBlocosJus);
      if(lisBlocosJus->Count <= 5)
      	return NULL;

      lisBlocosJus->Clear();
      GetBlocosJusanteLigacao((VTLigacao*) lisLigaIni->Items[1], lisBlocosJus);
      if(lisBlocosJus->Count <= 5)
      	return NULL;

      bifurc = new TBifurcacao(ligaRef);
      for(int i=0; i<lisLigaIni->Count; i++)
      {
         liga = (VTLigacao*) lisLigaIni->Items[i];
         bifurc->SetLigaIni(liga);
      }
   }

   delete lisLigaIni; lisLigaIni = NULL;
   if(lisBlocosJus) {delete lisBlocosJus; lisBlocosJus = NULL;}

   return bifurc;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::RemoveCaminhosSemelhantes(TList* lisCaminhos, double MaxPorcLigacoesIguais)
{

}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetLigaFilhas(VTLigacao* ligacao, TList* lisFilhas)
{
   VTLigacao* liga;

   if(ligacao == NULL || redeRef == NULL || lisFilhas == NULL) return;

   for(int i=0; i<redeRef->LisLigacao()->Count; i++)
   {
      liga = (VTLigacao*) redeRef->LisLigacao()->Items[i];

      if(liga->ligaPai == ligacao)
      	lisFilhas->Add(liga);
   }
}
//---------------------------------------------------------------------------
/***
 * M�todo que retorna lista (lisExt) com as liga��es ordenadas, desde a subesta��o
 * at� a barra de refer�ncia.
 *
 * Entrada: rede e barra de refer�ncia
 * Sa�da: lista ordenada de liga��es (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetCaminhoLigacoesSE_Barra(VTRede* rede, VTBarra* barraRef, TList* lisExt)
{
	VTLigacao *ligaRef, *ligaPai;

	// Se a lista externa for nula ou a barra for nula, sai do m�todo
	if((lisExt == NULL)|| (barraRef == NULL)) return;

   // Lista de liga��es da rede
	TList* listaLigacoes = rede->LisLigacao();

	// Verifica ordena��o das redes
	ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena));
	if(!ordena)
	{
		ordena = DLL_NewObjOrdena(this->apl);
		apl->Add(ordena);
		ordena->Executa(this->redes);
	}

   // Procura a liga��o de refer�ncia
   ligaRef = NULL;
   for(int i=0; i<listaLigacoes->Count; i++)
   {
      ligaRef = (VTLigacao*) listaLigacoes->Items[i];
      if((ligaRef->Barra(0) == barraRef) || (ligaRef->Barra(1) == barraRef))
			break;
      else
      	ligaRef = NULL;
   }

   // Se n�o achou a liga��o, sai do m�todo
   if(ligaRef == NULL) return;

   // Percorre as liga��es at� chegar � liga��o inicial (n�o tem ligaPai)
   while(true)
   {
      // Adiciona as barras � lista externa
      if(lisExt->IndexOf(ligaRef) < 0)
      {
	      lisExt->Insert(0, ligaRef);
      }

      // Verifica se j� chegou � liga��o inicial
      if(ligaRef->ligaPai == NULL)
      {
         break;
      }
      else
      {
      	// Seta a liga��o de refer�ncia
    		ligaRef = ligaRef->ligaPai;
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Percorre as liga��es at� totalizar uma dist�ncia = Dist.
 */
void __fastcall TFuncoesDeRede::GetCaminhoComprimento(TList* caminho, double Dist)
{
	double somaMetros = 0.;

   // Prote��o
   if(caminho->Count == 0) return;

   double comp = 0.;
   String Codigo = "";
   for(int i=0; i<caminho->Count; i++)
   {
      VTLigacao* liga = (VTLigacao*) caminho->Items[i];
      comp = liga->ComprimentoUtm_cm() / 100.;
      Codigo = liga->Codigo;
      somaMetros += liga->ComprimentoUtm_cm() / 100.;
   }

   // Se a soma for inferior � dist�ncia desejada, limpa a lista
   if(somaMetros < Dist)
   {
      caminho->Clear();
      return;
   }

   // Percorre o caminho de tr�s para frente, verificando se o comprimento
   // desejado foi atingido
   for(int i=caminho->Count-1; i>=0; i--)
   {
      VTLigacao* liga = (VTLigacao*) caminho->Items[i];
      somaMetros -= liga->ComprimentoUtm_cm() / 100.;
      if(somaMetros < Dist)
      	break;
      else
      {
         caminho->Remove(liga);
      }
   }

}
//---------------------------------------------------------------------------
/****
 * Para uma barra, retorna uma lista com os caminhos, a partir dela, cujo
 * comprimento � = Dist.
 **/
void __fastcall TFuncoesDeRede::GetCaminhosDistancia(VTRede* redeMT, String codBarra, double Dist, TList* listaExt)
{
	TList* lisLigacoesTerminais = new TList();
   TList* caminho = NULL;
   VTBarra* barra0 = NULL;
	VTRede* rede = NULL;

   if(listaExt == NULL) return;

   listaExt->Clear();
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
   	rede = (VTRede*) redes->LisRede()->Items[i];
      for(int j=0; j<rede->LisBarra()->Count; j++)
      {
       	barra0 = (VTBarra*) rede->LisBarra()->Items[j];
         if(barra0->Codigo == codBarra)
	         break;
         else
         	barra0 = NULL;
      }
      if(barra0 != NULL)
      	break;
      else
      	rede = NULL;
   }

	if((barra0 == NULL) || (listaExt == NULL) || (rede == NULL)) return;

   // Cria objeto de ordena��o, caso ainda n�o exista
   if(ordena == NULL)
   {
      ordena = DLL_NewObjOrdena(this->apl);
      // Ordena a rede investigada
      ordena->Executa(this->redes);
   }

   // Salva refer�ncia para a rede MT em quest�o
	rede = redeMT;

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
   if(rede == NULL) return;

	// Lista com liga��es terminais do circuito
	GetLigacoesTerminais(rede, lisLigacoesTerminais);

   int len = 0;
   for(int i=0; i<lisLigacoesTerminais->Count; i++)
   {
   	// Para cada liga��o terminal, obt�m o caminho desde a barra 0 at� ela
		VTLigacao* ligaTerm = (VTLigacao*) lisLigacoesTerminais->Items[i];
		caminho = new TList();
      GetCaminhoLigacoes_BarraRef_Barra(rede, barra0, ligaTerm->Barra(1), caminho);

      len = caminho->Count;
      // Percorre o caminho at� totalizar a dist�ncia desejada
      GetCaminhoComprimento(caminho, Dist);

      // Armazena caminho
      if((caminho->Count > 0) && !ExisteCaminhoLista(listaExt, caminho))
      {
         listaExt->Add(caminho);
      }
   }

   // Destroi lista
   delete lisLigacoesTerminais; lisLigacoesTerminais = NULL;
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de caminhos, a partir de uma barra de refer�ncia,
 * que tenham um comprimento = Dist.
 */
void __fastcall TFuncoesDeRede::GetCaminhosDistancia(VTBarra* barra, double Dist, TList* listaExt)
{
	GetCaminhosDistancia(barra->Codigo, Dist, listaExt);
}
//---------------------------------------------------------------------------
/****
 * Para uma barra, retorna uma lista com os caminhos, a partir dela, cujo
 * comprimento � = Dist.
 **/
void __fastcall TFuncoesDeRede::GetCaminhosDistancia(String codBarra, double Dist, TList* listaExt)
{
	TList* lisLigacoesTerminais = new TList();
   TList* caminho = NULL;
   VTBarra* barra0 = NULL;
	VTRede* rede = NULL;

   if(listaExt == NULL) return;

   listaExt->Clear();
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
   	rede = (VTRede*) redes->LisRede()->Items[i];
      for(int j=0; j<rede->LisBarra()->Count; j++)
      {
       	barra0 = (VTBarra*) rede->LisBarra()->Items[j];
         if(barra0->Codigo == codBarra)
	         break;
         else
         	barra0 = NULL;
      }
      if(barra0 != NULL)
      	break;
      else
      	rede = NULL;
   }

	if((barra0 == NULL) || (listaExt == NULL) || (rede == NULL)) return;

   // Cria objeto de ordena��o, caso ainda n�o exista
   if(ordena == NULL)
   {
      ordena = DLL_NewObjOrdena(this->apl);
      // Ordena a rede investigada
      ordena->Executa(this->redes);
   }

   // Procura a rede que tem a barra de refer�ncia
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      if(rede->ExisteBarra(barra0)) break;
      else rede = NULL;
   }

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
   if(rede == NULL) return;

	// Lista com liga��es terminais do circuito
	GetLigacoesTerminais(rede, lisLigacoesTerminais);

   int len = 0;
   for(int i=0; i<lisLigacoesTerminais->Count; i++)
   {
   	// Para cada liga��o terminal, obt�m o caminho desde a barra 0 at� ela
		VTLigacao* ligaTerm = (VTLigacao*) lisLigacoesTerminais->Items[i];
		caminho = new TList();
      GetCaminhoLigacoes_BarraRef_Barra(rede, barra0, ligaTerm->Barra(1), caminho);

      len = caminho->Count;
      // Percorre o caminho at� totalizar a dist�ncia desejada
      GetCaminhoComprimento(caminho, Dist);

      // Armazena caminho
      if((caminho->Count > 0) && !ExisteCaminhoLista(listaExt, caminho))
      {
         listaExt->Add(caminho);
      }
   }

   // Destroi lista
   delete lisLigacoesTerminais; lisLigacoesTerminais = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::BlocosJusanteLigacao_RompCabo(VTLigacao* ligacaoRef, TList* lisExt)
{
   TList* listaBarrasCaminho = NULL;
   TList* listaLigacoesCaminho = NULL;

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligacaoRef == NULL) || (lisExt == NULL)) return;

	// Cria objeto de ordena��o
	if(this->ordena == NULL)
	{
		this->ordena = DLL_NewObjOrdena(this->apl);
		// Ordena a rede investigada
		this->ordena->Executa(this->redes);
	}


	// Procura a rede que tem a liga��o de refer�ncia
	VTRede* rede = NULL;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      if(rede->ExisteLigacao(ligacaoRef)) break;
      else rede = NULL;
   }

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
	if(rede == NULL) return;

	if(rede != rede_RompCabo)
	{
		rede_RompCabo = rede;
		listaBlocosRede_RompCabo->Clear();
		GetBlocosRede(rede, listaBlocosRede_RompCabo);
	}

   // Cria listas para os caminhos
   listaBarrasCaminho = new TList();
	listaLigacoesCaminho = new TList();
	// Para um bloco, pega a �ltima barra e obt�m o caminho da barra � SE
	for(int i=0; i<listaBlocosRede_RompCabo->Count; i++)
   {
   	// Informa��es de um bloco
      VTBloco* bloco = (VTBloco*) listaBlocosRede_RompCabo->Items[i];
		TList* listaBarrasBloco = bloco->LisBarra();
      TList* listaLigacoesBloco = bloco->LisLigacao();

      // Se o bloco cont�m a liga��o de refer�ncia,
      // adiciona o bloco e passa para o pr�ximo
      if(listaLigacoesBloco->IndexOf(ligacaoRef) >= 0)
      {
         if(lisExt->IndexOf(bloco) < 0) lisExt->Add(bloco);
         continue;
      }

      // Pega a �ltima barra do bloco
      VTBarra* barraRef = (VTBarra*) listaBarrasBloco->Items[listaBarrasBloco->Count-1];

      // Perfaz o caminho desde a barra de refer�ncia at� a SE
      listaBarrasCaminho->Clear();
      listaLigacoesCaminho->Clear();
      GetCaminhoBarrasSE_Barra(rede_RompCabo, barraRef, listaBarrasCaminho);
      GetCaminhoLigacoesSE_Barra(rede_RompCabo, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
      {
      	if(lisExt->IndexOf(bloco) < 0) lisExt->Add(bloco);
		}
   }

   // Destroi listas
   if(listaBarrasCaminho) {delete listaBarrasCaminho; listaBarrasCaminho = NULL;}
	if(listaLigacoesCaminho) {delete listaLigacoesCaminho; listaLigacoesCaminho = NULL;}
}
//---------------------------------------------------------------------------
int __fastcall TFuncoesDeRede::NumeroConsJusLigacao_SolucoesRompCabo(String CodLigacao)
{
	int NumConsumidores = 0;
	VTRede* rede;
	TList* lisLiga;
	TList* blocosJus;
	VTCarga* carga;
	VTLigacao* liga;

	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Codigo == CodLigacao)
         {
         	break;
         }
         else
         	liga = NULL;
      }

      if(liga != NULL) break;
      else rede = NULL;
   }
	if(rede == NULL || liga == NULL) return 0;

   // Pega lista de blocos � jusante da liga��o de refer�ncia
	blocosJus = new TList();
	BlocosJusanteLigacao_RompCabo(liga, blocosJus);

   // Para cada bloco, pega suas cargas e adiciona � lista externa
	TList* lisCargas = new TList();
	for(int i=0; i<blocosJus->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) blocosJus->Items[i];

		lisCargas->Clear();
		bloco->LisEqbar(lisCargas, eqptoCARGA);

		for(int j=0; j<lisCargas->Count; j++)
		{
			carga = (VTCarga*) lisCargas->Items[j];
			NumConsumidores += carga->NumConsTotal;
      }
	}

	// Destroi listas
	if(blocosJus) {delete blocosJus; blocosJus = NULL;}
   if(lisCargas) {delete lisCargas; lisCargas = NULL;}

	return NumConsumidores;
}
//---------------------------------------------------------------------------
/***
 * M�todo que retorna a lista de consumidores � jusante de uma Ligacao
 */
int __fastcall TFuncoesDeRede::GetNumConsJusLigacao(String CodLigacao)
{
	int NumConsumidores;
	TList* blocosJus;
   TList* lisCargas;
   TList* lisLiga;
	VTCarga* carga;
   VTLigacao* liga;
   VTRede* rede;

	// Pega a rede MT
   liga = NULL;
   rede = NULL;

   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Codigo == CodLigacao)
         {
         	break;
         }
         else
         	liga = NULL;
      }

      if(liga != NULL) break;
      else rede = NULL;
   }
   if(rede == NULL || liga == NULL) return 0;

	// Pega lista de blocos � jusante da liga��o de refer�ncia
	blocosJus = new TList();
	GetBlocosJusanteLigacao(liga, blocosJus);

   // Inicializa contador
   NumConsumidores = 0;

   // Para cada bloco, pega suas cargas e adiciona � lista externa
	lisCargas = new TList();
	for(int i=0; i<blocosJus->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) blocosJus->Items[i];

      lisCargas->Clear();
      bloco->LisEqbar(lisCargas, eqptoCARGA);

      for(int j=0; j<lisCargas->Count; j++)
      {
			carga = (VTCarga*) lisCargas->Items[j];

       	NumConsumidores += carga->NumConsTotal;
      }
	}

	// Destroi listas
	if(blocosJus) {delete blocosJus; blocosJus = NULL;}
   if(lisCargas) {delete lisCargas; lisCargas = NULL;}

	return NumConsumidores;
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece a dist�ncia entre barra1 e barra2
 *
 * Entrada: barra1 e barra2
 * Sa�da: double com a dist�ncia (em metros) entre barra1 e barra2
 */
double __fastcall TFuncoesDeRede::GetDistanciaMetros(VTBarra* barra1, VTBarra* barra2)
{
	double Metros;
	TList* lisCaminho = NULL;
   VTLigacao* liga;
	VTRede* rede;
   VTTrecho* trecho;

	// Verifica��o
   if(barra1 == NULL || barra2 == NULL) return -1.;

   // Pega a rede MT
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteBarra(barra1) || rede->ExisteBarra(barra2))
      	break;
      else
      	rede = NULL;
   }

   // Inicializa a dist�ncia total
   Metros = 0.;

   // Pega o caminho entre barra 1 e barra 2 e acumula os comprimentos dos trechos
   if(rede != NULL)
   {
   	lisCaminho = new TList();
     	GetCaminhoLigacoes_BarraRef_Barra(rede, barra1, barra2, lisCaminho);
      for(int i=0; i<lisCaminho->Count; i++)
      {
      	liga = (VTLigacao*) lisCaminho->Items[i];
			if(liga->Tipo() != eqptoTRECHO) continue;

         trecho = (VTTrecho*) liga;
         Metros += trecho->Comprimento_m;
      }
   }

   if(lisCaminho) {delete lisCaminho; lisCaminho = NULL;}

   return Metros;
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece a dist�ncia entre uma barra e a subesta��o
 *
 */
double __fastcall TFuncoesDeRede::GetDistancia_KM_DaSubestacao(VTBarra* barraRef)
{
	double KM;
   TList* lisCaminho = NULL;
   VTBarra* barraIni;
   VTLigacao* liga;
	VTRede* rede;
   VTTrecho* trecho;

	// Verifica��o
   if(barraRef == NULL)
   	return 0.;

   // Pega a rede MT
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteBarra(barraRef))
      	break;
      else
      	rede = NULL;
   }

   KM = 0.;

   // Pega o caminho entre barra 1 e barra 2 e acumula os comprimentos dos trechos
   if(rede != NULL)
   {
   	lisCaminho = new TList();
      barraIni = rede->BarraInicial();
     	GetCaminhoLigacoes_BarraRef_Barra(rede, barraIni, barraRef, lisCaminho);
      for(int i=0; i<lisCaminho->Count; i++)
      {
      	liga = (VTLigacao*) lisCaminho->Items[i];
			if(liga->Tipo() != eqptoTRECHO) continue;

         trecho = (VTTrecho*) liga;
         KM += trecho->Comprimento_m / 1000.;
      }
   }

   if(lisCaminho) {delete lisCaminho; lisCaminho = NULL;}

   return KM;
}
//---------------------------------------------------------------------------
String __fastcall TFuncoesDeRede::GetFases(VTRede* redeMT, VTBarra* barra)
{
	int fases = -1;
   String resp = "";

	if(!redeMT || !barra)
   	return "";

   for(int i=0; i<redeMT->LisLigacao()->Count; i++)
   {
		VTLigacao* liga = (VTLigacao*) redeMT->LisLigacao()->Items[i];
      if(liga->Barra(0) == barra || liga->Barra(1) == barra)
		{
      	fases = liga->Fases(barra);
         break;
      }
   }

   if(fases == -1)
   	return "";

   switch(fases)
   {
   case faseA:
   case faseAN:
   case faseAT:
   case faseANT:
   	resp = "A";
      break;

   case faseB:
   case faseBN:
   case faseBT:
   case faseBNT:
   	resp = "B";
      break;

   case faseC:
   case faseCN:
   case faseCT:
   case faseCNT:
   	resp = "C";
      break;

   case faseAB:
   case faseABN:
   case faseABT:
   case faseABNT:
   	resp = "AB";
      break;

   case faseBC:
   case faseBCN:
   case faseBCT:
   case faseBCNT:
   	resp = "BC";
      break;

   case faseCA:
   case faseCAN:
   case faseCAT:
   case faseCANT:
   	resp = "CA";
      break;

   case faseABC:
   case faseABCN:
   case faseABCT:
   case faseABCNT:
   	resp = "ABC";
      break;

   default:
   	resp = "";
      break;
   }

   return resp;
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de listas de liga��es dos blocos.
 *
 * Entrada: lista de blocos
 * Sa�da: lista com listas de liga��es
 */
void __fastcall TFuncoesDeRede::GetLigacoesBlocos(TList* listaBlocos, TList* listaExt)
{
	if(listaExt == NULL) return;

	TList* listaLigacoes = NULL;
	//Para cada bloco, gera uma lista de barras
	for(int i=0; i<listaBlocos->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) listaBlocos->Items[i];
		listaLigacoes = new TList();
		bloco->LisLigacao(listaLigacoes);
		//adiciona lista de barras � lista externa
		listaExt->Add(listaLigacoes);
	}
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de liga��es � jusante de uma liga��o
 *
 * entrada: liga��o de refer�ncia
 * sa�da: lista ordenada de liga��es (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetLigacoesJusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt)
{
	TList* listaBlocosRede;
   TList* listaLigacoesCaminho;
   VTBloco* blocoDaLigacao = NULL;

	TList* listaBlocosJusante = new TList();

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligacaoRef == NULL) || (lisExt == NULL)) return;

	if(this->ordena == NULL)
	{
		// Cria objeto de ordena��o
		this->ordena = DLL_NewObjOrdena(this->apl);
		// Ordena a rede investigada
		this->ordena->Executa(this->redes);
	}

	// Procura a rede que tem a liga��o de refer�ncia
	VTRede* rede = NULL;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
	{
		rede = (VTRede*) this->redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteLigacao(ligacaoRef)) break;
		else rede = NULL;
	}

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
   if(rede == NULL) return;

	// Obt�m os blocos da rede
	listaBlocosRede = new TList();
   GetBlocosRede(rede, listaBlocosRede);

   // Cria listas para os caminhos
   listaLigacoesCaminho = new TList();
   // Para um bloco, pega a �ltima barra e obt�m o caminho da barra � SE
	for(int i=0; i<listaBlocosRede->Count; i++)
   {
   	// Informa��es de um bloco
      VTBloco* bloco = (VTBloco*) listaBlocosRede->Items[i];
      TList* listaBarrasBloco = bloco->LisBarra();
      TList* listaLigacoesBloco = bloco->LisLigacao();

      // Se o bloco cont�m a liga��o de refer�ncia, salva refer�ncia
      if(listaLigacoesBloco->IndexOf(ligacaoRef) >= 0)
      {
         blocoDaLigacao = bloco;
         continue;
      }

      // Pega a �ltima barra do bloco
      VTBarra* barraRef = (VTBarra*) listaBarrasBloco->Items[listaBarrasBloco->Count-1];

      // Perfaz o caminho desde a barra de refer�ncia at� a SE
      listaLigacoesCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
      {
      	if(listaBlocosJusante->IndexOf(bloco) < 0) listaBlocosJusante->Add(bloco);
      }
   }

   // Para cada bloco, insere as liga��es na lista externa
   for(int i=0; i<listaBlocosJusante->Count; i++)
   {
      VTBloco* bloco = (VTBloco*) listaBlocosJusante->Items[i];
      TList* listaLiga = bloco->LisLigacao();

      // Insere cada liga��o do bloco na lista externa
		for(int j=0; j<listaLiga->Count; j++)
      {
         VTLigacao* liga = (VTLigacao*) listaLiga->Items[j];
         if(lisExt->IndexOf(liga) < 0) lisExt->Add(liga);
      }
   }

   // Para o bloco com a liga��o de refer�ncia, insere apenas as liga��es �
   // jusante da liga��o de refer�ncia
   if(blocoDaLigacao != NULL)
   {
      TList* listaLiga = blocoDaLigacao->LisLigacao();
      for(int i=0; i<listaLiga->Count; i++)
      {
         VTLigacao* liga = (VTLigacao*) listaLiga->Items[i];
         // Pega a barra 2 da liga��o como barra de refer�ncia
         VTBarra* barraRef = liga->Barra(1);
         // Pega o caminho, de liga��es ordenadas, desde a SE at� a barra de refer�ncia
         listaLigacoesCaminho->Clear();
         GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

         // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
         // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
         // adiciona o bloco � lista externa.
         if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
         {
            if(lisExt->IndexOf(liga) < 0) lisExt->Add(liga);
         }
      }
   }

   // Destroi listas
//	if(listaBlocosRede != NULL) {delete listaBlocosRede; listaBlocosRede = NULL;}
//   if(listaLigacoesCaminho != NULL) {delete listaLigacoesCaminho; listaLigacoesCaminho = NULL;}
}
//---------------------------------------------------------------------------
VTTrecho* __fastcall TFuncoesDeRede::GetTrechoJusanteLigacao(VTLigacao* ligaRef, VTRede* rede)
{
	VTLigacao* liga;

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligaRef == NULL) || (rede == NULL)) return NULL;

	if(this->ordena == NULL)
	{
		// Cria objeto de ordena��o
		this->ordena = DLL_NewObjOrdena(this->apl);
		// Ordena a rede investigada
		this->ordena->Executa(this->redes);
	}

	liga = GetLigacaoFilha(ligaRef, rede);
	while(liga->Tipo() != eqptoTRECHO)
	{
		GetLigacaoFilha(liga, rede);
	}
	return((VTTrecho*)liga);
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TFuncoesDeRede::GetLigacaoFilha(VTLigacao* ligacaoRef, VTRede* rede)
{
	VTLigacao* ligacao;

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligacaoRef == NULL) || (rede == NULL)) return NULL;

  	if(this->ordena == NULL)
	{
		// Cria objeto de ordena��o
		this->ordena = DLL_NewObjOrdena(this->apl);
		// Ordena a rede investigada
		this->ordena->Executa(this->redes);
	}

	ligacao = NULL;
	for(int i=0; i<rede->LisLigacao()->Count; i++)
	{
		ligacao = (VTLigacao*) rede->LisLigacao()->Items[i];
		if(ligacao->ligaPai == ligacaoRef)
			break;
		else
			ligacao = NULL;
	}
	return(ligacao);
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de liga��es ordenadas � jusante de uma liga��o
 *
 * entrada: liga��o de refer�ncia
 * sa�da: lista ordenada de liga��es (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetLigacoesOrdenadas_JusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt)
{
	TList* listaBlocosRede;
	TList* listaLigacoesCaminho;
   VTBloco* blocoDaLigacao = NULL;

   TList* listaBlocosJusante = new TList();

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((ligacaoRef == NULL) || (lisExt == NULL)) return;

   if(this->ordena == NULL)
   {
      // Cria objeto de ordena��o
      this->ordena = DLL_NewObjOrdena(this->apl);
      // Ordena a rede investigada
      this->ordena->Executa(this->redes);
   }

	// Procura a rede que tem a liga��o de refer�ncia
	VTRede* rede = NULL;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      if(rede->ExisteLigacao(ligacaoRef)) break;
      else rede = NULL;
   }

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
   if(rede == NULL) return;

	// Obt�m os blocos da rede
	listaBlocosRede = new TList();
   GetBlocosRede(rede, listaBlocosRede);

   // Cria listas para os caminhos
   listaLigacoesCaminho = new TList();
   // Para um bloco, pega a �ltima barra e obt�m o caminho da barra � SE
	for(int i=0; i<listaBlocosRede->Count; i++)
   {
   	// Informa��es de um bloco
      VTBloco* bloco = (VTBloco*) listaBlocosRede->Items[i];
      TList* listaBarrasBloco = bloco->LisBarra();
      TList* listaLigacoesBloco = bloco->LisLigacao();

      // Se o bloco cont�m a liga��o de refer�ncia, salva refer�ncia
      if(listaLigacoesBloco->IndexOf(ligacaoRef) >= 0)
      {
         blocoDaLigacao = bloco;
         continue;
      }

      // Pega a �ltima barra do bloco
      VTBarra* barraRef = (VTBarra*) listaBarrasBloco->Items[listaBarrasBloco->Count-1];

      // Perfaz o caminho desde a barra de refer�ncia at� a SE
      listaLigacoesCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
      {
      	if(listaBlocosJusante->IndexOf(bloco) < 0) listaBlocosJusante->Add(bloco);
      }
   }

   // Para cada bloco, insere as liga��es na lista externa
   for(int i=0; i<listaBlocosJusante->Count; i++)
   {
      VTBloco* bloco = (VTBloco*) listaBlocosJusante->Items[i];
      TList* listaLiga = bloco->LisLigacao();

      // Insere cada liga��o do bloco na lista externa
		for(int j=0; j<listaLiga->Count; j++)
      {
         VTLigacao* liga = (VTLigacao*) listaLiga->Items[j];
         if(lisExt->IndexOf(liga) < 0) lisExt->Add(liga);
      }
   }

   // Para o bloco com a liga��o de refer�ncia, insere apenas as liga��es �
   // jusante da liga��o de refer�ncia
   if(blocoDaLigacao != NULL)
   {
      TList* listaLiga = blocoDaLigacao->LisLigacao();
      for(int i=0; i<listaLiga->Count; i++)
      {
         VTLigacao* liga = (VTLigacao*) listaLiga->Items[i];
         // Pega a barra 2 da liga��o como barra de refer�ncia
         VTBarra* barraRef = liga->Barra(1);
         // Pega o caminho, de liga��es ordenadas, desde a SE at� a barra de refer�ncia
         listaLigacoesCaminho->Clear();
         GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

         // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
         // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
         // adiciona o bloco � lista externa.
         if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
         {
				if(lisExt->IndexOf(liga) < 0) lisExt->Add(liga);
         }
      }
	}

	// Verifica a ordem das liga��es
	bool ok = false;
	while(!ok)
	{
		ok = true;
		for(int i=0; i<lisExt->Count; i++)
		{
			VTLigacao* liga = (VTLigacao*) lisExt->Items[i];

			for(int j=0; j<lisExt->Count; j++)
			{
				VTLigacao* liga2 = (VTLigacao*) lisExt->Items[j];
				if(liga2 == liga->ligaPai && j > i)
				{
					lisExt->Remove(liga2);
					lisExt->Insert(i, liga2);
					ok = false;
					break;
				}
			}
		}
	}

   // Destroi listas
//	if(listaBlocosRede != NULL) {delete listaBlocosRede; listaBlocosRede = NULL;}
//   if(listaLigacoesCaminho != NULL) {delete listaLigacoesCaminho; listaLigacoesCaminho = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Este m�todo fornece uma lista de liga��es � jusante de uma liga��o
 *
 * entrada: liga��o de refer�ncia
 * sa�da: lista ordenada de liga��es (lisExt)
 **/
void __fastcall TFuncoesDeRede::GetLigacoesJusanteLigacao(String codLigacaoRef, TStringList* lisExt)
{
	TList* listaBlocosRede;
   TList* listaLigacoesCaminho;
   VTBloco* blocoDaLigacao = NULL;

   TList* listaBlocosJusante = new TList();

	// Se a liga��o de refer�ncia � nula ou a lista "lisExt" � nula,  sai do m�todo
	if((codLigacaoRef == "") || (lisExt == NULL)) return;

   if(this->ordena == NULL)
   {
      // Cria objeto de ordena��o
      this->ordena = DLL_NewObjOrdena(this->apl);
      // Ordena a rede investigada
      this->ordena->Executa(this->redes);
   }

	// Procura a rede que tem a liga��o de refer�ncia
	VTRede* rede = NULL;
   VTLigacao* liga = NULL;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      TList* listaLigacoes = rede->LisLigacao();
      for(int i=0; i<listaLigacoes->Count; i++)
      {
         liga = (VTLigacao*) listaLigacoes->Items[i];
         if(liga->Codigo == codLigacaoRef) break;
         else liga = NULL;
      }

      if(liga == NULL)
      	rede = NULL;
      else
      	break;
   }

   // Se n�o encontrou a rede com aquela liga��o, sai do m�todo
   if(rede == NULL) return;

	// Obt�m os blocos da rede
	listaBlocosRede = new TList();
	GetBlocosRede(rede, listaBlocosRede);

   // Cria listas para os caminhos
   listaLigacoesCaminho = new TList();
   // Para um bloco, pega a �ltima barra e obt�m o caminho da barra � SE
	for(int i=0; i<listaBlocosRede->Count; i++)
   {
   	// Informa��es de um bloco
      VTBloco* bloco = (VTBloco*) listaBlocosRede->Items[i];
      TList* listaBarrasBloco = bloco->LisBarra();
      TList* listaLigacoesBloco = bloco->LisLigacao();

      // Se o bloco cont�m a liga��o de refer�ncia, salva refer�ncia
//      if(listaLigacoesBloco->IndexOf(ligacaoRef) >= 0)
//      {
//         blocoDaLigacao = bloco;
//         continue;
//      }
      for(int j=0; j<listaLigacoesBloco->Count; j++)
      {
			VTLigacao* ligaAux = (VTLigacao*) listaLigacoesBloco->Items[j];
         if(ligaAux->Codigo == codLigacaoRef)
         {
         	blocoDaLigacao = bloco;
         	break;
         }
      }

      // Pega a �ltima barra do bloco
      VTBarra* barraRef = (VTBarra*) listaBarrasBloco->Items[listaBarrasBloco->Count-1];

      // Perfaz o caminho desde a barra de refer�ncia at� a SE
      listaLigacoesCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
//      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
//      {
//      	if(listaBlocosJusante->IndexOf(bloco) < 0) listaBlocosJusante->Add(bloco);
//      }
      for(int j=0; j<listaLigacoesCaminho->Count; j++)
      {
      	VTLigacao* ligaAux = (VTLigacao*) listaLigacoesCaminho->Items[j];

         if(ligaAux->Codigo == codLigacaoRef)
         {
            if(listaBlocosJusante->IndexOf(bloco) < 0) listaBlocosJusante->Add(bloco);

         	break;
         }
      }
   }

   // Para cada bloco, insere as liga��es na lista externa
   for(int i=0; i<listaBlocosJusante->Count; i++)
   {
      VTBloco* bloco = (VTBloco*) listaBlocosJusante->Items[i];
      TList* listaLiga = bloco->LisLigacao();

      // Insere cada liga��o do bloco na lista externa
		for(int j=0; j<listaLiga->Count; j++)
      {
         VTLigacao* liga = (VTLigacao*) listaLiga->Items[j];
         if(lisExt->IndexOf(liga->Codigo) < 0) lisExt->Add(liga->Codigo);
      }
   }

   // Para o bloco com a liga��o de refer�ncia, insere apenas as liga��es �
   // jusante da liga��o de refer�ncia
   TList* listaLiga = blocoDaLigacao->LisLigacao();
   for(int i=0; i<listaLiga->Count; i++)
   {
      VTLigacao* liga = (VTLigacao*) listaLiga->Items[i];
     	// Pega a barra 2 da liga��o como barra de refer�ncia
      VTBarra* barraRef = liga->Barra(1);
      // Pega o caminho, de liga��es ordenadas, desde a SE at� a barra de refer�ncia
      listaLigacoesCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, barraRef, listaLigacoesCaminho);

      // Se o caminho de liga��es cont�m a liga��o de refer�ncia, ent�o
      // o bloco investigado est� � jusante da liga��o de refer�ncia. Assim,
      // adiciona o bloco � lista externa.
//      if(listaLigacoesCaminho->IndexOf(ligacaoRef) >= 0)
//      {
//      	if(lisExt->IndexOf(liga->Codigo) < 0) lisExt->Add(liga->Codigo);
//      }
      for(int j=0; j<listaLigacoesCaminho->Count; j++)
      {
			VTLigacao* ligaAux = (VTLigacao*) listaLigacoesCaminho->Items[j];

         if(ligaAux->Codigo == codLigacaoRef)
         {
            if(lisExt->IndexOf(liga->Codigo) < 0) lisExt->Add(liga->Codigo);
	         break;
         }
      }
   }

   // Destroi listas
	if(listaBlocosRede != NULL) {delete listaBlocosRede; listaBlocosRede = NULL;}
   if(listaLigacoesCaminho != NULL) {delete listaLigacoesCaminho; listaLigacoesCaminho = NULL;}
}
//---------------------------------------------------------------------------
/**
 * Determina quais liga��es n�o t�m "liga��o filha", ou liga��o � jusante
 * 	Obs: � necess�rio que a rede esteja ordenada
 */
void __fastcall TFuncoesDeRede::GetLigacoesTerminais(VTRede* rede, TList* listaExt)
{
   VTLigacao *liga = NULL, *ligaJus = NULL;

   if(listaExt == NULL) return;
   listaExt->Clear();

	for(int i=0; i<rede->LisLigacao()->Count; i++)
   {
		liga = (VTLigacao*) rede->LisLigacao()->Items[i];

      for(int i=0; i<rede->LisLigacao()->Count; i++)
      {
      	ligaJus = (VTLigacao*) rede->LisLigacao()->Items[i];
         if(liga == ligaJus->ligaPai)
         	break;
         else
         	ligaJus = NULL;
      }

      // Se n�o encontrou liga��o � jusante, a liga��o "liga" � terminal
      if(ligaJus == NULL)
      {
         listaExt->Add(liga);
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Par�metros de entrada:
 * CodChaveRef: c�digo da chave � montante dos sensores de interesse
 * lisCadSensores: lista formato CSV com o cadastro de todos os sensores <alimentador>;<c�digo sensor>;<c�digo chave>
 *
 * Lista de sa�da: lisSensoresJusante
 */
void __fastcall TFuncoesDeRede::GetSensoresJusanteChave(String CodChaveRef, TStringList* lisCadSensores, TStringList* lisSensoresJusante)
{
	String codLigacao;
   TList* lisAux;

	// Verifica��o
	if(lisSensoresJusante == NULL || CodChaveRef == "") return;

   // Cria objeto de ordena��o
   if(ordena == NULL)
   {
      ordena = DLL_NewObjOrdena(apl);
      // Ordena a rede investigada
      ordena->Executa(redes);
   }

   // Limpa lista externa (de sensores � jusante)
   lisSensoresJusante->Clear();

   // Pega todas as liga��es � jusante de uma chave
   VTLigacao* liga = NULL;
   VTRede* rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      TList* lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Codigo == CodChaveRef)
         {
         	break;
         }
         else
         {
         	liga = NULL;
         }
      }

      if(liga != NULL) break;
      else rede = NULL;
   }

   TList* listaChavesJusLiga = new TList();
   GetChavesJusanteLigacao(liga, listaChavesJusLiga);


   for(int i=0; i<listaChavesJusLiga->Count; i++)
   {
		VTChave* chv = (VTChave*) listaChavesJusLiga->Items[i];

      // Se o c�digo da liga��o for de um sensor � jusante, adiciona � lista externa

		for(int j=0; j<lisCadSensores->Count; j++)
      {
         String campoCodChave = GetCampoCSV(lisCadSensores->Strings[j], 2, ";");  // c�d. chave = 3o campo (campo = 2)

         if(chv->Codigo == campoCodChave)
         {
				String campoCodSensor = GetCampoCSV(lisCadSensores->Strings[j], 1, ";");  // c�d. sensor = 2o campo (campo = 1)
         	lisSensoresJusante->Add(campoCodSensor);
            break;
         }
      }
   }

   delete listaChavesJusLiga; listaChavesJusLiga = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetChaves(TStringList* lisCodChaves, TList* lisEXT)
{
	bool found = false;
   TList* lisLiga = NULL;
   VTRede* rede = NULL;

	if(!lisEXT || !lisCodChaves)
   	return;

	// Procura a rede que tem a liga��o de refer�ncia
   found = false;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			VTLigacao* liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Tipo() != eqptoCHAVE) continue;

         if(liga->Codigo == lisCodChaves->Strings[0])
         {
         	found = true;
            break;
         }
      }

		if(found)
      	break;
      else
      	rede = NULL;
   }

   if(!rede)
   	return;

   for(int j=0; j<lisLiga->Count; j++)
   {
   	VTLigacao* liga = (VTLigacao*) lisLiga->Items[j];
      if(liga->Tipo() != eqptoCHAVE) continue;

      if(lisCodChaves->IndexOf(liga->Codigo) >= 0 && lisEXT->IndexOf(liga) < 0)
      	lisEXT->Add(liga);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetTrechos(TStringList* lisCodTrechos, TList* lisEXT)
{
	bool found = false;
   TList* lisLiga = NULL;
   VTRede* rede = NULL;

	if(!lisEXT || !lisCodTrechos)
   	return;

	// Procura a rede que tem a liga��o de refer�ncia
   found = false;
	for(int i=0; i<this->redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) this->redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			VTLigacao* liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Tipo() != eqptoTRECHO) continue;

         if(liga->Codigo == lisCodTrechos->Strings[0])
         {
         	found = true;
            break;
         }
      }

		if(found)
      	break;
      else
      	rede = NULL;
   }

   if(!rede)
   	return;

   for(int j=0; j<lisLiga->Count; j++)
   {
   	VTLigacao* liga = (VTLigacao*) lisLiga->Items[j];
      if(liga->Tipo() != eqptoTRECHO) continue;

      if(lisCodTrechos->IndexOf(liga->Codigo) >= 0 && lisEXT->IndexOf(liga) < 0)
      	lisEXT->Add(liga);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetChavesJusanteLigacao(VTLigacao* liga, TList* listaChavesJusLiga)
{
   listaChavesJusLiga->Clear();

	VTRede* rede = liga->rede;

   TList* listaChavesRede = new TList();
   rede->LisLigacao(listaChavesRede, eqptoCHAVE);

   TList* listaCaminho = new TList();
   for(int i=0; i<listaChavesRede->Count; i++)
   {
   	VTChave* chave = (VTChave*) listaChavesRede->Items[i];

      listaCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, chave->Barra(0), listaCaminho);

		// Verifica se a liga��o de refer�ncia (liga) est� no caminho
      if(listaCaminho->IndexOf(liga) >= 0)
      	listaChavesJusLiga->Add(chave);
   }

   delete listaChavesRede; listaChavesRede = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetCargasJusanteLigacao(VTLigacao* ligaRef, TList* lisEXT)
{
   lisEXT->Clear();

   VTRede* rede = ligaRef->rede;

   TList* listaCargasRede = new TList();
   rede->LisEqpto(listaCargasRede, eqptoCARGA);

   TList* listaCaminho = new TList();
   for(int i=0; i<listaCargasRede->Count; i++)
   {
   	VTCarga* carga = (VTCarga*) listaCargasRede->Items[i];

      listaCaminho->Clear();
      GetCaminhoLigacoesSE_Barra(rede, carga->pbarra, listaCaminho);

		// Verifica se a liga��o de refer�ncia (liga) est� no caminho
      if(listaCaminho->IndexOf(ligaRef) >= 0)
      	lisEXT->Add(carga);
   }

   // Destroi listas
   delete listaCargasRede; listaCargasRede = NULL;
   delete listaCaminho; listaCaminho = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetChavesJusanteBarra(VTRede* redeRef, VTBarra* barraRef, TList* listaChavesJusLiga)
{
	if(barraRef == NULL || listaChavesJusLiga == NULL)
		return;

	// Cria objeto de ordena��o
	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

   if(barraRef == redeRef->BarraInicial())
   {
      listaChavesJusLiga->Clear();
      redeRef->LisChave(listaChavesJusLiga, chvFECHADA);
   }
   else
   {
//      VTLigacao* ligaRef = GetLigacaoJusanteBarra(barraRef, redeRef);
      VTLigacao* ligaRef = GetLigacaoMontanteBarra(barraRef);
      if(ligaRef->Tipo() == eqptoCHAVE) ligaRef = ligaRef->ligaPai;

      GetChavesJusanteLigacao(ligaRef, listaChavesJusLiga);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetCargasJusanteBarra(VTRede* redeRef, VTBarra* barraRef, TList* lisEXT)
{
	if(!barraRef || !lisEXT) return;

	// Cria objeto de ordena��o
	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

   if(barraRef == redeRef->BarraInicial())
   {
      lisEXT->Clear();
      redeRef->LisEqpto(lisEXT, eqptoCARGA);
   }
   else
   {
      VTLigacao* ligaRef = GetLigacaoMontanteBarra(barraRef);
      GetCargasJusanteLigacao(ligaRef, lisEXT);
   }
}
//---------------------------------------------------------------------------
/***
 * A partir do c�digo de uma chave, retorna a VTChave � montante
 */
VTChave* __fastcall TFuncoesDeRede::GetChaveMontante(String CodChaveRef)
{
	TList* lisLiga;
   TList* listaCaminho = NULL;
	VTChave* chaveMontante = NULL;
   VTLigacao* liga;
   VTRede* rede;

	// Cria objeto de ordena��o
	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		// Ordena a rede investigada
		ordena->Executa(redes);
	}


   // Pega a rede MT
   liga = NULL;
   rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

      lisLiga = rede->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Codigo == CodChaveRef)
         {
         	break;
         }
         else
         	liga = NULL;
      }

      if(liga != NULL) break;
      else rede = NULL;
   }

   if(liga != NULL && rede != NULL)
   {
      // Pega o caminho desde a SE at� a chave de refer�ncia
   	listaCaminho = new TList();
      GetCaminhoLigacoesSE_Barra(rede, liga->Barra(0), listaCaminho);
      VTLigacao* ligaAux = NULL;

      // Percorre  o caminho de tr�s para frente
      for(int i=listaCaminho->Count-1; i>=0; i--)
      {
			ligaAux = (VTLigacao*) listaCaminho->Items[i];
         if(ligaAux->Tipo() == eqptoCHAVE)
         {
				chaveMontante = (VTChave*) ligaAux;
				if(chaveMontante->TipoDisjuntor || chaveMontante->TipoReligadora)
	            break;
            else
            	chaveMontante = NULL;
			}
      }
   }

   if(listaCaminho) {delete listaCaminho; listaCaminho = NULL;}

   return chaveMontante;
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TFuncoesDeRede::GetBarraMontanteLigacao(VTLigacao* liga)
{
	VTBarra* barraMontante = NULL;

	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

	if(liga->ligaPai != NULL)
	{
		if(liga->Barra(0) == liga->ligaPai->Barra(0) || liga->Barra(0) == liga->ligaPai->Barra(1))
			barraMontante = liga->Barra(0);
		else if(liga->Barra(1) == liga->ligaPai->Barra(0) || liga->Barra(1) == liga->ligaPai->Barra(1))
			barraMontante = liga->Barra(1);
	}
	return(barraMontante);
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TFuncoesDeRede::GetLigacaoJusanteBarra(VTBarra* barraRef, VTRede* rede)
{
	if(barraRef == NULL || rede == NULL) return NULL;

	TList* lisLigacoes;
	VTLigacao* ligaJusante;

	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

	ligaJusante = NULL;
	lisLigacoes = rede->LisLigacao();
	for(int i=0; i<lisLigacoes->Count; i++)
	{
		ligaJusante = (VTLigacao*) lisLigacoes->Items[i];
		if(GetBarraMontanteLigacao(ligaJusante) == barraRef)
			break;
		else
			ligaJusante = NULL;
	}
	return(ligaJusante);
}
//---------------------------------------------------------------------------
VTTrecho* __fastcall TFuncoesDeRede::GetTrechoJusanteBarra(VTBarra* barraRef, VTRede* rede)
{

	TList* lisLigacoes;
	VTLigacao *liga, *ligaAux;
	int contador = 0;

	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

	liga = GetLigacaoJusanteBarra(barraRef, rede);
	while(liga->Tipo() != eqptoTRECHO)
	{
		lisLigacoes = rede->LisLigacao();
		for(int i=0; i<lisLigacoes->Count; i++)
		{
			ligaAux = (VTLigacao*) lisLigacoes->Items[i];
			if(ligaAux->ligaPai == liga)
			{
				liga = ligaAux;
				break;
			}
		}
	}
	return((VTTrecho*)liga);
}
//---------------------------------------------------------------------------
VTTrecho* __fastcall TFuncoesDeRede::GetTrechoMontanteBarra(VTBarra* barraRef, VTRede* rede)
{
	VTLigacao *liga;

	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		ordena->Executa(redes);
	}

	TList* lisLigacoes = rede->LisLigacao();
	for(int i=0; i<lisLigacoes->Count; i++)
	{
		liga = (VTLigacao*) lisLigacoes->Items[i];
		if(liga->Barra(0) == barraRef || liga->Barra(1) == barraRef)
			break;
	}

	while(liga->Tipo() != eqptoTRECHO)
	{
		liga = liga->ligaPai;
	}
	return((VTTrecho*)liga);
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TFuncoesDeRede::GetLigacaoMontanteBarra(VTBarra* barraRef)
{
	if(!barraRef) return(NULL);
	VTRede* rede;

	// Cria objeto de ordena��o
	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		// Ordena a rede investigada
		ordena->Executa(redes);
	}

	// Pega a rede MT
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteBarra(barraRef))
			break;
		else
			rede = NULL;
	}

	for(int i=0; i<rede->LisLigacao()->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) rede->LisLigacao()->Items[i];
		VTLigacao* ligaPai = liga->ligaPai;
		if(ligaPai == NULL) continue;

		if(liga->Barra(0) == barraRef || liga->Barra(1) == barraRef)
		{
			if(ligaPai->Barra(0) != barraRef && ligaPai->Barra(1) != barraRef)
			{
				return(liga);
			}
		}
	}
	return(NULL);
}
//---------------------------------------------------------------------------
bool __fastcall TFuncoesDeRede::TemCarga(VTBarra* barra)
{
	if(!barra || !barra->LisEqbar()) return(NULL);
	for(int i=0; i<barra->LisEqbar()->Count; i++)
	{
		VTEqbar* eqbar = (VTEqbar*) barra->LisEqbar()->Items[i];
		if(eqbar->Tipo() == eqptoCARGA)
			return(true);
	}
	return(false);
}
//---------------------------------------------------------------------------
/***
 * A partir de uma barra, retorna barra associada a uma chave ou a um trafo (ET)
 * ou a uma bifurca��o � montante.
 */
VTBarra* __fastcall TFuncoesDeRede::GetBarra_ChaveTrafoMontante(VTBarra* barraRef)
{
	TList *listaCaminho, *lisLigacoesRede;
	VTLigacao* ligaAux;
	VTRede* rede;
	VTBarra *barra1, *barra2, *barraResp;

	// Inicializa ponteiros
	rede = NULL;
	barra1 = NULL;
	barra2 = NULL;
	barraResp = NULL;

	// Cria objeto de ordena��o
	if(ordena == NULL)
	{
		ordena = DLL_NewObjOrdena(apl);
		// Ordena a rede investigada
		ordena->Executa(redes);
	}

	// Pega a rede MT
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteBarra(barraRef))
			break;
		else
			rede = NULL;
	}


	// Debug - teste
	if(rede != NULL)
	{
		ligaAux = GetLigacaoMontanteBarra(barraRef);

		// Percorre as liga��es em dire��o � SE
		while(ligaAux != NULL)
		{
			// Barra pai da liga��o
			barraRef = GetBarraMontanteLigacao(ligaAux);

			// Se a barra pai tem carga/trafo, se a liga��o � uma chave
			// ou se a liga��o � uma bifurca��o, interrompe o processo.
			if(TemCarga(barraRef) || ligaAux->Tipo() == eqptoCHAVE ||
				VerificaBifurcacao(ligaAux, rede->LisLigacao()))
			{
				barraResp = barraRef;
				break;
			}
			ligaAux = ligaAux->ligaPai;
		}
	}


//	if(rede != NULL)
//	{
//		// Pega o caminho desde a SE at� a barra de refer�ncia
//		listaCaminho = new TList();
//		GetCaminhoLigacoesSE_Barra(rede, barraRef, listaCaminho);
//		ligaAux = NULL;
//
//		lisLigacoesRede = rede->LisLigacao();
//
//		// Percorre o caminho de tr�s para frente
//      for(int i=listaCaminho->Count-2; i>=0; i--)
//      {
//			ligaAux = (VTLigacao*) listaCaminho->Items[i];
//
//			if(ligaAux->Barra(0) == barraRef || ligaAux->Barra(1) == barraRef)
//				continue;
//
//			barra1 = ligaAux->Barra(0);
//			barra2 = ligaAux->Barra(1);
//
//			// Verifica se a barra tem carga associada
//			if(barra1->LisEqbar() != NULL && barra1->LisEqbar()->Count > 0)
//			{
//				for(int j=0; j<barra1->LisEqbar()->Count; j++)
//				{
//					VTEqbar* eqbar = (VTEqbar*) barra1->LisEqbar()->Items[j];
//					if(eqbar->Tipo() == eqptoCARGA)
//					{
//						barraResp = barra1;
//						break;
//					}
//				}
//				if(barraResp != NULL)
//					break;
//			}
//
//			// Verifica se a barra tem carga associada
//			if(barra2->LisEqbar() != NULL && barra2->LisEqbar()->Count > 0)
//			{
//				for(int j=0; j<barra2->LisEqbar()->Count; j++)
//				{
//					VTEqbar* eqbar = (VTEqbar*) barra2->LisEqbar()->Items[j];
//					if(eqbar->Tipo() == eqptoCARGA)
//					{
//						barraResp = barra2;
//						break;
//					}
//				}
//				if(barraResp != NULL)
//					break;
//			}
//
//			// Verifica se a liga��o � uma chave
//			if(ligaAux->Tipo() == eqptoCHAVE)
//			{
//				VTLigacao* ligaMontante = ligaAux->ligaPai;
//				if(barra1 == ligaMontante->Barra(0) || barra1 == ligaMontante->Barra(1))
//				{
//					barraResp = barra1;
//					break;
//				}
//				else if(barra2 == ligaMontante->Barra(0) || barra2 == ligaMontante->Barra(1))
//				{
//					barraResp = barra2;
//					break;
//				}
//			}
//
//			// Verifica se � bifurca��o
//			if(VerificaBifurcacao(ligaAux, lisLigacoesRede))
//			{
//				barraResp = GetBarraMontanteLigacao(ligaAux);
//				break;
//         }
//		}
//		delete listaCaminho; listaCaminho = NULL;
//	}


	return(barraResp);
}
//---------------------------------------------------------------------------
// Se a liga��o ligaRef tiver pelo menos 2 liga��es filhas, � bifurca��o
bool __fastcall TFuncoesDeRede::VerificaBifurcacao(VTLigacao* ligaRef, TList* lisLigacoesRede)
{
	if(ligaRef == NULL || lisLigacoesRede == NULL) return false;

	bool resp = false;
	TList* listaLigacoesFilhas = new TList();

	for(int i=0; i<lisLigacoesRede->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) lisLigacoesRede->Items[i];
		if(liga->ligaPai == ligaRef && listaLigacoesFilhas->IndexOf(liga) < 0)
			listaLigacoesFilhas->Add(liga);
	}
	if(listaLigacoesFilhas->Count > 1)
		resp = true;

	delete listaLigacoesFilhas; listaLigacoesFilhas = NULL;
	return(resp);
}
//---------------------------------------------------------------------------
/***
 * A partir de uma chave, retorna a VTChave � montante
 */
VTChave* __fastcall TFuncoesDeRede::GetChaveMontante(VTBarra* barraRef)
{
	TList* lisLiga;
   TList* listaCaminho = NULL;
	VTChave* chaveMontante = NULL;
	VTLigacao* ligaAux;
   VTRede* rede;

	// Inicializa ponteiros
   chaveMontante = NULL;
	rede = NULL;

   // Cria objeto de ordena��o
   if(ordena == NULL)
   {
      ordena = DLL_NewObjOrdena(apl);
      // Ordena a rede investigada
      ordena->Executa(redes);
   }

   // Pega a rede MT
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

		if(rede->ExisteBarra(barraRef))
      	break;
      else
      	rede = NULL;
   }

   if(rede != NULL)
   {
      // Pega o caminho desde a SE at� a chave de refer�ncia
   	listaCaminho = new TList();
      GetCaminhoLigacoesSE_Barra(rede, barraRef, listaCaminho);
      ligaAux = NULL;

      // Percorre  o caminho de tr�s para frente
      for(int i=listaCaminho->Count-1; i>=0; i--)
      {
			ligaAux = (VTLigacao*) listaCaminho->Items[i];
//         if(ligaAux->Tipo() == eqptoCHAVE)
//         {
//				chaveMontante = (VTChave*) ligaAux;
//				if(chaveMontante->TipoDisjuntor || chaveMontante->TipoReligadora)
//	            break;
//            else
//            	chaveMontante = NULL;
//			}

			if(ligaAux->Tipo() == eqptoCHAVE)
         {
            chaveMontante = (VTChave*) ligaAux;
            break;
         }
         else
         {
            chaveMontante = NULL;
         }
      }
   }
   if(listaCaminho) {delete listaCaminho; listaCaminho = NULL;}

   return chaveMontante;
}
//---------------------------------------------------------------------------
VTChave* __fastcall TFuncoesDeRede::GetChaveMontante(VTBloco* blocoRef)
{
   TList* lisBarras;
   VTBarra* barraRef;
   VTChave* chaveMontante = NULL;

	if(!blocoRef)
   	return NULL;

   if(!blocoRef->LisBarra())
      return NULL;

   lisBarras = blocoRef->LisBarra();
   if(lisBarras == NULL || lisBarras->Count == 0)
   	return NULL;

   barraRef = (VTBarra*) lisBarras->Items[0];
   if(barraRef == NULL)
   	return NULL;

	chaveMontante = GetChaveMontante(barraRef);

   return chaveMontante;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::OrdCaminhosMaiorMenor(TList* lisCaminhos)
{
	bool ok;
   int NLigaAnt;
   TList* caminho;

	if(lisCaminhos == NULL) return;

	ok = false;
   while(!ok)
   {
   	ok = true;

   	for(int i=0; i<lisCaminhos->Count; i++)
      {
      	caminho = (TList*) lisCaminhos->Items[i];

         if(i > 0 && caminho->Count > NLigaAnt)
         {
            lisCaminhos->Delete(i);
            lisCaminhos->Insert(i-1, caminho);
            ok = false;
         }
         else
         {
				NLigaAnt = caminho->Count;
         }
      }
	}
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::OrdenaClusteres(TList* lisClusteres)
{
   bool ok;
   TCluster *cluster, *clusterAnt;
   TList *ligaCluster, *ligaClusterAnt;

	if(lisClusteres == NULL || lisClusteres->Count <= 1) return;

   ligaCluster = new TList();
   ligaClusterAnt = new TList();

	ok = false;
   while(!ok)
   {
      ok = true;

      for(int i=1; i<lisClusteres->Count && ok; i++)
      {
       	cluster = (TCluster*) lisClusteres->Items[i];
         clusterAnt = (TCluster*) lisClusteres->Items[i-1];

         ligaCluster->Clear();
         ligaClusterAnt->Clear();

         GetLigacoesCluster(cluster, ligaCluster);
         GetLigacoesCluster(clusterAnt, ligaClusterAnt);

         if(ligaCluster->Count > ligaClusterAnt->Count)
         {
            lisClusteres->Delete(i);
            lisClusteres->Insert(i-1, cluster);
            ok = false;
         }
      }
   }

   // Destroi listas
   delete ligaCluster; ligaCluster = NULL;
   delete ligaClusterAnt; ligaClusterAnt = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetLigacoesCluster(TCluster* cluster, TList* lisExt)
{
	if(cluster == NULL || lisExt == NULL)
   	return;

   lisExt->Clear();

   for(int i=0; i<cluster->GetBlocos()->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) cluster->GetBlocos()->Items[i];
		TList* lisAux = bloco->LisLigacao();
      for(int j=0; j<lisAux->Count; j++)
      {
			VTLigacao* liga = (VTLigacao*) lisAux->Items[j];
         lisExt->Add(liga);
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::RessetaParametros()
{
   redeRef = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::SetRedeMT(VTRede* redeRef)
{
	this->redeRef = redeRef;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TFuncoesDeRede::GetRede(String codEqpto)
{
   VTRede* rede = NULL;
   TList* lisEQP = new TList();

   // Procura a rede em quest�o
   rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];

      if(rede->TipoRede->Segmento != redePRI)
      	continue;

   	lisEQP->Clear();
      rede->LisEqpto(lisEQP);
      for(int j=0; j<lisEQP->Count; j++)
      {
         VTEqpto* eqpto = (VTEqpto*) lisEQP->Items[j];
         if(eqpto->Codigo == codEqpto)
         {
         	delete lisEQP;
            return rede;
         }
      }
   }

   // Destroi lista
   delete lisEQP; lisEQP = NULL;

   return NULL;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TFuncoesDeRede::GetRede_CodRede(String CodRede)
{
   VTRede* rede;

   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];

      if(rede->TipoRede->Segmento != redePRI)
      	continue;

		String codigo = ReplaceStr(rede->Codigo, "-", "");
		codigo = ReplaceStr(rede->Codigo, " ", "");
		if(codigo == CodRede)
      	return rede;
   }

   return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetListBlocosRadiais(TList* lisBlocos, TList* lisExt)
{
	if(lisBlocos == NULL || lisExt == NULL)
   	return;

   // Cria objeto de ordena��o
   if(this->ordena == NULL)
   {
      this->ordena = DLL_NewObjOrdena(this->apl);
      // Ordena a rede investigada
      this->ordena->Executa(this->redes);
   }

   // Gera lista de blocos radiais
   for(int i=0; i<lisBlocos->Count; i++)
   {
   	VTBloco* bloco = (VTBloco*) lisBlocos->Items[i];
//      if(bloco->LisLigacao()->Count == 0 || bloco->LisBarra()->Count == 1)
//         continue;
    	TBloco_Radial* blRd = new TBloco_Radial(bloco);
      lisExt->Add(blRd);
   }

   // Lista de Chaves entre blocos
   TList* lisChavesEntreBlocos = new TList();
	GetLisChavesEntreBlocos(redeRef, lisBlocos, lisChavesEntreBlocos);

   TList* lisElos = blocos->LisElo();
   for(int i=0; i<lisChavesEntreBlocos->Count; i++)
   {
		VTChave* Chave = (VTChave*) lisChavesEntreBlocos->Items[i];

      if(Chave != NULL && Chave->Codigo == "Y05849B")
      {
         int c = 0;
      }

      VTElo* elo = NULL;
		for(int j=0; j<lisElos->Count; j++)
      {
			elo = (VTElo*) lisElos->Items[j];
         if(elo->Chave == Chave)
         	break;
         else
         	elo = NULL;
      }
      if(elo == NULL)
      	continue;

      VTBloco* bl1 = elo->Bloco1;
		VTBloco* bl2 = elo->Bloco2;
      VTLigacao* ligaPai = Chave->ligaPai;

      if(elo->Chave->Codigo == "A45176")
      {
         int a = 0;
      }
      if(elo->Chave->Codigo == "A45768")
      {
         int a = 0;
      }
      if(elo->Chave->Codigo == "A45157")
      {
         int a = 0;
      }

      if(GetBloco_Radial(lisExt, bl1) != NULL && GetBloco_Radial(lisExt, bl2) != NULL)
      {
      	if(bl1->ExisteLigacao(ligaPai))
         {
            TBloco_Radial* blRd_Mont = GetBloco_Radial(lisExt, bl1);
         	TBloco_Radial*	blRd_Jus = GetBloco_Radial(lisExt, bl2);
				blRd_Mont->lisBlocosRadJus->Add(blRd_Jus);
            blRd_Jus->BlocoRadMont = blRd_Mont;
            blRd_Jus->chvMont = Chave;
            int c= 0 ;
         }
      	else if(bl2->ExisteLigacao(ligaPai))
         {
            TBloco_Radial* blRd_Mont = GetBloco_Radial(lisExt, bl2);
         	TBloco_Radial*	blRd_Jus = GetBloco_Radial(lisExt, bl1);
				blRd_Mont->lisBlocosRadJus->Add(blRd_Jus);
            blRd_Jus->BlocoRadMont = blRd_Mont;
            blRd_Jus->chvMont = Chave;
                int c= 0 ;
         }
      }
      else if(GetBloco_Radial(lisExt, bl1) == NULL && GetBloco_Radial(lisExt, bl2) != NULL)
      {
      	if(bl1->ExisteLigacao(ligaPai))
         {
         	TBloco_Radial*	blRd_Jus = GetBloco_Radial(lisExt, bl2);
            blRd_Jus->BlocoRadMont = NULL;
            blRd_Jus->chvMont = Chave;
            int c= 0 ;
         }
      }
   }

   // Detroi lista
   delete lisChavesEntreBlocos; lisChavesEntreBlocos = NULL;
}
//---------------------------------------------------------------------------
TBloco_Radial* __fastcall TFuncoesDeRede::GetBloco_Radial(TList* lisBloRad, VTBloco* bloco)
{
   TBloco_Radial* blRd;

	if(!lisBloRad || !bloco)
   	return NULL;


   for(int i=0; i<lisBloRad->Count; i++)
   {
		blRd = (TBloco_Radial*) lisBloRad->Items[i];
      if(blRd->bloco == bloco)
      	return blRd;
   }

   return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFuncoesDeRede::GetLisChavesEntreBlocos(VTRede* rede, TList* lisBlocos, TList* lisExt)
{
	if(lisBlocos == NULL || lisExt == NULL)
   	return;

   TList* lisElos = blocos->LisElo();
   for(int j=0; j<lisElos->Count; j++)
   {
      VTElo* elo = (VTElo*) lisElos->Items[j];
      if(lisBlocos->IndexOf(elo->Bloco1) >= 0 || lisBlocos->IndexOf(elo->Bloco2) >= 0 && elo->Chave->Fechada)
      {
      	VTChave* chave = GetChaveStr(rede, elo->Chave->Codigo);
         if(chave == NULL) continue;

			if(lisExt->IndexOf(chave) < 0)
         	lisExt->Add(chave);
      }
   }
}
//---------------------------------------------------------------------------
VTChave* __fastcall TFuncoesDeRede::GetChaveStr(VTRede* rede, String CodChave)
{
	if(rede == NULL || CodChave == "")
   	return NULL;

   TList* lisChaves = new TList();
   rede->LisChave(lisChaves, chvFECHADA);
	for(int i=0; i<lisChaves->Count; i++)
   {
		VTChave* chave = (VTChave*) lisChaves->Items[i];
		if(chave->Codigo == CodChave)
      {
      	delete lisChaves;
      	return chave;
      }
   }

	delete lisChaves; lisChaves = NULL;
   return NULL;
}
//---------------------------------------------------------------------------
VTCarga* __fastcall TFuncoesDeRede::GetCargaStr(VTRede* rede, String CodCarga)
{
	if(rede == NULL || CodCarga == "") return NULL;

   TList* lisCargas = new TList();
   rede->LisEqpto(lisCargas, eqptoCARGA);
	for(int i=0; i<lisCargas->Count; i++)
   {
		VTCarga* carga = (VTCarga*) lisCargas->Items[i];
		if(carga->Codigo == CodCarga)
      {
      	delete lisCargas; lisCargas = NULL;
      	return carga;
      }
   }
	delete lisCargas; lisCargas = NULL;
   return NULL;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TBifurcacao::TBifurcacao(VTLigacao* ligaPai)
{
   this->ligaPai = ligaPai;

   lisCaminhos = new TList();
   lisLigaIni = new TList();
}
//---------------------------------------------------------------------------
__fastcall TBifurcacao::~TBifurcacao()
{
   this->ligaPai = ligaPai;

   if(lisCaminhos) {delete lisCaminhos; lisCaminhos = NULL;}
	if(lisLigaIni) {delete lisLigaIni; lisLigaIni = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TBifurcacao::SetCaminho(TList* lisCaminho)
{
   lisCaminhos->Add(lisCaminho);
}
//---------------------------------------------------------------------------
void __fastcall TBifurcacao::SetCaminhos(TList* lisCaminhos)
{
   this->lisCaminhos = lisCaminhos;
}
//---------------------------------------------------------------------------
void __fastcall TBifurcacao::SetLigaIni(VTLigacao* ligaIni)
{
   lisLigaIni->Add(ligaIni);
}
//---------------------------------------------------------------------------
TList* __fastcall TBifurcacao::GetLigaIni()
{
	return lisLigaIni;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TBloco_Radial::TBloco_Radial(VTBloco* bloco)
{
	this->bloco = bloco;
   this->chvMont = NULL;

   lisBlocosRadJus = new TList();
}
//---------------------------------------------------------------------------
__fastcall TBloco_Radial::~TBloco_Radial()
{
	if(lisBlocosRadJus) {delete lisBlocosRadJus; lisBlocosRadJus = NULL;}
}
//---------------------------------------------------------------------------
//eof

