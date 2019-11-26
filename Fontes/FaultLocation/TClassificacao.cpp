//---------------------------------------------------------------------------
#pragma hdrstop
#include "TClassificacao.h"
#include "Auxiliares\TConfiguracoes.h"
#include "Auxiliares\TDados.h"
#include "Auxiliares\Enums.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TEqptoCampo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TBarraSemTensao.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TChaveMonitorada.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TFusivel.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TITrafo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TQualimetro.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TSensor.h>
//---------------------------------------------------------------------------
__fastcall TClassificacao::TClassificacao(TDados* dados, double* vetorParamComparacao)
{
	// Salva ponteiro para objeto de dados
	this->dados = dados;
   CorrentesOpostasFasesSas = false;

   // Salva par�metros de compara��o
	SetParamComparacao(vetorParamComparacao);

   // Inicializa par�metros de classifica��o
	TipoFalta = faltaINDEF;
	TipoRompCabo = rompINDEF;
   FonteClassificacao = fonteINDEF;
	NumFasesFalta = 0;
	configGerais = NULL;
   ToleranciaFasesSasOk = 30.;
}
//---------------------------------------------------------------------------
__fastcall TClassificacao::~TClassificacao(void)
{
   // Destroi objetos
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta com base nas correntes do sensor mais pr�ximo do defeito
 */
void __fastcall TClassificacao::ClassificaComSensor(TSensor* sensor, TChaveMonitorada* eqptoProtecao)
{
	// Define a fonte da classifica��o do defeito
   FonteClassificacao = fonteSENSOR;

	// Obt�m as medi��es de corrente do monitor mais pr�ximo
   GetCorrentes(sensor, I);

	//	Compara as correntes das fases e define o tipo de falta (TipoFalta)
   ComparaCorrentesFase(I, eqptoProtecao);
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta (FL Offline) com base nas correntes de um
 * rel� monitorando DISJUNTOR ou RELIGADORA de subesta��o ou RELIGADORA de
 * meio de rede.
 */
void __fastcall TClassificacao::ClassificaComEqptoProt_FLOffline(TChaveMonitorada* chvDJ_RE)
{
   double Ia, Ib, Ic, Imed;
	double I0, I1, relI0_I1, dIa, dIb, dIc;
   double max1, min1, min2;
	std::complex<double> a = std::complex<double>(1. * cos(120. * M_PI/180.), 1. * sin(120. * M_PI/180.));

	if(chvDJ_RE == NULL)
   	return;

	I[0] = chvDJ_RE->medicaoVI.falta.I[0];
	I[1] = chvDJ_RE->medicaoVI.falta.I[1];
	I[2] = chvDJ_RE->medicaoVI.falta.I[2];

   // M�dulos das correntes de fase
   Ia = abs(I[0]);
   Ib = abs(I[1]);
   Ic = abs(I[2]);

   // Determina corrente m�dia
 	Imed = (Ia + Ib + Ic) / 3.;

 	if(Imed == 0)
   	return;

   // Calcula as componentes sim�tricas da corrente de defeito
   I0 = abs((I[0] + I[1] + I[2]) / 3.);
   I1 = abs((I[0] + a*I[1] + a*a*I[2]) / 3.);

   // Determina a rela��o 100 * I0 / I1 (%)
   if(I1 == 0.)
   	relI0_I1 = 0.;
   else
		relI0_I1 = 100.* I0 / I1;

	// Determina os desvios das correntes de fase, em (%), em rela��o � Imed
   dIa = 100. * (Ia - Imed) / Imed;
   dIb = 100. * (Ib - Imed) / Imed;
   dIc = 100. * (Ic - Imed) / Imed;

   // Pega os par�metros de compara��o
	max1 = ParametrosComparacao.maxPorc_3f;
   min1 = ParametrosComparacao.minPorc_1f;
   min2 = ParametrosComparacao.minRelI0I1_2ft;

   // Verifica limites para defeito 3F
	if(abs(dIa) < max1 && abs(dIb) < max1 && abs(dIc) < max1)
   {
    	TipoFalta = falta3F;
		NumFasesFalta = 3;
      FonteClassificacao = fontePROTECAO;
   }

   // Verifica limites para defeito FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && abs(dIa) > min1 && dIb < 0. && dIc < 0.)
		{
			TipoFalta = faltaAG;
			FonteClassificacao = fontePROTECAO;
			NumFasesFalta = 1;
		}
		else if(dIb > 0. && abs(dIb) > min1 && dIa < 0. && dIc < 0.)
		{
			TipoFalta = faltaBG;
			FonteClassificacao = fontePROTECAO;
			NumFasesFalta = 1;
		}
		else if(dIc > 0. && abs(dIc) > min1 && dIa < 0. && dIb < 0.)
		{
			TipoFalta = faltaCG;
         FonteClassificacao = fontePROTECAO;
         NumFasesFalta = 1;
      }
   }

   // Verifica limites para defeito 2F
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && dIb > 0. && dIc < 0.)
      {
       	TipoFalta = faltaAB;
         FonteClassificacao = fontePROTECAO;
         NumFasesFalta = 2;
      }
      else if(dIb > 0. && dIc > 0. && dIa < 0.)
      {
       	TipoFalta = faltaBC;
         FonteClassificacao = fontePROTECAO;
         NumFasesFalta = 2;
      }
      else if(dIc > 0. && dIa > 0. && dIb < 0.)
      {
       	TipoFalta = faltaCA;
         FonteClassificacao = fontePROTECAO;
         NumFasesFalta = 2;
      }
   }
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar o rompimento de cabo com base nas medi��es de tens�o
 * do qual�metro mais pr�ximo do problema.
 */
void __fastcall TClassificacao::ClassificaRompCaboComQualimetro(TQualimetro* qualimetro)
{
	double Ia, Ib, Ic, Imed;
	double Va, Vb, Vc, Vmed, dVa, dVb, dVc;
	double I0, I1, relI0_I1, dIa, dIb, dIc;
	double max1, min1, min2;
	std::complex<double> a = std::complex<double>(1. * cos(120. * M_PI/180.), 1. * sin(120. * M_PI/180.));

	if(qualimetro == NULL)
		return;

	V_afund[0] = qualimetro->medicaoVI.falta.V[0];
	V_afund[1] = qualimetro->medicaoVI.falta.V[1];
	V_afund[2] = qualimetro->medicaoVI.falta.V[2];

	// M�dulos das tens�es de afundamento
	Va = abs(V_afund[0]);
	Vb = abs(V_afund[1]);
	Vc = abs(V_afund[2]);

	// Determina a tens�o m�dia
	Vmed = (Va + Vb + Vc) / 3.;

	if(Vmed == 0)
   	return;

	// Determina os desvios das tens�es de fase, em (%), em rela��o � Vmed
	dVa = 100. * (Va - Vmed) / Vmed;
	dVb = 100. * (Vb - Vmed) / Vmed;
	dVc = 100. * (Vc - Vmed) / Vmed;

	// Pega os par�metros de compara��o
	min1 = ParametrosComparacaoRompCabo.minPorc_1f;

//	// Verifica limites para rompimento de cabo envolvendo 1 fase
//	if(TipoRompCabo == rompINDEF)
//	{
//		if(dVa < 0. && abs(dVa) > abs(dVb) && abs(dVa) > abs(dVc))
//		{
//			if(dVa < 0. && abs(dVa) > min1 && dVb > 0. && dVc > 0.)
//			{
//				TipoFalta = rompA;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//
//			if(TipoRompCabo == rompINDEF && (dVa < 0. && abs(dVa) > min1 && abs(dVb) < min1 && abs(dVc) < min1))
//			{
//				TipoFalta = rompA;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//		}
//		else if(dVb < 0. && abs(dVb) > abs(dVa) && abs(dVb) > abs(dVc))
//		{
//			if(dVb < 0. && abs(dVb) > min1 && dVa > 0. && dVc > 0.)
//			{
//				TipoFalta = rompB;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//
//			if(TipoRompCabo == rompINDEF && (dVb < 0. && abs(dVb) > min1 && abs(dVa) < min1 && abs(dVc) < min1))
//			{
//				TipoFalta = rompB;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//		}
//		else if(dVc < 0. && abs(dVc) > abs(dVa) && abs(dVc) > abs(dVb))
//		{
//			if(dVc < 0. && abs(dVc) > min1 && dVa > 0. && dVb > 0.)
//			{
//				TipoFalta = rompC;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//
//			if(TipoRompCabo == rompINDEF && (dVc < 0. && abs(dVc) > min1 && abs(dVa) < min1 && abs(dVb) < min1))
//			{
//				TipoFalta = rompC;
//				FonteClassificacao = fonteQUALIMETRO;
//				NumFasesRompCabo = 1;
//			}
//		}
//	}

	if(TipoRompCabo == rompINDEF)
	{
		if(dVa < 0.)
		{
			if(dVb > 0. && dVc > 0.)
			{
				TipoRompCabo = rompA;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVb > 0. && dVc < 0. && abs(dVc) < min1)
			{
				TipoRompCabo = rompA;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVb > 0. && dVc < 0. && abs(dVc) > min1)
			{
				TipoRompCabo = rompCA;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
			else if(dVc > 0. && dVb < 0. && abs(dVb) < min1)
			{
				TipoRompCabo = rompA;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVc > 0. && dVb < 0. && abs(dVb) > min1)
			{
				TipoRompCabo = rompAB;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
		}

		else if(dVb < 0.)
		{
			if(dVa > 0. && dVc > 0.)
			{
				TipoRompCabo = rompB;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVa > 0. && dVc < 0. && abs(dVc) < min1)
			{
				TipoRompCabo = rompB;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVa > 0. && dVc < 0. && abs(dVc) > min1)
			{
				TipoRompCabo = rompBC;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
			else if(dVc > 0. && dVa < 0. && abs(dVa) < min1)
			{
				TipoRompCabo = rompB;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVc > 0. && dVa < 0. && abs(dVa) > min1)
			{
				TipoRompCabo = rompAB;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
		}

		else if(dVc < 0.)
		{
			if(dVa > 0. && dVb > 0.)
			{
				TipoRompCabo = rompC;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVa > 0. && dVb < 0. && abs(dVb) < min1)
			{
				TipoRompCabo = rompC;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVa > 0. && dVb < 0. && abs(dVb) > min1)
			{
				TipoRompCabo = rompBC;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
			else if(dVb > 0. && dVa < 0. && abs(dVa) < min1)
			{
				TipoRompCabo = rompC;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 1;
			}
			else if(dVb > 0. && dVa < 0. && abs(dVa) > min1)
			{
				TipoRompCabo = rompCA;
				FonteClassificacao = fonteQUALIMETRO;
				NumFasesRompCabo = 2;
			}
		}
	}
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta com base nos dados do qual�metro eqptoRef,
 * aquele que mediu corrente e est� mais pr�ximo do defeito
 */
void __fastcall TClassificacao::ClassificaComQualimetroEqptoRef(TQualimetro* qualimetroEqptoRef, TChaveMonitorada* eqptoProtecaoProx)
{
	double Ia, Ib, Ic, Imed;
	double I0, I1, relI0_I1, dIa, dIb, dIc;
	double max1, min1, min2;
	std::complex<double> a = std::complex<double>(1. * cos(120. * M_PI/180.), 1. * sin(120. * M_PI/180.));

	if(qualimetroEqptoRef == NULL)
		return;

	I[0] = qualimetroEqptoRef->medicaoVI.falta.I[0];
	I[1] = qualimetroEqptoRef->medicaoVI.falta.I[1];
	I[2] = qualimetroEqptoRef->medicaoVI.falta.I[2];

   // M�dulos das correntes de fase
   Ia = abs(I[0]);
   Ib = abs(I[1]);
   Ic = abs(I[2]);

   // Determina corrente m�dia
 	Imed = (Ia + Ib + Ic) / 3.;

 	if(Imed == 0)
   	return;

   // Calcula as componentes sim�tricas da corrente de defeito
   I0 = abs((I[0] + I[1] + I[2]) / 3.);
   I1 = abs((I[0] + a*I[1] + a*a*I[2]) / 3.);

   // Determina a rela��o 100 * I0 / I1 (%)
   if(I1 == 0.)
   	relI0_I1 = 0.;
   else
		relI0_I1 = 100.* I0 / I1;

	// Determina os desvios das correntes de fase, em (%), em rela��o � Imed
   dIa = 100. * (Ia - Imed) / Imed;
   dIb = 100. * (Ib - Imed) / Imed;
   dIc = 100. * (Ic - Imed) / Imed;

   // Pega os par�metros de compara��o
   max1 = ParametrosComparacao.maxPorc_3f;
   min1 = ParametrosComparacao.minPorc_1f;
   min2 = ParametrosComparacao.minRelI0I1_2ft;

   // Verifica limites para defeito 3F
	if(abs(dIa) < max1 && abs(dIb) < max1 && abs(dIc) < max1)
   {
    	TipoFalta = falta3F;
		NumFasesFalta = 3;
      FonteClassificacao = fonteQUALIMETRO;
   }

   // Verifica limites para defeito FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && abs(Ia) > min1 && dIb < 0. && dIc < 0.)
      {
       	TipoFalta = faltaAG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
      else if(dIb > 0. && abs(dIb) > min1 && dIa < 0. && dIc < 0.)
      {
       	TipoFalta = faltaBG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
      else if(dIc > 0. && abs(dIc) > min1 && dIa < 0. && dIb < 0.)
      {
       	TipoFalta = faltaCG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
   }

   // Verifica limites para defeito 2F ou 2FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && dIb > 0. && dIc < 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaABG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaAB;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
               TipoFalta = faltaABG;
            else
               TipoFalta = faltaAB;
            FonteClassificacao = fonteQUALIMETRO;
         }
         NumFasesFalta = 2;
      }
      else if(dIa > 0. && dIb < 0. && dIc > 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaCAG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaCA;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
               TipoFalta = faltaCAG;
            else
               TipoFalta = faltaCA;
            FonteClassificacao = fonteQUALIMETRO;
         }
         NumFasesFalta = 2;
      }
      else if(dIa < 0. && dIb > 0. && dIc > 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaBCG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaBC;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
					TipoFalta = faltaBCG;
				else
					TipoFalta = faltaBC;
				FonteClassificacao = fonteQUALIMETRO;
			}
			NumFasesFalta = 2;
		}
	}
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta com base nos dados do qual�metro instalado
 * no in�cio do alimentador
 */
void __fastcall TClassificacao::ClassificaComQualimetroSE(TQualimetro* qualimetroSE, TChaveMonitorada* eqptoProtecaoProx)
{
	double Ia, Ib, Ic, Imed;
	double I0, I1, relI0_I1, dIa, dIb, dIc;
	double max1, min1, min2;
	std::complex<double> a = std::complex<double>(1. * cos(120. * M_PI/180.), 1. * sin(120. * M_PI/180.));

	if(qualimetroSE == NULL)
		return;

	I[0] = qualimetroSE->medicaoVI.falta.I[0];
	I[1] = qualimetroSE->medicaoVI.falta.I[1];
	I[2] = qualimetroSE->medicaoVI.falta.I[2];

   // M�dulos das correntes de fase
   Ia = abs(I[0]);
   Ib = abs(I[1]);
   Ic = abs(I[2]);

   // Determina corrente m�dia
 	Imed = (Ia + Ib + Ic) / 3.;

 	if(Imed == 0)
   	return;

   // Calcula as componentes sim�tricas da corrente de defeito
   I0 = abs((I[0] + I[1] + I[2]) / 3.);
   I1 = abs((I[0] + a*I[1] + a*a*I[2]) / 3.);

   // Determina a rela��o 100 * I0 / I1 (%)
   if(I1 == 0.)
   	relI0_I1 = 0.;
   else
		relI0_I1 = 100.* I0 / I1;

	// Determina os desvios das correntes de fase, em (%), em rela��o � Imed
   dIa = 100. * (Ia - Imed) / Imed;
   dIb = 100. * (Ib - Imed) / Imed;
   dIc = 100. * (Ic - Imed) / Imed;

   // Pega os par�metros de compara��o
   max1 = ParametrosComparacao.maxPorc_3f;
   min1 = ParametrosComparacao.minPorc_1f;
   min2 = ParametrosComparacao.minRelI0I1_2ft;

   // Verifica limites para defeito 3F
	if(abs(dIa) < max1 && abs(dIb) < max1 && abs(dIc) < max1)
   {
    	TipoFalta = falta3F;
		NumFasesFalta = 3;
      FonteClassificacao = fonteQUALIMETRO;
   }

   // Verifica limites para defeito FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && abs(Ia) > min1 && dIb < 0. && dIc < 0.)
      {
       	TipoFalta = faltaAG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
      else if(dIb > 0. && abs(dIb) > min1 && dIa < 0. && dIc < 0.)
      {
       	TipoFalta = faltaBG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
      else if(dIc > 0. && abs(dIc) > min1 && dIa < 0. && dIb < 0.)
      {
       	TipoFalta = faltaCG;
         FonteClassificacao = fonteQUALIMETRO;
         NumFasesFalta = 1;
      }
   }

   // Verifica limites para defeito 2F ou 2FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && dIb > 0. && dIc < 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaABG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaAB;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
               TipoFalta = faltaABG;
            else
               TipoFalta = faltaAB;
            FonteClassificacao = fonteQUALIMETRO;
         }
         NumFasesFalta = 2;
      }
      else if(dIa > 0. && dIb < 0. && dIc > 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaCAG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaCA;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
               TipoFalta = faltaCAG;
            else
               TipoFalta = faltaCA;
            FonteClassificacao = fonteQUALIMETRO;
         }
         NumFasesFalta = 2;
      }
      else if(dIa < 0. && dIb > 0. && dIc > 0.)
      {
         // Se houve atua��o de rel� de prote��o, analisa a fun��o de prote��o atuante
         if(eqptoProtecaoProx != NULL)
         {
            if(eqptoProtecaoProx->TipoAtuacao == "Neutro")
					TipoFalta = faltaBCG;
            else if(eqptoProtecaoProx->TipoAtuacao == "Fase")
					TipoFalta = faltaBC;
            FonteClassificacao = fontePROTECAO;
         }
         // Se n�o houve atua��o de rel� de prote��o, analisa a rela��o I0/I1
         else
         {
            if(relI0_I1 > min2)
					TipoFalta = faltaBCG;
				else
					TipoFalta = faltaBC;
				FonteClassificacao = fonteQUALIMETRO;
			}
			NumFasesFalta = 2;
		}
	}
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta com base nos dados de DJ ou RE
 */
void __fastcall TClassificacao::ClassificaComEqptoProtecao(TChaveMonitorada* chvProt)
{
	// Define a fonte da classifica��o do defeito
   FonteClassificacao = fontePROTECAO;
	String fase = chvProt->faseAfetada;
	if(chvProt->TipoAtuacao == "Neutro")
	{
		if(fase == "A")        {NumFasesFalta = 1; TipoFalta = faltaAG;}
		else if(fase == "B")   {NumFasesFalta = 1; TipoFalta = faltaBG;}
		else if(fase == "C")   {NumFasesFalta = 1; TipoFalta = faltaCG;}
		else if(fase == "AB")  {NumFasesFalta = 2; TipoFalta = faltaABG;}
		else if(fase == "BC")  {NumFasesFalta = 2; TipoFalta = faltaBCG;}
		else if(fase == "CA")  {NumFasesFalta = 2; TipoFalta = faltaCAG;}
		else if(fase == "ABC") {NumFasesFalta = 3; TipoFalta = falta3F;}
	}
	else
	{
		if(fase == "AB")       {NumFasesFalta = 2; TipoFalta = faltaAB;}
		else if(fase == "BC")  {NumFasesFalta = 2; TipoFalta = faltaBC;}
		else if(fase == "CA")  {NumFasesFalta = 2; TipoFalta = faltaCA;}
		else if(fase == "ABC") {NumFasesFalta = 3; TipoFalta = falta3F;}
	}
}
//---------------------------------------------------------------------------
/***
 * M�todo para classificar a falta com base nos dados de DJ ou RE
 */
void __fastcall TClassificacao::ClassificaComEqptoProt(TChaveMonitorada* chvProt)
{
//	TChaveMonitorada* chvProt;
//	TEqptoCampo* eqptoCampo;
//	TList* lisEqptoCampo;

//   // Pega o DJ / RE
//	lisEqptoCampo = dados->GetEqptosCampo();
//   chvProt = NULL;
//   for(int i=0; i<lisEqptoCampo->Count; i++)
//   {
//		eqptoCampo = (TEqptoCampo*) lisEqptoCampo->Items[i];
//      if(!(eqptoCampo->GetTipo() == chaveDJ || eqptoCampo->GetTipo() == chaveRE))
//      	continue;
//
//      chvProt = (TChaveMonitorada*) eqptoCampo;
//   }
//   if(chvProt == NULL) return;

	// Define a fonte da classifica��o do defeito
   FonteClassificacao = fontePROTECAO;

	// Se prot. de neutro atuou, tipo de falta = FT (AT, por exemplo).
	if(chvProt->TipoAtuacao == "Neutro")
	{
		TipoFalta = faltaAG;
		NumFasesFalta = 1;
	}
   else if(chvProt->TipoAtuacao == "Fase")
	{
      TipoFalta = falta2F_3F;
		NumFasesFalta = 2;
	}
}
//---------------------------------------------------------------------------
// Para a distin��o entre curto-circuito 2F e 2FT, verifica se houve sensibiliza��o
// da prote��o de neutro de um rel� de prote��o. Se n�o houve rel� de prote��o atuante,
// considera que o defeito foi 2F.
void __fastcall TClassificacao::ComparaCorrentesFase(std::complex<double>* I, TChaveMonitorada* eqptoProtecao)
{
	double Ia, Ib, Ic, I0, I1;
	double Imed, dIa, dIb, dIc;
   double max1, min1, min2;
   double relI0_I1;
   std::complex<double> a = std::complex<double>(1. * cos(120. * M_PI/180.), 1. * sin(120. * M_PI/180.));

   // Pega os par�metros de compara��o
   max1 = ParametrosComparacao.maxPorc_3f;
   min1 = ParametrosComparacao.minPorc_1f;
   min2 = ParametrosComparacao.minRelI0I1_2ft;

   // M�dulos das correntes de fase
   Ia = abs(I[0]);
   Ib = abs(I[1]);
   Ic = abs(I[2]);

   // Determina corrente m�dia
 	Imed = (Ia + Ib + Ic) / 3.;

//   // Calcula as componentes sim�tricas da corrente de defeito
//   I0 = abs((I[0] + I[1] + I[2]) / 3.);
//   I1 = abs((I[0] + a*I[1] + a*a*I[2]) / 3.);
//
//   // Determina a rela��o 100 * I0 / I1 (%)
//   relI0_I1 = 100.* I0 / I1;

	// Determina os desvios das correntes de fase, em (%), em rela��o � Imed
   dIa = 100. * (Ia - Imed) / Imed;
   dIb = 100. * (Ib - Imed) / Imed;
   dIc = 100. * (Ic - Imed) / Imed;

	// Verifica limites para defeito 3F
	if(abs(dIa) < max1 && abs(dIb) < max1 && abs(dIc) < max1)
   {
    	TipoFalta = falta3F;
		NumFasesFalta = 3;
   }

   // Verifica limites para defeito FT
   if(TipoFalta == faltaINDEF)
   {
		if(dIa > 0. && abs(Ia) > min1 && dIb < 0. && dIc < 0.)
      {
       	TipoFalta = faltaAG;
         NumFasesFalta = 1;
      }
      else if(dIb > 0. && abs(dIb) > min1 && dIa < 0. && dIc < 0.)
      {
       	TipoFalta = faltaBG;
         NumFasesFalta = 1;
      }
      else if(dIc > 0. && abs(dIc) > min1 && dIa < 0. && dIb < 0.)
      {
       	TipoFalta = faltaCG;
         NumFasesFalta = 1;
      }
   }

	if(TipoFalta == faltaINDEF)
   {
   	if(eqptoProtecao == NULL)
      {
         if(dIa > 0. && dIb > 0. && dIc < 0.)
         {
            TipoFalta = faltaAB;
            NumFasesFalta = 2;
         }
         else if(dIa > 0. && dIb < 0. && dIc > 0.)
         {
            TipoFalta = faltaCA;
            NumFasesFalta = 2;
         }
         else if(dIa < 0. && dIb > 0. && dIc > 0.)
         {
            TipoFalta = faltaBC;
            NumFasesFalta = 2;
         }
      }
      else
      {
         if(dIa > 0. && dIb > 0. && dIc < 0.)
         {
            if(eqptoProtecao->TipoAtuacao == "Neutro")
	            TipoFalta = faltaABG;
				else if(eqptoProtecao->TipoAtuacao == "Fase")
               TipoFalta = faltaAB;
            NumFasesFalta = 2;
         }
         else if(dIa > 0. && dIb < 0. && dIc > 0.)
         {
            if(eqptoProtecao->TipoAtuacao == "Neutro")
	            TipoFalta = faltaCAG;
				else if(eqptoProtecao->TipoAtuacao == "Fase")
               TipoFalta = faltaCA;
            NumFasesFalta = 2;
         }
         else if(dIa < 0. && dIb > 0. && dIc > 0.)
         {
            if(eqptoProtecao->TipoAtuacao == "Neutro")
	            TipoFalta = faltaBCG;
				else if(eqptoProtecao->TipoAtuacao == "Fase")
               TipoFalta = faltaBC;
            NumFasesFalta = 2;
         }
      }
   }


//   // Verifica limites para defeito 2F ou 2FT
//   if(TipoFalta == faltaINDEF)
//   {
//		if(dIa > 0. && dIb > 0. && dIc < 0.)
//      {
//      	if(relI0_I1 > min2)
//				TipoFalta = faltaABG;
//         else
//            TipoFalta = faltaAB;
//
//         NumFasesFalta = 2;
//      }
//      else if(dIa > 0. && dIb < 0. && dIc > 0.)
//      {
//      	if(relI0_I1 > min2)
//				TipoFalta = faltaCAG;
//         else
//            TipoFalta = faltaCA;
//
//         NumFasesFalta = 2;
//      }
//      else if(dIa < 0. && dIb > 0. && dIc > 0.)
//      {
//      	if(relI0_I1 > min2)
//				TipoFalta = faltaBCG;
//         else
//            TipoFalta = faltaBC;
//
//         NumFasesFalta = 2;
//      }
//   }

}
//---------------------------------------------------------------------------
/***
 * Determina o monitor (sensor ou rel�) mais pr�ximo do defeito
 */
TEqptoCampo* __fastcall TClassificacao::DeterminaMonitorProximo()
{
	TEqptoCampo* eqptoCampoProximo = NULL;
   TList* lisBlocosJusante = NULL;
	TList* lisEqptosCampo;

   // Eqptos dos quais temos dados
   lisEqptosCampo = dados->GetEqptosCampo();

   // Obt�m o eqpto com a menor lista de blocos � jusante (mais pr�ximo do defeito)
   for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
      if((eqptoCampo->GetTipo() != eqptoSENSOR) &&
         (eqptoCampo->GetTipo() != chaveDJ) &&
         (eqptoCampo->GetTipo() != chaveRE))
      {
      	continue;
      }

      // Compara a lista de blocos � jusante
		if((lisBlocosJusante == NULL) ||
      	(eqptoCampo->GetBlocosJusante()->Count < lisBlocosJusante->Count))
      {
         lisBlocosJusante = eqptoCampo->GetBlocosJusante();
         eqptoCampoProximo = eqptoCampo;
      }
   }

   return eqptoCampoProximo;
}
//---------------------------------------------------------------------------
/***
 * Determina o monitor (sensor ou rel�) mais pr�ximo do defeito
 */
TChaveMonitorada* __fastcall TClassificacao::DeterminaProtecaoProxima_FLOffline()
{
	TEqptoCampo* eqptoCampoProximo = NULL;
	TList* lisBlocosJusante = NULL;
	TList* lisEqptosCampo;

	// Eqptos dos quais temos dados
	lisEqptosCampo = dados->GetEqptosCampo();

	// Obt�m o eqpto com a menor lista de blocos � jusante (mais pr�ximo do defeito)
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if((eqptoCampo->GetTipo() != chaveDJ) && (eqptoCampo->GetTipo() != chaveRE))
		{
			continue;
      }

      // Compara a lista de blocos � jusante
		if((lisBlocosJusante == NULL) ||
      	(eqptoCampo->GetBlocosJusante()->Count < lisBlocosJusante->Count))
		{
			lisBlocosJusante = eqptoCampo->GetBlocosJusante();
			eqptoCampoProximo = eqptoCampo;
		}
	}

	return ((TChaveMonitorada*)eqptoCampoProximo);
}
//---------------------------------------------------------------------------
/***
 * Determina o monitor (sensor ou rel�) mais pr�ximo do defeito
 */
TChaveMonitorada* __fastcall TClassificacao::DeterminaProtecaoProxima()
{
	TEqptoCampo* eqptoCampoProximo = NULL;
	TList* lisBlocosJusante = NULL;
	TList* lisEqptosCampo;

	// Eqptos dos quais temos dados
	lisEqptosCampo = dados->GetEqptosCampo();

	// Obt�m o eqpto com a menor lista de blocos � jusante (mais pr�ximo do defeito)
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if((eqptoCampo->GetTipo() != chaveDJ) && (eqptoCampo->GetTipo() != chaveRE))
		{
			continue;
      }

      // Compara a lista de blocos � jusante
		if((lisBlocosJusante == NULL) ||
      	(eqptoCampo->GetBlocosJusante()->Count < lisBlocosJusante->Count))
		{
			lisBlocosJusante = eqptoCampo->GetBlocosJusante();
			eqptoCampoProximo = eqptoCampo;
		}
	}

	return ((TChaveMonitorada*)eqptoCampoProximo);
}
//---------------------------------------------------------------------------
/***
 * Determina o sensor mais pr�ximo do defeito
 */
TSensor* __fastcall TClassificacao::DeterminaSensorProximo()
{
	TSensor* sensorProximo = NULL;
	TSensor* sensorCandidato = NULL;
   TList* lisBlocosJusante = NULL;
	TList* lisEqptosCampo;

	// Eqptos dos quais temos dados
	lisEqptosCampo = dados->GetEqptosCampo();

	// Obt�m o eqpto com a menor lista de blocos � jusante (mais pr�ximo do defeito)
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoSENSOR) continue;

		sensorCandidato = (TSensor*) eqptoCampo;
		if(!sensorCandidato->Sensibilizado()) continue;

		// Compara a lista de blocos � jusante
		if((lisBlocosJusante == NULL) || (eqptoCampo->GetBlocosJusante()->Count < lisBlocosJusante->Count))
		{
			lisBlocosJusante = eqptoCampo->GetBlocosJusante();
			sensorProximo = sensorCandidato;
      }
   }

   return sensorProximo;
}
////---------------------------------------------------------------------------
///***
// * Determina o qual�metro instalado no in�cio do alimentador
// */
//TQualimetro* __fastcall TClassificacao::DeterminaQualimetroSE()
//{
//	TQualimetro* qualimetro = NULL;
//	TList* lisEqptosCampo;
//
//	// Eqptos de campo de que temos dados
//	lisEqptosCampo = dados->GetEqptosCampo();
//
//	for(int i=0; i<lisEqptosCampo->Count; i++)
//	{
//		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
//		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;
//
//		qualimetro = (TQualimetro*) eqptoCampo;
//		if(qualimetro->SE)
//      	break;
//		else
//			qualimetro = NULL;
//	}
//
//	return qualimetro;
//}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::InicializaConfiguracoes(String CaminhoDirFaultLocation)
{
   TIniFile* file = NULL;

   // Obt�m o m�ximo desvio porcentual admiss�vel, em rela��o �s medi��es de tens�o
   String pathConfigGerais = CaminhoDirFaultLocation + "\\ConfigGerais.ini";

   try
   {
      file = new TIniFile(pathConfigGerais);
      ToleranciaFasesSasOk = file->ReadFloat("ALGO_FASORIAL", "ToleranciaFasesSasOk", 30.);
	   file->Free();
   }
   catch(Exception &e)
   {
      ToleranciaFasesSasOk = 30.;
   }
}
//---------------------------------------------------------------------------
/***
 * Determina o qual�metro que mediu corrente e est� mais pr�ximo do defeito
 */
TQualimetro* __fastcall TClassificacao::QualimetroEqptoRef()
{
	TQualimetro* qualimetro = NULL;
	TList* lisEqptosCampo;

	// Eqptos de campo de que temos dados
	lisEqptosCampo = dados->GetEqptosCampo();

	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		qualimetro = (TQualimetro*) eqptoCampo;
		if(qualimetro->candidatoEqptoRef)
      	break;
		else
			qualimetro = NULL;
	}

	return qualimetro;
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::Executa_FLOffline()
{
	TChaveMonitorada* eqptoProtecaoProx = NULL;

   // Inicializa tipo de falta
	TipoFalta = faltaINDEF;

	// Determina chave de prote��o (Disjuntor ou religadora) mais pr�xima do defeito
	eqptoProtecaoProx = DeterminaProtecaoProxima_FLOffline();

   // Tenta classificar com o disjuntor do alimentador
   if(TipoFalta == faltaINDEF && eqptoProtecaoProx != NULL)
   {
		ClassificaComEqptoProt_FLOffline(eqptoProtecaoProx);
      VerificaCorrentesFasesSasOpostas(eqptoProtecaoProx);
   }
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::VerificaCorrentesFasesSasOpostas(TChaveMonitorada* chaveMonitorada)
{
   if(!chaveMonitorada) return;

   double deltaThetaArg = 0., deltaTheta = 0.;
   std::complex<double> Ia = std::complex<double>(0., 0.);
   std::complex<double> Ib = std::complex<double>(0., 0.);
   std::complex<double> Ic = std::complex<double>(0., 0.);

   if(TipoFalta == faltaAG)
   {
      Ib = chaveMonitorada->medicaoVI.falta.I[1];
      Ic = chaveMonitorada->medicaoVI.falta.I[2];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ib) - std::arg(Ic));
   }
   else if(TipoFalta == faltaBG)
   {
      Ia = chaveMonitorada->medicaoVI.falta.I[0];
      Ic = chaveMonitorada->medicaoVI.falta.I[2];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ia) - std::arg(Ic));
   }
   else if(TipoFalta == faltaCG)
   {
      Ia = chaveMonitorada->medicaoVI.falta.I[0];
      Ib = chaveMonitorada->medicaoVI.falta.I[1];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ia) - std::arg(Ib));
   }

   // Calcula o desvio do deltaThetaArg em rela��o a 180 graus
   deltaTheta = fabs(180. - deltaThetaArg);

   if(deltaTheta < ToleranciaFasesSasOk)
   {
      CorrentesOpostasFasesSas = true;
   }
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::VerificaCorrentesFasesSasOpostas(TQualimetro* qualimetroEqptoRef)
{
   if(!qualimetroEqptoRef) return;

   double deltaThetaArg = 0., deltaTheta = 0.;
   std::complex<double> Ia = std::complex<double>(0., 0.);
   std::complex<double> Ib = std::complex<double>(0., 0.);
   std::complex<double> Ic = std::complex<double>(0., 0.);

   if(TipoFalta == faltaAG)
   {
      Ib = qualimetroEqptoRef->medicaoVI.falta.I[1];
      Ic = qualimetroEqptoRef->medicaoVI.falta.I[2];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ib) - std::arg(Ic));
   }
   else if(TipoFalta == faltaBG)
   {
      Ia = qualimetroEqptoRef->medicaoVI.falta.I[0];
      Ic = qualimetroEqptoRef->medicaoVI.falta.I[2];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ia) - std::arg(Ic));
   }
   else if(TipoFalta == faltaCG)
   {
      Ia = qualimetroEqptoRef->medicaoVI.falta.I[0];
      Ib = qualimetroEqptoRef->medicaoVI.falta.I[1];
      deltaThetaArg = (180. / M_PI) * fabs(std::arg(Ia) - std::arg(Ib));
   }

   // Calcula o desvio do deltaThetaArg em rela��o a 180 graus
   deltaTheta = fabs(180. - deltaThetaArg);

   if(deltaTheta < ToleranciaFasesSasOk)
   {
      CorrentesOpostasFasesSas = true;
   }
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::ExecutaRompCabo()
{
	TQualimetro* qualimetroMaisProximo = NULL;

	// Inicializa classifica��o
	TipoRompCabo = rompINDEF;

	qualimetroMaisProximo = DeterminaQualimetroProximo_RompCabo();
	ClassificaRompCaboComQualimetro(qualimetroMaisProximo);
}
//---------------------------------------------------------------------------
// Obt�m o qual�metro, sensibilizado pelo rompimento de cabo, mais pr�ximo do problema.
TQualimetro* __fastcall TClassificacao::DeterminaQualimetroProximo_RompCabo()
{
	TList* lisEqptosCampo;
	TQualimetro* qualimetroProximo = NULL;

   // Eqptos dos quais temos dados
	lisEqptosCampo = dados->GetEqptosCampo();

	// Obt�m o eqpto com a menor lista de blocos � jusante (mais pr�ximo do defeito)
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		qualimetroProximo = (TQualimetro*) eqptoCampo;
		break;
	}
	return(qualimetroProximo);
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::Executa()
{
	// Inicializa tipo de falta
	TipoFalta = faltaINDEF;

	// Determina o eqpto de prote��o mais pr�ximo
	TChaveMonitorada* eqptoProtecaoProx = DeterminaProtecaoProxima();

	if(eqptoProtecaoProx)
	{
		ClassificaComEqptoProtecao(eqptoProtecaoProx);
	}

//	// Inicializa tipo de falta
//	TipoFalta = faltaINDEF;
//	ConsideraQualidadeSensor = configGerais->GetConsiderarQualidadeSensor();
//
//	// Verifica se tem sensor sensibilizado pelo defeito
//	sensor = DeterminaSensorProximo();
//	// Determina chave de prote��o mais pr�xima do defeito
//	eqptoProtecaoProx = DeterminaProtecaoProxima();
//	// Pega o qual�metro com medi��o de corrente
//	qualimetroEqptoRef = QualimetroEqptoRef();
//
//	// Na melhor das hip�teses, tenta classificar com qual�metro eqptoRef,
//	// aquele que est� mais pr�ximo do defeito e que mediu corrente
//	if(TipoFalta == faltaINDEF && qualimetroEqptoRef != NULL)
//   {
//		ClassificaComQualimetroEqptoRef(qualimetroEqptoRef, eqptoProtecaoProx);
//      VerificaCorrentesFasesSasOpostas(qualimetroEqptoRef);
//   }
//
//	// Se ainda n�o classificou, tenta classificar com sensor sensibilizado pelo
//	// defeito, supondo alarme de eqpto de prote��o
//	if(TipoFalta == faltaINDEF && sensor != NULL && eqptoProtecaoProx != NULL)
//	{
//		if(sensor->qualidadeOK && sensor->Sensibilizado())
//			ClassificaComSensor(sensor, eqptoProtecaoProx);
//		else
//			ClassificaComEqptoProt(eqptoProtecaoProx);
//	}
//	// Se ainda n�o classificou, tenta classificar com sensor sensibilizado pelo
//	// defeito, supondo que n�o haja alarme de atua��o de eqpto de prote��o
//	if(TipoFalta == faltaINDEF && sensor != NULL && eqptoProtecaoProx == NULL)
//	{
//		if(sensor->Sensibilizado())
//		{
//			if(!ConsideraQualidadeSensor || (ConsideraQualidadeSensor && sensor->qualidadeOK))
//			{
//				ClassificaComSensor(sensor, eqptoProtecaoProx);
//			}
//		}
//	}
//   // Na pior das hip�teses, tenta classificar com eqpto de prote��o (rel�)
//	if(TipoFalta == faltaINDEF && eqptoProtecaoProx != NULL)
//	{
//		ClassificaComEqptoProt(eqptoProtecaoProx);
//	}
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::GetSensoresDefeito(TList* lisSensoresDef)
{
	TEqptoCampo* eqptoCampo;
	TEqptoCampo* eqptoCampoProximo = NULL;
   TList* lisEqptosCampo;
   TList* lisBlocosJusante = NULL;
	TSensor* sensor;

	if(lisSensoresDef == NULL) return;
   lisSensoresDef->Clear();

   lisEqptosCampo = dados->GetEqptosCampo();

   for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
      if(eqptoCampo->GetTipo() != eqptoSENSOR) continue;

      sensor = (TSensor*) eqptoCampo;

      // Compara a lista de blocos � jusante
		if((lisBlocosJusante == NULL) ||
      	(eqptoCampo->GetBlocosJusante()->Count < lisBlocosJusante->Count))
      {
         lisBlocosJusante = eqptoCampo->GetBlocosJusante();
         eqptoCampoProximo = eqptoCampo;
      }
	}
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::GetCorrentes(TEqptoCampo* eqptoCampo, std::complex<double>* I)
{
	// Prote��o
	if(eqptoCampo == NULL) return;

   // Verifica tipo de equipamento e obt�m as correntes registradas
   if(eqptoCampo->GetTipo() == eqptoSENSOR)
   {
      TSensor* sensor = (TSensor*) eqptoCampo;
      for(int i=0; i<3; i++)
      {
	      I[i] = sensor->medicaoI.falta.I[i];
      }
   }
   else if(eqptoCampo->GetTipo() == chaveDJ)
   {
      TChaveMonitorada* DJ = (TChaveMonitorada*) eqptoCampo;
      for(int i=0; i<3; i++)
      {
	      I[i] = DJ->medicaoVI.falta.I[i];
      }
   }
   else if(eqptoCampo->GetTipo() == chaveRE)
   {
      TChaveMonitorada* RE = (TChaveMonitorada*) eqptoCampo;
      for(int i=0; i<3; i++)
      {
	      I[i] = RE->medicaoVI.falta.I[i];
      }
	}
}
//---------------------------------------------------------------------------
int __fastcall TClassificacao::GetNumFasesFalta()
{
	return NumFasesFalta;
}
//---------------------------------------------------------------------------
String __fastcall TClassificacao::GetStrTipoRompCabo()
{
	String strTipoRomp;
	switch(TipoRompCabo)
	{
	case rompA:
		strTipoRomp = "A";
		break;

	case rompB:
		strTipoRomp = "B";
		break;

	case rompC:
		strTipoRomp = "C";
		break;

	case rompAB:
		strTipoRomp = "AB";
		break;

	case rompBC:
		strTipoRomp = "BC";
		break;

	case rompCA:
		strTipoRomp = "CA";
		break;

	case rompABC:
		strTipoRomp = "ABC";
		break;

	default:
		strTipoRomp = "ABC";
		break;
	}
	return(strTipoRomp);
}
////---------------------------------------------------------------------------
//String __fastcall TClassificacao::GetStrTipoRompCabo()
//{
//	String strTipoRomp;
//
//	switch(TipoRompCabo)
//	{
//	case rompA:
//		strTipoRomp = "A";
//		break;
//
//	case rompB:
//		strTipoRomp = "B";
//		break;
//
//	case rompC:
//		strTipoRomp = "C";
//		break;
//
//	case rompAB:
//		strTipoRomp = "AB;
//		break;
//	}
//	return strTipoRomp;
//}//---------------------------------------------------------------------------
/***
 * falta3F => "ABC", faltaAG => "A", faltaAB => "AB", etc...
 */
String __fastcall TClassificacao::GetStrTipoFalta()
{
	String strTipoFalta;

	switch(TipoFalta)
   {
	case faltaAG:
		strTipoFalta = "A";
		break;

   case faltaBG:
   	strTipoFalta = "B";
      break;

   case faltaCG:
   	strTipoFalta = "C";
      break;

   case faltaAB:
   	strTipoFalta = "AB";
      break;

   case faltaBC:
   	strTipoFalta = "BC";
      break;

   case faltaCA:
   	strTipoFalta = "CA";
      break;

   case faltaABG:
   	strTipoFalta = "ABN";
      break;

   case faltaBCG:
   	strTipoFalta = "BCN";
      break;

   case faltaCAG:
   	strTipoFalta = "ACN";
      break;

	case falta3F:
   	strTipoFalta = "ABC";
      break;

   case falta2F_3F:
   	strTipoFalta = "ABABC";
      break;

   default:
   	strTipoFalta = "ABC";
      break;
   }

	return strTipoFalta;
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::SetConfigGerais(TConfiguracoes* configGerais)
{
	this->configGerais  = configGerais;
}
//---------------------------------------------------------------------------
void __fastcall TClassificacao::SetParamComparacao(double* vetorParamComparacao)
{
	// Par�metros para classifica��o de falta
	ParametrosComparacao.maxPorc_3f = vetorParamComparacao[0];
	ParametrosComparacao.minPorc_1f = vetorParamComparacao[1];
	ParametrosComparacao.minRelI0I1_2ft = vetorParamComparacao[2];

	// Par�metros para classifica��o de rompimento de cabo
	ParametrosComparacaoRompCabo.minPorc_1f = vetorParamComparacao[3];
}
//---------------------------------------------------------------------------
//eof
