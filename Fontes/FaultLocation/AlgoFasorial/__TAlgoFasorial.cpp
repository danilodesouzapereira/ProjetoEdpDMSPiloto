//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlgoFasorial.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TConfiguracoes.h"
#include "..\Auxiliares\TDados.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Auxiliares\TLog.h"
#include "..\DSS\TTryFault.h"
#include "..\Equipamentos\TEqptoCampo.h"
#include "..\Equipamentos\TQualimetro.h"
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
	funcoesRede = new TFuncoesDeRede(apl);
	lisBarrasCandidatas = new TList();
   logFL = NULL;

	tryFault = new TTryFault();   //< objeto para testes de defeitos utilizando o OpenDSS
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
__fastcall TAlgoFasorial::~TAlgoFasorial()
{
	if(lisBarrasCandidatas) {delete lisBarrasCandidatas; lisBarrasCandidatas = NULL;}
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
void __fastcall TAlgoFasorial::CalculaVI_Sequencias012(StrFasor* Vse, StrFasor* Ise)
{
	std::complex<double> Va, Vb, Vc, Ia, Ib, Ic;
   std::complex<double> a = std::complex<double>(cos(2.*M_PI/3.), sin(2.*M_PI/3.));

   // Obtém os fasores das tensões e das correntes, por fase
   Va = Vse->faseA;
   Vb = Vse->faseB;
   Vc = Vse->faseC;

   Ia = Ise->faseA;
   Ib = Ise->faseB;
   Ic = Ise->faseC;

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
// Cálculo da impedância total vista do início do alimentador, para defeitos FT
void __fastcall TAlgoFasorial::CalculaZtotal_FT(StrFasor* Vse, StrFasor* Ise)
{
   // Verificações
   if(I0 == std::complex<double>(0., 0.))
   {
   	Ztotal = std::complex<double>(0., 0.);
   	return;
   }

   // Cálculo da impedância total vista do início do alimentador
	Ztotal = (V0 + V1 + V2) / I0;
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
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::CarregaMedicoesVI(TFormFaultLocation* formFL)
{
   VTPath* path = (VTPath*) apl->GetObject(__classid(VTPath));

   String pathMedicoesVI = path->DirDat() + "\\FaultLocation\\FLDistancia\\MedicoesVI.txt";

   TStringList* linhasMedicoesVI = new TStringList();
   linhasMedicoesVI->LoadFromFile(pathMedicoesVI);

   formFL->edtModVa->Text = linhasMedicoesVI->Strings[0];
   formFL->edtFaseVa->Text = linhasMedicoesVI->Strings[1];
   formFL->edtModVb->Text = linhasMedicoesVI->Strings[2];
   formFL->edtFaseVb->Text = linhasMedicoesVI->Strings[3];
   formFL->edtModVc->Text = linhasMedicoesVI->Strings[4];
   formFL->edtFaseVc->Text = linhasMedicoesVI->Strings[5];

   formFL->edtModIa->Text = linhasMedicoesVI->Strings[6];
   formFL->edtFaseIa->Text = linhasMedicoesVI->Strings[7];
   formFL->edtModIb->Text = linhasMedicoesVI->Strings[8];
   formFL->edtFaseIb->Text = linhasMedicoesVI->Strings[9];
   formFL->edtModIc->Text = linhasMedicoesVI->Strings[10];
   formFL->edtFaseIc->Text = linhasMedicoesVI->Strings[11];

   delete linhasMedicoesVI; linhasMedicoesVI = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::ExecutaFL_2FT(String codRede, double Rtotal, double Xtotal)
{
	if(codRede == "" || Xtotal == 0.)
   	return;

   BarrasCandidatas_2FT(codRede, Xtotal, lisBarrasCandidatas);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::ExecutaFL_FT(String codRede, double Rtotal, double Xtotal)
{
	if(codRede == "" || Xtotal == 0.)
   	return;

   BarrasCandidatas_FT(codRede, Xtotal, lisBarrasCandidatas);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoFasorial::ExecutaFL_3F(String codRede, double Rtotal, double Xtotal)
{
	if(codRede == "" || Xtotal == 0.)
   	return;

   BarrasCandidatas_3F(codRede, Xtotal, lisBarrasCandidatas);
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

//   lisaux->SaveToFile("c:\\users\\usrsnp\\desktop\\barrasFLDist.txt");
	delete lisaux;
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
   tryFault->SetMedicoesBarras(dados->GetMedicoesBarras_AlgoFasorial());
   tryFault->SetMedicoesTrechos(dados->GetMedicoesTrechos_AlgoFasorial());
}
//---------------------------------------------------------------------------
// Se houver 2 ou mais qualímetros de rede instalados no alimentador, executa
// o refinamento das soluções, considerando os afundamentos medidos e os calculados.
bool __fastcall TAlgoFasorial::RefinarSolucoes()
{
	bool resp;
   int NQualimetrosRede;

   NQualimetrosRede = 0;
   for(int i=0; i<dados->GetEqptosCampo()->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) dados->GetEqptosCampo()->Items[i];
      if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

      TQualimetro* qualimetro = (TQualimetro*) eqptoCampo;
      if(qualimetro->SE) continue;

      NQualimetrosRede += 1;
   }

   if(NQualimetrosRede > 1)
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
   }
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
