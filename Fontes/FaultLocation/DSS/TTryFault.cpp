//---------------------------------------------------------------------------
#pragma hdrstop
#include "TTryFault.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TConfiguracoes.h"
#include "TDSS.h"
#include "DSSEnums.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
__fastcall TTryFault::TTryFault()
{
	// Cria o objeto de OpenDSS
   DSS = new TDSS();

   // Cria listas
	lisIDBarrasMonitoradas = new TStringList();
   lisCodTrechosMonitorados = new TStringList();

   lisMedV = new TList();
   lisMedI = new TList();
	lisCalcV = new TStringList();
	lisCalcI = new TStringList();

   // Inicializa valores-padrão dos dados de nível de curto-circuito no secundário do trafo
   Vnom = 13.8;
   IccMT = 100000/(sqrt(3.) * 13.8);

   strIDBarraMedV_QualimetroRef = "";
   codLigacaoQualimetroRef = "";
}
//---------------------------------------------------------------------------
__fastcall TTryFault::~TTryFault()
{
	// Destroi objetos
   delete DSS; DSS = NULL;

   if(lisIDBarrasMonitoradas)
   {
      for(int i=lisIDBarrasMonitoradas->Count-1; i>=0; i--) lisIDBarrasMonitoradas->Delete(i);
      delete lisIDBarrasMonitoradas; lisIDBarrasMonitoradas = NULL;
   }
   if(lisCodTrechosMonitorados)
   {
      for(int i=lisCodTrechosMonitorados->Count-1; i>=0; i--) lisCodTrechosMonitorados->Delete(i);
      delete lisCodTrechosMonitorados; lisCodTrechosMonitorados = NULL;
   }
   if(lisMedV)
   {
      for(int i=lisMedV->Count-1; i>=0; i--) delete(lisMedV->Items[i]);
      delete lisMedV; lisMedV = NULL;
   }
   if(lisMedI)
   {
      for(int i=lisMedI->Count-1; i>=0; i--) delete(lisMedI->Items[i]);
      delete lisMedI; lisMedI = NULL;
   }
   if(lisCalcV)
   {
      for(int i=lisCalcV->Count-1; i>=0; i--) lisCalcV->Delete(i);
      delete lisCalcV; lisCalcV = NULL;
   }
   if(lisCalcI)
   {
      for(int i=lisCalcI->Count-1; i>=0; i--) lisCalcI->Delete(i);
      delete lisCalcI; lisCalcI = NULL;
   }
}
//---------------------------------------------------------------------------
/***
 * Método que calcula o erro associado a um teste de defeito, com base em:
 *   1) Medições de V e I, obtidas através do barramento
 *   2) Cálculos de V e I, nos mesmos pontos das medições
 */
double __fastcall TTryFault::CalculaErro()
{
	double delta;
   double medV[10], medI[10], calcV[10], calcI[10];
   double erroV, erroI, erroGlobal;
   double pesoV, pesoI;
   int nV, nI, comp, contV, contI;
   String strCalc, valor;

   // ::::::::::::::::::::
   // Guarda as medições em vetor
   // ::::::::::::::::::::
	nV = 0;
   for(int i=0; i<lisMedV->Count; i++)
   {
   	medicaoV* medTensao = (medicaoV*) lisMedV->Items[i];

      for(int j=0; j<medTensao->numFases; j++)
      {
         medV[nV] = medTensao->V[j];
         nV += 1;
      }
   }
   nI = 0;
	for(int i=0; i<lisMedI->Count; i++)
   {
   	medicaoI* medCorrente = (medicaoI*) lisMedI->Items[i];

      for(int j=0; j<medCorrente->numFases; j++)
      {
         medI[nI] = medCorrente->I[j];
         nI += 1;
      }
   }

   // ::::::::::::::::::::
   // Guarda os cálculos em vetor
   // ::::::::::::::::::::
   contV = 0;
   for(int i=0; i<lisCalcV->Count; i++)
   {
      strCalc = lisCalcV->Strings[i];
      comp = CSVCamposCount(strCalc, ";");
      for(int j=0; j<comp; j++)
      {
			valor = GetCampoCSV(strCalc, j, ";");
	      calcV[contV++] = valor.ToDouble();
      }
   }
   contI = 0;
   for(int i=0; i<lisCalcI->Count; i++)
   {
      strCalc = lisCalcI->Strings[i];
      comp = CSVCamposCount(strCalc, ";");
      for(int j=0; j<comp; j++)
      {
			valor = GetCampoCSV(strCalc, j, ";");
	      calcI[contI++] = valor.ToDouble();
      }
   }

   // Verificação: NÚMERO DE MEDIÇÕES tem que ser = NÚMERO DE CÁLCULOS
   if(nV != contV || nI != contI || nV == 0. || nI == 0. || Vnom == 0.) return -1.;

   // Erro de tensão (deltas normalizados em relação ao valor medido):
   erroV = 0.;
   contV = 0;
	for(int i=0; i<nV; i++)
   {
   	if(medV[i] > 0.)
      {

         delta = fabs(medV[i] - calcV[i]) / medV[i];
         if(delta > 1.)
         	delta = 1.;

         erroV += delta * delta;
         contV += 1;
      }
   }
   if(contV > 0) erroV = sqrt(erroV / contV);

   // Erro de corrente (deltas normalizados em relação ao valor medido):
   erroI = 0.;
   contI = 0;
	for(int i=0; i<nI; i++)
   {
   	if(medI[i] > 0.)
      {

         delta = fabs(medI[i] - calcI[i]) / medI[i];
         if(delta > 1.)
         	delta = 1.;

         erroI += delta * delta;
         contI += 1;
      }
   }
   if(contI > 0) erroI = sqrt(erroI / contI);

   if(config)
   {
		pesoV = config->GetPesoV();
      pesoI = config->GetPesoI();
   }
   else
   {
      pesoV = 1.0;
      pesoI = 1.0;
   }

	// Aplica os pesos para obter o erro global
   erroGlobal = (pesoV * erroV + pesoI * erroI) / (pesoV + pesoI);

   return erroGlobal;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::CorrentePreFalta(std::complex<double> &Ia_pre, std::complex<double> &Ib_pre, std::complex<double> &Ic_pre)
{

}
//---------------------------------------------------------------------------
/***
 * Método que calcula o erro associado a um teste de defeito, com base em:
 *   1) Medições de V e I, obtidas através do barramento
 *   2) Cálculos de V e I, nos mesmos pontos das medições
 */
double __fastcall TTryFault::CalculaErro_AlgoFasorial()
{
	double delta;
   double medV[20], calcV[20];
   double erroV, erroGlobal;
   int nV, comp, contV;
   String strCalc, valor;

   // ::::::::::::::::::::
   // Guarda as medições em vetor
   // ::::::::::::::::::::
	nV = 0;
   for(int i=0; i<lisMedV->Count; i++)
   {
   	medicaoV* medTensao = (medicaoV*) lisMedV->Items[i];

      for(int j=0; j<medTensao->numFases; j++)
      {
         medV[nV] = medTensao->V[j];
         nV += 1;
      }
   }

   // ::::::::::::::::::::
   // Guarda os cálculos em vetor
   // ::::::::::::::::::::
   contV = 0;
   for(int i=0; i<lisCalcV->Count; i++)
   {
      strCalc = lisCalcV->Strings[i];
      comp = CSVCamposCount(strCalc, ";");
      for(int j=0; j<comp; j++)
      {
			valor = GetCampoCSV(strCalc, j, ";");
	      calcV[contV++] = valor.ToDouble();
      }
   }
	if(nV != contV) return -1.;

   // Erro de tensão (deltas normalizados em relação ao valor medido):
   erroV = 0.;
   contV = 0;
	for(int i=0; i<nV; i++)
   {
   	if(medV[i] > 0.)
      {

         delta = fabs(medV[i] - calcV[i]) / medV[i];

         erroV += delta * delta;
         contV += 1;
      }
   }
   if(contV > 0) erroV = sqrt(erroV / contV);

	// Aplica os pesos para obter o erro global
	erroGlobal = erroV;

   return erroGlobal;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::ExecutaTeste(int patamar)
{
   // Salva o patamar diário para execução da simulação
	this->patamar = patamar;

   int res = DSS->Start();
   AnsiString comando = "";
   if(res == 1)
   {
      comando = "compile (" + caminhoDSS + ")";
      DSS->WriteCommand(comando);
   	comando = "set number=" + AnsiString(patamar);
      DSS->WriteCommand(comando);
      DSS->Solve();
      comando = "show current elements";
      DSS->WriteCommand(comando);
   }
}
//---------------------------------------------------------------------------
String __fastcall TTryFault::GetComandoFalta(String faseDefeito, int idBarraCurto, double Rf, bool Inicial)
{
	String comando = "";

   if((faseDefeito == "A") || (faseDefeito == "B") || (faseDefeito == "C"))
   {
   	// Rodando pela primeira vez, deve criar o objeto de falta
   	if(Inicial)
      	comando = "new fault.falta phases=1 bus1=" + String(idBarraCurto);
      else
         comando = "edit fault.falta phases=1 bus1=" + String(idBarraCurto);

      // Fases do defeito
      if(faseDefeito == "A")
         comando += ".1";
      else if(faseDefeito == "B")
         comando += ".2";
      else if(faseDefeito == "C")
         comando += ".3";

      // Resistência de falta
      comando += " r=" + String(DoubleToString(Rf,2));
   }
	else if((faseDefeito == "AB") || (faseDefeito == "BC") || (faseDefeito == "AC") || (faseDefeito == "CA"))
	{
		// Rodando pela primeira vez, deve criar o objeto de falta
		if(Inicial)
			comando = "new fault.falta phases=1";
		else
			comando = "edit fault.falta phases=1";

		// Fases do defeito
		if(faseDefeito == "AB")
		{
			comando += " bus1=" + String(idBarraCurto) + ".1";
			comando += " bus2=" + String(idBarraCurto) + ".2";
		}
		else if(faseDefeito == "BC")
		{
			comando += " bus1=" + String(idBarraCurto) + ".2";
			comando += " bus2=" + String(idBarraCurto) + ".3";
		}
		else if(faseDefeito == "AC" || faseDefeito == "CA")
		{
			comando += " bus1=" + String(idBarraCurto) + ".1";
			comando += " bus2=" + String(idBarraCurto) + ".3";
		}

		// Resistência de falta
		comando += " r=" + String(DoubleToString(Rf,2));
	}
	else if((faseDefeito == "ABN") || (faseDefeito == "BCN") || (faseDefeito == "ACN") || (faseDefeito == "CAN"))
	{
   	// Rodando pela primeira vez, deve criar o objeto de falta
      if(Inicial)
      {
         if(faseDefeito == "ABN")
         {
            comando = "new fault.falta phases=2 bus1=" + String(idBarraCurto);
            comando += ".1.1 bus2=" + String(idBarraCurto) + ".2.0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
         else if(faseDefeito == "BCN")
         {
            comando = "new fault.falta phases=2 bus1=" + String(idBarraCurto);
            comando += ".2.2 bus2=" + String(idBarraCurto) + ".3.0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
			else if(faseDefeito == "ACN")
			{
				comando = "new fault.falta phases=2 bus1=" + String(idBarraCurto);
				comando += ".1.1 bus2=" + String(idBarraCurto) + ".3.0";
				comando += " r=" + String(DoubleToString(Rf,2));
			}
			else if(faseDefeito == "CAN")
			{
				comando = "new fault.falta phases=2 bus1=" + String(idBarraCurto);
				comando += ".1.1 bus2=" + String(idBarraCurto) + ".3.0";
				comando += " r=" + String(DoubleToString(Rf,2));
			}
      }
      else
		{
			if(faseDefeito == "ABN")
         {
            comando = "edit fault.falta phases=2 bus1=" + String(idBarraCurto);
            comando += ".1.1 bus2=" + String(idBarraCurto) + ".2.0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
         else if(faseDefeito == "BCN")
         {
            comando = "edit fault.falta phases=2 bus1=" + String(idBarraCurto);
            comando += ".2.2 bus2=" + String(idBarraCurto) + ".3.0";
            comando += " r=" + String(DoubleToString(Rf,2));
			}
			else if(faseDefeito == "ACN" || faseDefeito == "CAN")
			{
            comando = "edit fault.falta phases=2 bus1=" + String(idBarraCurto);
            comando += ".1.1 bus2=" + String(idBarraCurto) + ".3.0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
   }
   else if(faseDefeito == "ABC")
   {
		// Rodando pela primeira vez, deve criar o objeto de falta
      if(Inicial)
	      comando = "new fault.falta phases=3 bus1=" + String(idBarraCurto);
      else
	      comando = "edit fault.falta phases=3 bus1=" + String(idBarraCurto);

      // Resistência de falta
      comando += " r=" + String(DoubleToString(Rf,2));
   }

   return comando;
}
//---------------------------------------------------------------------------
String __fastcall TTryFault::GetComandoFalta_2FT(int idFalta, String faseDefeito, int idBarraCurto, double Rf, bool Inicial)
{
	String comando = "";

   // Rodando pela primeira vez, deve criar o objeto de falta
   if(Inicial)
   {
      if(faseDefeito == "ABN")
      {
      	if(idFalta == 1)
         {
            comando = "new fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".1 bus2=" + String(idBarraCurto) + ".2 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "new fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".2 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
      else if(faseDefeito == "BCN")
      {
         if(idFalta == 1)
         {
            comando = "new fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".2 bus2=" + String(idBarraCurto) + ".3 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "new fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".3 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
      else if(faseDefeito == "ACN")
      {
         if(idFalta == 1)
         {
            comando = "new fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".1 bus2=" + String(idBarraCurto) + ".3 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "new fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".3 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
   }
   else
   {
      if(faseDefeito == "ABN")
      {
         if(idFalta == 1)
         {
            comando = "edit fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".1 bus2=" + String(idBarraCurto) + ".2 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "edit fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".2 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
      else if(faseDefeito == "BCN")
      {
         if(idFalta == 1)
         {
            comando = "edit fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".2 bus2=" + String(idBarraCurto) + ".3 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "edit fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".3 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
      else if(faseDefeito == "ACN")
      {
         if(idFalta == 1)
         {
            comando = "edit fault.falta1 phases=1 bus1=" + String(idBarraCurto);
            comando += ".1 bus2=" + String(idBarraCurto) + ".3 r=0.001";
         }
         else if(idFalta == 2)
         {
            comando = "edit fault.falta2 phases=1 bus1=" + String(idBarraCurto);
            comando += ".3 bus2=" + String(idBarraCurto) + ".0";
            comando += " r=" + String(DoubleToString(Rf,2));
         }
      }
   }

   return comando;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::GetLisCalcI(TStringList* lisEXT)
{
	if(!lisEXT) return;

   lisEXT->Clear();
   for(int i=0; i<lisCalcI->Count; i++)
   {
		String valor = lisCalcI->Strings[i];
      lisEXT->Add(valor);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::GetLisCalcV(TStringList* lisEXT)
{
	if(!lisEXT) return;

   lisEXT->Clear();
   for(int i=0; i<lisCalcV->Count; i++)
   {
		String valor = lisCalcV->Strings[i];
      lisEXT->Add(valor);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::GetLisMedV(TStringList* lisEXT)
{
	String valor;

	if(!lisEXT) return;

   lisEXT->Clear();
   for(int i=0; i<lisMedV->Count; i++)
   {
		medicaoV* medV = (medicaoV*) lisMedV->Items[i];
      valor = String(medV->V[0]) + ";" + String(medV->V[1]) + ";" + String(medV->V[2]);
      lisEXT->Add(valor);
   }
}
//---------------------------------------------------------------------------
int __fastcall TTryFault::GetNumFases(String fasesDefeito)
{
	int resp;

	if(fasesDefeito == "A" || fasesDefeito == "B" || fasesDefeito == "C")
   {
		resp = 1;
   }
   else if(fasesDefeito == "AB" || fasesDefeito == "BC" || fasesDefeito == "AC")
   {
		resp = 1;
   }
   else if(fasesDefeito == "ABN" || fasesDefeito == "BCN" || fasesDefeito == "ACN")
   {
		resp = 2;
   }
   else if(fasesDefeito == "ABC")
   {
		resp = 3;
   }
   return(resp);
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::IniciaBarrasTrechos()
{
   AnsiString comando;
   int res;

	// Inicializa o DSS para pegar as identificações das barras e dos trechos
   res = DSS->Start();

   // Verificação
   if(res != 1) return;

   comando = "compile (" + caminhoDSS + ")";
   DSS->WriteCommand(comando);

   // DSS resgata as identificações das barras
   DSS->GetDSSBarras();
   // DSS resgata as identificações das ligações
   DSS->GetDSSLigacoes();
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetCaminhoDSS(String caminhoDSS)
{
	this->caminhoDSS = caminhoDSS + "\\Master.dss";
   this->dirDSS = caminhoDSS;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetConfig(TConfiguracoes* config)
{
	this->config = config;
}
//---------------------------------------------------------------------------
/***
 * Método para salvar os IDs das barras das quais serão obtidos os cálculos de tensão
 */
void __fastcall TTryFault::SetBarrasMonitoradas(String CSV_IDBarrasMon)
{
	int comp;
	String IDbarra;

	// Verificação
	if(CSV_IDBarrasMon == "") return;

   comp = CSVCamposCount(CSV_IDBarrasMon, ";");
	for(int i=0; i<comp; i++)
   {
		IDbarra = GetCampoCSV(CSV_IDBarrasMon, i, ";");

      if(lisIDBarrasMonitoradas->IndexOf(IDbarra) < 0)
      	lisIDBarrasMonitoradas->Add(IDbarra);
   }
}
//---------------------------------------------------------------------------
/***
 * Método para salvar os códigos dos trechos ds quais serão obtidos os cálculos de corrente
 */
void __fastcall TTryFault::SetTrechosMonitorados(String CSV_CodTrechosMon)
{
	int comp;
	String codTrecho;

	// Verificação
	if(CSV_CodTrechosMon == "") return;

   comp = CSVCamposCount(CSV_CodTrechosMon, ";");
	for(int i=0; i<comp; i++)
   {
		codTrecho = GetCampoCSV(CSV_CodTrechosMon, i, ";");

      if(lisCodTrechosMonitorados->IndexOf(codTrecho) < 0)
      	lisCodTrechosMonitorados->Add(codTrecho);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetIDBarraMonitorada(String strIDBarraMedV_QualimetroRef)
{
   this->strIDBarraMedV_QualimetroRef = strIDBarraMedV_QualimetroRef;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetMedicoesBarras(TStringList* lisMedV)
{
	int comp;
	String medV, strV, strIDbarra;
   medicaoV* medTensao;

	// Verificação
	if(lisMedV == NULL) return;

   for(int i=0; i<lisMedV->Count; i++)
   {
		medV = lisMedV->Strings[i];

      comp = CSVCamposCount(medV, ";");
      strIDbarra = GetCampoCSV(medV, 0, ";");

      medTensao = new medicaoV();
      medTensao->IDbarra = strIDbarra.ToInt();
      medTensao->numFases = comp-1;
      for(int j=0; j<comp-1; j++)
      {
      	strV = GetCampoCSV(medV, j+1, ";");
         medTensao->V[j] = strV.ToDouble();
      }
      this->lisMedV->Add(medTensao);

      // Adiciona ID de barra monitorada
      lisIDBarrasMonitoradas->Add(strIDbarra);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetCodLigacaoRefMonitorada(String codLigacaoQualimetroRef)
{
   this->codLigacaoQualimetroRef = codLigacaoQualimetroRef;
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetMedicoesTrechos(TStringList* lisMedI)
{
	int comp;
	String medI, strI, strCodTrecho;
   medicaoI* medCorrente;

	// Verificação
	if(lisMedV == NULL) return;

   for(int i=0; i<lisMedI->Count; i++)
   {
		medI = lisMedI->Strings[i];

      comp = CSVCamposCount(medI, ";");
      strCodTrecho = GetCampoCSV(medI, 0, ";");

      medCorrente = new medicaoI();
      medCorrente->codTrecho = strCodTrecho;
      medCorrente->numFases = comp-1;
      for(int j=0; j<comp-1; j++)
      {
      	strI = GetCampoCSV(medI, j+1, ";");
         medCorrente->I[j] = strI.ToDouble();
      }
      this->lisMedI->Add(medCorrente);

      // Adiciona ID de barra monitorada
      lisCodTrechosMonitorados->Add(strCodTrecho);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryFault::SetNivelCurtoMT()
{
	// Insere no Geral.ini dados de potência de curto
  	TIniFile* file = new TIniFile(dirDSS + "\\..\\Geral.ini");

   Vnom = file->ReadFloat("NIVELCURTOMT", "Vnom", 13.8);
   IccMT = file->ReadFloat("NIVELCURTOMT", "IccMT", (100000./(sqrt(3.)*13.8)));

   // Destroi objeto
   if(file) {delete file; file = NULL;}
}
////---------------------------------------------------------------------------
//void __fastcall TTryFault::TestaCurto(int patamar, int barra_ID, TStringList* lisCalcV, TStringList* lisCalcI)
//{
//	// Verificação
//	if(lisCalcV == NULL || lisCalcI == NULL) return;
//
//   // Salva o patamar diário para execução da simulação
//	this->patamar = patamar;
//
//   int res = DSS->Start();
//   AnsiString comando = "";
//   if(res == 1)
//   {
//      comando = "compile (" + caminhoDSS + ")";
//      DSS->WriteCommand(comando);
//   	comando = "set number=" + AnsiString(patamar);
//      DSS->WriteCommand(comando);
//      comando = "new fault.falta phases=3 bus1=" + AnsiString(barra_ID);
//      DSS->WriteCommand(comando);
//      DSS->Solve();
////      comando = "show current elements";
////      DSS->WriteCommand(comando);
////      comando = "show voltage ln nodes";
////      DSS->WriteCommand(comando);
//
//      // Barras de interesse:
//      TStringList* lisCodBarrasMon = new TStringList();
//      lisCodBarrasMon->Add("61636");
//      lisCodBarrasMon->Add("62460");
//      lisCodBarrasMon->Add("60467");
//      lisCodBarrasMon->Add("60771");
//      DSS->GetVoltages(lisCodBarrasMon, lisCalcV);
//      TStringList* lisCodLigacoesMon = new TStringList();
//      lisCodLigacoesMon->Add("A79416");
//      lisCodLigacoesMon->Add("1786440");
//      DSS->GetCurrents(lisCodLigacoesMon, lisCalcI);
//
////      //debug - verificação dos valores das tensões e das correntes nas ligações
////      String valor;
////      for(int i=0; i<lisCalcV->Count; i++)
////      {
////			valor = lisCalcV->Strings[i];
////         int b = 0;
////      }
////      for(int i=0; i<lisCalcI->Count; i++)
////      {
////			valor = lisCalcI->Strings[i];
////         int c = 0;
////      }
//   }
//}

//---------------------------------------------------------------------------
void __fastcall TTryFault::TestaPreFalta(int patamar, std::complex<double> &Ia_pre, std::complex<double> &Ib_pre, std::complex<double> &Ic_pre)
{
	this->patamar = patamar;

   DSS->Start();

   AnsiString comando = "";

   comando = "compile (" + caminhoDSS + ")";
   DSS->WriteCommand(comando);

   // Altera o equivalente no ponto do qualímetro para 1 pu
   comando = "edit vsource.faseA pu=1 angle=0";
   DSS->WriteCommand(comando);
   comando = "edit vsource.faseB pu=1 angle=-120";
   DSS->WriteCommand(comando);
   comando = "edit vsource.faseC pu=1 angle=120";
   DSS->WriteCommand(comando);

   comando = "set number=" + AnsiString(patamar);
   DSS->WriteCommand(comando);

   DSS->Solve();

   // Resgata os fasores de I calculados no ponto de medição.
   TStringList* lisCalcI_qualRef = new TStringList;
   DSS->GetCurrents_QualimetroRef(codLigacaoQualimetroRef, lisCalcI_qualRef);

   if(lisCalcI_qualRef->Count >= 6)
   {
      double real = lisCalcI_qualRef->Strings[0].ToDouble();
      double imag = lisCalcI_qualRef->Strings[1].ToDouble();
      Ia_pre = std::complex<double>(real, imag);

      real = lisCalcI_qualRef->Strings[2].ToDouble();
      imag = lisCalcI_qualRef->Strings[3].ToDouble();
      Ib_pre = std::complex<double>(real, imag);

      real = lisCalcI_qualRef->Strings[4].ToDouble();
      imag = lisCalcI_qualRef->Strings[5].ToDouble();
      Ic_pre = std::complex<double>(real, imag);
   }
}
//---------------------------------------------------------------------------
// faseDefeito: A, B, C, AB, BC, AC, ABC, ABN, etc
void __fastcall TTryFault::TestaCurtoRf(int patamar, int idBarraCurto, String faseDefeito, double Rf, bool Inicial)
{
	//debug
   TStringList* lisaux = new TStringList();
   unsigned short hora, minuto, segundo, mseg;
   TTime t1;
   TTime t2;
   TTime delta;
   double deltaT;

   //debug: tempo
   t1 = Time();

	// Limpa listas de cálculos
   lisCalcI->Clear();
   lisCalcV->Clear();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Limpa listas: " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   // Salva o patamar diário para execução da simulação
	this->patamar = patamar;

   //debug: tempo
   t1 = Time();

   if(Inicial)
   	DSS->Start();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("DSS->Start " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   AnsiString comando = "";
   if(Inicial)
   {
      //debug: tempo
      t1 = Time();

      comando = "compile (" + caminhoDSS + ")";
      DSS->WriteCommand(comando);

      //debug: tempo
      t2 = Time();
      delta = t2 - t1;
      delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
      deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
      lisaux->Add("Write command - caminhoDSS " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));
   }

   //debug: tempo
   t1 = Time();

   comando = "set number=" + AnsiString(patamar);
   DSS->WriteCommand(comando);

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Write command - SET NUMBER " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   //debug: tempo
   t1 = Time();

	if(faseDefeito == "ABN" || faseDefeito == "BCN" || faseDefeito == "ACN" || faseDefeito == "CAN")
   {
   	comando = AnsiString(GetComandoFalta_2FT(1, faseDefeito, idBarraCurto, Rf, Inicial));
		DSS->WriteCommand(comando);
      comando = AnsiString(GetComandoFalta_2FT(2, faseDefeito, idBarraCurto, Rf, Inicial));
      DSS->WriteCommand(comando);
	}
	else
   {
		comando = AnsiString(GetComandoFalta(faseDefeito, idBarraCurto, Rf, Inicial));
		DSS->WriteCommand(comando);
   }

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Write command - new fault " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   //debug: tempo
   t1 = Time();

   DSS->Solve();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Solve " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));


//      comando = "show current elements";
//      DSS->WriteCommand(comando);
//      comando = "show voltage ln nodes";
//      DSS->WriteCommand(comando);

   //debug: tempo
   t1 = Time();

   // Resgata V e I calculadas nos mesmos pontos de medição. Salva em listas.
   DSS->GetVoltages(lisIDBarrasMonitoradas, lisCalcV);
   DSS->GetCurrents(lisCodTrechosMonitorados, lisCalcI);

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("get voltages e currents " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

//   //debug
//   lisaux->SaveToFile("c:\\users\\user\\desktop\\TemposTestaCurtoRf.txt");
//   delete lisaux;

//      //debug - verificação dos valores das tensões e das correntes nas ligações
//      String valor;
//      for(int i=0; i<lisCalcV->Count; i++)
//      {
//			valor = lisCalcV->Strings[i];
//         int b = 0;
//      }
//      for(int i=0; i<lisCalcI->Count; i++)
//      {
//			valor = lisCalcI->Strings[i];
//         int c = 0;
//      }

   delete lisaux; lisaux = NULL;
}
//---------------------------------------------------------------------------
// faseDefeito: A, B, C, AB, BC, AC, ABC, ABN, etc
void __fastcall TTryFault::TestaCurtoRf_GeraDefeitos(int patamar, int idBarraCurto, String faseDefeito, double Rf, bool Inicial)
{
	//debug
   TStringList* lisaux = new TStringList();
   unsigned short hora, minuto, segundo, mseg;
   TTime t1;
   TTime t2;
   TTime delta;
   double deltaT;


   //debug: tempo
   t1 = Time();

	// Limpa listas de cálculos
   lisCalcI->Clear();
   lisCalcV->Clear();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Limpa listas: " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));


   // Salva o patamar diário para execução da simulação
	this->patamar = patamar;

   //debug: tempo
   t1 = Time();

   if(Inicial)
   	DSS->Start();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("DSS->Start " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   AnsiString comando = "";
   if(Inicial)
   {
      //debug: tempo
      t1 = Time();

      comando = "compile (" + caminhoDSS + ")";
      DSS->WriteCommand(comando);

      //debug: tempo
      t2 = Time();
      delta = t2 - t1;
      delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
      deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
      lisaux->Add("Write command - caminhoDSS " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));
   }

   //debug: tempo
   t1 = Time();

   comando = "set number=" + AnsiString(patamar);
   DSS->WriteCommand(comando);

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Write command - SET NUMBER " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

   //debug: tempo
   t1 = Time();

   if(faseDefeito == "ABN" || faseDefeito == "BCN" || faseDefeito == "ACN")
   {
   	comando = AnsiString(GetComandoFalta_2FT(1, faseDefeito, idBarraCurto, Rf, Inicial));
		DSS->WriteCommand(comando);
      comando = AnsiString(GetComandoFalta_2FT(2, faseDefeito, idBarraCurto, Rf, Inicial));
      DSS->WriteCommand(comando);
   }
   else
   {
		comando = AnsiString(GetComandoFalta(faseDefeito, idBarraCurto, Rf, Inicial));
		DSS->WriteCommand(comando);
   }

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Write command - new fault " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));


   //debug: tempo
   t1 = Time();


   DSS->Solve();

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("Solve " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));


//      comando = "show current elements";
//      DSS->WriteCommand(comando);
//      comando = "show voltage ln nodes";
//      DSS->WriteCommand(comando);

   //debug: tempo
   t1 = Time();

   // Resgata V e I calculadas nos mesmos pontos de medição. Salva em listas.
   DSS->GetVoltages_GeraDefeitos(lisIDBarrasMonitoradas, lisCalcV);
   DSS->GetCurrents_GeraDefeitos(lisCodTrechosMonitorados, lisCalcI);

   //debug: tempo
   t2 = Time();
   delta = t2 - t1;
   delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
   deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
   lisaux->Add("get voltages e currents " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));

//   //debug
//   lisaux->SaveToFile("c:\\users\\user\\desktop\\TemposTestaCurtoRf.txt");
//   delete lisaux;

//      //debug - verificação dos valores das tensões e das correntes nas ligações
//      String valor;
//      for(int i=0; i<lisCalcV->Count; i++)
//      {
//			valor = lisCalcV->Strings[i];
//         int b = 0;
//      }
//      for(int i=0; i<lisCalcI->Count; i++)
//      {
//			valor = lisCalcI->Strings[i];
//         int c = 0;
//      }

   delete lisaux; lisaux = NULL;
}
//---------------------------------------------------------------------------
//eof
