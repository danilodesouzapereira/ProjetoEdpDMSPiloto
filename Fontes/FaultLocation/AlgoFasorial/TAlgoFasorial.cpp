//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlgoFasorial.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TConfiguracoes.h"
#include "..\Auxiliares\TDados.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Auxiliares\TLog.h"
#include "..\DSS\TTryFault.h"
#include "..\Equipamentos\TChaveMonitorada.h"
#include "..\Equipamentos\TEqptoCampo.h"
#include "..\Equipamentos\TQualimetro.h"
#include "..\Equipamentos\TITrafo.h"
#include "..\TAreaBusca.h"
#include "..\TClassificacao.h"
#include "..\TFormFaultLocation.h"
#include "..\TThreadFaultLocation.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Grafico.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Grafico\VTGrafico.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
double Truncar(double valor, int numDecimais)
{
	double fator = 1.;

   for(int i=0; i<numDecimais; i++) fator *= 10.;
   valor = (int)valor + (int)((valor - (int)valor)*fator)/fator;

   return valor;
}
//---------------------------------------------------------------------------
__fastcall TAlgoFasorial::TAlgoFasorial(VTApl* apl)
{
	this->apl = apl;
	graf = (VTGrafico*) apl->GetObject(__classid(VTGrafico));
   redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
   redeMT = NULL;
	redeSE = NULL;
	chaveRef = NULL;
	funcoesRede = new TFuncoesDeRede(apl);
	lisBarrasCandidatas = new TList();
   logFL = NULL;

	tryFault = new TTryFault();   //< objeto para testes de defeitos utilizando o OpenDSS
   preFalta = new TTryFault();  //< objeto para rodar fluxo de potência pré-falta
}
//---------------------------------------------------------------------------
__fastcall TAlgoFasorial::~TAlgoFasorial()
{
	if(lisBarrasCandidatas)
   {
      for(int i=lisBarrasCandidatas->Count-1; i>=0; i--) delete(lisBarrasCandidatas->Items[i]);
      delete lisBarrasCandidatas; lisBarrasCandidatas = NULL;
   }
   if(tryFault) {delete tryFault; tryFault = NULL;}
   if(preFalta) {delete preFalta; preFalta = NULL;}
   if(funcoesRede) {delete funcoesRede; funcoesRede = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::InicializaConfiguracoes(String CaminhoDirFaultLocation)
{
   TIniFile* file = NULL;

   // Obtém o máximo desvio porcentual admissível, em relação às medições de tensão
   String pathConfigGerais = CaminhoDirFaultLocation + "\\ConfigGerais.ini";

   try
   {
      file = new TIniFile(pathConfigGerais);
      Tolerancia = file->ReadFloat("ALGO_FASORIAL", "ToleranciaPorc", 1.0);
      AgruparSolucoes = file->ReadBool("ALGO_FASORIAL", "AgruparSolucoes", 1);
	   file->Free();
   }
   catch(Exception &e)
   {
   	Tolerancia = 1.0;
      AgruparSolucoes = true;
   }
}
//---------------------------------------------------------------------------
// Tenta remover soluções muito próximas (com mesma chave à montante)
void __fastcall TAlgoFasorial::AgrupaSolucoes(TList* lisEXT)
{
	if(!lisEXT || lisEXT->Count == 1)
   	return;

   // Transfere objetos da lista de origem (lisEXT) para uma lista auxiliar
	TList* lisAux = new TList();
	CopiaTList(lisEXT, lisAux);
   lisEXT->Clear();

   // À medida que encontra soluções com a mesma chave à montante, salva em lisTMP
   TList* lisTMP = new TList();
	for(int i=0; i<lisAux->Count; i++)
   {
     	strSolucao* solucao = (strSolucao*) lisAux->Items[i];

   	if(lisTMP->Count == 0)
      {
         lisTMP->Add(solucao);
         continue;
      }

		// Quando a chave à montante muda, salva em lisEXT apenas uma solução
 		strSolucao* solucaoTMP = (strSolucao*) lisTMP->Items[0];
     	if(solucao->ChvMont == solucaoTMP->ChvMont)
      {
			lisTMP->Add(solucao);
      }
      else
      {
        	lisEXT->Add(solucaoTMP);
         lisTMP->Clear();
      }
   }

   if(lisTMP->Count > 0)
   {
   	strSolucao* solucaoTMP = (strSolucao*) lisTMP->Items[0];
      lisEXT->Add(solucaoTMP);
      lisTMP->Clear();
   }

   // Limpa e destroi listas auxiliares
	for(int i=0; i<lisAux->Count; i++)
   {
     	strSolucao* solucao = (strSolucao*) lisAux->Items[i];
      if(lisEXT->IndexOf(solucao) < 0)
      	delete solucao;
   }
   delete lisAux; lisAux = NULL;
	delete lisTMP; lisTMP = NULL;
}

//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2FT(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !qualimetroEqptoRef || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_QualimetroEqptoRef(redeMT, qualimetroEqptoRef);

	funcoesRede->GetBarras_CaminhoReatancia2FT(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2FT(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !chaveMonit_referencia || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_ChaveMonitorada(redeMT, chaveMonit_referencia);

	funcoesRede->GetBarras_CaminhoReatancia2FT(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2FT(String codRede, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;

	if(codRede == "" || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	redeSE = GetRedeSE(redeMT);

	funcoesRede->GetBarras_CaminhoReatancia2FT(redeSE, redeMT, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2F(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !qualimetroEqptoRef || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_QualimetroEqptoRef(redeMT, qualimetroEqptoRef);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2F(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !chaveMonit_referencia || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_ChaveMonitorada(redeMT, chaveMonit_referencia);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_2F(String codRede, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;

	if(codRede == "" || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	redeSE = GetRedeSE(redeMT);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeSE, redeMT, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_FT(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !qualimetroEqptoRef || Xtotal == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_QualimetroEqptoRef(redeMT, qualimetroEqptoRef);

	funcoesRede->GetBarras_CaminhoReatanciaFT(redeMT, barraReferencia, Xtotal, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_FT(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !chaveMonit_referencia || Xtotal == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_ChaveMonitorada(redeMT, chaveMonit_referencia);

	funcoesRede->GetBarras_CaminhoReatanciaFT(redeMT, barraReferencia, Xtotal, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_FT(String codRede, double Xtotal, TList* lisEXT)
{
	TList* lisEqptoDestaque;

	if(codRede == "" || Xtotal == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	redeSE = GetRedeSE(redeMT);

	funcoesRede->GetBarras_CaminhoReatanciaFT(redeSE, redeMT, Xtotal, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_3F(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !qualimetroEqptoRef || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_QualimetroEqptoRef(redeMT, qualimetroEqptoRef);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_3F(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;
	VTBarra* barraReferencia;

	if(codRede == "" || !chaveMonit_referencia || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	barraReferencia = BarraReferencia_ChaveMonitorada(redeMT, chaveMonit_referencia);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeMT, barraReferencia, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::BarrasCandidatas_3F(String codRede, double Xtotal_1, TList* lisEXT)
{
	TList* lisEqptoDestaque;

	if(codRede == "" || Xtotal_1 == 0. || !lisEXT)
		return;

	// Define a rede MT em questão
	redeMT = GetRedeMT(codRede);
	redeSE = GetRedeSE(redeMT);

	funcoesRede->GetBarras_CaminhoReatancia2F(redeSE, redeMT, Xtotal_1, Tolerancia, lisEXT);
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TAlgoFasorial::BarraReferencia_QualimetroEqptoRef(VTRede* redeMT, TQualimetro* qualimetroEqptoRef)
{
	if(!redeMT || !qualimetroEqptoRef)
		return(NULL);

	VTBarra* barraRef;
	VTLigacao* ligacao;

	// Se for qualímetro de trafo MT/BT, não pode ser eqpto de referência
	if(!qualimetroEqptoRef->GetLigacaoAssociada())
		return(NULL);

	ligacao = qualimetroEqptoRef->GetLigacaoAssociada();
	barraRef = ligacao->Barra(0);

	return(barraRef);
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TAlgoFasorial::BarraReferencia_ChaveMonitorada(VTRede* redeMT, TChaveMonitorada* chaveMonit_referencia)
{
	if(!redeMT || !chaveMonit_referencia)
		return(NULL);

	VTBarra* barraRef;
	VTChave* chaveRef;

	chaveRef = chaveMonit_referencia->GetChaveAssociada();
	if(chaveRef->TipoDisjuntor)
		barraRef = redeMT->BarraInicial();
	else
		barraRef = chaveRef->Barra(0);

	return(barraRef);
}
//---------------------------------------------------------------------------
double __fastcall TAlgoFasorial::CalculaRf_Solucao(strBarraAlgoFasorial* barraAlgoFasorial)
{
   double Rf_solucao;

	if(!barraAlgoFasorial)
   	return -1.;

   // Impedâncias de seq. 0 e 1 desde o início do alimentador até o ponto do defeito
   std::complex<double> Z0 = std::complex<double>(barraAlgoFasorial->Rtotal_0, barraAlgoFasorial->Xtotal_0);
   std::complex<double> Z1 = std::complex<double>(barraAlgoFasorial->Rtotal_1, barraAlgoFasorial->Xtotal_1);

   // Computa Rf para a solução, com base em Z0 e Z1 até o ponto e com base nas medições (V0, V1, I0, I1)
	Rf_solucao = std::abs((V0 - V1 + Z1*I1 - Z0*I0) / (3.*I0));

   return Rf_solucao;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::CalculaZtotal(double mVa, double mVb, double mVc,
												  double pVa, double pVb, double pVc,
                             			  double mIa, double mIb, double mIc,
                                      double pIa, double pIb, double pIc)
{
	std::complex<double> V0, V1, V2, I0, I1, I2;
   std::complex<double> Va, Vb, Vc, Ia, Ib, Ic;

   std::complex<double> a = std::complex<double>(cos(2.*M_PI/3.), sin(2.*M_PI/3.));

   Va = std::complex<double>(mVa * cos(pVa * M_PI/180.), mVa * sin(pVa * M_PI/180.));
   Vb = std::complex<double>(mVb * cos(pVb * M_PI/180.), mVb * sin(pVb * M_PI/180.));
   Vc = std::complex<double>(mVc * cos(pVc * M_PI/180.), mVc * sin(pVc * M_PI/180.));
   Ia = std::complex<double>(mIa * cos(pIa * M_PI/180.), mIa * sin(pIa * M_PI/180.));
   Ib = std::complex<double>(mIb * cos(pIb * M_PI/180.), mIb * sin(pIb * M_PI/180.));
   Ic = std::complex<double>(mIc * cos(pIc * M_PI/180.), mIc * sin(pIc * M_PI/180.));

	V0 = (1/3.) * (Va + Vb + Vc);
   V1 = (1/3.) * (Va + a*Vb + a*a*Vc);
   V2 = (1/3.) * (Va + a*a*Vb + a*Vc);

	I0 = (1/3.) * (Ia + Ib + Ic);
   I1 = (1/3.) * (Ia + a*Ib + a*a*Ic);
   I2 = (1/3.) * (Ia + a*a*Ib + a*Ic);

	Ztotal = (V0 + V1 + V2) / I0;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::CalculaZtotal_3F(double mVa, double mVb, double mVc,
												  double pVa, double pVb, double pVc,
                             			  double mIa, double mIb, double mIc,
                                      double pIa, double pIb, double pIc)
{
	std::complex<double> V0, V1, V2, I0, I1, I2;
   std::complex<double> Va, Vb, Vc, Ia, Ib, Ic;

   std::complex<double> a = std::complex<double>(cos(2.*M_PI/3.), sin(2.*M_PI/3.));

   Va = std::complex<double>(mVa * cos(pVa * M_PI/180.), mVa * sin(pVa * M_PI/180.));
   Vb = std::complex<double>(mVb * cos(pVb * M_PI/180.), mVb * sin(pVb * M_PI/180.));
   Vc = std::complex<double>(mVc * cos(pVc * M_PI/180.), mVc * sin(pVc * M_PI/180.));
   Ia = std::complex<double>(mIa * cos(pIa * M_PI/180.), mIa * sin(pIa * M_PI/180.));
   Ib = std::complex<double>(mIb * cos(pIb * M_PI/180.), mIb * sin(pIb * M_PI/180.));
   Ic = std::complex<double>(mIc * cos(pIc * M_PI/180.), mIc * sin(pIc * M_PI/180.));

	V0 = (1/3.) * (Va + Vb + Vc);
   V1 = (1/3.) * (Va + a*Vb + a*a*Vc);
   V2 = (1/3.) * (Va + a*a*Vb + a*Vc);

	I0 = (1/3.) * (Ia + Ib + Ic);
   I1 = (1/3.) * (Ia + a*Ib + a*a*Ic);
   I2 = (1/3.) * (Ia + a*a*Ib + a*Ic);

	Ztotal_3F = V1 / I1;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::CalculaVI_Sequencias012(StrFasor* fasorVref, StrFasor* fasorIref)
{
	std::complex<double> Va, Vb, Vc, Ia, Ib, Ic;
   std::complex<double> a = std::complex<double>(cos(2.*M_PI/3.), sin(2.*M_PI/3.));

   // Obtém os fasores das tensões e das correntes, por fase
   Va = fasorVref->faseA;
   Vb = fasorVref->faseB;
   Vc = fasorVref->faseC;

   Ia = fasorIref->faseA;
   Ib = fasorIref->faseB;
   Ic = fasorIref->faseC;

   // Faz adequações para efeito de cálculo
   strTipoFalta = classificacao->GetStrTipoFalta();
   if(strTipoFalta == "B")
   {
   	std::complex<double> Iaux, Vaux;
      Iaux = Ia;
      Vaux = Va;
      Ia = Ib;
      Ib = Iaux;
      Va = Vb;
      Vb = Vaux;
   }
   else if(strTipoFalta == "C")
   {
   	std::complex<double> Iaux, Vaux;
      Iaux = Ia;
      Vaux = Va;
      Ia = Ic;
      Ic = Iaux;
      Va = Vc;
      Vc = Vaux;
   }
   else if(strTipoFalta == "ABN" || strTipoFalta == "AB")
   {
   	std::complex<double> Iaux, Vaux;
      Iaux = Ia;
      Vaux = Va;
      Ia = Ic;
      Ic = Iaux;
      Va = Vc;
      Vc = Vaux;
   }
   else if(strTipoFalta == "ACN" || strTipoFalta == "CAN" || strTipoFalta == "AC" || strTipoFalta == "CA")
   {
   	std::complex<double> Iaux, Vaux;
      Iaux = Ia;
      Vaux = Va;
      Ia = Ib;
      Ib = Iaux;
      Va = Vb;
      Vb = Vaux;
   }

   // Calculamos os fasores de comp. sim.
	V0 = (1/3.) * (Va + Vb + Vc);
   V1 = (1/3.) * (Va + a*Vb + a*a*Vc);
   V2 = (1/3.) * (Va + a*a*Vb + a*Vc);

	I0 = (1/3.) * (Ia + Ib + Ic);
   I1 = (1/3.) * (Ia + a*Ib + a*a*Ic);
   I2 = (1/3.) * (Ia + a*a*Ib + a*Ic);
}
//---------------------------------------------------------------------------
// Cálculo da impedância total vista de um qualímetro de referência, para defeitos FT
void __fastcall TAlgoFasorial::CalculaZtotal_FT(StrFasor* Vse, StrFasor* Ise)
{
   // Verificações
   if(I0 == std::complex<double>(0., 0.))
   {
   	Ztotal = std::complex<double>(0., 0.);
   	return;
   }

   std::complex<double> I012 = I0;

   I012 = AjustaCorrenteI012();

   // Cálculo da impedância total vista do início do alimentador
	Ztotal = (V0 + V1 + V2) / I012;
}
//---------------------------------------------------------------------------
std::complex<double> __fastcall TAlgoFasorial::AjustaCorrenteI012()
{
   std::complex<double> I012_ajustado = I0;
   std::complex<double> I0_pre, I1_pre, I2_pre;
   std::complex<double> I0_corr, I1_corr, I2_corr;
   std::complex<double> a = std::complex<double>(cos(2.*M_PI/3.), sin(2.*M_PI/3.));

   // Executa fluxo de potência pré-falta e pega as correntes Ia, Ib, Ic
   // no ponto do qualímetro, supondo eqv. Thev. em 1pu, 0 -120 120 graus
   std::complex<double> Ia_pre = std::complex<double>(0., 0.);
   std::complex<double> Ib_pre = std::complex<double>(0., 0.);
   std::complex<double> Ic_pre = std::complex<double>(0., 0.);
   preFalta->TestaPreFalta(1, Ia_pre, Ib_pre, Ic_pre);

   // Faz ajustes nas correntes Ia, Ib e Ic, para que a falta seja A-T.

   // Calcula I0_pre, I1_pre, I2_pre pré-falta
	I0_pre = (1/3.) * (Ia_pre + Ib_pre + Ic_pre);
   I1_pre = (1/3.) * (Ia_pre + a*Ib_pre + a*a*Ic_pre);
   I2_pre = (1/3.) * (Ia_pre + a*a*Ib_pre + a*Ic_pre);

   // Ajusta os ângulos de I0_pre, I1_pre, I2_pre, com base nos ângulos de I0, I1, I2
   double absI0_pre = std::abs(I0_pre);
   double absI1_pre = std::abs(I1_pre);
   double absI2_pre = std::abs(I2_pre);

   // Pega os ângulos de I0, I1, I2
   double argI0 = std::arg(I0);
   double argI1 = std::arg(I1);
   double argI2 = std::arg(I2);
   double argI012 = (argI0 + argI1 + argI2) / 3.;

//   // Refaz os fasores I0_pre, I1_pre, I2_pre
//   I0_pre = std::complex<double>(absI0_pre * cos(argI0), absI0_pre * sin(argI0));
//   I1_pre = std::complex<double>(absI1_pre * cos(argI1), absI1_pre * sin(argI1));
//   I2_pre = std::complex<double>(absI2_pre * cos(argI2), absI2_pre * sin(argI2));

   // Corrige a corrente I012 a ser utilizada:
   I0_corr = I0 - I0_pre;
   I1_corr = I1 - I1_pre;
   I2_corr = I2 - I2_pre;

   // Calcula um valor médio
//   I012_ajustado = (I0_corr + I1_corr + I2_corr) / 3.;
   I012_ajustado = I0_corr;

   return(I012_ajustado);
}
//---------------------------------------------------------------------------
// Cálculo da impedância total vista do início do alimentador, para defeitos 2FT
void __fastcall TAlgoFasorial::CalculaZtotal_seq1_2FT(StrFasor* Vse, StrFasor* Ise)
{
   // Verificações
   if(I0 == std::complex<double>(0., 0.))
   {
   	Ztotal_1 = std::complex<double>(0., 0.);
   	return;
   }

   // Cálculo da impedância total vista do início do alimentador, através das sequências 1 e 2
	Ztotal_1 = (V1 - V2) / (I1 - I2);
}
//---------------------------------------------------------------------------
// Cálculo da impedância total vista do início do alimentador, para defeitos 2F
void __fastcall TAlgoFasorial::CalculaZtotal_seq1_2F(StrFasor* Vse, StrFasor* Ise)
{
   // Cálculo da impedância total vista do início do alimentador, através das sequências 1 e 2
	Ztotal_1 = (V1 - V2) / (I1 - I2);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::CalculaZtotal_seq1_3F(StrFasor* Vse, StrFasor* Ise)
{
   // Cálculo da impedância total vista do início do alimentador, através das sequências 1 e 2
	Ztotal_1 = (V1) / (I1);
}
////---------------------------------------------------------------------------
//void __fastcall TAlgoFasorial::CarregaMedicoesVI(TFormFaultLocation* formFL)
//{
//   VTPath* path = (VTPath*) apl->GetObject(__classid(VTPath));
//
//   String pathMedicoesVI = path->DirDat() + "\\FaultLocation\\FLDistancia\\MedicoesVI.txt";
//
//   TStringList* linhasMedicoesVI = new TStringList();
//   linhasMedicoesVI->LoadFromFile(pathMedicoesVI);
//
//   formFL->edtModVa->Text = linhasMedicoesVI->Strings[0];
//   formFL->edtFaseVa->Text = linhasMedicoesVI->Strings[1];
//   formFL->edtModVb->Text = linhasMedicoesVI->Strings[2];
//   formFL->edtFaseVb->Text = linhasMedicoesVI->Strings[3];
//   formFL->edtModVc->Text = linhasMedicoesVI->Strings[4];
//	formFL->edtFaseVc->Text = linhasMedicoesVI->Strings[5];
//
//   formFL->edtModIa->Text = linhasMedicoesVI->Strings[6];
//   formFL->edtFaseIa->Text = linhasMedicoesVI->Strings[7];
//   formFL->edtModIb->Text = linhasMedicoesVI->Strings[8];
//   formFL->edtFaseIb->Text = linhasMedicoesVI->Strings[9];
//   formFL->edtModIc->Text = linhasMedicoesVI->Strings[10];
//   formFL->edtFaseIc->Text = linhasMedicoesVI->Strings[11];
//
//	delete linhasMedicoesVI; linhasMedicoesVI = NULL;
//}
////---------------------------------------------------------------------------
//void __fastcall TAlgoFasorial::ExecutaFL_2FT(String codRede, double Rtotal, double Xtotal)
//{
//	if(codRede == "" || Xtotal == 0.)
//   	return;
//
//   BarrasCandidatas_2FT(codRede, Xtotal, lisBarrasCandidatas);
//}
////---------------------------------------------------------------------------
//void __fastcall TAlgoFasorial::ExecutaFL_FT(String codRede, double Rtotal, double Xtotal)
//{
//	if(codRede == "" || Xtotal == 0.)
//		return;
//
//	BarrasCandidatas_FT(codRede, Xtotal, lisBarrasCandidatas);
//}
////---------------------------------------------------------------------------
//void __fastcall TAlgoFasorial::ExecutaFL_3F(String codRede, double Rtotal, double Xtotal)
//{
//	if(codRede == "" || Xtotal == 0.)
//		return;
//
//	BarrasCandidatas_3F(codRede, Xtotal, lisBarrasCandidatas);
//}

//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_FT(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal, double Xtotal, bool FLOffline)
{
	if(codRede == "" || !qualimetroEqptoRef || Rtotal == 0. || Xtotal == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_FT(codRede, qualimetroEqptoRef, Xtotal, lisBarrasCandidatas);

//	//debug - exporta a lista de barras candidatas
//	TStringList* listaaux = new TStringList();
//	for(int i=0; i<lisBarrasCandidatas->Count; i++)
//	{
//		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//		if(barraAlgoFasorial == NULL || barraAlgoFasorial->barra == NULL)
//			continue;
//		listaaux->Add(barraAlgoFasorial->barra->Codigo);
//	}
//	listaaux->SaveToFile("c:\\users\\usrsnp\\desktop\\barras.txt");
//	delete listaaux;


   if(FLOffline)
		return;

   // Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
	TestaCurtoCircuitos_FT(Rtotal);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
	}

//   //debug
//   TStringList* lisaux = new TStringList();
//   for(int i=0; i<lisBarrasCandidatas->Count; i++)
//   {
//      strSolucao* barraAlgoFasorial = (strSolucao*) lisBarrasCandidatas->Items[i];
//      String final = String(barraAlgoFasorial->barra->Id) + " " + String(barraAlgoFasorial->indiceErro);
//      lisaux->Add(final);
//   }
//   lisaux->SaveToFile("c:\\users\\user\\desktop\\barrasIndiceErro.txt");
//   delete lisaux; lisaux = NULL;

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
	String tensoesCalculadas, linha;
   logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
      linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_FT(String codRede, double Rtotal, double Xtotal, bool FLOffline)
{
	if(codRede == "" || Rtotal == 0. || Xtotal == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_FT(codRede, Xtotal, lisBarrasCandidatas);

//	//debug - exporta a lista de barras candidatas
//	TStringList* listaaux = new TStringList();
//	for(int i=0; i<lisBarrasCandidatas->Count; i++)
//	{
//		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//		if(barraAlgoFasorial == NULL || barraAlgoFasorial->barra == NULL)
//			continue;
//		listaaux->Add(barraAlgoFasorial->barra->Codigo);
//	}
//	listaaux->SaveToFile("c:\\users\\usrsnp\\desktop\\barras.txt");
//	delete listaaux;


   if(FLOffline)
		return;

   // Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
	TestaCurtoCircuitos_FT(Rtotal);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
	}

//   //debug
//   TStringList* lisaux = new TStringList();
//   for(int i=0; i<lisBarrasCandidatas->Count; i++)
//   {
//      strSolucao* barraAlgoFasorial = (strSolucao*) lisBarrasCandidatas->Items[i];
//      String final = String(barraAlgoFasorial->barra->Id) + " " + String(barraAlgoFasorial->indiceErro);
//      lisaux->Add(final);
//   }
//   lisaux->SaveToFile("c:\\users\\user\\desktop\\barrasIndiceErro.txt");
//   delete lisaux; lisaux = NULL;

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
	String tensoesCalculadas, linha;
   logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
      linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2FT(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || !qualimetroEqptoRef || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2FT(codRede, qualimetroEqptoRef, Xtotal_1, lisBarrasCandidatas);

	// Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
   TestaCurtoCircuitos_2FT(strTipoFalta);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
   }

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

   //debug: adiciona ao log os valores calculados de tensão
   String tensoesCalculadas, linha;
   logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2FT(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2FT(codRede, Xtotal_1, lisBarrasCandidatas);

	// Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
   TestaCurtoCircuitos_2FT(strTipoFalta);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
   }

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

   //debug: adiciona ao log os valores calculados de tensão
   String tensoesCalculadas, linha;
   logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2F(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || !qualimetroEqptoRef || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

   strTipoFalta = classificacao->GetStrTipoFalta();

   // Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2F(codRede, qualimetroEqptoRef, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_2F(strTipoFalta);

//   if(RefinarSolucoes())
//   {
//	   FiltraSolucoes_por_afundamento_coincidente();
//	}

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
	String tensoesCalculadas, linha;
	logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2F(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

   strTipoFalta = classificacao->GetStrTipoFalta();

   // Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2F(codRede, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_2F(strTipoFalta);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
	}

   // Com dados de sensores e outros eqptos, elimina possibilidades
   FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
	String tensoesCalculadas, linha;
	logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_3F(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || !qualimetroEqptoRef || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

	strTipoFalta = "ABC";

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_3F(codRede, qualimetroEqptoRef, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 3 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_3F(strTipoFalta);

   if(RefinarSolucoes())
   {
		FiltraSolucoes_por_afundamento_coincidente();
   }

	// Com dados de sensores e outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
   String tensoesCalculadas, linha;
	logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_3F(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline)
{
	if(codRede == "" || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

	strTipoFalta = "ABC";

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_3F(codRede, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 3 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   if(FLOffline)
		return;

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_3F(strTipoFalta);

   if(RefinarSolucoes())
   {
		FiltraSolucoes_por_afundamento_coincidente();
   }

	// Com dados de sensores e outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();

	//debug: adiciona ao log os valores calculados de tensão
   String tensoesCalculadas, linha;
	logFL->AddLinha("Tensões Calculadas:", true);
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		linha = "Barra " + String(barraAlgoFasorial->barra->Id);
		linha += ";" + barraAlgoFasorial->vetor_calcV;
		logFL->AddLinha(linha, false);
	}
	logFL->AddLinha("", true);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::ExecutaFiltroRf(double Rtotal)
{
	double Rf;

   TStringList* lisaux = new TStringList();
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
      Rf = (1/3.) * (Rtotal - barraAlgoFasorial->Rtotal);
      barraAlgoFasorial->Rf = Rf;

      lisaux->Add(String(barraAlgoFasorial->barra->Id) + "   " + String(Rf));
   }

//   lisaux->SaveToFile("c:\\users\\user\\desktop\\barrasFLDist.txt");
   delete lisaux;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_FT_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal, double Xtotal)
{
	if(codRede == "" || !chaveMonit_referencia || Rtotal == 0. || Xtotal == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_FT(codRede, chaveMonit_referencia, Xtotal, lisBarrasCandidatas);

	// Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
	TestaCurtoCircuitos_FT(Rtotal);

   if(RefinarSolucoes())
   {
	   FiltraSolucoes_por_afundamento_coincidente();
	}

	// Com dados de sensores e outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2F_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1)
{
	if(codRede == "" || !chaveMonit_referencia || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

	strTipoFalta = classificacao->GetStrTipoFalta();

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2F(codRede, chaveMonit_referencia, Xtotal_1, lisBarrasCandidatas);

	// Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_2F(strTipoFalta);

   if(RefinarSolucoes())
	{
		FiltraSolucoes_por_afundamento_coincidente();
	}

	// Com dados de outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_2FT_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1)
{
	if(codRede == "" || !chaveMonit_referencia || Rtotal_1 == 0. || Xtotal_1 == 0.)
   	return;

   strTipoFalta = classificacao->GetStrTipoFalta();

   // Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_2FT(codRede, chaveMonit_referencia, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 2 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

	// Para cada solução, calcula Rf e testa efetivamente o defeito com Rf
   TestaCurtoCircuitos_2FT(strTipoFalta);

   if(RefinarSolucoes())
	{
	   FiltraSolucoes_por_afundamento_coincidente();
   }

	// Com dados de outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::Executa_3F_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1)
{
	if(codRede == "" || !chaveMonit_referencia || Rtotal_1 == 0. || Xtotal_1 == 0.)
		return;

   strTipoFalta = "ABC";

	// Define conjunto de barras candidatas, identificadas com base na reatância total dos trechos
	BarrasCandidatas_3F(codRede, chaveMonit_referencia, Xtotal_1, lisBarrasCandidatas);

   // Remove possível solução se o local não comportar defeito envolvendo 3 fases
   FiltraBarrasPorAnaliseFases(lisBarrasCandidatas, strTipoFalta);

   // Para cada solução, testa efetivamente o defeito
   TestaCurtoCircuitos_3F(strTipoFalta);

   if(RefinarSolucoes())
   {
		FiltraSolucoes_por_afundamento_coincidente();
   }

	// Com dados de sensores e outros eqptos, elimina possibilidades
	FiltraBarrasCurtoCircuito();
}
//---------------------------------------------------------------------------
// Verifica se o trecho analisado suporta a falta desejada.
//    - Se falta bifásica ==> trecho não pode ser 1F
//    - Se falta trifásica ==> trecho não pode ser 1F nem 2F
void __fastcall TAlgoFasorial::FiltraBarrasPorAnaliseFases(TList* lisBarrasCandidatas, String strTipoFalta)
{
	if(!redeMT)
		return;

   if(strTipoFalta == "AB" || strTipoFalta == "BC" || strTipoFalta == "CA" ||
      strTipoFalta == "ABN" || strTipoFalta == "BCN" || strTipoFalta == "ACN")
   {
      for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
         VTBarra* barra = barraAlgoFasorial->barra;
         String fasesBarra = funcoesRede->GetFases(redeMT, barra);
         if(fasesBarra == "A" || fasesBarra == "B" || fasesBarra == "C")
         {
            lisBarrasCandidatas->Remove(barraAlgoFasorial);
         }
      }
   }
   else if(strTipoFalta == "ABC")
   {
      for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
      {
         strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
         VTBarra* barra = barraAlgoFasorial->barra;
         String fasesBarra = funcoesRede->GetFases(redeMT, barra);
         if(fasesBarra == "A" || fasesBarra == "B" || fasesBarra == "C" ||
            strTipoFalta == "AB" || strTipoFalta == "BC" || strTipoFalta == "CA")
         {
            lisBarrasCandidatas->Remove(barraAlgoFasorial);
         }
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::FiltraSolucoes_por_afundamento_coincidente()
{
	if(!lisBarrasCandidatas || lisBarrasCandidatas->Count == 0) return;
   for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
   {
      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		if(!barraAlgoFasorial->maxAfundamentoCoincidente)
      {
       	lisBarrasCandidatas->Remove(barraAlgoFasorial);
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::FiltraBarrasCurtoCircuito()
{
	double menorIndiceErro, maiorIndiceErro;
	TList* lisBarras = NULL;


   // Cruza com informações da área de busca
   lisBarras = new TList();
	areaBusca->GetAreaBusca_Barras(lisBarras);

   for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
   {
      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		if(lisBarras->IndexOf(barraAlgoFasorial->barra) < 0)
      {
       	lisBarrasCandidatas->Remove(barraAlgoFasorial);
         delete barraAlgoFasorial;
      }
   }
   delete lisBarras; lisBarras = NULL;

   if(lisBarrasCandidatas->Count == 1)
   	return;

//   // Verifica se índice de erro tem valor elevado
//   menorIndiceErro = ((strBarraAlgoFasorial*) lisBarrasCandidatas->Items[0])->indiceErro;
//   maiorIndiceErro = ((strBarraAlgoFasorial*) lisBarrasCandidatas->Items[0])->indiceErro;
//   for(int i=1; i<lisBarrasCandidatas->Count; i++)
//   {
//      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//		if(barraAlgoFasorial->indiceErro < menorIndiceErro)
//      	menorIndiceErro = barraAlgoFasorial->indiceErro;
//		if(barraAlgoFasorial->indiceErro > maiorIndiceErro)
//      	maiorIndiceErro = barraAlgoFasorial->indiceErro;
//   }
//
//   if(menorIndiceErro < 1. && maiorIndiceErro > 1.)
//   {
//      for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
//      {
//         strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//         if(barraAlgoFasorial->indiceErro > 1.)
//         {
//            lisBarrasCandidatas->Remove(barraAlgoFasorial);
//            delete barraAlgoFasorial;
//         }
//      }
//   }
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::OrdenaBarrasCurtoCircuito()
{
	bool ok = false;

	if(lisBarrasCandidatas->Count == 1)
   	return;

   while(!ok)
   {
      ok = true;
      for(int i=1; i<lisBarrasCandidatas->Count; i++)
      {
         strBarraAlgoFasorial* barraAlgoFasorial_ant = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i-1];
         strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
         if(barraAlgoFasorial->indiceErro < barraAlgoFasorial_ant->indiceErro)
         {
            lisBarrasCandidatas->Remove(barraAlgoFasorial);
            lisBarrasCandidatas->Insert(i-1, barraAlgoFasorial);
            ok = false;
         }
      }
   }
}
//---------------------------------------------------------------------------
VTRede* __fastcall TAlgoFasorial::GetRedeMT(String codRede)
{
   VTRede* rede;

	if(codRede == "")
		return NULL;

   // Procura a rede em questão
   rede = NULL;
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];

      if(rede->TipoRede->Segmento != redePRI)
      	continue;

		if(!rede->Carregada)
      	continue;

		if(rede->Codigo.AnsiCompare(codRede) == 0)
			break;
      else
      	rede = NULL;
   }

	return rede;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TAlgoFasorial::GetRedeSE(VTRede* redeMT)
{
	VTRede* redeSE;

	if(!redeMT)
   	return NULL;

   for(int i=0; i<redes->LisRede()->Count; i++)
   {
		redeSE = (VTRede*) redes->LisRede()->Items[i];

      if(redeSE->TipoRede->Segmento != redeETD)
      	continue;

		if(!redeSE->Carregada)
      	continue;

		for(int j=0; j<redeSE->LisLigacao()->Count; j++)
      {
         VTLigacao* liga = (VTLigacao*) redeSE->LisLigacao()->Items[j];

         if(liga->Barra(0) == redeMT->BarraInicial() || liga->Barra(1) == redeMT->BarraInicial())
				return redeSE;
      }
   }

   return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::GetSolucoes(TFuncoesDeRede* funcoesRede, TList* lisEXT)
{
	double   distanciaChvMontante;
	int      utmx, utmy;
	int      numConsumidoresJusanteLigacao;
	VTBarra* barraSolucao;
	VTChave* chaveMontante;

	if(!lisEXT || !funcoesRede) return;

   lisEXT->Clear();

   // Ordena as soluções da mais para a menos provável (em função do índice de erro das soluções)
   OrdenaBarrasCurtoCircuito();

   // Para cada alternativa de solução, gera uma struct de solução
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
     	strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];

		barraSolucao = barraAlgoFasorial->barra;
  		chaveMontante = funcoesRede->GetChaveMontante(barraSolucao);
      distanciaChvMontante = funcoesRede->GetDistanciaMetros(chaveMontante->Barra(1), barraSolucao);
      numConsumidoresJusanteLigacao = funcoesRede->GetNumConsJusLigacao(chaveMontante->Codigo);
		barraSolucao->CoordenadasUtm_cm(utmx, utmy);

      // Ajusta coordenadas "fake" do Sinap
		double lat = double(utmy * 1e-7) - 100.;
      double lon = double(utmx * 1e-7) - 100.;

      strSolucao* solucao = new strSolucao();
      solucao->Probabilidade = i+1;
		solucao->DefX = String(utmx);
      solucao->DefY = String(utmy);
      solucao->DefLat = String(lat);
      solucao->DefLon = String(lon);
      solucao->barraSolucao = barraSolucao;
      solucao->CodBarra = barraSolucao->Codigo;
      solucao->IdBarra = barraSolucao->Id;
      solucao->DefTipo = strTipoFalta;
      solucao->ChvMont = chaveMontante->Codigo;
      solucao->DistChvMont = String(Round(distanciaChvMontante, 2));
      solucao->DistRele = funcoesRede->GetDistancia_KM_DaSubestacao(barraSolucao);
      solucao->ClientesDepoisChvMont = String(numConsumidoresJusanteLigacao);
      solucao->IndiceErro = barraAlgoFasorial->indiceErro;
      solucao->Rfalta_estimado = barraAlgoFasorial->Rf;

      lisEXT->Add(solucao);
   }

	if(AgruparSolucoes)
	{
      // Analisa chave montante para agrupar soluções
      AgrupaSolucoes(lisEXT);
   }
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::GetZTotal(double &reZtotal, double &imZtotal)
{
	reZtotal = Truncar(Ztotal.real(),3);
   imZtotal = Truncar(Ztotal.imag(),3);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::GetZTotal_1(double &reZtotal_1, double &imZtotal_1)
{
	reZtotal_1 = Truncar(Ztotal_1.real(),3);
   imZtotal_1 = Truncar(Ztotal_1.imag(),3);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::GetZTotal_3F(double &reZtotal, double &imZtotal)
{
	reZtotal = Truncar(Ztotal_3F.real(),3);
   imZtotal = Truncar(Ztotal_3F.imag(),3);
}
//---------------------------------------------------------------------------
//  #1 - Inicialização do objeto de testes de faltas (TTryFault)
void __fastcall TAlgoFasorial::IniciaTryFault(String caminhoDSS)
{
	// Prepara o TryFault
	tryFault->SetCaminhoDSS(caminhoDSS);
   tryFault->SetNivelCurtoMT();
	tryFault->IniciaBarrasTrechos();

   TStringList* lisMedicoesBarras = new TStringList();
   TStringList* lisMedicoesTrechos = new TStringList();

   dados->GetMedicoesBarras_AlgoFasorial(lisMedicoesBarras);
	tryFault->SetMedicoesBarras(lisMedicoesBarras);

   dados->GetMedicoesTrechos_AlgoFasorial(lisMedicoesTrechos);
	tryFault->SetMedicoesTrechos(lisMedicoesTrechos);

   for(int i=lisMedicoesBarras->Count-1; i>=0; i--) lisMedicoesBarras->Delete(i);
   delete lisMedicoesBarras; lisMedicoesBarras = NULL;
   for(int i=lisMedicoesTrechos->Count-1; i>=0; i--) lisMedicoesTrechos->Delete(i);
   delete lisMedicoesTrechos; lisMedicoesTrechos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::IniciaPreFalta(String caminhoDSS)
{
	// Prepara o preFalta
	preFalta->SetCaminhoDSS(caminhoDSS);
   preFalta->SetNivelCurtoMT();
	preFalta->IniciaBarrasTrechos();

   TStringList* lisMedicoesBarras = new TStringList();
   TStringList* lisMedicoesTrechos = new TStringList();

//   TQualimetro* qualimetroRef = dados->GetFasoresVI_QualimetroEqptoRef();
//   if(!qualimetroRef) return;

   // Pega o trecho imediatamente à jusante do qualímetro de referência
//   VTTrecho* trechoRef = qualimetroRef->trechoJusante;

//   // Pega a ligação (chave) associada ao qualímetro de referência
//   VTLigacao* ligacaoAssociada = qualimetroRef->ligacaoAssociada;

//   // Insere o ID da barra do qualímetro de referência
//   String strIDBarraQualimetroRef = String(ligacaoAssociada->Barra(0)->Id);
//	preFalta->SetIDBarraMonitorada(strIDBarraQualimetroRef);

//   String codLigacaoQualimetroRef = ligacaoAssociada->Codigo;
   // Insere o código do trecho à jusante do qualímetro de referência
//   preFalta->SetCodLigacaoRefMonitorada(codLigacaoQualimetroRef);
//   preFalta->SetCodLigacaoRefMonitorada(trechoRef->Codigo);

   for(int i=lisMedicoesBarras->Count-1; i>=0; i--) lisMedicoesBarras->Delete(i);
   delete lisMedicoesBarras; lisMedicoesBarras = NULL;
   for(int i=lisMedicoesTrechos->Count-1; i>=0; i--) lisMedicoesTrechos->Delete(i);
   delete lisMedicoesTrechos; lisMedicoesTrechos = NULL;
}
//---------------------------------------------------------------------------
// Se houver 2 ou mais qualímetros de rede instalados no alimentador, executa
// o refinamento das soluções, considerando os afundamentos medidos e os calculados.
bool __fastcall TAlgoFasorial::RefinarSolucoes()
{
	bool resp;
   int NQualimetrosRede, NTrafosInteligentes;

   // Contabiliza os qualímetros de rede
   NQualimetrosRede = 0;
   for(int i=0; i<dados->GetEqptosCampo()->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) dados->GetEqptosCampo()->Items[i];
      if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

      TQualimetro* qualimetro = (TQualimetro*) eqptoCampo;
      if(qualimetro->ligacaoAssociada)
      {
         if(qualimetro->ligacaoAssociada->Barra(0) == redeMT->BarraInicial() ||
            qualimetro->ligacaoAssociada->Barra(1) == redeMT->BarraInicial())
            continue;
      }
      NQualimetrosRede += 1;
   }

   // Contabiliza os trafos inteligentes
   NTrafosInteligentes = 0;
   for(int i=0; i<dados->GetEqptosCampo()->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) dados->GetEqptosCampo()->Items[i];
      if(eqptoCampo->GetTipo() != eqptoITRAFO) continue;

      TITrafo* trafoInteligente = (TITrafo*) eqptoCampo;
      if(trafoInteligente->cargaAssociada)
      {
         NTrafosInteligentes += 1;
      }
   }

   // Verifica quantidade de qualímetros de rede + trafos inteligentes
   if(NQualimetrosRede + NTrafosInteligentes > 1)
   	resp = true;
   else
   	resp = false;

   return(resp);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::SetParametros(TConfiguracoes* config,
                                       TClassificacao* classificacao,
                                       TAreaBusca* areaBusca,
                                       TDados* dados,
                                       TFormFaultLocation* formFL)
{
   this->classificacao = classificacao;
   this->config = config;
   this->areaBusca = areaBusca;
   this->dados = dados;
   this->formFL = formFL;
   tryFault->SetConfig(config);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::SetLogFL(TLog* logFL)
{
	this->logFL = logFL;
}
//---------------------------------------------------------------------------
bool __fastcall TAlgoFasorial::TemRepeticaoDeMinimo(double* medias, int NumeroMedicoes, double desvio)
{
	bool repete = false;
   double minimo = 0.;
   int indice_minimo = -1;

   for(int i=0; i<NumeroMedicoes; i++)
   {
		if(indice_minimo == -1 || medias[i] < minimo)
      {
         minimo = medias[i];
         indice_minimo = i;
      }
   }
   for(int i=0; i<NumeroMedicoes; i++)
   {
		if(i == indice_minimo) continue;
		if(fabs(medias[i] - minimo) < desvio)
         repete = true;
   }
//	for(int i=0; i<NumeroMedicoes; i++)
//   {
//      for(int j=0; j<NumeroMedicoes; j++)
//      {
//			if(j == i) continue;
//
//         if(fabs(medias[i] - medias[j]) < desvio)
//         {
//            repete = true;
//            break;
//         }
//      }
//   }
   return(repete);
}
//---------------------------------------------------------------------------
// Esse método testa o curto-circuito 3F, em cada barra candidata
void __fastcall TAlgoFasorial::TestaCurtoCircuitos_3F(String strTipoFalta)
{
	bool Inicial;
	double Rf, indiceErro;
	int patamar = 1;
   int IDbarra1;
   String faseDefeito, fasesFalta;

   // Entre as fases afetadas, consideramos Rf=0
   Rf = 0.0001;

//   //debug
//   int numCoincidentes = 0;
//   int numNaoCoincidentes = 0;

	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];

		barraAlgoFasorial->Rf = Rf;

		// Usa obj da classe TTryFault para testar a falta representada pelo indivíduo
      IDbarra1 = barraAlgoFasorial->barra->Id;
      fasesFalta = classificacao->GetStrTipoFalta();
      Inicial = true;
      tryFault->TestaCurtoRf(patamar, IDbarra1, fasesFalta, Rf, Inicial);

      // Calcula erro associado ao teste
      indiceErro = tryFault->CalculaErro_AlgoFasorial();

      // Seta o índice de erro na estrutura de barra FLDist
      barraAlgoFasorial->indiceErro = indiceErro;

      // Verifica se o máx. afund. gerado pela solução coincide com o máx. afund. medido
      TStringList* lisAux_calcV = new TStringList();
      TStringList* lisAux_medV = new TStringList();
      tryFault->GetLisCalcV(lisAux_calcV);
      tryFault->GetLisMedV(lisAux_medV);
      bool AfundamentoCoincidente = VerificaAfundCoincidente(strTipoFalta, lisAux_medV, lisAux_calcV);
      barraAlgoFasorial->vetor_calcV = "";
      for(int j=0; j<lisAux_calcV->Count; j++)
      {
	      barraAlgoFasorial->vetor_calcV += lisAux_calcV->Strings[j] + ";";
      }
      barraAlgoFasorial->maxAfundamentoCoincidente = AfundamentoCoincidente;

      delete lisAux_calcV; lisAux_calcV = NULL;
      delete lisAux_medV; lisAux_medV = NULL;
   }
}
//---------------------------------------------------------------------------
// Esse método testa o curto-circuito 2F, em cada barra candidata
void __fastcall TAlgoFasorial::TestaCurtoCircuitos_2F(String strTipoFalta)
{
	bool Inicial;
	double Rf, indiceErro;
	int patamar = 1;
   int IDbarra1;
   String faseDefeito, fasesFalta;

   TStringList* lisAux_calcV = new TStringList();
   TStringList* lisAux_medV = new TStringList();

   // Entre as fases afetadas, consideramos Rf=0
   Rf = 0.0001;

   for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];

      barraAlgoFasorial->Rf = Rf;

      // Usa obj da classe TTryFault para testar a falta representada pelo indivíduo
      IDbarra1 = barraAlgoFasorial->barra->Id;
      fasesFalta = classificacao->GetStrTipoFalta();
      Inicial = true;
      tryFault->TestaCurtoRf(patamar, IDbarra1, fasesFalta, Rf, Inicial);

      // Calcula erro associado ao teste
      indiceErro = tryFault->CalculaErro_AlgoFasorial();

      // Seta o índice de erro na estrutura de barra FLDist
      barraAlgoFasorial->indiceErro = indiceErro;

      // Verifica se o máx. afund. gerado pela solução coincide com o máx. afund. medido
      lisAux_calcV->Clear();
      lisAux_medV->Clear();
      tryFault->GetLisCalcV(lisAux_calcV);
      tryFault->GetLisMedV(lisAux_medV);
      bool AfundamentoCoincidente = VerificaAfundCoincidente(strTipoFalta, lisAux_medV, lisAux_calcV);
      barraAlgoFasorial->vetor_calcV = "";
      for(int j=0; j<lisAux_calcV->Count; j++)
      {
	      barraAlgoFasorial->vetor_calcV += lisAux_calcV->Strings[j] + ";";
      }
      barraAlgoFasorial->maxAfundamentoCoincidente = AfundamentoCoincidente;
   }

   delete lisAux_calcV; lisAux_calcV = NULL;
   delete lisAux_medV; lisAux_medV = NULL;
}
//---------------------------------------------------------------------------
// Esse método testa o curto-circuito 2FT, em cada barra candidata
void __fastcall TAlgoFasorial::TestaCurtoCircuitos_2FT(String strTipoFalta)
{
	bool Inicial;
	double Rf, indiceErro;
	int patamar = 1;
   int IDbarra1;
   String fasesFalta;

   for(int i=0; i<lisBarrasCandidatas->Count; i++)
   {
      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];

     	Rf = CalculaRf_Solucao(barraAlgoFasorial);
      barraAlgoFasorial->Rf = Rf;
      IDbarra1 = barraAlgoFasorial->barra->Id;
      fasesFalta = classificacao->GetStrTipoFalta();
      Inicial = true;
      tryFault->TestaCurtoRf(patamar, IDbarra1, fasesFalta, Rf, Inicial);

      // Calcula erro associado ao teste
      indiceErro = tryFault->CalculaErro_AlgoFasorial();

      // Seta o índice de erro na estrutura de barra FLDist
      barraAlgoFasorial->indiceErro = indiceErro;

      // Verifica se o máx. afund. gerado pela solução coincide com o máx. afund. medido
      TStringList* lisAux_calcV = new TStringList();
      TStringList* lisAux_medV = new TStringList();
      tryFault->GetLisCalcV(lisAux_calcV);
      tryFault->GetLisMedV(lisAux_medV);
      bool AfundamentoCoincidente = VerificaAfundCoincidente(strTipoFalta, lisAux_medV, lisAux_calcV);
      barraAlgoFasorial->vetor_calcV = "";
      for(int j=0; j<lisAux_calcV->Count; j++)
      {
	      barraAlgoFasorial->vetor_calcV += lisAux_calcV->Strings[j] + ";";
      }
      barraAlgoFasorial->maxAfundamentoCoincidente = AfundamentoCoincidente;

      delete lisAux_calcV; lisAux_calcV = NULL;
      delete lisAux_medV; lisAux_medV = NULL;
   }
}
//---------------------------------------------------------------------------
// Esse método testa o curto-circuito FT, com resistência de falta Rf, em cada
// barra candidata
void __fastcall TAlgoFasorial::TestaCurtoCircuitos_FT(double Rtotal)
{
	bool Inicial;
	double Rf, indiceErro;
	int patamar = 1;
   int IDbarra1;
   String faseDefeito;

//   //debug
//   for(int i=0; i<lisBarrasCandidatas->Count; i++)
//   {
//		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//      Rf = (1/3.) * (Rtotal - barraAlgoFasorial->Rtotal);
//      barraAlgoFasorial->Rf = Rf;
//   }
//
//   //debug
//   TStringList* lisaux1 = new TStringList();
//   for(int i=0; i<lisBarrasCandidatas->Count; i++)
//   {
//      strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
//      String final = String(barraAlgoFasorial->barra->Id) + " " + String(barraAlgoFasorial->Rf);
//      lisaux1->Add(final);
//   }
//   lisaux1->SaveToFile("c:\\users\\user\\desktop\\barrasRf.txt");
//   delete lisaux1; lisaux1 = NULL;

	int cont = 0;
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];

		Rf = (1/3.) * (Rtotal - barraAlgoFasorial->Rtotal);
		if(Rf < 0.)
		{
			barraAlgoFasorial->indiceErro = -1.;
			cont += 1;
			continue;
		}
		barraAlgoFasorial->Rf = Rf;

      // Usa obj da classe TTryFault para testar a falta representada pelo indivíduo
      IDbarra1 = barraAlgoFasorial->barra->Id;

      faseDefeito = "A";
      Inicial = true;
      tryFault->TestaCurtoRf(patamar, IDbarra1, faseDefeito, Rf, Inicial);

      // Calcula erro associado ao teste
      indiceErro = tryFault->CalculaErro_AlgoFasorial();

      // Seta o índice de erro na estrutura de barra FLDist
      barraAlgoFasorial->indiceErro = indiceErro;

      // Verifica se o máx. afund. gerado pela solução coincide com o máx. afund. medido
      TStringList* lisAux_calcV = new TStringList();
      TStringList* lisAux_medV = new TStringList();
      tryFault->GetLisCalcV(lisAux_calcV);
      tryFault->GetLisMedV(lisAux_medV);
      bool AfundamentoCoincidente = VerificaAfundCoincidente(strTipoFalta, lisAux_medV, lisAux_calcV);
      barraAlgoFasorial->vetor_calcV = "";
      for(int j=0; j<lisAux_calcV->Count; j++)
      {
	      barraAlgoFasorial->vetor_calcV += lisAux_calcV->Strings[j] + ";";
      }
      barraAlgoFasorial->maxAfundamentoCoincidente = AfundamentoCoincidente;

      delete lisAux_calcV; lisAux_calcV = NULL;
      delete lisAux_medV; lisAux_medV = NULL;
	}

	for(int i=lisBarrasCandidatas->Count-1; i>=0; i--)
	{
		strBarraAlgoFasorial* barraAlgoFasorial = (strBarraAlgoFasorial*) lisBarrasCandidatas->Items[i];
		if(barraAlgoFasorial->indiceErro == -1.)
			lisBarrasCandidatas->Remove(barraAlgoFasorial);
	}
}
////---------------------------------------------------------------------------
//bool __fastcall TAlgoFasorial::VerificaAfundCoincidente(String tipoFalta, TStringList* lisAux_medV, TStringList* lisAux_calcV)
//{
//   bool resp = false;
//	double media;
//   double min_MedMedia, min_CalcMedia;
//   int indice_MedMedia, indice_CalcMedia;
//
//	if(!lisAux_medV || !lisAux_calcV) return false;
//
//   // Inicializa valores
//   min_MedMedia = 0.;
//   min_CalcMedia = 0.;
//   indice_MedMedia = -1;
//   indice_CalcMedia = -1;
//
//   // Verifica o equipamento que registrou o maior afundamento, a partir da menor média de medições
//   for(int i=0; i<lisAux_medV->Count; i++)
//   {
//      String medicaoV = lisAux_medV->Strings[i];
//
//      media = GetValorMedio(tipoFalta, medicaoV);
//
//      if(indice_MedMedia == -1 || media < min_MedMedia)
//      {
//         indice_MedMedia = i;
//         min_MedMedia = media;
//      }
//   }
//   // Verifica o equipamento que registrou o maior afundamento, a partir da menor média de valores calculados
//   for(int i=0; i<lisAux_calcV->Count; i++)
//   {
//      String calculoV = lisAux_calcV->Strings[i];
//      media = GetValorMedio(tipoFalta, calculoV);
//
//      if(indice_CalcMedia == -1 || media < min_CalcMedia)
//      {
//         indice_CalcMedia = i;
//         min_CalcMedia = media;
//      }
//   }
//   // Verifica a coincidência de medição e cálculo do máx. afundamento
//   if(indice_MedMedia == indice_CalcMedia)
//      resp = true;
//
//   return (resp);
//}//---------------------------------------------------------------------------
bool __fastcall TAlgoFasorial::VerificaAfundCoincidente(String tipoFalta, TStringList* lisAux_medV, TStringList* lisAux_calcV)
{
   bool resp = false;
	double media, medias[10];
   double min_MedMedia, min_CalcMedia;
   int indice_MedMedia, indice_CalcMedia;
   int NumeroMedicoes;

	if(!lisAux_medV || !lisAux_calcV) return false;

   // Inicializa valores
   min_MedMedia = 0.;
   min_CalcMedia = 0.;
   indice_MedMedia = -1;
   indice_CalcMedia = -1;
   for(int i=0; i<10; i++) medias[i] = 0.;

   // Obtém os valores médios de afundamento, por equipamento de medição
   NumeroMedicoes = lisAux_medV->Count;
   for(int i=0; i<lisAux_medV->Count; i++)
   {
      String medicaoV = lisAux_medV->Strings[i];

      media = GetValorMedio(tipoFalta, medicaoV);
      medias[i] = media;

      if(indice_MedMedia == -1 || media < min_MedMedia)
      {
         indice_MedMedia = i;
         min_MedMedia = media;
      }
   }

   // Se houver medições com afundamentos muito próximos, assume que medições e
   // cálculos coincidem
	if(TemRepeticaoDeMinimo(medias, NumeroMedicoes, 0.1))
   	return true;

   // Verifica o equipamento que registrou o maior afundamento, a partir da menor média de valores calculados
   for(int i=0; i<lisAux_calcV->Count; i++)
   {
      String calculoV = lisAux_calcV->Strings[i];
      media = GetValorMedio(tipoFalta, calculoV);

      if(indice_CalcMedia == -1 || media < min_CalcMedia)
      {
         indice_CalcMedia = i;
         min_CalcMedia = media;
      }
   }
   // Verifica a coincidência de medição e cálculo do máx. afundamento
   if(indice_MedMedia == indice_CalcMedia)
      resp = true;

   return (resp);
}
//---------------------------------------------------------------------------
double  __fastcall TAlgoFasorial::GetValorMedio(String tipoFalta, String medicaoV)
{
	double valor1, valor2, valor3, media;

	if(tipoFalta == "" || medicaoV == "") return -1.;

   media = 0.;
	if(tipoFalta == "A")
   {
      valor1 = GetCampoCSV(medicaoV, 0, ";").ToDouble();
		media = valor1;
   }
   else if(tipoFalta == "B")
   {
      valor1 = GetCampoCSV(medicaoV, 1, ";").ToDouble();
		media = valor1;
   }
   else if(tipoFalta == "C")
   {
      valor1 = GetCampoCSV(medicaoV, 2, ";").ToDouble();
		media = valor1;
   }
	else if(tipoFalta == "AB" || tipoFalta == "ABN")
   {
      valor1 = GetCampoCSV(medicaoV, 0, ";").ToDouble();
      valor2 = GetCampoCSV(medicaoV, 1, ";").ToDouble();
		media = (valor1 + valor2) / 2.;
   }
	else if(tipoFalta == "BC" || tipoFalta == "BCN")
   {
      valor1 = GetCampoCSV(medicaoV, 1, ";").ToDouble();
      valor2 = GetCampoCSV(medicaoV, 2, ";").ToDouble();
		media = (valor1 + valor2) / 2.;
   }
	else if(tipoFalta == "CA" || tipoFalta == "ACN")
   {
      valor1 = GetCampoCSV(medicaoV, 0, ";").ToDouble();
      valor2 = GetCampoCSV(medicaoV, 2, ";").ToDouble();
		media = (valor1 + valor2) / 2.;
   }
   else if(tipoFalta == "ABC")
   {
      valor1 = GetCampoCSV(medicaoV, 0, ";").ToDouble();
      valor2 = GetCampoCSV(medicaoV, 1, ";").ToDouble();
      valor3 = GetCampoCSV(medicaoV, 2, ";").ToDouble();
		media = (valor1 + valor2 + valor3) / 3.;
   }
   return (media);
}
//---------------------------------------------------------------------------
