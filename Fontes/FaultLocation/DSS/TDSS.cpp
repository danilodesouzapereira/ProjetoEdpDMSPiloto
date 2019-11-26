//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TDSS.h"
#include "TDSSBarra.h"
#include "TDSSLigacao.h"
#include "DSSEnums.h"
#include "..\Auxiliares\FuncoesFL.h"
#include <complex>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TDSS::TDSS()
{
	// Carrega a DLL
	if((OpenDSSDirect = LoadLibrary(L"OpenDSSDirect.dll")) == NULL)
	{
		ShowMessage(L"Cannot load dll.");
		return;
	}

   // Cria listas
   lisDSSBarras = new TList();
   lisDSSLigacoes = new TList();
}
//---------------------------------------------------------------------------
__fastcall TDSS::~TDSS()
{
	// Libera a DLL
	FreeLibrary(OpenDSSDirect);

	// Destroi objetos
   if(lisDSSBarras)
   {
      for(int i=lisDSSBarras->Count-1; i>=0; i--) delete(lisDSSBarras->Items[i]);
      delete lisDSSBarras; lisDSSBarras = NULL;
   }
   if(lisDSSLigacoes)
   {
      for(int i=lisDSSLigacoes->Count-1; i>=0; i--) delete(lisDSSLigacoes->Items[i]);
      delete lisDSSLigacoes; lisDSSLigacoes = NULL;
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::AjustaCorrentes_ModFase(TStringList* calcI)
{
	String val;
   double valReal, valImag, valModulo;
   std::complex<double> fasor;

   // Verificação
   if(calcI == NULL) return;

   // Obtém os valores dos módulos
	for(int i=0; i<calcI->Count/2; i++)
   {
		val = calcI->Strings[2*i];   valReal = val.ToDouble();
      val = calcI->Strings[2*i+1]; valImag = val.ToDouble();
		fasor = std::complex<double> (valReal, valImag);

		valModulo = sqrt(valReal*valReal + valImag*valImag);

      calcI->Delete(2*i);
      calcI->Insert(2*i, String(valModulo));
   }
   const int limite = calcI->Count/2;
   for(int i=0; i<limite; i++)
   {
		calcI->Delete(calcI->Count-1-i);
   }

   // Remove metade da lista (medições de corrente no terminal oposto)
   const int limite2 = calcI->Count/2;
   for(int i=0; i<limite2; i++)
   {
		calcI->Delete(calcI->Count-1-i);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::AjustaCorrentes(TStringList* calcI)
{
	String val;
   double valReal, valImag, valModulo;

   // Verificação
   if(calcI == NULL) return;

   // Obtém os valores dos módulos
	for(int i=0; i<calcI->Count/2; i++)
   {
		val = calcI->Strings[2*i];
      valReal = val.ToDouble();
      val = calcI->Strings[2*i+1];
      valImag = val.ToDouble();

		valModulo = sqrt(valReal*valReal + valImag*valImag);

      calcI->Delete(2*i);
      calcI->Insert(2*i, String(valModulo));
   }
   const int limite = calcI->Count/2;
   for(int i=0; i<limite; i++)
   {
		calcI->Delete(calcI->Count-1-i);
   }

   // Remove metade da lista (medições de corrente no terminal oposto)
   const int limite2 = calcI->Count/2;
   for(int i=0; i<limite2; i++)
   {
		calcI->Delete(calcI->Count-1-i);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::AjustaTensoes(TStringList* calcV)
{
	int cont;
   double valReal, valImag, valModulo;
	String linhaFinal, val;
	TStringList* lisAux = new TStringList();

   for(int i=0; i<calcV->Count; i++)
   {
		String linhaV = calcV->Strings[i];

      linhaFinal = "";
		cont = CSVCamposCount(linhaV, ";");
      for(int j=0; j<cont/2; j++)
      {
      	val = GetCampoCSV(linhaV, 2*j, ";");
         valReal = val.ToDouble();
         val = GetCampoCSV(linhaV, 2*j+1, ";");
         valImag = val.ToDouble();

         valModulo = sqrt(valReal*valReal + valImag*valImag);
         linhaFinal += String(valModulo) + ";";
      }
      lisAux->Add(linhaFinal);
   }

   calcV->Clear();
   for(int i=0; i<lisAux->Count; i++)
   {
		String linha = lisAux->Strings[i];
      calcV->Add(linha);
   }

   // Destroi lista auxiliar
   delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::AjustaTensoes_ModFase(TStringList* calcV)
{
	int cont;
   double valReal, valImag, valModulo, valFase;
   std::complex<double> fasor;
	String linhaFinal, val;
	TStringList* lisAux = new TStringList();

   for(int i=0; i<calcV->Count; i++)
   {
		String linhaV = calcV->Strings[i];

      linhaFinal = "";
		cont = CSVCamposCount(linhaV, ";");
      for(int j=0; j<cont/2; j++)
      {
      	val = GetCampoCSV(linhaV, 2*j, ";");   valReal = val.ToDouble();
         val = GetCampoCSV(linhaV, 2*j+1, ";"); valImag = val.ToDouble();
         valModulo = sqrt(valReal*valReal + valImag*valImag);
			fasor = std::complex<double>(valReal, valImag);
			valFase = std::arg(fasor) * 180. / M_PI;

         linhaFinal += String(valModulo) + ";";
         linhaFinal += String(valFase) + ";";
      }
      lisAux->Add(linhaFinal);
   }

   calcV->Clear();
   for(int i=0; i<lisAux->Count; i++)
   {
		String linha = lisAux->Strings[i];
      calcV->Add(linha);
   }

   // Destroi lista auxiliar
   delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::ClearAll()
{
	// Obtém a interface
	if((DSSI = (pfDSSI)GetProcAddress(OpenDSSDirect, "DSSI")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}

	// DSS.ClearAll = 1
	DSSI(1,0);
}
//---------------------------------------------------------------------------
void __fastcall TDSS::ExecutaFaultStudy()
{
 	AnsiString comando;

   comando = "solve";
   WriteCommand(comando);
	comando = "set mode=faultstudy";
   WriteCommand(comando);
   comando = "solve";
   WriteCommand(comando);
   comando = "export faultstudy";
   WriteCommand(comando);
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GeraDSSLigacoes(TStringList* lisLinesNames)
{
	String codigoLiga;
   TDSSLigacao* dssLiga = NULL;

	for(int i=0; i<lisLinesNames->Count; i++)
   {
		codigoLiga = lisLinesNames->Strings[i];

      if((dssLiga = ProcuraDSSLigacao(codigoLiga)) == NULL)
      {
			dssLiga = new TDSSLigacao();
         dssLiga->SetCodigo(codigoLiga);
         lisDSSLigacoes->Add(dssLiga);
         dssLiga->SetIndice_VetorLigacoes(i);
      }
   }
}
//---------------------------------------------------------------------------
int __fastcall TDSS::GetCircuitNumNodes()
{
    // Obtém a interface
	if((CircuitI = (pfCircuitI)GetProcAddress(OpenDSSDirect, "CircuitI")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return 0;
	}

	// Circuit.NumNodes = 2
	int resp = CircuitI(2, 0);

	return resp;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetCurrents_GeraDefeitos(TStringList* lisCodLigacoes, TStringList* calcI)
{
	int cont = 0;
   int resp, indice = 0;
	char* lineCode;
   String valor, linhaFinal;
   double iReal, iImag, iModulo, iFase;
   TStringList* valoresCalcI;
   TList* lisMonCorrente = new TList();
  	std::complex<double> fasor;

   struct monCorrente
   {
		int indiceLiga;
      TStringList* calcI;
      ~monCorrente()
      {
         if(calcI) delete calcI;
      }
   };


	resp = SetActiveLine(0);
	while(resp > 0 && cont < lisCodLigacoes->Count)
   {
      lineCode = GetLineCode();
      // Verifica o código da ligação
      if(lisCodLigacoes->IndexOf(lineCode) < 0)
		{
	      resp = SetActiveLine(1);
         continue;
      }
      else
      {
         indice = lisCodLigacoes->IndexOf(lineCode);
      }

      // Pega os cálculos de correntes
      valoresCalcI = new TStringList();
      GetLineCurrents(valoresCalcI);

      // Guarda resultados em estrutura
      monCorrente* monCorr = new monCorrente();
      monCorr->indiceLiga = indice;
      monCorr->calcI = valoresCalcI;
      // Salva estrutura em lista
      lisMonCorrente->Add(monCorr);

      // Comando Next (próxima line)
      resp = SetActiveLine(1);
      // Incrementa número de ligações consideradas
      cont += 1;
   }

   // Salva as correntes na lista externa, na ordem correta
   for(int i=0; i<lisCodLigacoes->Count; i++)
   {
		for(int j=0; j<lisMonCorrente->Count; j++)
      {
	      linhaFinal = "";
			monCorrente* monCorr = (monCorrente*) lisMonCorrente->Items[j];
         if(monCorr->indiceLiga == i)
         {
            for(int k=0; k<monCorr->calcI->Count/4; k++)
            {
            	valor = monCorr->calcI->Strings[2*k];   iReal = valor.ToDouble();
               valor = monCorr->calcI->Strings[2*k+1]; iImag = valor.ToDouble();

               iModulo = sqrt(iReal*iReal + iImag*iImag);
               fasor = std::complex<double> (iReal, iImag);
               iFase = std::arg(fasor) * 180. / M_PI;

               linhaFinal += String(iModulo) + ";";
               linhaFinal += String(iFase) + ";";
            }
				calcI->Add(linhaFinal);
         }
      }
   }

   // Destroi objetos e lista
   for(int i=lisMonCorrente->Count-1; i>=0; i--) delete lisMonCorrente->Items[i];
   delete lisMonCorrente; lisMonCorrente = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetCurrents_QualimetroRef(String codLigacaoQualimetroRef, TStringList* lisCalcI_qualRef)
{
   int resp, indice = 0;
	char* lineCode;
   String valor, linhaFinal;

	resp = SetActiveLine(0);
	while(resp > 0)
   {
      lineCode = GetLineCode();

      // Verifica o código da ligação
      if(LowerCase(lineCode) == LowerCase(codLigacaoQualimetroRef))
      {
         // Pega os cálculos de correntes
         GetLineCurrents(lisCalcI_qualRef);
         break;
      }

      // Comando Next (próxima line)
      resp = SetActiveLine(1);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetCurrents(TStringList* lisCodLigacoes, TStringList* calcI)
{
	int cont = 0;
   int resp, indice = 0;
	char* lineCode;
   String valor, linhaFinal;
   double iReal, iImag, iModulo;
   TStringList* valoresCalcI;
   TList* lisMonCorrente = new TList();

   struct monCorrente
   {
		int indiceLiga;
      TStringList* calcI;
      ~monCorrente()
      {
         if(calcI) delete calcI;
      }
   };


	resp = SetActiveLine(0);
	while(resp > 0 && cont < lisCodLigacoes->Count)
   {
      lineCode = GetLineCode();
      // Verifica o código da ligação
      if(lisCodLigacoes->IndexOf(lineCode) < 0)
		{
	      resp = SetActiveLine(1);
         continue;
      }
      else
      {
         indice = lisCodLigacoes->IndexOf(lineCode);
      }

      // Pega os cálculos de correntes
      valoresCalcI = new TStringList();
      GetLineCurrents(valoresCalcI);

      // Guarda resultados em estrutura
      monCorrente* monCorr = new monCorrente();
      monCorr->indiceLiga = indice;
      monCorr->calcI = valoresCalcI;
      // Salva estrutura em lista
      lisMonCorrente->Add(monCorr);

      // Comando Next (próxima line)
      resp = SetActiveLine(1);
      // Incrementa número de ligações consideradas
      cont += 1;
   }

   // Salva as correntes na lista externa, na ordem correta
   for(int i=0; i<lisCodLigacoes->Count; i++)
   {
		for(int j=0; j<lisMonCorrente->Count; j++)
      {
	      linhaFinal = "";
			monCorrente* monCorr = (monCorrente*) lisMonCorrente->Items[j];
         if(monCorr->indiceLiga == i)
         {
            for(int k=0; k<monCorr->calcI->Count/4; k++)
            {
            	valor = monCorr->calcI->Strings[2*k];
               iReal = valor.ToDouble();
               valor = monCorr->calcI->Strings[2*k+1];
               iImag = valor.ToDouble();

               iModulo = sqrt(iReal*iReal + iImag*iImag);
               linhaFinal += String(iModulo) + ";";
            }
				calcI->Add(linhaFinal);
         }
      }
   }

   // Destroi objetos e lista
   for(int i=lisMonCorrente->Count-1; i>=0; i--) delete lisMonCorrente->Items[i];
   delete lisMonCorrente; lisMonCorrente = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetCurrents(TStringList* calcI)
{
	int resp, indice = 0;
	char* lineCode;

	resp = SetActiveLine(0);
	while(resp > 0)
   {
      lineCode = GetLineCode();
      GetLineCurrents(calcI);
      resp = SetActiveLine(1);
   }
   lineCode = lineCode;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetCurrentsDoubleArray(int NumValores, Variant* allCurrents, TStringList* lisI)
{
	double num = 0.;

	// Now read the elements of the array.
   int a = VarArrayLowBound(allCurrents,1);
   int b = VarArrayHighBound(allCurrents,1);
	for (int i = VarArrayLowBound(allCurrents, 1); i <= VarArrayHighBound(allCurrents, 1); i++)
	{
		// Put the element I at index I.
		Variant valor = VarArrayGet(allCurrents, &i, 0);
		num = valor;
      lisI->Add(String(num));
	}
	a = a;
	b = b;
	num = num;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetDSSBarras()
{
	String codigoNode = "";
   String linhaFinal = "";
   String linha = "";
	TStringList* lisNodeNames = new TStringList();

	if(lisNodeNames == NULL) return;
  	// Obtém a interface
	if((CircuitV = (pfCircuitV)GetProcAddress(OpenDSSDirect, "CircuitV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}
   int numNodes = GetCircuitNumNodes();

   // Circuit.AllNodeNames = 10
	int bounds[2] = {0, numNodes-1};
	Variant allNodeNames = VarArrayCreate(bounds, 1, varOleStr);
	CircuitV(10, &allNodeNames, 0);

   GetNodeNamesArray(&allNodeNames, lisNodeNames);

   GetFases(lisNodeNames);
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetDSSBarras_Codigos(TStringList* lisLinhas)
{
   TList* lisDSSBarras;
	String codigo, linha;
   int fases;

	if(lisLinhas == NULL) return;

   lisDSSBarras = GetLisDSSBarras();
   for(int i=0; i<lisDSSBarras->Count; i++)
   {
		TDSSBarra* dssBarra = (TDSSBarra*) lisDSSBarras->Items[i];

      codigo = dssBarra->GetCodigo();
      fases = dssBarra->GetFases();

      linha = "Barra: " + codigo + " fases: " + String(fases);

      lisLinhas->Add(linha);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetDSSBarras_IDs(TStringList* lisLinhas)
{
   TList* lisDSSBarras;
	String linha;
   int fases, ID;

	if(lisLinhas == NULL) return;

//   lisDSSBarras = GetLisDSSBarras();
   for(int i=0; i<lisDSSBarras->Count; i++)
   {
		TDSSBarra* dssBarra = (TDSSBarra*) lisDSSBarras->Items[i];

      ID = dssBarra->GetID();
      fases = dssBarra->GetFases();

      linha = "Barra_ID: " + String(ID) + " fases: " + String(fases);

      lisLinhas->Add(linha);
   }
}
//---------------------------------------------------------------------------
TList* __fastcall TDSS::GetDSSBarras_Monitoradas(TStringList* lisCodBarras)
{
	int IDbarra;
	String IDbarra_str;
	TList* lisDSSBarrasMonitoradas = new TList();

   for(int i=0; i<lisCodBarras->Count; i++)
   {
		IDbarra_str = lisCodBarras->Strings[i];
		IDbarra = IDbarra_str.ToInt();

		for(int j=0; j<lisDSSBarras->Count; j++)
      {
			TDSSBarra* dssBarra = (TDSSBarra*) lisDSSBarras->Items[j];
         if(dssBarra->GetID() != IDbarra) continue;

         lisDSSBarrasMonitoradas->Add(dssBarra);
         break;
      }
   }

   return lisDSSBarrasMonitoradas;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetDSSLigacoes_Codigos(TStringList* lisLinhas)
{
   TList* lisDSSBarras;
	String linha, codigo;
   int fases, ID;

	if(lisLinhas == NULL) return;

   for(int i=0; i<lisDSSLigacoes->Count; i++)
   {
		TDSSLigacao* dssLiga = (TDSSLigacao*) lisDSSLigacoes->Items[i];

      codigo = dssLiga->GetCodigo();

      linha = "Ligação_Código: " + codigo;

      lisLinhas->Add(linha);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetDSSLigacoes()
{
	TStringList* lisLinesNames = new TStringList();

  	// Obtém a interface
	if((LinesV = (pfLinesV)GetProcAddress(OpenDSSDirect, "LinesV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}
   int numNodes = GetCircuitNumNodes();

   // Circuit.AllNodeNames = 10
	int bounds[2] = {0, numNodes-1};
	Variant allNames = VarArrayCreate(bounds, 1, varOleStr);
	LinesV(0, &allNames);

   GetLinesNamesArray(&allNames, lisLinesNames);

   GeraDSSLigacoes(lisLinesNames);

	GetFasesLigacoes();
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetFasesLigacoes()
{
	char *codLiga, *codBus1, *codBus2;
   int indice = 0, resp;
   String codLigaStr, codBus1Str, codBus2Str;

   // Utiliza os comandos First e Next para setar a line ativa
	resp = SetActiveLine(0);
   indice = 0;
	while(resp > 0)
   {
      codBus1 = GetLineBus(0);
      codBus1Str = String(codBus1);
      codBus2 = GetLineBus(1);
      codBus2Str = String(codBus2);

      // Obtém o objeto dssLiga (na mesma sequência do OpenDSS) para setar as fases
   	TDSSLigacao* dssLiga = (TDSSLigacao*) lisDSSLigacoes->Items[indice++];
		dssLiga->SetFases(codBus1Str, codBus2Str);

      // Chama line seguinte (set active)
      resp = SetActiveLine(1);
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetFases(TStringList* lisNodeNames)
{
	int comp, j, barraID;
	String str, IDstr, indiceFase;
   TDSSBarra* dssBarra = NULL;

	if(lisNodeNames == NULL) return;

	// Strings do tipo: 100.1, 100.2, 100.3, ... etc.
	for(int i=0; i<lisNodeNames->Count; i++)
   {
		str = lisNodeNames->Strings[i];

      // Pega o ID da barra e o índice de fase
      comp = str.Length();
      IDstr = "";
		for(j=1; j<=comp; j++)
      {
			if(str.SubString(j,1) == ".") break;
         else
         	IDstr += str.SubString(j,1);
      }
      indiceFase = str.SubString(j+1, comp);
      barraID = IDstr.ToInt();

      // Se a DSSBarra ainda não existe, cria uma e armazena na lista
      if((dssBarra = ProcuraDSSBarra(barraID)) == NULL)
      {
      	dssBarra = new TDSSBarra();
         dssBarra->SetIndice_VetorBarras(2*i);
         dssBarra->SetID(barraID);
    		dssBarra->SetFases(indiceFase.ToInt());
	      lisDSSBarras->Add(dssBarra);
      }
      else
      {
       	dssBarra->SetFases(indiceFase.ToInt());
      }
   }
}
//---------------------------------------------------------------------------
char* __fastcall TDSS::GetLineBus(int iBarra)
{
	char* codigo;

   if((LinesS = (pfLinesS)GetProcAddress(OpenDSSDirect, "LinesS")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return NULL;
	}
   if(iBarra == 0)
   {
      codigo = LinesS(2,0);
   }
   else if(iBarra == 1)
   {
      codigo = LinesS(4,0);
   }
   else
   	codigo = NULL;

   return codigo;
}
//---------------------------------------------------------------------------
char* __fastcall TDSS::GetLineCode()
{
   if((LinesS = (pfLinesS)GetProcAddress(OpenDSSDirect, "LinesS")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return NULL;
	}

   char* codigo = LinesS(0,0);

   return codigo;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetLineCurrents(TStringList* lisI)
{
   // Obtém a interface
	if((CktElementV = (pfCktElementV)GetProcAddress(OpenDSSDirect, "CktElementV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}

	int bounds[2] = {0, 11};
	Variant allCurrents = VarArrayCreate(bounds, 1, varDouble);
	CktElementV(3, &allCurrents, 0);

   GetCurrentsDoubleArray(12, &allCurrents, lisI);
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetLinesNamesArray(Variant* allNames, TStringList* lisLinesNames)
{
	double num = 0.;
   AnsiString linha = "";

	// Now read the elements of the array.
   int a = VarArrayLowBound(allNames,1);
   int b = VarArrayHighBound(allNames,1);
	for (int i = VarArrayLowBound(allNames, 1); i <= VarArrayHighBound(allNames, 1); i++)
	{
		// Put the element I at index I.
		Variant valor = VarArrayGet(allNames, &i, 0);
		linha = valor;
      lisLinesNames->Add(linha);
	}
}
//---------------------------------------------------------------------------
TList* __fastcall TDSS::GetLisDSSBarras()
{
   return lisDSSBarras;
}
//---------------------------------------------------------------------------
TList* __fastcall TDSS::GetLisDSSLigacoes()
{
   return lisDSSLigacoes;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetNodeNamesArray(Variant* allNodeNames, TStringList* lisNodeNames)
{
	double num = 0.;
   AnsiString linha = "";

	// Now read the elements of the array.
   int a = VarArrayLowBound(allNodeNames,1);
   int b = VarArrayHighBound(allNodeNames,1);
	for (int i = VarArrayLowBound(allNodeNames, 1); i <= VarArrayHighBound(allNodeNames, 1); i++)
	{
		// Put the element I at index I.
		Variant valor = VarArrayGet(allNodeNames, &i, 0);
		linha = valor;
      lisNodeNames->Add(linha);
	}
	a = a;
	b = b;
	num = num;
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetVoltages_GeraDefeitos(TStringList* lisCodBarras, TStringList* calcV)
{
	if(calcV == NULL) return;

	// Obtém a interface
	if((CircuitV = (pfCircuitV)GetProcAddress(OpenDSSDirect, "CircuitV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}
   int numNodes = GetCircuitNumNodes();

   // Circuit.AllBusVolts = 4
	int bounds[2] = {0, 2*numNodes-1};
	Variant allBusVolts = VarArrayCreate(bounds, 1, varDouble);
	CircuitV(4, &allBusVolts, 0);

   TList* lisDSSBarrasMonitoradas = GetDSSBarras_Monitoradas(lisCodBarras);

   GetVoltagesDoubleArray_Monitoradas(lisDSSBarrasMonitoradas, &allBusVolts, calcV);

   AjustaTensoes_ModFase(calcV); // pega apenas os módulos das tensões
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetVoltages(TStringList* lisCodBarras, TStringList* calcV)
{
	if(calcV == NULL) return;

	// Obtém a interface
	if((CircuitV = (pfCircuitV)GetProcAddress(OpenDSSDirect, "CircuitV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}
   int numNodes = GetCircuitNumNodes();

   // Circuit.AllBusVolts = 4
	int bounds[2] = {0, 2*numNodes-1};
	Variant allBusVolts = VarArrayCreate(bounds, 1, varDouble);
	CircuitV(4, &allBusVolts, 0);

   TList* lisDSSBarrasMonitoradas = GetDSSBarras_Monitoradas(lisCodBarras);
   GetVoltagesDoubleArray_Monitoradas(lisDSSBarrasMonitoradas, &allBusVolts, calcV);

   // Destroi lista
   delete lisDSSBarrasMonitoradas; lisDSSBarrasMonitoradas = NULL;

   AjustaTensoes(calcV); // pega apenas os módulos das tensões
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetVoltages(TStringList* calcV)
{
	if(calcV == NULL) return;

	// Obtém a interface
	if((CircuitV = (pfCircuitV)GetProcAddress(OpenDSSDirect, "CircuitV")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return;
	}
   int numNodes = GetCircuitNumNodes();

   // Circuit.AllBusVolts = 4
	int bounds[2] = {0, 2*numNodes-1};
	Variant allBusVolts = VarArrayCreate(bounds, 1, varDouble);
	CircuitV(4, &allBusVolts, 0);

   GetVoltagesDoubleArray(2*numNodes, &allBusVolts, calcV);
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetVoltagesDoubleArray(int NumValores, Variant* allBusVolts, TStringList* lisV)
{
	double num = 0.;

	// Now read the elements of the array.
	for (int i = VarArrayLowBound(allBusVolts, 1); i <= VarArrayHighBound(allBusVolts, 1); i++)
	{
		// Put the element I at index I.
		Variant valor = VarArrayGet(allBusVolts, &i, 0);
		num = valor;
      lisV->Add(String(num));
	}
}
//---------------------------------------------------------------------------
void __fastcall TDSS::GetVoltagesDoubleArray_Monitoradas(TList* lisDSSBarrasMonitoradas, Variant* allBusVolts, TStringList* lisV)
{
	double num = 0.;
   int indice, numFases;
   String linhaFinal;

   for(int i=0; i<lisDSSBarrasMonitoradas->Count; i++)
   {
		TDSSBarra* dssBarra = (TDSSBarra*) lisDSSBarrasMonitoradas->Items[i];

      indice = dssBarra->GetIndice_VetorBarras();
     	numFases = dssBarra->GetNumFases();

      linhaFinal = "";
      // Now read the elements of the array.
      for (int i = indice; i < indice+2*numFases; i++)
      {
         // Put the element I at index I.
         Variant valor = VarArrayGet(allBusVolts, &i, 0);
         num = valor;
         linhaFinal += String(num) + ";";
//         lisV->Add(String(num));
      }
      lisV->Add(linhaFinal);
   }

//	// Now read the elements of the array.
//	for (int i = VarArrayLowBound(allBusVolts, 1); i <= VarArrayHighBound(allBusVolts, 1); i++)
//	{
//		// Put the element I at index I.
//		Variant valor = VarArrayGet(allBusVolts, &i, 0);
//		num = valor;
//      lisV->Add(String(num));
//	}
}
//---------------------------------------------------------------------------
TDSSBarra* __fastcall TDSS::ProcuraDSSBarra(int IDbarra)
{
	TDSSBarra* dssBarra = NULL;

   for(int i=0; i<lisDSSBarras->Count; i++)
   {
		dssBarra = (TDSSBarra*) lisDSSBarras->Items[i];

      if(dssBarra->GetID() == IDbarra)
      	return dssBarra;
   }

   return NULL;
}
//---------------------------------------------------------------------------
TDSSLigacao* __fastcall TDSS::ProcuraDSSLigacao(String codLiga)
{
	TDSSLigacao* dssLiga = NULL;

   for(int i=0; i<lisDSSLigacoes->Count; i++)
   {
		dssLiga = (TDSSLigacao*) lisDSSLigacoes->Items[i];

      if(dssLiga->GetCodigo() == codLiga)
      	return dssLiga;
   }

   return NULL;
}
//---------------------------------------------------------------------------
int __fastcall TDSS::SetActiveLine(int iLine)
{
	// Obtém a interface
	if((LinesI = (pfLinesI)GetProcAddress(OpenDSSDirect, "LinesI")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return -1;
	}

	int resp = LinesI(iLine, 0);

   return resp;
}
//---------------------------------------------------------------------------
int __fastcall TDSS::Solve()
{
	// Obtém a interface
	if((SolutionI = (pfSolutionI)GetProcAddress(OpenDSSDirect, "SolutionI")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return 0;
	}

	// Solution.Solve = 0
	int resp = SolutionI(0,0);

	return resp;
}
//---------------------------------------------------------------------------
int __fastcall TDSS::Start()
{
	// Obtém a interface
	if((DSSI = (pfDSSI)GetProcAddress(OpenDSSDirect, "DSSI")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return 0;
	}

	// DSS.Start = 3
	int resp = DSSI(3,0);

	return resp;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDSS::WriteCommand(AnsiString comando)
{
	// Obtém a interface
	if((DSSPut_Command = (pfDSSPut_Command)GetProcAddress(OpenDSSDirect, "DSSPut_Command")) == NULL)
	{
		ShowMessage(L"Cannot find dll function.");
		return 0;
	}

   // Converte tipo de string
   char* charComando = new char[comando.Length() + 1];
   strcpy(charComando, comando.c_str());
   // Envia comando
	char* resp = DSSPut_Command(charComando);
   // Converte tipo da resposta
   AnsiString resposta(resp);

	return resposta;
}
//---------------------------------------------------------------------------
