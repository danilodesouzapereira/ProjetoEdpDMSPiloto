//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//   Essa classe executa o OpenDSS no modo Fault Study, permitindo a comparação
//   das correntes de curto-circuito em todas as barras (nós) do alimentador
//   com o módulo da corrente de defeito (apenas para curtos que não envolvem,
//   ou têm pouca influência, da resistência de defeito - 3F e 2F).
//
//   CHAMADAS DOS MÉTODOS:
//
//   1- Método Inicializa(...), que recebe como parâmetros o path dos DSS e o
//      código do alimentador.
//   2- Método Executa(), que escreve comando de set faultstudy, solve e export
//      faultstudy para um arquivo CSV, no mesmo diretório dos arquivos DSS do
//      evento corrente.
//   3- Método AnalisaArquivoFaultStudy(), que analisa o arquivo CSV gerado a
//      partir do FaultStudy, gerando objetos de resultados de curtos (3F, 1F e 2F
//      para cada barra, ou nó, do circuito).
//   4- Método EliminaPossiveisSolucoes(areaBusca), que elimina alguns resultados
//      de curto, restringindo a lista de resultados àqueles que ocorrem na área
//      de busca previamente determinada.
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TConfiguracoes.h"
#include "..\Auxiliares\TDados.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Equipamentos\TEqptoCampo.h"
#include "..\Equipamentos\TChaveMonitorada.h"
#include "..\Equipamentos\TSensor.h"
#include "..\TAreaBusca.h"
#include "..\TClassificacao.h"
#include "..\TThreadFaultLocation.h"
#include "TFaultStudyFL.h"
#include "TDSS.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <StrUtils.hpp>
#include <Character.hpp>
//---------------------------------------------------------------------------
__fastcall TFaultStudyFL::TFaultStudyFL(TAreaBusca* areaBusca)
{
	// Salva parâmetros
   this->areaBusca = areaBusca;

	// Cria o objeto de OpenDSS
   DSS = new TDSS();

   // Inicializa parâmetros
   CaminhoDSS = "";
   CodigoAlimentador = "";
	NumMaxSolucoes = 0;
	Ifalta = 0.;
	configGerais = NULL;

   // Cria listas de strings
   listaLinhasFaultStudy = new TStringList();

   // Cria listas
   lisResCurto = new TList();

   // Inicializa lista com as barras (VTBarra) da área de busca
   lisBarrasAreaBusca = new TList();
}
//---------------------------------------------------------------------------
__fastcall TFaultStudyFL::~TFaultStudyFL()
{
	// Destroi objetos
	if(listaLinhasFaultStudy) {delete listaLinhasFaultStudy; listaLinhasFaultStudy = NULL;}
	if(lisBarrasAreaBusca) {delete lisBarrasAreaBusca; lisBarrasAreaBusca = NULL;}
   if(lisResCurto) {delete lisResCurto; lisResCurto = NULL;}
}
//---------------------------------------------------------------------------
String __fastcall TFaultStudyFL::AjustaLinha(String linha)
{
   int comp;
	String substr, linhaFinal = "";

   comp = linha.Length();
   for(int i=1; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
      if(substr == " ") continue;
      if(substr == ",")
      {
	      linhaFinal += ";";
         continue;
      }
      linhaFinal += substr;
   }

   return linhaFinal;
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::ConsideraAreaBusca()
{
	int idBarra;
	resCurto* res;
//	TList* lisBarrasAreaBusca = new TList();
   VTBarra* barra = NULL;

//	areaBusca->GetAreaBusca_Barras(lisBarrasAreaBusca);

   for(int i=lisResCurto->Count-1; i>=0; i--)
   {
   	// pega um resultado de curto qualquer
		res = (resCurto*) lisResCurto->Items[i];
      idBarra = res->IDbarra;

      // verifica se a área de busca contém a barra da posição do curto
      for(int j=0; j<lisBarrasAreaBusca->Count; j++)
      {
			barra = (VTBarra*) lisBarrasAreaBusca->Items[j];
         if(barra->Id == idBarra)
         	break;
         else
         	barra = NULL;
      }

      // se a área de busca não tem a posição do curto, elimina possibilidade
      if(barra == NULL)
      {
         lisResCurto->Remove(res);
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::ExecutaFaultStudy()
{
   int res = DSS->Start();
   AnsiString caminhoMaster = CaminhoDSS + "\\Master.dss";
   AnsiString comando = "";

   if(res == 1)
   {
      comando = "compile (" + caminhoMaster + ")";
      DSS->WriteCommand(comando);
      DSS->Solve();
      comando = "set mode=faultstudy";
      DSS->WriteCommand(comando);
      DSS->Solve();
      comando = "export faultstudy";
      DSS->WriteCommand(comando);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::ExecutaLocalizFaultStudy(TClassificacao* classificacao, TDados* dados)
{
	AnsiString comando, caminhoMaster;
	int idBarra, numFases, resp;
	resCurto* res;
	String linhaArq, linhaAjustada, arqFaultStudy;
	TChaveMonitorada* chaveMon;
	TList* lisEqptosCampo;
   TStringList* arquivo;
	TStringList* lisMedicoesTrechos;
	VTBarra* barra = NULL;

   // ::::::::::::::::::::::::::::::::
	// Comando para executar o FaultStudy no OpenDSS
	// ::::::::::::::::::::::::::::::::
   resp = DSS->Start();
   caminhoMaster = CaminhoDSS + "\\Master.dss";
	comando = "";
	if(resp == 1)
   {
      comando = "compile (" + caminhoMaster + ")";
      DSS->WriteCommand(comando);
      DSS->Solve();
      comando = "set mode=faultstudy";
      DSS->WriteCommand(comando);
      DSS->Solve();
      comando = "export faultstudy";
      DSS->WriteCommand(comando);
   }

   // ::::::::::::::::::::::::::::::::
   // Lê arquivo de resultados do FaultStudy
   // ::::::::::::::::::::::::::::::::
   arquivo = new TStringList();
	arqFaultStudy = CaminhoDSS + "\\" + CodigoAlimentador + "_EXP_FAULTS.csv";
   arquivo->LoadFromFile(arqFaultStudy);
	// Percorre arquivo
   for(int i=1; i<arquivo->Count; i++)
   {
		linhaArq = arquivo->Strings[i];
		linhaAjustada = AjustaLinha(linhaArq);

		// Verifica os valores da linha
		if(ProblemaLinha(linhaAjustada)) continue;

      listaLinhasFaultStudy->Add(linhaAjustada);

      // Cria e guarda o objeto de resultado de curto
		res = GetResCurto(linhaAjustada);
      lisResCurto->Add(res);
   }
   delete arquivo;

   // ::::::::::::::::::::::::::::::::
   // Considera a área de busca
   // ::::::::::::::::::::::::::::::::
   for(int i=lisResCurto->Count-1; i>=0; i--)
   {
   	// pega um resultado de curto qualquer
		res = (resCurto*) lisResCurto->Items[i];
      idBarra = res->IDbarra;

      // verifica se a área de busca contém a barra da posição do curto
      for(int j=0; j<lisBarrasAreaBusca->Count; j++)
      {
			barra = (VTBarra*) lisBarrasAreaBusca->Items[j];
         if(barra->Id == idBarra)
         	break;
         else
         	barra = NULL;
      }

      // se a área de busca não tem a posição do curto, elimina possibilidade
      if(barra == NULL)
      {
         lisResCurto->Remove(res);
      }
   }

   // ::::::::::::::::::::::::::::::::
   // Faz efetivamente a localização do defeito
   // ::::::::::::::::::::::::::::::::

   // Obtém as fases afetadas pela falta
	strTipoFalta = classificacao->GetStrTipoFalta();

	// Analisa as correntes medidas
	lisEqptosCampo = dados->GetEqptosCampo();

	// Verifica chaves telecomandadas
   for(int i=0; (i<lisEqptosCampo->Count) && (Ifalta == 0.); i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

		if((eqptoCampo->GetTipo() != chaveDJ) && (eqptoCampo->GetTipo() != chaveRE) && (eqptoCampo->GetTipo() != chaveSC))
			continue;

		chaveMon = (TChaveMonitorada*) eqptoCampo;
		Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);

		if     (strTipoFalta == "ABC") LocalizaDefeito(3, Ifalta);
		else if(strTipoFalta == "AB") LocalizaDefeito(2, Ifalta);
		else if(strTipoFalta == "BC") LocalizaDefeito(2, Ifalta);
		else if(strTipoFalta == "CA") LocalizaDefeito(2, Ifalta);

		OrdenaResultados();

//		if(strTipoFalta == "ABABC")
//		{
//			Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);
//			// Método para localizar defeito (2F ou 3F) com Ifalta
//			LocalizaDefeitoAB_ABC(Ifalta);
//			OrdenaResultados();
//		}
//		else
//		{
//			if(strTipoFalta == "ABC")
//			{
//				numFases = 3;
//				Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);
//				Ifalta += std::abs(chaveMon->medicaoVI.falta.I[1]);
//				Ifalta += std::abs(chaveMon->medicaoVI.falta.I[2]);
//				Ifalta /= 3.;
//			}
//			else if((strTipoFalta == "AB") || (strTipoFalta == "ABN"))
//			{
//				numFases = 2;
//				Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);
//				Ifalta += std::abs(chaveMon->medicaoVI.falta.I[1]);
//				Ifalta /= 2.;
//			}
//			else if((strTipoFalta == "BC") || (strTipoFalta == "BCN"))
//			{
//				numFases = 2;
//				Ifalta = std::abs(chaveMon->medicaoVI.falta.I[1]);
//				Ifalta += std::abs(chaveMon->medicaoVI.falta.I[2]);
//				Ifalta /= 2.;
//			}
//			else if((strTipoFalta == "AC") || (strTipoFalta == "ACN"))
//			{
//				numFases = 2;
//				Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);
//				Ifalta += std::abs(chaveMon->medicaoVI.falta.I[2]);
//				Ifalta /= 2.;
//			}
//			// Método para localizar defeito com: número de fases e corrente de curto
//			LocalizaDefeito(numFases, Ifalta);
//			OrdenaResultados();
//		}
	}
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TFaultStudyFL::GetBarra(int ID)
{
	VTBarra* barra;

	if(ID == -1) return NULL;

	for(int i=0; i<lisBarrasAreaBusca->Count; i++)
	{
		barra = (VTBarra*) lisBarrasAreaBusca->Items[i];
      if(barra->Id == ID)
		{
			return barra;
      }
   }

   return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::GetDesvioResultadoAB_ABC(resCurto* res, double Icc, double &ext_desvio, String &detalheTipoFalta)
{
	double desvio2f, desvio3f;
	double resp;

	if(Icc == 0.)
	{
		ext_desvio = -1;
		return;
	}

	// Verificação adicional
	if(res->Icc3f > 0 && res->Icc2f == 0)
	{
		ext_desvio = 100.;
		return;
	}

	// Verifica defeitos 2F e 3F
	desvio2f = 100. * fabs(Icc - res->Icc2f) / Icc;
	desvio3f =  100. * fabs(Icc - res->Icc3f) / Icc;

   // Pega o menor desvio
	if(desvio2f < desvio3f)
	{
		ext_desvio = desvio2f;
		detalheTipoFalta = "AB";
	}
	else
	{
		ext_desvio = desvio3f;
		detalheTipoFalta = "ABC";
	}
}
//---------------------------------------------------------------------------
double __fastcall TFaultStudyFL::GetDesvioResultado(resCurto* res, double Icc, int numFases)
{
	double resp;

   if(Icc == 0.) return -1.;

   // Verificação adicional
	if(res->Icc3f > 0 && res->Icc2f == 0)
   	return 100.;

	if(numFases == 2)  // defeitos 2F
   {
   	resp = 100. * fabs(Icc - res->Icc2f) / Icc;
   }
   else if(numFases == 3) // defeitos 3F
   {
	   resp = 100. * fabs(Icc - res->Icc3f) / Icc;
   }
   else
   {
      resp = -1.;
   }

   return resp;
}
//---------------------------------------------------------------------------
double __fastcall TFaultStudyFL::GetMaxDesvioIporc()
{
	return MaxDesvioIporc;
}
//---------------------------------------------------------------------------
resCurto* __fastcall TFaultStudyFL::GetResCurto(String linhaAjustada)
{
   int IDbarra;
   double Icc3f, Icc1f, Icc2f;
   String valor;
   resCurto* res = new resCurto();

	if(linhaAjustada == "") return NULL;

   // Pega os campos da string
	valor = GetCampoCSV(linhaAjustada, 0, ";");
   IDbarra = valor.ToInt();
   valor = GetCampoCSV(linhaAjustada, 1, ";");
   Icc3f = valor.ToDouble();
   valor = GetCampoCSV(linhaAjustada, 2, ";");
   Icc1f = valor.ToDouble();
	valor = GetCampoCSV(linhaAjustada, 3, ";");
   Icc2f = valor.ToDouble();

   // Gera struct de resultado de curto
   res->IDbarra = IDbarra;
   res->barraCurto = NULL;
   res->Icc3f = Icc3f;
   res->Icc1f = Icc1f;
   res->Icc2f = Icc2f;

   return res;
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::GetSolucoes(TFuncoesDeRede* funcoesRede, StrDadosLog* strLog, TList* lisEXT)
{
	double distanciaChvMontante;
	int utmx, utmy;
	strSolucao* solucao;
	resCurto* res;
	VTBarra* barra;
	VTChave* chaveMontante;

	// Verificações
	if(lisEXT == NULL || funcoesRede == NULL) return;
//	if(FiltrarSolucoesPorBloco)
//		FiltraSolucoesBloco(funcoesRede, lisResCurto);

	strLog->MeasuredFaultCurrent = Ifalta;
	strLog->solutionsAmount = lisResCurto->Count;

   lisEXT->Clear();
	for(int i=0; i<lisResCurto->Count; i++)
   {
		res = (resCurto*) lisResCurto->Items[i];
      barra = res->barraCurto;

      // Gera obj de solução
      solucao = new strSolucao();

		solucao->Probabilidade = lisEXT->Count + 1;
		solucao->DefTipo = strTipoFalta;
		solucao->DefTipo_detalhe = res->detalheTipoFalta;
		solucao->barraSolucao = barra;

      // Coordenadas
		barra->CoordenadasUtm_cm(utmx, utmy);
		solucao->DefX = String(utmx);
      solucao->DefY = String(utmy);

		// Ajusta coordenadas "fake" do Sinap
		double lat = double(utmy * 1e-7) - 100.;
		double lon = double(utmx * 1e-7) - 100.;

      solucao->DefLat = String(lat);
      solucao->DefLon = String(lon);

      // Distância da solução em relação ao relé do disjuntor
      solucao->DistRele = funcoesRede->GetDistancia_KM_DaSubestacao(barra);

      // Informações da chave à montante
 		chaveMontante = funcoesRede->GetChaveMontante(barra);
      if(chaveMontante != NULL)
      {
      	distanciaChvMontante = funcoesRede->GetDistanciaMetros(chaveMontante->Barra(1), barra);
         solucao->ChvMont = chaveMontante->Codigo;
         solucao->DistChvMont = String(Round(distanciaChvMontante, 2));
         solucao->ClientesDepoisChvMont = funcoesRede->GetNumConsJusLigacao(chaveMontante->Codigo);
      }
      else
      {
         solucao->ChvMont = "";
         solucao->DistChvMont = "";
      }

      // debug: (remover depois)
      solucao->CodBarra = barra->Codigo;
      solucao->IdBarra = barra->Id;

		// Insere o obj de solução na lista externa
      lisEXT->Add(solucao);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::FiltraSolucoesBloco(TFuncoesDeRede* funcoesRede, TList* lisResCurto)
{
   resCurto* res;
   TList *lisBlocos, *lisAux;
   VTBloco* bloco;

	// Verificações
	if(funcoesRede == NULL || lisResCurto == NULL) return;
   if(lisResCurto->Count == 0) return;

   funcoesRede->RessetaParametros();

   // Pega lista de blocos da rede
   lisBlocos = new TList();
   funcoesRede->GetBlocosRede(CodigoAlimentador, lisBlocos);
   if(lisBlocos->Count == 0) return;

   // Passa resultados de curto para lista auxiliar
   lisAux = new TList();
   CopiaTList(lisResCurto, lisAux);
   lisResCurto->Clear();

   // Para cada bloco, guarda apenas um resultado de curto
   for(int i=0; i<lisBlocos->Count; i++)
   {
		bloco = (VTBloco*) lisBlocos->Items[i];

		for(int j=0; j<lisAux->Count; j++)
      {
			res = (resCurto*) lisAux->Items[j];

         if(bloco->ExisteBarra(res->barraCurto))
         {
            lisResCurto->Add(res);
            break;
         }
      }
   }

   delete lisAux;
}
//---------------------------------------------------------------------------
double __fastcall TFaultStudyFL::GetCorrenteFaltaMedida()
{
	return(Ifalta);
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::Inicializa(String CaminhoDSS, String CaminhoDirFaultLocation, String CodigoAlimentador)
{
	// Salva parâmetros
	this->CaminhoDSS = CaminhoDSS;
   SetCodigoAlimentador(CodigoAlimentador);

   // Obtém o máximo desvio porcentual admissível, em relação à medição de corrente
   String pathConfigGerais = CaminhoDirFaultLocation + "\\ConfigGerais.ini";
	TIniFile* file = new TIniFile(pathConfigGerais);
   MaxDesvioIporc = file->ReadFloat("FAULTSTUDY", "MaxDesvioIporc", 1.0);
   NumMaxSolucoes = file->ReadFloat("FAULTSTUDY", "NumMaximoSolucoes", 5);
   FiltrarSolucoesPorBloco = file->ReadBool("FAULTSTUDY", "FiltrarSolucoesPorBloco", 0);

   // Seta a lista de barras da área de busca
   areaBusca->GetAreaBusca_Barras(this->lisBarrasAreaBusca);

   file->Free();
//   // Destroi obj de arquivo INI
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Com TClassificacao, define o número de fases afetadas. Com TDados, obtém a
 * corrente de defeito.
 */
void __fastcall TFaultStudyFL::LocalizaDefeito(TClassificacao* classificacao, TDados* dados)
{
//	String strTipoFalta;
   TStringList* lisMedicoesTrechos;
	TList* lisEqptosCampo;
   TChaveMonitorada* chaveMon;
   TSensor* sensor;
   double Ifalta;
   int numFases;

   // Obtém as fases afetadas pela falta
	strTipoFalta = classificacao->GetStrTipoFalta();

	// Inicializa corrente de falta
   Ifalta = 0.;

   lisEqptosCampo = dados->GetEqptosCampo();
   for(int i=0; (i<lisEqptosCampo->Count) && (Ifalta == 0.); i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

		if(eqptoCampo->GetTipo() == eqptoSENSOR)
      {
         sensor = (TSensor*) eqptoCampo;

         if((strTipoFalta == "AB") || (strTipoFalta == "ABN"))
         {
         	numFases = 2;
	         Ifalta = std::abs(sensor->medicaoI.falta.I[0]);
         }
         else if((strTipoFalta == "BC") || (strTipoFalta == "BCN"))
         {
	         numFases = 2;
	         Ifalta = std::abs(sensor->medicaoI.falta.I[1]);
         }
         else if((strTipoFalta == "AC") || (strTipoFalta == "ACN"))
         {
	         numFases = 2;
	         Ifalta = std::abs(sensor->medicaoI.falta.I[2]);
         }
      }
		else if((eqptoCampo->GetTipo() == chaveDJ) || (eqptoCampo->GetTipo() == chaveRE) || (eqptoCampo->GetTipo() == chaveSC))
      {
       	chaveMon = (TChaveMonitorada*) eqptoCampo;

         if((strTipoFalta == "AB") || (strTipoFalta == "ABN"))
         {
         	numFases = 2;
	         Ifalta = std::abs(chaveMon->medicaoVI.falta.I[0]);
         }
         else if((strTipoFalta == "BC") || (strTipoFalta == "BCN"))
         {
	         numFases = 2;
	         Ifalta = std::abs(chaveMon->medicaoVI.falta.I[1]);
         }
         else if((strTipoFalta == "AC") || (strTipoFalta == "ACN"))
         {
	         numFases = 2;
	         Ifalta = std::abs(chaveMon->medicaoVI.falta.I[2]);
         }
      }
   }

   LocalizaDefeito(numFases, Ifalta);
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::LocalizaDefeito(int numFases, double Icc)
{
	double desvioResultado;
	int IDbarra, indice;
	resCurto* res = NULL;
   VTBarra* barra;

   // Não será executado para defeitos fase-terra
   if(numFases == 1) return;

   // Para um determinado resultado, compara a corrente calculada com a medida,
	// gerando um desvio (em %) em relação ao medido. Se o desvio superar o máximo
   // desvio admissível, ignora o resultado e o remove da lista de resultados.
	for(int i=lisResCurto->Count-1; i>=0; i--)
	{
		res = (resCurto*) lisResCurto->Items[i];
      desvioResultado = GetDesvioResultado(res, Icc, numFases);

      if(desvioResultado > MaxDesvioIporc)
      {
      	lisResCurto->Remove(res);
         res = NULL;
      }

      if(res != NULL)
      {
      	// Seta o valor do resultado de curto
      	res->desvio = desvioResultado;
         // Adiciona a barra à lista de barras de solução
         if((barra = GetBarra(res->IDbarra)) != NULL)
         {
         	res->barraCurto = barra;
         }
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::LocalizaDefeitoAB_ABC(double Icc)
{
	double desvioResultado;
	resCurto* res = NULL;
	String detalheTipoFalta;
	VTBarra* barra;

	// Para um determinado resultado, compara a corrente calculada com a medida,
	// gerando um desvio (em %) em relação ao medido. Se o desvio superar o máximo
	// desvio admissível, ignora o resultado e o remove da lista de resultados.

	// Obs: o resultado de faultstudy é mantido na lista se:
	//    1) Icalc_2f é próx. do Imedido
	//    ou
	//    2) Icalc_3f é próx. do Imedido
	for(int i=lisResCurto->Count-1; i>=0; i--)
	{
		res = (resCurto*) lisResCurto->Items[i];
//      desvioResultado = GetDesvioResultadoAB_ABC(res, Icc);

		desvioResultado = 0.;
		detalheTipoFalta = "";
		GetDesvioResultadoAB_ABC(res, Icc, desvioResultado, detalheTipoFalta);

      if(desvioResultado > MaxDesvioIporc)
      {
      	lisResCurto->Remove(res);
         res = NULL;
      }

      if(res != NULL)
      {
      	// Seta o valor do resultado de curto
			res->desvio = desvioResultado;
			// Seta o detalhe do tipo de falta (AB ou ABC)
			res->detalheTipoFalta = detalheTipoFalta;
         // Adiciona a barra à lista de barras de solução
         if((barra = GetBarra(res->IDbarra)) != NULL)
         {
         	res->barraCurto = barra;
         }
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Ordena resultados do menor desvio para o maior desvio
 */
void __fastcall TFaultStudyFL::OrdenaResultados()
{
	resCurto *res, *resAux;
   resCurto* resMenorDesvio;

	if(lisResCurto->Count <= 1) return;

   // Pega o resultado de menor desvio
	resMenorDesvio = (resCurto*) lisResCurto->Items[0];
   for(int i=0; i<lisResCurto->Count; i++)
   {
   	res = (resCurto*) lisResCurto->Items[i];
      if(res->desvio < resMenorDesvio->desvio)
      	resMenorDesvio = res;
   }

   lisResCurto->Remove(resMenorDesvio);
   lisResCurto->Insert(0, resMenorDesvio);

   // Para cada resultado, verifica sua posição em relação aos
	// resultados já ordenados
   int Ponteiro = 1;
	for(int i=Ponteiro; i<lisResCurto->Count; i++)
   {
		res = (resCurto*) lisResCurto->Items[i];

      int posFinal=0;
      for(posFinal=0; posFinal<Ponteiro; posFinal++)
      {
			resAux = (resCurto*) lisResCurto->Items[posFinal];

         if(res->desvio < resAux->desvio)
         	break;
      }

      lisResCurto->Remove(res);
      lisResCurto->Insert(posFinal, res);

      Ponteiro += 1;
	}
}
//---------------------------------------------------------------------------
bool __fastcall TFaultStudyFL::ProblemaLinha(String linha)
{
	if(linha == "") return(true);

	int numCampos = CSVCamposCount(linha, ";");
	for(int i=0; i<numCampos; i++)
	{
		String campo = GetCampoCSV(linha, i, ";");
		if(campo == "Nan")
			return(true);
	}

	return(false);
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::SetAreaBusca(TList* lisBarrasAreaBusca)
{
	if(lisBarrasAreaBusca == NULL) return;

   // Salva referência para a lista de barras da área de busca
   this->lisBarrasAreaBusca = lisBarrasAreaBusca;
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::SetCodigoAlimentador(String CodigoAlimentador)
{
	int comp = CodigoAlimentador.Length();
   String substr;
	String CodigoFinal;

	// Remove "-" e " "
	CodigoAlimentador = ReplaceStr(CodigoAlimentador, "-", "");
	CodigoAlimentador = ReplaceStr(CodigoAlimentador, " ", "");

//	String inicio = "";
//	String restante = "";
//	int indice;
//	for(int i=1; i<=CodigoAlimentador.Length(); i++)
//	{
//		if(!IsNumber(CodigoAlimentador, i))
//			inicio +=  CodigoAlimentador.SubString(i,1);
//		else
//		{
//			indice = i;
//			break;
//		}
//	}
//	if(inicio.Length() == 4) inicio = inicio.SubString(2,3);
//	CodigoFinal = inicio + CodigoAlimentador.SubString(indice, CodigoAlimentador.Length() - indice + 1);

   // Salva o código
	this->CodigoAlimentador = CodigoAlimentador;
}
//---------------------------------------------------------------------------
void __fastcall TFaultStudyFL::SetConfiguracoes(TConfiguracoes* configGerais)
{
	this->configGerais = configGerais;
}
//---------------------------------------------------------------------------
