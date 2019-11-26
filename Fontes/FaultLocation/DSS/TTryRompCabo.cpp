//---------------------------------------------------------------------------
#pragma hdrstop
#include "TTryRompCabo.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "TDSS.h"
#include "DSSEnums.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TTryRompCabo::TTryRompCabo()
{
	caminhoDSS = "";

	// Cria o objeto de OpenDSS
	DSS = new TDSS();

   // Cria listas
	lisIDBarrasMonitoradas = new TStringList();
	lisMedV = new TList();
	lisCalcV = new TStringList();
}
//---------------------------------------------------------------------------
__fastcall TTryRompCabo::~TTryRompCabo()
{
	// Destroi objetos
   delete DSS; DSS = NULL;
   if(lisIDBarrasMonitoradas)
   {
      for(int i=lisIDBarrasMonitoradas->Count-1; i>=0; i--) lisIDBarrasMonitoradas->Delete(i);
      delete lisIDBarrasMonitoradas; lisIDBarrasMonitoradas = NULL;
   }
	if(lisMedV)
   {
      for(int i=lisMedV->Count-1; i>=0; i--) delete(lisMedV->Items[i]);
      delete lisMedV; lisMedV = NULL;
   }
	if(lisCalcV)
   {
      for(int i=lisCalcV->Count-1; i>=0; i--) lisCalcV->Delete(i);
      delete lisCalcV; lisCalcV = NULL;
   }
}
//---------------------------------------------------------------------------
String __fastcall TTryRompCabo::Bus1Trecho(String codTrecho)
{
	String campo, campo1, campo2, linha, idBarra;
	String pathLines = dirDSS + "\\lines.dss";
	TStringList* listaLinhas = new TStringList();
	listaLinhas->LoadFromFile(pathLines);

	for(int i=0; i<listaLinhas->Count; i++)
	{
		linha = listaLinhas->Strings[i];

		campo = GetCampo(linha, 1, " ");
		if(GetSubstr(campo + " ", ".", " ") != codTrecho) continue;

		campo1 = GetCampo(linha, 2, " ");
		campo2 = GetCampo(linha, 3, " ");
		if(ContemSubstr(campo1, "Bus1") || ContemSubstr(campo1, "bus1"))
		{
			idBarra = GetSubstr(campo1 + " ", "=", " ");
			break;
		}
		else if(ContemSubstr(campo2, "Bus1") || ContemSubstr(campo2, "bus1"))
		{
			idBarra = GetSubstr(campo2 + " ", "=", " ");
			break;
		}
	}
	delete listaLinhas; listaLinhas = NULL;
	return(idBarra);
}
//---------------------------------------------------------------------------
String __fastcall TTryRompCabo::Bus2Trecho(String codTrecho)
{
	String campo, campo1, campo2, linha, idBarra;
	String pathLines = dirDSS + "\\lines.dss";
	TStringList* listaLinhas = new TStringList();
	listaLinhas->LoadFromFile(pathLines);

	for(int i=0; i<listaLinhas->Count; i++)
	{
		linha = listaLinhas->Strings[i];

		campo = GetCampo(linha, 1, " ");
		if(GetSubstr(campo + " ", ".", " ") != codTrecho) continue;

		campo1 = GetCampo(linha, 2, " ");
		campo2 = GetCampo(linha, 3, " ");
		if(ContemSubstr(campo1, "Bus2") || ContemSubstr(campo1, "bus2"))
		{
			idBarra = GetSubstr(campo1 + " ", "=", " ");
			break;
		}
		else if(ContemSubstr(campo2, "Bus2") || ContemSubstr(campo2, "bus2"))
		{
			idBarra = GetSubstr(campo2 + " ", "=", " ");
			break;
		}
	}
	delete listaLinhas; listaLinhas = NULL;
	return(idBarra);
}
//---------------------------------------------------------------------------
String __fastcall TTryRompCabo::CodigoArranjo(String codTrecho)
{
	String campo, linha, codArranjo;
	String pathLines = dirDSS + "\\lines.dss";
	TStringList* listaLinhas = new TStringList();
	listaLinhas->LoadFromFile(pathLines);

	for(int i=0; i<listaLinhas->Count; i++)
	{
		linha = listaLinhas->Strings[i];

		campo = GetCampo(linha, 1, " ");
		if(GetSubstr(campo + " ", ".", " ") != codTrecho) continue;

		campo = GetCampo(linha, 5, " ");
		if(ContemSubstr(campo, "LineCode") || ContemSubstr(campo, "linecode") || ContemSubstr(campo, "Linecode"))
		{
			codArranjo = GetSubstr(campo + " ", "=", " ");
			break;
		}
	}
	delete listaLinhas; listaLinhas = NULL;
	return(codArranjo);
}
//---------------------------------------------------------------------------
double __fastcall TTryRompCabo::CalculaErro_RompCabo()
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
String __fastcall TTryRompCabo::Trecho(int idBarraRompimento)
{
	String codTrecho, campo, linha, idBarra;
	String pathLines = dirDSS + "\\lines.dss";
	TStringList* listaLinhas = new TStringList();
	listaLinhas->LoadFromFile(pathLines);

	codTrecho = "";
	for(int i=0; i<listaLinhas->Count; i++)
	{
		linha = listaLinhas->Strings[i];
		campo = GetCampo(linha, 2, " ");
		if(!ContemSubstr(campo, "Bus1") && !ContemSubstr(campo, "bus1"))
		{
			campo = GetCampo(linha, 3, " ");
			if(!ContemSubstr(campo, "Bus1") && !ContemSubstr(campo, "bus1"))
				continue;
		}

		idBarra = GetSubstr(campo, "=", ".");
		if(idBarra.ToInt() == idBarraRompimento)
		{
			campo = GetCampo(linha, 1, " ");
			codTrecho = GetSubstr(campo + " ", ".", " ");
			break;
		}
	}
	return(codTrecho);
}
//---------------------------------------------------------------------------
String __fastcall TTryRompCabo::GetComandoRompimento_1fase(String faseRompimento, String codigoTrecho, String idBarraRomp1, String idBarraRomp2, String codigoArranjo)
{
	String comando = "";
	String fasesBarra1 = "." + GetSubstr(idBarraRomp1 + " ", ".", " ");
	String idBarra1 = GetSubstr(" " + idBarraRomp1, " ", ".");
	String fasesBarra2 = "." + GetSubstr(idBarraRomp2 + " ", ".", " ");
	String idBarra2 = GetSubstr(" " + idBarraRomp2, " ", ".");

	// Faz backup das condições originais da barra
	backup_fasesBarra1 = fasesBarra1;
	backup_fasesBarra2 = fasesBarra2;
	backup_codTrecho = codigoTrecho;
	backup_codBarra1 = idBarra1;
	backup_codBarra2 = idBarra2;
	backup_codArranjo = codigoArranjo;

	if(faseRompimento == "A")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux2F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".2.3";
			comando += " bus2=" + idBarra2 + ".2.3";
		}
		else if(fasesBarra1 == ".1.3" && fasesBarra2 == ".1.3")
		{
			comando += " bus1=" + idBarra1 + ".3";
			comando += " bus2=" + idBarra2 + ".3";
		}
		else if(fasesBarra1 == ".1.2" && fasesBarra2 == ".1.2")
		{
			comando += " bus1=" + idBarra1 + ".2";
			comando += " bus2=" + idBarra2 + ".2";
		}
		else
		{
			comando = "";
		}
	}
	else if(faseRompimento == "B")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux2F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".1.3";
			comando += " bus2=" + idBarra2 + ".1.3";
		}
		else if(fasesBarra1 == ".2.3" && fasesBarra2 == ".2.3")
		{
			comando += " bus1=" + idBarra1 + ".3";
			comando += " bus2=" + idBarra2 + ".3";
		}
		else if(fasesBarra1 == ".1.2" && fasesBarra2 == ".1.2")
		{
			comando += " bus1=" + idBarra1 + ".1";
			comando += " bus2=" + idBarra2 + ".1";
		}
		else
		{
			comando = "";
		}
	}
	else if(faseRompimento == "C")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux2F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".1.2";
			comando += " bus2=" + idBarra2 + ".1.2";
		}
		else if(fasesBarra1 == ".2.3" && fasesBarra2 == ".2.3")
		{
			comando += " bus1=" + idBarra1 + ".2";
			comando += " bus2=" + idBarra2 + ".2";
		}
		else if(fasesBarra1 == ".1.3" && fasesBarra2 == ".1.3")
		{
			comando += " bus1=" + idBarra1 + ".1";
			comando += " bus2=" + idBarra2 + ".1";
		}
		else
		{
			comando = "";
		}
	}
	else if(faseRompimento == "AB")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux1F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".3";
			comando += " bus2=" + idBarra2 + ".3";
		}
		else
		{
			comando = "";
		}
	}
	else if(faseRompimento == "BC")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux1F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".1";
			comando += " bus2=" + idBarra2 + ".1";
		}
		else
		{
			comando = "";
		}
	}
	else if(faseRompimento == "CA")
	{
		// Altera o arranjo do trecho, para que seja bifásico
		comando = "line." + codigoTrecho + ".linecode=Aux1F";

		if(fasesBarra1 == ".1.2.3" && fasesBarra2 == ".1.2.3")
		{
			comando += " bus1=" + idBarra1 + ".2";
			comando += " bus2=" + idBarra2 + ".2";
		}
		else
		{
			comando = "";
		}
	}

//	if(faseRompimento == "A")
//	{
//		if(fasesBarra == ".1.2.3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4.2.3";
//			comando = "line." + codigoTrecho + ".linecode=Aux2F";
//			comando += "";
//		}
//		else if(fasesBarra == ".1.2")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4.2";
//		}
//		else if(fasesBarra == ".1")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4";
//		}
//	}
//	else if(faseRompimento == "B")
//	{
//		if(fasesBarra == ".1.2.3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".1.4.3";
//		}
//		else if(fasesBarra == ".2.3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4.3";
//		}
//		else if(fasesBarra == ".2")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4";
//		}
//	}
//	else if(faseRompimento == "C")
//	{
//		if(fasesBarra == ".1.2.3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".1.2.4";
//		}
//		else if(fasesBarra == ".1.3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".1.4";
//		}
//		else if(fasesBarra == ".3")
//		{
//			comando = "line." + codigoTrecho + ".bus1=";
//			comando += idBarra + ".4";
//		}
//	}
	return(comando);
}
//---------------------------------------------------------------------------
void __fastcall TTryRompCabo::IniciaBarras()
{
   AnsiString comando;
   int patamar = 1, res;

	// Inicializa o DSS para pegar as identificações das barras e dos trechos
   res = DSS->Start();

   // Verificação
   if(res != 1) return;

   comando = "compile (" + caminhoDSS + ")";
   DSS->WriteCommand(comando);

   // DSS resgata as identificações das barras
   DSS->GetDSSBarras();
}
//---------------------------------------------------------------------------
void __fastcall TTryRompCabo::SetCaminhoDSS(String caminhoDSS)
{
	this->caminhoDSS = caminhoDSS + "\\Master.dss";
	dirDSS = caminhoDSS;
}
//---------------------------------------------------------------------------
void __fastcall TTryRompCabo::SetMedicoesBarras(TStringList* lisMedV)
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
      strIDbarra = GetCampo(medV, 0, ";");

      medTensao = new medicaoV();
      medTensao->IDbarra = strIDbarra.ToInt();
      medTensao->numFases = comp-1;
      for(int j=0; j<comp-1; j++)
      {
			strV = GetCampo(medV, j+1, ";");
         medTensao->V[j] = strV.ToDouble();
      }
      this->lisMedV->Add(medTensao);

		// Adiciona ID de barra monitorada
      lisIDBarrasMonitoradas->Add(strIDbarra);
   }
}
//---------------------------------------------------------------------------
void __fastcall TTryRompCabo::TestaRompCabo(int patamar, int idBarraRompimento, String faseRompimento, bool Inicial)
{
	String codigoTrecho, bus1TrechoRomp, bus2TrechoRomp, codigoArranjo;
	AnsiString comando = "";

	// Limpa listas de cálculos
	lisCalcV->Clear();

	this->patamar = patamar;

	if(Inicial)
		DSS->Start();

	comando = "compile (" + caminhoDSS + ")";
	DSS->WriteCommand(comando);
	comando = "set number=" + AnsiString(patamar);
	DSS->WriteCommand(comando);

	// Pega parâmetros para construir a string do comando
	codigoTrecho = Trecho(idBarraRompimento);
	bus1TrechoRomp = Bus1Trecho(codigoTrecho);
	bus2TrechoRomp = Bus2Trecho(codigoTrecho);
	codigoArranjo = CodigoArranjo(codigoTrecho);

	// Constroi a string do comando de teste de rompimento de cabo
	comando = AnsiString(GetComandoRompimento_1fase(faseRompimento, codigoTrecho, bus1TrechoRomp, bus2TrechoRomp, codigoArranjo));

   // Executa o teste de rompimento de cabo
	if(comando != "")
	{
		DSS->WriteCommand(comando);
   }
	DSS->Solve();

	// Resgata V calculadas nos mesmos pontos de medição. Salva em listas.
	DSS->GetVoltages(lisIDBarrasMonitoradas, lisCalcV);
}
//---------------------------------------------------------------------------
void __fastcall TTryRompCabo::RestauraRompCabo()
{
	AnsiString comandoRestaura;
	String comando;

	comando = "line." + backup_codTrecho + ".LineCode=" + backup_codArranjo;
	comando += " bus1=" + backup_codBarra1 + backup_fasesBarra1;
	comando += " bus2=" + backup_codBarra2 + backup_fasesBarra2;
	comandoRestaura = AnsiString(comando);

	if(comandoRestaura != "")
	{
		DSS->WriteCommand(comandoRestaura);
	}
}
//---------------------------------------------------------------------------
