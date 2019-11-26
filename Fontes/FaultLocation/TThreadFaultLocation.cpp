//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//   THREAD para executar todos os processos relativos � localiza��o
//   de um determinado evento de defeito, permitindo localiza��o de
//   outros defeitos em paralelo.
//
//   1- Aguarda X segundos (param. configur�vel) para recep��o dos XML de alarmes
//   2- Inicia os processos (m�todo Inicia)
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//---------------------------------------------------------------------------
#include <vcl.h>
#include <System.hpp>
#include <System.StrUtils.hpp>
#include <complex>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "TThreadFaultLocation.h"
#include "TAreaBusca.h"
#include "TClassificacao.h"
#include "TConfiguracoes.h"
#include "TFormFaultLocation.h"
#include "TGerenciadorEventos.h"
#include "AlgoFasorial\TAlgoFasorial.h"
#include "AlgoRompCabo\TAlgoRompCabo.h"
#include "AlgoritmoDMS3\TAlgoritmoDMS3.h"
#include "Auxiliares\TCronos.h"
#include "Auxiliares\TDados.h"
#include "Auxiliares\Enums.h"
#include "Auxiliares\FuncoesFL.h"
#include "Auxiliares\TFuncoesDeRede.h"
#include "Auxiliares\TImportaXMLs.h"
#include "Auxiliares\TLog.h"
#include "Auxiliares\TOpcoesGraficas.h"
#include "ComunicacaoXML\TAlarme.h"
#include "ComunicacaoXML\TXMLComunicacao.h"
#include "ComunicacaoXML\TXMLParser.h"
#include "DSS\TFaultStudyFL.h"
#include "DSS\TConversorSinapDSS.h"
#include "EstrategiaEvolutiva\TEstrategiaEvolutiva.h"
//---------------------------------------------------------------------------
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TEqptoCampo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TBarraSemTensao.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TChaveMonitorada.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TFusivel.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TITrafo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TMedidorInteligenteBalanco.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TQualimetro.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TSensor.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
//---------------------------------------------------------------------------
__fastcall TThreadFaultLocation::TThreadFaultLocation(bool CreateSuspended,
																		VTApl* apl,
                                                      TFormFaultLocation* formFL,
                                                      TGerenciadorEventos* gerenciador,
                                                      TAlarme* alarme) : TThread(CreateSuspended)
{
	// Salva par�metros do apl
   this->apl = apl;
	this->path = (VTPath*)apl->GetObject(__classid(VTPath));
	this->redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	this->redeMT = NULL;
	this->redeSE = NULL;
	this->formFL = formFL;
	this->gerenciador = gerenciador;

   // Inicializa xml de request
   xmlCom = NULL;

   // Cria lista para os alarmes associados � thread
	listaAlarmes = new TList();            // lista com os objetos do tipo alarme provenientes do barramento
	lisNomesXMLMsg = new TStringList();    // lista com os nomes dos arq. XML de resposta do M�d. de Supervis�o

	// Inicia par�metros
	CodigoAlimentador = "";
	TimeStamp = "";
	CodigoEvento = "";
	caboRompido = false;
	tipoLF = -1;
	solicitacaoMedicoesAdicionais = false;

   // Identifica��o do evento de curto-circuito. Salva ponteiro para o alarme que lhe deu origem
   SalvaAlarme(alarme);
	// Seta o alarme gatilho do processo
   alarmeGatilho = alarme;


 	// LOG de erros
	String pathLogErros = path->DirDat() + "\\FaultLocation\\Logs\\" + CodigoEvento + ".txt";
   logErros = new TLog(pathLogErros);

   // Cria objeto de importa��o de dados (XML -> dados formatados)
   // C�digo do evento = Nome do primeiro arquivo XML inserido na pasta
   importaXMLs = new TImportaXMLs(apl, formFL, listaAlarmes);
	importaXMLs->SetLogErros(logErros);

   // Inicializa o timer de janela de inser��o de dados
   IniciaFlags();

   // Inicializa objeto de dados (para leitura dos arquivos INI e gera��o dos respectivos objetos)
   dados = new TDados(apl, listaAlarmes);
	caminhoDSS = dados->GetPathEvento() + "\\RedeDSS";

   // Obj para auxiliar varreduras na rede
   funcoesRede = new TFuncoesDeRede(apl);

	config = new TConfiguracoes(path->DirDat());

   // Obj para determinar a �rea de busca do defeito
   areaBusca = new TAreaBusca(apl, dados, funcoesRede);
	areaBusca->SetConfiguracoes(config);

	double* vetorParamComparacao = new double[4];
	vetorParamComparacao[0] = 8.;
	vetorParamComparacao[1] = 40.;
	vetorParamComparacao[2] = 5.;
   vetorParamComparacao[3] = 10.;
	classificacao = new TClassificacao(dados, vetorParamComparacao);
   classificacao->SetConfigGerais(config);

   // Cria conversor de rede (Sinap -> OpenDSS)
   conversor = new TConversorSinapDSS(apl);

	// Cria objeto para solu��o atrav�s do FaultStudy
	FS = new TFaultStudyFL(areaBusca);
	FS->SetConfiguracoes(config);
	OpGraf = new TOpcoesGraficas(apl);
	lisBarrasDefeitoFaultStudy = NULL;

	// Objeto para solu��o atrav�s de estrat�gia evolutiva
   estrEvol = new TEstrategiaEvolutiva();
   estrEvol->SetParametros(config, classificacao, areaBusca, dados, formFL);

   // Incializa objeto de localiza��o de faltas baseado em fasores
   AlgoFasorial = new TAlgoFasorial(apl);
	AlgoFasorial->SetParametros(config, classificacao, areaBusca, dados, formFL);
	AlgoFasorial->SetLogFL(logErros);

	// Inicializa objeto de localiza��o de rompimento de cabo
	AlgoRompCabo = new TAlgoRompCabo(apl);
	AlgoRompCabo->SetParametros(areaBusca, classificacao, dados);
	AlgoRompCabo->SetLogRompCabo(logErros);

	// Objeto para dados de Log
	InicializaDadosLog();
}
//---------------------------------------------------------------------------
__fastcall TThreadFaultLocation::~TThreadFaultLocation()
{
   // Salva log
	if(!logErros->Vazio())
		logErros->ImprimeLog();

   // Destroi objetos
	for(int i=listaAlarmes->Count-1; i>=0; i--)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
   	delete alarme;
   }
   delete listaAlarmes; listaAlarmes = NULL;

   lisNomesXMLMsg->Clear();
   delete lisNomesXMLMsg; lisNomesXMLMsg = NULL;

	delete importaXMLs; importaXMLs = NULL;
   delete dados; dados = NULL;
	delete funcoesRede; funcoesRede = NULL;
   delete areaBusca; areaBusca = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::AddAlarme(TAlarme* alarme)
{
	if(listaAlarmes->IndexOf(alarme) >= 0)
		return;

   listaAlarmes->Add(alarme);

	OrdenaListaAlarmes();
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::OrdenaListaAlarmes()
{
   bool ok;
   TAlarme* alarme1;
	TAlarme* alarme2;

	if(listaAlarmes->Count == 1)
	{
      // O �nico alarme da lista � o alarme gatilho
   	alarmeGatilho = (TAlarme*) listaAlarmes->Items[0];
		return;
   }

	ok = false;
	while(!ok)
	{
      ok = true;
      for(int i=1; i<listaAlarmes->Count; i++)
      {
         alarme1 = (TAlarme*) listaAlarmes->Items[i-1];
         alarme2 = (TAlarme*) listaAlarmes->Items[i];

         TDateTime timestamp1 = GetTimeStamp(alarme1);
         TDateTime timestamp2 = GetTimeStamp(alarme2);

			if(timestamp2 < timestamp1)
         {
            ok = false;
         	listaAlarmes->Remove(alarme2);
         	listaAlarmes->Insert(i-1, alarme2);
         }
      }
   }

   // Atualiza o alarme gatilho (timestamp mais antigo)
   alarmeGatilho = (TAlarme*) listaAlarmes->Items[0];
}
//---------------------------------------------------------------------------
TAlarme* __fastcall TThreadFaultLocation::GetAlarmeGatilho()
{
	return alarmeGatilho;
}
//---------------------------------------------------------------------------
TDateTime __fastcall TThreadFaultLocation::GetTimeStamp(TAlarme* alarme)
{
   unsigned short ano, mes, dia, hora, minuto, segundo, mseg;
   String str;

	String strTimeStamp = alarme->GetTimeStamp();
	str = strTimeStamp.SubString(1,4); ano = str.ToInt();
	str = strTimeStamp.SubString(5,2); mes = str.ToInt();
	str = strTimeStamp.SubString(7,2); dia = str.ToInt();
	str = strTimeStamp.SubString(9,2); hora = str.ToInt();
	str = strTimeStamp.SubString(11,2); minuto = str.ToInt();
	str = strTimeStamp.SubString(13,2); segundo = str.ToInt();
	mseg = 0;

	TDateTime timeStamp = TDateTime(ano, mes, dia, hora, minuto, segundo, mseg);

   return timeStamp;
}
////---------------------------------------------------------------------------
//bool __fastcall TThreadFaultLocation::Eh_AtualizacaoAlarme(TAlarme* novoAlarme, TList* listaAlarmes)
//{
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//
//		// Alarme novo n�o pode ser de alimentador diferente do alimentador dos demais alarmes.
//		if(alarme->GetCodAlimentador() != novoAlarme->GetCodAlimentador())
//		{
//			return(false);
//		}
//
//		// Se alarme novo for do mesmo equipamento e do mesmo tipo, n�o deve ser considerado.
//		if(alarme->GetCodEqpto()   == novoAlarme->GetCodEqpto() &&
//			alarme->GetTipoAlarme() == novoAlarme->GetTipoAlarme())
//		{
//			return(false);
//		}
//	}
//	return(true);
//}
////---------------------------------------------------------------------------
//void __fastcall TThreadFaultLocation::VerificaAtualizacaoAlarmes()
//{
//	TList* lisAtualizacoesAlarmes = new TList;
//	TXMLParser* xmlParser = new TXMLParser(apl, formFL);
//
//	// Para cada alarme original, pega as atualiza��es contidas nos seus respectivos arquivos de alarme.
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//		xmlParser->GetAtualizacoesAlarmes(alarme->GetPathArquivoAlarme(), lisAtualizacoesAlarmes);
//	}
//
//	// Verifica quais novos alarmes devem ser considerados
//	for(int i=lisAtualizacoesAlarmes->Count-1; i>=0; i--)
//	{
//		TAlarme* novoAlarme = (TAlarme*) lisAtualizacoesAlarmes->Items[i];
//		if(Eh_AtualizacaoAlarme(novoAlarme, listaAlarmes))
//		{
//			lisAtualizacoesAlarmes->Remove(novoAlarme);
//			listaAlarmes->Add(novoAlarme);
//		}
//		else
//		{
//			delete novoAlarme;
//		}
//	}
//
//	delete lisAtualizacoesAlarmes;
//	delete xmlParser;
//}
//---------------------------------------------------------------------------
/***
 * Monitora o tempo. Ap�s t=MaxJanelaDados, inicia a Thread.
 */
void __fastcall TThreadFaultLocation::AtualizaTimer(int indexProcesso)
{
	timerTempo += 1;
   if(timerTempo <= MaxJanelaDados) return;

   if(indexProcesso == 1)
	{
//		// Verifica se h� atualiza��o nos alarmes
//		VerificaAtualizacaoAlarmes();

		// Para o timer e seta janela fechada
		janelaAberta = false;

		AjustaAlarmesEvento();

		OrdenaListaAlarmes();

		AtualizaCodigoEvento();

		MemoProcessosLF->Lines->Add("In�cio do processo do evento " + CodigoEvento);

		VerificaTipoLocalizacao();

		// Verifica se deve solicitar medi��es adicionais
		// (ser� necess�rio apenas quando houver alarmes de medidores inteligentes)
		solicitacaoMedicoesAdicionais = DecideSolicitacaoMedicoes();

		if(solicitacaoMedicoesAdicionais)
		{
			XMLSolicitaMedicoes();
			MemoProcessosLF->Lines->Add("   Evento " + CodigoEvento + " - Aguardando medi��es");
		}
	}
   else if(indexProcesso == 2)
	{
		timerTimeout += 1;

		// Se j� temos um XML de resposta (do M�d. de Supervis�o), executa a thread de localiza��o
		if(!solicitacaoMedicoesAdicionais || (solicitacaoMedicoesAdicionais && ExistemMedicoes()))
		{
			Execute();
		}
	}
}
//---------------------------------------------------------------------------
// Se pelo menos um alarme n�o for dos tipos 5 ou 6, retorna false
// OBS: Alarme tipo 1: FREL (Alarme de lock-out de DJ / RE)
//      Alarme tipo 2: RAUT (Alarme de lock-out de SECC)
//      Alarme tipo 3: FPE (Alarme de falta permanente)
//      Alarme tipo 4: AIM (Alarme de surto)
bool __fastcall TThreadFaultLocation::CaboRompido()
{
	bool RompimentoCabo = true;
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		if(alarme->GetTipoAlarme() == 1 || alarme->GetTipoAlarme() == 2 ||
			alarme->GetTipoAlarme() == 3 || alarme->GetTipoAlarme() == 4)
		{
			RompimentoCabo = false;
			break;
		}
	}
	return(RompimentoCabo);
}
//---------------------------------------------------------------------------
double __fastcall TThreadFaultLocation::CorrenteFaltaMedida(StrFasor* fasorCorrente)
{
   double Imedida = 0.;
   std::complex<double> I;

   if(classificacao->GetStrTipoFalta() == "A" || classificacao->GetStrTipoFalta() == "AN" ||
      classificacao->GetStrTipoFalta() == "AB" || classificacao->GetStrTipoFalta() == "ABN")
   {
      I = fasorCorrente->faseA;
   }
   else if(classificacao->GetStrTipoFalta() == "B" || classificacao->GetStrTipoFalta() == "BN" ||
           classificacao->GetStrTipoFalta() == "BC" || classificacao->GetStrTipoFalta() == "BCN")
   {
      I = fasorCorrente->faseB;
   }
   else if(classificacao->GetStrTipoFalta() == "C" || classificacao->GetStrTipoFalta() == "CN"  ||
           classificacao->GetStrTipoFalta() == "CA" || classificacao->GetStrTipoFalta() == "CAN" ||
           classificacao->GetStrTipoFalta() == "AC" || classificacao->GetStrTipoFalta() == "ACN")
   {
      I = fasorCorrente->faseC;
   }

   Imedida = abs(I);

   return(Imedida);
}
//---------------------------------------------------------------------------
/****
 * TODOS OS M�TODOS DEVEM SER CHAMADOS A PARTIR DAQUI
 **/
void __fastcall TThreadFaultLocation::Execute()
{
	if(tipoLF == faultlocationDMS1)
	{
		ExecutaFaultLocation_DMS1();
	}
	else if(tipoLF == faultlocationDMS2)
	{
		ExecutaFaultLocation_DMS2();
	}
	else if(tipoLF == faultlocationDMS3)
	{
		ExecutaFaultLocation_DMS3();
	}

//	if(tipoLF == faultlocationFS)
//	{
//		ExecutaFaultLocation_FaultStudy();
//	}
//	else if(tipoLF == faultlocationALGOFASORIAL)
//	{
//		ExecutaFaultLocation_AlgoFasorial();
//	}
//	else if(tipoLF == faultlocationROMPCABO)
//	{
//		ExecutaLocRompCabo();
//	}
}
//---------------------------------------------------------------------------
/***
 *    Localiza��o de faltas que n�o envolvem a terra. Baseia-se na fun��o
 *    Fault Study (FS) do OpenDSS.
 */
void __fastcall TThreadFaultLocation::ExecutaFaultLocation_DMS1()
{
	// Inicia flag de thread inicializada
	iniciado = true;
	gerenciador->FL_Executando = this;

	// Cronometragem do tempo de execu��o
	TCronos* cronometroFL = new TCronos();

   // Faz a tradu��o (XML -> INI) do arquivo de response
   // Gera estrutura de pasta e arquivo Geral.ini
	importaXMLs->GeraBaseDados();

	if(solicitacaoMedicoesAdicionais)
	{
		// L� os dados solicitados
		importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);

		// L� os dados formatados (arquivos .INI -> objetos em mem�ria)
		dados->LeDadosFormatados();
	}

	// Insere nos objetos de equipamentos os dados e medi��es trazidos pelos alarmes
	dados->AcrescentaDadosMedicoes(listaAlarmes);

	// Determina a �rea de busca
	areaBusca->Executa();

	// Obt�m a rede MT em quest�o
	redeMT = areaBusca->GetRedeMT();
	redeSE = areaBusca->GetRedeSE();

	// Classifica o defeito (fases afetadas)
	classificacao->Executa();

	// Trata da convers�o da rede SINAP -> OPENDSS
	conversor->Inicializa(dados->GetPathEvento() + "\\RedeDSS", redeMT, redeSE);
	conversor->posicaoSuprimento = "AT";
	conversor->Executa();

	if(classificacao->GetStrTipoFalta() == "AB" ||
		classificacao->GetStrTipoFalta() == "BC" ||
		classificacao->GetStrTipoFalta() == "CA" ||
		classificacao->GetStrTipoFalta() == "ABC" ||
		classificacao->GetStrTipoFalta() == "ABABC")
	{
		// ::::::::::::::::::::::::::::::::::::::::::
		// Faz a localiza��o de defeitos que n�o envolvem resit�ncia de falta
		// -> defeitos 3F e 2F.
		// ::::::::::::::::::::::::::::::::::::::::::
      FS->Inicializa(dados->GetPathEvento() + "\\RedeDSS", path->DirDat() + "\\FaultLocation" ,CodigoAlimentador);
      FS->ExecutaLocalizFaultStudy(classificacao, dados);

		lisSolucoes = new TList();
		FS->GetSolucoes(funcoesRede, strLog, lisSolucoes);

		// Insere dados de LOG
		strLog->FLMethod = "FaultStudy";
		strLog->FLTolerance = FS->GetMaxDesvioIporc();
      strLog->MeasuredFaultCurrent = FS->GetCorrenteFaltaMedida();
		InformacoesAgrupamentoAlarmes(strLog->lisAlarmes);

      // Gera os XMLs de solu��o no diret�rio Exporta
		XMLGeraSolucoesDMS1(lisSolucoes, GetDadosGerais());
	}

	// Fecha a contagem de tempo de execu��o do algoritmo
	logErros->AddLinha("Tempo de execu��o do Algo Fasorial: " + String(Round(cronometroFL->GetSegundos(), 2)), true);

	// Ativa flag indicando processo de FL finalizado => para apagar arquivos de diret�rios
	FLfinalizado = true;
}
//---------------------------------------------------------------------------
/***
 *    Localiza��o de faltas que envolvem a terra (2FT e FT). Fornece a �rea de busca
 *    como resultado da localiza��o.
 */
void __fastcall TThreadFaultLocation::ExecutaFaultLocation_DMS2()
{
	// Inicia flag de thread inicializada
	iniciado = true;
	gerenciador->FL_Executando = this;

	// Cronometragem do tempo de execu��o
	TCronos* cronometroFL = new TCronos();

   // Faz a tradu��o (XML -> INI) do arquivo de response
   // Gera estrutura de pasta e arquivo Geral.ini
	importaXMLs->GeraBaseDados();

	if(solicitacaoMedicoesAdicionais)
	{
		// L� os dados solicitados
		importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);

		// L� os dados formatados (arquivos .INI -> objetos em mem�ria)
		dados->LeDadosFormatados();
	}

	// Insere nos objetos de equipamentos os dados e medi��es trazidos pelos alarmes
	dados->AcrescentaDadosMedicoes(listaAlarmes);

	// Determina a �rea de busca
	areaBusca->Executa_DMS2();

	//	// Obt�m a rede MT em quest�o
	redeMT = areaBusca->GetRedeMT();
	redeSE = areaBusca->GetRedeSE();

//	TList* lisBlocosAreaBusca = areaBusca->lisBlocosPesquisa;
//	TStringList* lisCodChaves_BlocosAreaBusca = new TStringList;
	TList* lisStrAreaBuscaDMS2 = new TList;
	areaBusca->GetAreaBusca_DMS2(lisStrAreaBuscaDMS2);
	XMLGeraSolucoesDMS2(lisStrAreaBuscaDMS2, GetDadosGerais());

//	//debug
//	TList* lisBlocosAreaBusca = areaBusca->lisBlocosPesquisa;
//	TStringList* lisTrechos = new TStringList;
//	for(int i=0; i<lisBlocosAreaBusca->Count; i++)
//	{
//		VTBloco* bloco = (VTBloco*) lisBlocosAreaBusca->Items[i];
//		for(int j=0; j<bloco->LisLigacao()->Count; j++)
//		{
//			VTLigacao* liga = (VTLigacao*) bloco->LisLigacao()->Items[j];
//			if(liga->Tipo() == eqptoTRECHO)
//			{
//				lisTrechos->Add(liga->Codigo);
//			}
//      }
//	}
//	lisTrechos->SaveToFile("c:\\users\\sinapsis\\desktop\\trechosAreaBusca.txt");
//   delete lisTrechos;

//	// Obt�m a rede MT em quest�o
//	redeMT = areaBusca->GetRedeMT();
//	redeSE = areaBusca->GetRedeSE();

//	// Classifica o defeito (fases afetadas)
//	classificacao->Executa();

//	// Trata da convers�o da rede SINAP -> OPENDSS
//	conversor->Inicializa(dados->GetPathEvento() + "\\RedeDSS", redeMT, redeSE);
//	conversor->posicaoSuprimento = "AT";
//	conversor->Executa();

//	if(classificacao->GetStrTipoFalta() == "AB" ||
//		classificacao->GetStrTipoFalta() == "BC" ||
//		classificacao->GetStrTipoFalta() == "CA" ||
//		classificacao->GetStrTipoFalta() == "ABC" ||
//		classificacao->GetStrTipoFalta() == "ABABC")
//	{
//		// ::::::::::::::::::::::::::::::::::::::::::
//		// Faz a localiza��o de defeitos que n�o envolvem resit�ncia de falta
//		// -> defeitos 3F e 2F.
//		// ::::::::::::::::::::::::::::::::::::::::::
//      FS->Inicializa(dados->GetPathEvento() + "\\RedeDSS", path->DirDat() + "\\FaultLocation" ,CodigoAlimentador);
//      FS->ExecutaLocalizFaultStudy(classificacao, dados);
//
//		lisSolucoes = new TList();
//		FS->GetSolucoes(funcoesRede, strLog, lisSolucoes);
//
//		// Insere dados de LOG
//		strLog->FLMethod = "FaultStudy";
//		strLog->FLTolerance = FS->GetMaxDesvioIporc();
//		strLog->MeasuredFaultCurrent = FS->GetCorrenteFaltaMedida();
//		InformacoesAgrupamentoAlarmes(strLog->lisAlarmes);
//
//		// Gera os XMLs de solu��o no diret�rio Exporta
//		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
//	}

	// Fecha a contagem de tempo de execu��o do algoritmo
	logErros->AddLinha("Tempo de execu��o do Algo Fasorial: " + String(Round(cronometroFL->GetSegundos(), 2)), true);

	// Ativa flag indicando processo de FL finalizado => para apagar arquivos de diret�rios
	FLfinalizado = true;
}
//---------------------------------------------------------------------------
///***
// *    Localiza��o de Faltas com Algoritmo Fasorial (ALGO FASORIAL)
// */
////---------------------------------------------------------------------------
//void __fastcall TThreadFaultLocation::ExecutaFaultLocation_AlgoFasorial()
//{
//// Inicia flag de thread inicializada
//	iniciado = true;
//	gerenciador->FL_Executando = this;
//
//   // Cronometragem do tempo de execu��o
//   TCronos* cronometroFL = new TCronos();
//
//   // Faz a tradu��o (XML -> INI) do arquivo de response
//   // Gera estrutura de pasta e arquivo Geral.ini
//   importaXMLs->GeraBaseDados();
//
//   importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);
//
//   // L� os dados formatados (arquivos .INI -> objetos em mem�ria)
//	dados->LeDadosFormatados();
//
////   // Verifica se faltam dados de medi��o de corrente
////	if(dados->FaltamDados())
////	{
////		formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " por falta de dados.");
////		FLfinalizado = true;
////		return;
////	}
//
//   // Executa determina��o da �rea de busca
//	areaBusca->Executa();
//
//   // Exporta �rea de busca para o self healing
////	XMLExportaAreaBusca_SH();
//
//	// Obt�m a rede MT em quest�o
//	redeMT = areaBusca->GetRedeMT();
//	redeSE = areaBusca->GetRedeSE();
//
//	// Classifica o defeito (fases afetadas)
//   classificacao->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
//	classificacao->Executa();
//
//	// Trata da convers�o da rede SINAP -> OPENDSS
//	conversor->Inicializa(dados->GetPathEvento() + "\\RedeDSS", redeMT, redeSE);
//	conversor->dados = dados;
//	conversor->posicaoSuprimento = "EqptoRef";
//	conversor->Executa();
//
//   if(classificacao->GetStrTipoFalta() == "A" ||
//		classificacao->GetStrTipoFalta() == "B" ||
//      classificacao->GetStrTipoFalta() == "C" ||
//		classificacao->GetStrTipoFalta() == "ABN" ||
//      classificacao->GetStrTipoFalta() == "BCN" ||
//      classificacao->GetStrTipoFalta() == "ACN")
//	{
//		// ::::::::::::::::::::::::::::::::::::::::::
//		// Faz a localiza��o de defeitos que envolvem resist�ncia de falta
//		// atrav�s do m�todo que utiliza fasores de tens�o e corrente medidos
//      // durante a falta
//		// ::::::::::::::::::::::::::::::::::::::::::
//
//		// Inicializa tryFault para teste de curto-circuito e teste de fluxo de pot�ncia
//		AlgoFasorial->IniciaTryFault(caminhoDSS);
//      AlgoFasorial->IniciaPreFalta(caminhoDSS);
//		AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
//
//		// Obt�m os fasores de V,I medidos pelo eqptoRef (qual�metro mais pr�ximo
//		// do defeito)
//		StrFasor* fasorVref = new StrFasor();
//		StrFasor* fasorIref = new StrFasor();
//		TQualimetro* qualimetroEqptoRef = dados->GetFasoresVI_QualimetroEqptoRef(fasorVref, fasorIref);
//
//		// Chama o c�lculo da imped�ncia total a partir dos fasores V, I na SE
//		AlgoFasorial->CalculaVI_Sequencias012(fasorVref, fasorIref);
//
//      if(classificacao->GetStrTipoFalta() == "A" || classificacao->GetStrTipoFalta() == "B" || classificacao->GetStrTipoFalta() == "C")
//   	{
//			AlgoFasorial->CalculaZtotal_FT(fasorVref, fasorIref);
//
//         // Pega o Ztotal medido a partir dos fasores V, I na SE
//         double reZtotal, imZtotal;
//         AlgoFasorial->GetZTotal(reZtotal, imZtotal);
//
//         // Pega os m�dulos de r e x:
//         reZtotal = fabs(reZtotal);
//			imZtotal = fabs(imZtotal);
//
//			String codigoRede = redeMT->Codigo;
//			AlgoFasorial->Executa_FT(codigoRede, qualimetroEqptoRef, reZtotal, imZtotal);
//      }
//      else if(classificacao->GetStrTipoFalta() == "ABN" || classificacao->GetStrTipoFalta() == "BCN" || classificacao->GetStrTipoFalta() == "ACN")
//   	{
//			AlgoFasorial->CalculaZtotal_seq1_2FT(fasorVref, fasorIref);
//
//         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
//         double reZtotal_1, imZtotal_1;
//			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
//
//			// Pega os m�dulos de r e x:
//			reZtotal_1 = fabs(reZtotal_1);
//			imZtotal_1 = fabs(imZtotal_1);
//
//         String codigoRede = redeMT->Codigo;
//			AlgoFasorial->Executa_2FT(codigoRede, qualimetroEqptoRef, reZtotal_1, imZtotal_1);
//		}
//
//      // Obt�m lista de structs de solu��o
//      lisSolucoes = new TList();
//		AlgoFasorial->GetSolucoes(funcoesRede, lisSolucoes);
//
//		// Insere dados de LOG
//		strLog->FLMethod = "Phasorial methodology";
//		strLog->FLTolerance = AlgoFasorial->Tolerancia;
//      strLog->MeasuredFaultCurrent = CorrenteFaltaMedida(fasorIref);
//      strLog->solutionsAmount = lisSolucoes->Count;
//      strLog->fasesSasOpostas = classificacao->CorrentesOpostasFasesSas;
//
//		// Gera os XMLs de solu��o no diret�rio Exporta
//		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
//
//		// Exporta solu��es para o self healing
////		XMLExportaSolucoes_SH();
//      XMLExporta_SH();
//	}
//	else if(classificacao->GetStrTipoFalta() == "AB" || classificacao->GetStrTipoFalta() == "BC" ||
//			  classificacao->GetStrTipoFalta() == "CA" || classificacao->GetStrTipoFalta() == "ABC" ||
//			  classificacao->GetStrTipoFalta() == "ABABC")
//	{
//		// ::::::::::::::::::::::::::::::::::::::::::
//		// Faz a localiza��o de defeitos que envolvem resist�ncia de falta
//      // atrav�s do m�todo que utiliza fasores de tens�o e corrente medidos
//      // durante a falta
//      // ::::::::::::::::::::::::::::::::::::::::::
//
//   	// Inicializa o objeto de testes de curto-circuito
//      AlgoFasorial->IniciaTryFault(caminhoDSS);
//      AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
//
//		// Obt�m os fasores de V,I medidos no in�cio do alimentador
//		StrFasor* fasorVref = new StrFasor();
//		StrFasor* fasorIref = new StrFasor();
//		TQualimetro* qualimetroEqptoRef = dados->GetFasoresVI_QualimetroEqptoRef(fasorVref, fasorIref);
//
//		// Chama o c�lculo da imped�ncia total a partir dos fasores V, I na SE
//		AlgoFasorial->CalculaVI_Sequencias012(fasorVref, fasorIref);
//
//      if(classificacao->GetStrTipoFalta() == "A" || classificacao->GetStrTipoFalta() == "B" || classificacao->GetStrTipoFalta() == "C")
//   	{
//			AlgoFasorial->CalculaZtotal_FT(fasorVref, fasorIref);
//
//         // Pega o Ztotal medido a partir dos fasores V, I na SE
//         double reZtotal, imZtotal;
//			AlgoFasorial->GetZTotal(reZtotal, imZtotal);
//
//         // Pega os m�dulos de r e x:
//         reZtotal = fabs(reZtotal);
//			imZtotal = fabs(imZtotal);
//
//         String codigoRede = redeMT->Codigo;
//			AlgoFasorial->Executa_FT(codigoRede, qualimetroEqptoRef, reZtotal, imZtotal);
//      }
//      else if(classificacao->GetStrTipoFalta() == "ABN" || classificacao->GetStrTipoFalta() == "BCN" || classificacao->GetStrTipoFalta() == "ACN")
//   	{
//			AlgoFasorial->CalculaZtotal_seq1_2FT(fasorVref, fasorIref);
//
//         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
//         double reZtotal_1, imZtotal_1;
//			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
//
//			// Pega os m�dulos de r e x:
//			reZtotal_1 = fabs(reZtotal_1);
//			imZtotal_1 = fabs(imZtotal_1);
//
//         String codigoRede = redeMT->Codigo;
//			AlgoFasorial->Executa_2FT(codigoRede, qualimetroEqptoRef, reZtotal_1, imZtotal_1);
//		}
//      else if(classificacao->GetStrTipoFalta() == "AB" || classificacao->GetStrTipoFalta() == "BC" || classificacao->GetStrTipoFalta() == "AC")
//   	{
//			AlgoFasorial->CalculaZtotal_seq1_2F(fasorVref, fasorIref);
//
//         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
//         double reZtotal_1, imZtotal_1;
//			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
//
//         // Pega os m�dulos de r e x:
//			reZtotal_1 = fabs(reZtotal_1);
//			imZtotal_1 = fabs(imZtotal_1);
//
//         String codigoRede = redeMT->Codigo;
//         AlgoFasorial->Executa_2F(codigoRede, qualimetroEqptoRef, reZtotal_1, imZtotal_1);
//      }
//      else if(classificacao->GetStrTipoFalta() == "ABC")
//   	{
//			AlgoFasorial->CalculaZtotal_seq1_3F(fasorVref, fasorIref);
//
//         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
//         double reZtotal_1, imZtotal_1;
//			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
//
//         // Pega os m�dulos de r e x:
//			reZtotal_1 = fabs(reZtotal_1);
//			imZtotal_1 = fabs(imZtotal_1);
//
//         String codigoRede = redeMT->Codigo;
//         AlgoFasorial->Executa_3F(codigoRede, qualimetroEqptoRef, reZtotal_1, imZtotal_1);
//      }
//
//      // Obt�m lista de structs de solu��o
//      lisSolucoes = new TList();
//		AlgoFasorial->GetSolucoes(funcoesRede, lisSolucoes);
//
//		// Insere dados de LOG
//		strLog->FLMethod = "Phasorial methodology";
//		strLog->FLTolerance = AlgoFasorial->Tolerancia;
//      strLog->solutionsAmount = lisSolucoes->Count;
//      strLog->MeasuredFaultCurrent = CorrenteFaltaMedida(fasorIref);
//      strLog->fasesSasOpostas = classificacao->CorrentesOpostasFasesSas;
//
//		// Gera os XMLs de solu��o no diret�rio Exporta
//		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
//
//		// Exporta solu��es para o self healing
////		XMLExportaSolucoes_SH();
//      XMLExporta_SH();
//	}
//
//   // Fecha a contagem e tempo de execu��o do algoritmo
//   logErros->AddLinha("Tempo de execu��o do Algo Fasorial: " + String(Round(cronometroFL->GetSegundos(), 2)), true);
//
//   // Ativa flag indicando processo de FL finalizado => para apagar arquivos de diret�rios
//	FLfinalizado = true;
//}
//---------------------------------------------------------------------------
///***
// *    M�todo geral de Localiza��o de Faltas
// */
//void __fastcall TThreadFaultLocation::ExecutaFaultLocation()
//{
//	// Inicia flag de thread inicializada
//	iniciado = true;
//	gerenciador->FL_Executando = this;
//
//   // Cronometragem do tempo de execu��o
//   TCronos* cronometroFL = new TCronos();
//
//   // Faz a tradu��o (XML -> INI) do arquivo de response
//   // Gera estrutura de pasta e arquivo Geral.ini
//   importaXMLs->GeraBaseDados();
//
//   importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);
//
//   // L� os dados formatados (arquivos .INI -> objetos em mem�ria)
//	dados->LeDadosFormatados();
//
//   // Verifica se faltam dados de medi��o de corrente
//   if(dados->FaltamDados())
//	{
//      formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " por falta de dados.");
//      FLfinalizado = true;
//   	return;
//   }
//
//   // Executa determina��o da �rea de busca
//	areaBusca->Executa();
//
////   //debug
////	TList* lisBlocosAreaBusca = new TList();
////	TStringList* lisCodTrechosAreaBusca = new TStringList();
////	areaBusca->GetAreaBusca_Blocos(lisBlocosAreaBusca);
////	for(int i=0; i<lisBlocosAreaBusca->Count; i++)
////	{
////		VTBloco* blocoAreaBusca = (VTBloco*) lisBlocosAreaBusca->Items[i];
////		if(!blocoAreaBusca) continue;
////		TList* lisLigaAreaBusca = blocoAreaBusca->LisLigacao();
////		if(!lisLigaAreaBusca) continue;
////
////		for(int j=0; j<lisLigaAreaBusca->Count; j++)
////      {
////			VTLigacao* liga = (VTLigacao*) lisLigaAreaBusca->Items[j];
////			if(liga->Tipo() != eqptoTRECHO) continue;
////         lisCodTrechosAreaBusca->Add(liga->Codigo);
////		}
////	}
////	lisCodTrechosAreaBusca->SaveToFile("c:\\users\\usrsnp\\desktop\\codTrechos.txt");
////	delete lisCodTrechosAreaBusca; lisCodTrechosAreaBusca = NULL;
////	delete lisBlocosAreaBusca; lisBlocosAreaBusca = NULL;
//
//
//   // Exporta �rea de busca para o self healing
//	XMLExportaAreaBusca_SH();
//
//	// Obt�m a rede MT em quest�o
//	redeMT = areaBusca->GetRedeMT();
//	redeSE = areaBusca->GetRedeSE();
//
//	// Classifica o defeito (fases afetadas)
//	classificacao->Executa();
//
//	// Trata da convers�o da rede SINAP -> OPENDSS
//	conversor->Inicializa(dados->GetPathEvento() + "\\RedeDSS", redeMT, redeSE);
//	conversor->Executa();
//
////   if(classificacao->GetStrTipoFalta() == "A" ||
////		classificacao->GetStrTipoFalta() == "B" ||
////      classificacao->GetStrTipoFalta() == "C" ||
////		classificacao->GetStrTipoFalta() == "ABN" ||
////      classificacao->GetStrTipoFalta() == "BCN" ||
////      classificacao->GetStrTipoFalta() == "ACN")
////	{
//////		// IN�CIO ALGORITMO EVOLUTIVO
//////		// ::::::::::::::::::::::::::::::::::::::::::
//////		// Faz a localiza��o de defeitos que envolvem resist�ncia de falta
//////		// -> defeitos 1F
//////		// ::::::::::::::::::::::::::::::::::::::::::
//////
//////		// Insere dados de LOG
//////		strLog->FLMethod = "EvolutionaryAlgorithm";
//////
//////		estrEvol->IniciaTryFault(caminhoDSS);
//////		estrEvol->Executa();
//////
//////		lisSolucoes = new TList();
//////		estrEvol->GetSolucoes(funcoesRede, lisSolucoes);
//////
//////		// Gera os XMLs de solu��o no diret�rio Exporta
//////		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
//////		// FINAL ALGORITMO EVOLUTIVO
////
////
////
////		// In�cio ALGORITMO FASORIAL
////		// ::::::::::::::::::::::::::::::::::::::::::
////		// Faz a localiza��o de defeitos que envolvem resist�ncia de falta
////		// atrav�s do m�todo que utiliza fasores de tens�o e corrente na subesta��o
////		// ::::::::::::::::::::::::::::::::::::::::::
////
////		// Inicializa o objeto de testes de curto-circuito
////		AlgoFasorial->IniciaTryFault(caminhoDSS);
////		AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
////
////   	// Obt�m os fasores de V,I medidos no in�cio do alimentador
////   	StrFasor* Vse = new StrFasor();
////   	StrFasor* Ise = new StrFasor();
////   	dados->GetFasoresVI_SE(Vse, Ise);
////
////		// Chama o c�lculo da imped�ncia total a partir dos fasores V, I na SE
////		AlgoFasorial->CalculaVI_Sequencias012(Vse, Ise);
////
////      if(classificacao->GetStrTipoFalta() == "A" || classificacao->GetStrTipoFalta() == "B" || classificacao->GetStrTipoFalta() == "C")
////   	{
////      	AlgoFasorial->CalculaZtotal_FT(Vse, Ise);
////
////         // Pega o Ztotal medido a partir dos fasores V, I na SE
////         double reZtotal, imZtotal;
////         AlgoFasorial->GetZTotal(reZtotal, imZtotal);
////
////         // Pega os m�dulos de r e x:
////         reZtotal = fabs(reZtotal);
////			imZtotal = fabs(imZtotal);
////
////         String codigoRede = redeMT->Codigo;
////			AlgoFasorial->Executa_FT(codigoRede, reZtotal, imZtotal);
////      }
////      else if(classificacao->GetStrTipoFalta() == "ABN" || classificacao->GetStrTipoFalta() == "BCN" || classificacao->GetStrTipoFalta() == "ACN")
////   	{
////      	AlgoFasorial->CalculaZtotal_seq1_2FT(Vse, Ise);
////
////         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
////         double reZtotal_1, imZtotal_1;
////			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
////
////			// Pega os m�dulos de r e x:
////			reZtotal_1 = fabs(reZtotal_1);
////			imZtotal_1 = fabs(imZtotal_1);
////
////         String codigoRede = redeMT->Codigo;
////         AlgoFasorial->Executa_2FT(codigoRede, reZtotal_1, imZtotal_1);
////		}
////
////      // Obt�m lista de structs de solu��o
////      lisSolucoes = new TList();
////		AlgoFasorial->GetSolucoes(funcoesRede, lisSolucoes);
////
////		// Insere dados de LOG
////		strLog->FLMethod = "";
////		strLog->FLTolerance = 0.1; //mudar aqui
////
////		// Gera os XMLs de solu��o no diret�rio Exporta
////		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
////
////		// Exporta solu��es para o self healing
////		XMLExportaSolucoes_SH();
////		// FINAL ALGORITMO FASORIAL
////	}
//
//
////	//debug - executando o algo fasorial tamb�m para defeitos que n�o envolvem a terra
////	if(classificacao->GetStrTipoFalta() == "AB" ||
////		classificacao->GetStrTipoFalta() == "BC" ||
////		classificacao->GetStrTipoFalta() == "CA" ||
////		classificacao->GetStrTipoFalta() == "ABC" ||
////		classificacao->GetStrTipoFalta() == "ABABC")
////	{
////		// ::::::::::::::::::::::::::::::::::::::::::
////		// Faz a localiza��o de defeitos que envolvem resist�ncia de falta
////      // atrav�s do m�todo que utiliza fasores de tens�o e corrente na subesta��o
////      // ::::::::::::::::::::::::::::::::::::::::::
////
////   	// Inicializa o objeto de testes de curto-circuito
////      AlgoFasorial->IniciaTryFault(caminhoDSS);
////      AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
////
////   	// Obt�m os fasores de V,I medidos no in�cio do alimentador
////   	StrFasor* Vse = new StrFasor();
////   	StrFasor* Ise = new StrFasor();
////   	dados->GetFasoresVI_SE(Vse, Ise);
////
////		// Chama o c�lculo da imped�ncia total a partir dos fasores V, I na SE
////		AlgoFasorial->CalculaVI_Sequencias012(Vse, Ise);
////
////      if(classificacao->GetStrTipoFalta() == "A" || classificacao->GetStrTipoFalta() == "B" || classificacao->GetStrTipoFalta() == "C")
////   	{
////      	AlgoFasorial->CalculaZtotal_FT(Vse, Ise);
////
////         // Pega o Ztotal medido a partir dos fasores V, I na SE
////         double reZtotal, imZtotal;
////			AlgoFasorial->GetZTotal(reZtotal, imZtotal);
////
////         // Pega os m�dulos de r e x:
////         reZtotal = fabs(reZtotal);
////			imZtotal = fabs(imZtotal);
////
////         String codigoRede = redeMT->Codigo;
////         AlgoFasorial->Executa_FT(codigoRede, reZtotal, imZtotal);
////      }
////      else if(classificacao->GetStrTipoFalta() == "ABN" || classificacao->GetStrTipoFalta() == "BCN" || classificacao->GetStrTipoFalta() == "ACN")
////   	{
////      	AlgoFasorial->CalculaZtotal_seq1_2FT(Vse, Ise);
////
////         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
////         double reZtotal_1, imZtotal_1;
////			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
////
////			// Pega os m�dulos de r e x:
////			reZtotal_1 = fabs(reZtotal_1);
////			imZtotal_1 = fabs(imZtotal_1);
////
////         String codigoRede = redeMT->Codigo;
////         AlgoFasorial->Executa_2FT(codigoRede, reZtotal_1, imZtotal_1);
////      }
////      else if(classificacao->GetStrTipoFalta() == "AB" || classificacao->GetStrTipoFalta() == "BC" || classificacao->GetStrTipoFalta() == "AC")
////   	{
////      	AlgoFasorial->CalculaZtotal_seq1_2F(Vse, Ise);
////
////         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
////         double reZtotal_1, imZtotal_1;
////			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
////
////         // Pega os m�dulos de r e x:
////			reZtotal_1 = fabs(reZtotal_1);
////			imZtotal_1 = fabs(imZtotal_1);
////
////         String codigoRede = redeMT->Codigo;
////         AlgoFasorial->Executa_2F(codigoRede, reZtotal_1, imZtotal_1);
////      }
////      else if(classificacao->GetStrTipoFalta() == "ABC")
////   	{
////      	AlgoFasorial->CalculaZtotal_seq1_3F(Vse, Ise);
////
////         // Pega o Ztotal de sequ�ncia positiva, medido a partir dos fasores V, I na SE
////         double reZtotal_1, imZtotal_1;
////			AlgoFasorial->GetZTotal_1(reZtotal_1, imZtotal_1);
////
////         // Pega os m�dulos de r e x:
////			reZtotal_1 = fabs(reZtotal_1);
////			imZtotal_1 = fabs(imZtotal_1);
////
////         String codigoRede = redeMT->Codigo;
////         AlgoFasorial->Executa_3F(codigoRede, reZtotal_1, imZtotal_1);
////      }
////
////
////
////      // Obt�m lista de structs de solu��o
////      lisSolucoes = new TList();
////		AlgoFasorial->GetSolucoes(funcoesRede, lisSolucoes);
////
////		// Insere dados de LOG
////		strLog->FLMethod = "";
////		strLog->FLTolerance = 0.1; //mudar aqui
////
////		// Gera os XMLs de solu��o no diret�rio Exporta
////		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
////
////		// Exporta solu��es para o self healing
////		XMLExportaSolucoes_SH();
////	}
//
//
//	// FAULT STUDY:
//	if(classificacao->GetStrTipoFalta() == "AB" ||
//		classificacao->GetStrTipoFalta() == "BC" ||
//		classificacao->GetStrTipoFalta() == "CA" ||
//		classificacao->GetStrTipoFalta() == "ABC" ||
//		classificacao->GetStrTipoFalta() == "ABABC")
//	{
//		// ::::::::::::::::::::::::::::::::::::::::::
//		// Faz a localiza��o de defeitos que n�o envolvem resit�ncia de falta
//		// -> defeitos 3F e 2F.
//		// ::::::::::::::::::::::::::::::::::::::::::
//      FS->Inicializa(dados->GetPathEvento() + "\\RedeDSS", path->DirDat() + "\\FaultLocation" ,CodigoAlimentador);
//      FS->ExecutaLocalizFaultStudy(classificacao, dados);
//
//		lisSolucoes = new TList();
//		FS->GetSolucoes(funcoesRede, strLog, lisSolucoes);
//
//		// Insere dados de LOG
//		strLog->FLMethod = "FaultStudy";
//		strLog->FLTolerance = FS->GetMaxDesvioIporc();
//		InformacoesAgrupamentoAlarmes(strLog->lisAlarmes);
//
//      // Gera os XMLs de solu��o no diret�rio Exporta
//		XMLGeraSolucoes(lisSolucoes, GetDadosGerais());
//
//		// Exporta solu��es para o self healing
//	   XMLExportaSolucoes_SH();
//
//      // Mostra as barras solu��o no gr�fico
////      OpGraf->InsereMoldurasBarrasFaultStudy(lisSolucoes);
//	}
//
//
//   // Fecha a contagem e tempo de execu��o do algoritmo
//   logErros->AddLinha("Tempo de execu��o do Algo Fasorial: " + String(Round(cronometroFL->GetSegundos(), 2)), true);
//
//   // Ativa flag indicando processo de FL finalizado => para apagar arquivos de diret�rios
//	FLfinalizado = true;
//}

//---------------------------------------------------------------------------
/***
 *    M�todo geral de Localiza��o de Rompimento de Cabo (DMS3)
 */
void __fastcall TThreadFaultLocation::ExecutaFaultLocation_DMS3()
{
	// Inicia flag de thread inicializada
	iniciado = true;
	gerenciador->FL_Executando = this;

   // Faz a tradu��o (XML -> INI) do arquivo de response
	// Gera estrutura de pasta e arquivo Geral.ini
	importaXMLs->GeraBaseDados();
	importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);

	// L� os dados formatados (arquivos .INI -> objetos em mem�ria)
	dados->LeDadosFormatados();

	// Inicializa algoritmo DMS3, para localiza��o de rompimento de cabo
	TAlgoritmoDMS3* algoDMS3 = new TAlgoritmoDMS3(apl, dados);

	algoDMS3->Inicializa();
	algoDMS3->ExecutaLocRompCabo();

	// Obt�m a rede MT em quest�o
	redeMT = algoDMS3->redeMT;
	redeSE = algoDMS3->redeSE;

	// Obt�m objetos de representam os trechos da �rea de busca
	TList* lisStrAreaBuscaDMS3 = new TList;
	algoDMS3->GetAreaBusca(lisStrAreaBuscaDMS3);
	XMLGeraSolucoesDMS3(lisStrAreaBuscaDMS3, GetDadosGerais_DMS3());

	delete algoDMS3;
}
//---------------------------------------------------------------------------
/***
 *    M�todo geral de Localiza��o de Rompimento de Cabo
 */
void __fastcall TThreadFaultLocation::ExecutaLocRompCabo()
{
	// Inicia flag de thread inicializada
	iniciado = true;
	gerenciador->FL_Executando = this;

	// Faz a tradu��o (XML -> INI) do arquivo de response
	// Gera estrutura de pasta e arquivo Geral.ini
	importaXMLs->GeraBaseDados();
	importaXMLs->LeValidaDadosXML(nomeXMLreq, xmlCom, strLog);

	// L� os dados formatados (arquivos .INI -> objetos em mem�ria)
	dados->LeDadosFormatados();

   // Determina a �rea de busca
	areaBusca->Executa_RompCabo();

//	//debug
//	TList* lisAux_trechos = new TList();
//	TStringList* lisAux_codtrechos = new TStringList();
//	areaBusca->GetAreaBuscaRompCabo_Trechos(lisAux_trechos);
//	for(int i=0; i<lisAux_trechos->Count; i++)
//	{
//		VTLigacao* liga = (VTLigacao*) lisAux_trechos->Items[i];
//		lisAux_codtrechos->Add(liga->Codigo);
//	}
//	lisAux_codtrechos->SaveToFile("c:\\users\\usrsnp\\desktop\\trechosAreaBuscaRompCabo.csv");
//	delete lisAux_codtrechos;


	// Obt�m a rede MT em quest�o
	redeMT = areaBusca->GetRedeMT();
	redeSE = areaBusca->GetRedeSE();

	// Classifica o defeito (fases afetadas)
	classificacao->ExecutaRompCabo();

	// Trata da convers�o da rede SINAP -> OPENDSS
	conversor->Inicializa(dados->GetPathEvento() + "\\RedeDSS", redeMT, redeSE);
	conversor->Executa(true);

	// Inicializa o objeto de testes de rompimento de cabo
	AlgoRompCabo->IniciaTryRompCabo(caminhoDSS);

	// Executa o algoritmo de localiza��o do rompimento de cabo
	AlgoRompCabo->ExecutaLocRompCabo();

	// Insere dados de LOG
	strLog->FLMethod = "High Impedance";
	strLog->FLTolerance = 0;

	// Obt�m lista de structs de solu��o
	lisSolucoes = new TList();
	AlgoRompCabo->GetSolucoes(funcoesRede, lisSolucoes);

	// Gera os XMLs de solu��o no diret�rio Exporta
	XMLGeraSolucoesRompCabo(lisSolucoes, GetDadosGerais());

	// Ativa flag indicando processo de FL finalizado => para apagar arquivos de diret�rios
   FLfinalizado = true;
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::ExistemAlarmesQualimetros(bool qualimetroEmChaveProtecao)
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 5: alarme de afundamento de tens�o de qual�metro
		// 6: alarme de sobrecorrente registrada por qual�metro
		if(alarme->GetTipoAlarme() == 5 || alarme->GetTipoAlarme() == 6)
      {
         // Verifica se o qual�metro estiver associado a eqpto de prote��o (DJ ou RE)
         if(qualimetroEmChaveProtecao && QualimetroChaveProtecao(alarme->GetCodAlimentador(), alarme->GetCodEqpto()))
            return(true);
         else if(!qualimetroEmChaveProtecao)
            return(true);
      }
	}
	return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::ExistemAlarmesTrafoInteligente()
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 7: alarme de afundamento de tens�o registrado por trafo inteligente (VSAGTR)
		if(alarme->GetTipoAlarme() == 7)
		{
			return(true);
      }
	}
	return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::ExistemAlarmesLastGasp()
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 22: alarme de last gasp
		if(alarme->GetTipoAlarme() == 22)
		{
			return(true);
      }
	}
	return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::ExistemAlarmesSubtensao()
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 5: alarme de afundamento de tens�o registrado por qual�metro
		// 7: alarme de afundamento de tens�o registrado por trafo inteligente
		if(alarme->GetTipoAlarme() == 5 || alarme->GetTipoAlarme() == 7)
      {
            return(true);
      }
	}
	return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::ExistemAlarmesSobrecorrenteQual()
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 6: alarme de sobrecorrente registrado por qual�metro
		if(alarme->GetTipoAlarme() == 6)
      {
            return(true);
      }
	}
	return(false);
}
////---------------------------------------------------------------------------
//bool __fastcall TThreadFaultLocation::ExistemAlarmesSubTensaoQualimetro()
//{
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//		// 5: alarme de afundamento de tens�o de qual�metro
//		if(alarme->GetTipoAlarme() == 5)
//			return(true);
//	}
//	return(false);
//}
////---------------------------------------------------------------------------
//bool __fastcall TThreadFaultLocation::ExistemAlarmesSobrecorrenteQualimetro()
//{
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//		// 5: alarme de afundamento de tens�o de qual�metro
//		if(alarme->GetTipoAlarme() == 5)
//			return(true);
//	}
//	return(false);
//}
////---------------------------------------------------------------------------
//bool __fastcall TThreadFaultLocation::ExistemAlarmesProtecao()
//{
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//
//		// 1: FREL, 2: RAUT
//		if(alarme->GetTipoAlarme() == 1 || alarme->GetTipoAlarme() == 2)
//			return(true);
//	}
//	return(false);
//}//---------------------------------------------------------------------------bool __fastcall TThreadFaultLocation::ExistemAlarmesProtecao()
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		// 20: alarme de tentativa de religamento
		// 21: alarme de autobloqueio
		if(alarme->GetTipoAlarme() == 20 || alarme->GetTipoAlarme() == 21)
			return(true);
	}
	return(false);
}//---------------------------------------------------------------------------bool __fastcall TThreadFaultLocation::FaltaEnvolveTerra()
{
	bool apenasA, apenasB, apenasC, umaFase, neutro;
	bool apenasAB, apenasBC, apenasCA, duasFases;
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
		if(alarme->TipoAlarme != 20 && alarme->TipoAlarme != 21) continue;

		// Verifica se apenas uma fase foi afetada
		apenasA = (alarme->funcao50A && !alarme->funcao50B && !alarme->funcao50C);
		apenasB = (!alarme->funcao50A && alarme->funcao50B && !alarme->funcao50C);
		apenasC = (!alarme->funcao50A && !alarme->funcao50B && alarme->funcao50C);
		umaFase = (apenasA || apenasB || apenasC);

		// Verifica se apenas duas fases foram afetadas
		apenasAB = (alarme->funcao50A && alarme->funcao50B && !alarme->funcao50C);
		apenasBC = (!alarme->funcao50A && alarme->funcao50B && alarme->funcao50C);
		apenasCA = (alarme->funcao50A && !alarme->funcao50B && alarme->funcao50C);
		duasFases = (apenasAB || apenasBC || apenasCA);

		// Verifica se a fun��o de neutro foi sensibilizada
		neutro = (alarme->funcao50N || alarme->funcao51N);

		if((umaFase || duasFases) && neutro)
			return(true);
	}
	return(false);
}//---------------------------------------------------------------------------
/***
 * M�todo monitora a pasta Importa/FaultLocation/Mensagens para verificar
 * se chegou o XML com as medi��es (dados adicionais) solicitados ao M�d. de Supervis�o.
 */
bool __fastcall TThreadFaultLocation::ExistemMedicoes()
{
  	bool resp;
	String CodAlimentador;
	String TimeStamp;
   String pathDirMensagens = path->DirImporta() + "\\FaultLocation\\Mensagens\\";
	String nomeXML, nomeXMLbuscado;
   TAlarme* alarme;

	// Pega o alarme trigger do processo (timestamp mais antigo)
   alarme = (TAlarme*) listaAlarmes->Items[0];

   // Lista de strings com os nomes dos arquivos do diret�rio
 	lisNomesXMLMsg->Clear();
   get_all_files_names_within_folder(pathDirMensagens, lisNomesXMLMsg);

	resp = false;
	for(int i=0; i<lisNomesXMLMsg->Count; i++)
	{
		nomeXML = lisNomesXMLMsg->Strings[i];
   	nomeXMLbuscado = "REQ_" + alarme->GetCodAlimentador() + "_" + alarme->GetTimeStamp() + ".xml";

   	if(nomeXML == nomeXMLbuscado)
   	{
			resp = true;
      	nomeXMLreq = nomeXMLbuscado;
      }
   }

	return resp;
}
//---------------------------------------------------------------------------
TList* __fastcall TThreadFaultLocation::GetAlarmes()
{
	return listaAlarmes;
}
//---------------------------------------------------------------------------
String __fastcall TThreadFaultLocation::GetCodigoAlimentador()
{
	return CodigoAlimentador;
}
//---------------------------------------------------------------------------
/***
 * Gera struct com dados gerais acerca do processo de localiza��o.
 */
StrDadosGerais* __fastcall TThreadFaultLocation::GetDadosGerais()
{
   StrDadosGerais* strGer;
	String codEqptoTrigger;
   String codigoEqptoCampoAreaBusca;
	TAlarme* alarme;
   TEqptoCampo* eqptoCampoAreaBusca;

   strGer = new StrDadosGerais();
	strGer->TimeStamp = TimeStamp;

   // Topologia
	strGer->Alimentador = CodigoAlimentador;
	strGer->SE = CodigoAlimentador.SubString(1, 3);

	// Conjunto de alarmes / triggers
	strGer->lisTriggers = listaAlarmes;

	// Informa��es do primeiro trigger (1o alarme)
	alarme = (TAlarme*) listaAlarmes->Items[0];
	codEqptoTrigger = alarme->GetCodEqpto();
	strGer->EqptoTrigger = codEqptoTrigger;

	// Informa��es da conting�ncia
   eqptoCampoAreaBusca = areaBusca->eqptoCampo_DeterminaAreaBusca;
   if(eqptoCampoAreaBusca->GetTipo() == chaveDJ || eqptoCampoAreaBusca->GetTipo() == chaveRE)
   {
      TChaveMonitorada* chaveMon = (TChaveMonitorada*) eqptoCampoAreaBusca;
      codigoEqptoCampoAreaBusca = chaveMon->GetChaveAssociada()->Codigo;
   }
   else if(eqptoCampoAreaBusca->GetTipo() == eqptoSENSOR)
   {
      TSensor* sensor = (TSensor*) eqptoCampoAreaBusca;
      codigoEqptoCampoAreaBusca = sensor->GetLigacaoAssociada()->Codigo;
   }

   // Pega o n�mero de clientes afetados
   strGer->ClientesAfetados = funcoesRede->GetNumConsJusLigacao(codigoEqptoCampoAreaBusca);

	// Tipo de falta
	strGer->TipoFalta = classificacao->GetStrTipoFalta();

   // Indica��o de rompimento de cabo, com base nos �ngulos de fase das correntes
   // das fases s�s
   strGer->rompCabo = classificacao->CorrentesOpostasFasesSas;

	return strGer;
}
//---------------------------------------------------------------------------
/***
 * Gera struct com dados gerais acerca do processo de localiza��o de rompimento de cabo
 */
StrDadosGerais* __fastcall TThreadFaultLocation::GetDadosGerais_DMS3()
{
   StrDadosGerais* strGer;
	String codEqptoTrigger;
	TAlarme* alarme;

	strGer = new StrDadosGerais();
	strGer->TimeStamp = TimeStamp;

   // Topologia
	strGer->Alimentador = CodigoAlimentador;
	strGer->SE = CodigoAlimentador.SubString(1, 3);

	// Conjunto de alarmes / triggers
	strGer->lisTriggers = listaAlarmes;

	// Informa��es do primeiro trigger (1o alarme)
	alarme = (TAlarme*) listaAlarmes->Items[0];
	codEqptoTrigger = alarme->GetCodEqpto();
	strGer->EqptoTrigger = codEqptoTrigger;

	// Informa��es da conting�ncia
//	strGer->ClientesAfetados = funcoesRede->GetNumConsJusLigacao(codEqptoTrigger);

	// Tipo de falta
	strGer->TipoFalta = "";
   strGer->rompCabo = true;

	return strGer;
}
//---------------------------------------------------------------------------
/***
 * Gera struct com dados gerais acerca do processo de localiza��o de rompimento de cabo
 */
StrDadosGerais* __fastcall TThreadFaultLocation::GetDadosGeraisRompCabo()
{
   StrDadosGerais* strGer;
	String codEqptoTrigger;
	TAlarme* alarme;

   strGer = new StrDadosGerais();
	strGer->TimeStamp = TimeStamp;

   // Topologia
	strGer->Alimentador = CodigoAlimentador;
	strGer->SE = CodigoAlimentador.SubString(1, 3);

	// Conjunto de alarmes / triggers
	strGer->lisTriggers = listaAlarmes;

	// Informa��es do primeiro trigger (1o alarme)
	alarme = (TAlarme*) listaAlarmes->Items[0];
	codEqptoTrigger = alarme->GetCodEqpto();
	strGer->EqptoTrigger = codEqptoTrigger;

	// Informa��es da conting�ncia
	strGer->ClientesAfetados = funcoesRede->GetNumConsJusLigacao(codEqptoTrigger);

	// Tipo de falta
	strGer->TipoFalta = "";
   strGer->rompCabo = true;

	return strGer;
}
//---------------------------------------------------------------------------
StrDadosLog* __fastcall TThreadFaultLocation::GetDadosLog()
{
	StrDadosLog* strLog;

   strLog = new StrDadosLog();

	return strLog;
}
//---------------------------------------------------------------------------
TList* __fastcall TThreadFaultLocation::GetListaAlarmes()
{
	return listaAlarmes;
}
//---------------------------------------------------------------------------
String __fastcall TThreadFaultLocation::GetNomeArquivo()
{
	return NomeArquivo;
}
//---------------------------------------------------------------------------
String __fastcall TThreadFaultLocation::GetPathRootDir()
{
	String CodigoEvento = "", rootDir;
	TAlarme* alarme;


   alarme = (TAlarme*) listaAlarmes->Items[0];
   CodigoEvento = alarme->GetCodAlimentador() + alarme->GetTimeStamp();

   rootDir = path->DirDat() + "\\FaultLocation\\Dados\\" + CodigoEvento;

	return rootDir;
}
////---------------------------------------------------------------------------
//StrProcessoLF* __fastcall TThreadFaultLocation::GetStrProcessoLF()
//{
//	StrProcessoLF* strProcLF = new StrProcessoLF();
//
//	// Salva ponteiro para a rede MT afetada pelo defeito
//	strProcLF->redeMT = redeMT;
//}//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::InformacoesAgrupamentoAlarmes(TList* lisEXT)
{
	if(!lisEXT) return;
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
		lisEXT->Add(alarme);
   }
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::InicializaDadosLog()
{
	strLog = new StrDadosLog();
	strLog->FLMethod = "";
	strLog->FLTolerance = 0.;
	strLog->lisInputData = new TList();
	strLog->lisBadData = new TList();
	strLog->lisEqptosErrosAquisicao = new TStringList();
	strLog->lisAlarmes = new TList();
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::IniciaFlags()
{
	nomeXMLreq = "";
   timerTempo = 0;
	timerTimeout = 0;
   janelaAberta = true;
	iniciado = false;
	FLfinalizado = false;
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::JanelaAberta()
{
	return janelaAberta;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::MostraResultados()
{
	TList* lisAreaBusca = new TList();
   areaBusca->GetAreaBusca_Blocos(lisAreaBusca);

	// Limpa memo
   MemoResultados->Lines->Clear();

	// Insere os IDs das barras de localiza��o por fault study
	MemoResultados->Lines->Add("BARRAS DE LOCALIZA��O POR FAULT STUDY:\n");
   for(int i=0; i<lisBarrasDefeitoFaultStudy->Count; i++)
   {
		VTBarra* barra = (VTBarra*) lisBarrasDefeitoFaultStudy->Items[i];
      MemoResultados->Lines->Add(barra->Codigo);
   }

   MemoResultados->Lines->Add("");

	// Insere os c�digos das barras de localiza��o por dist�ncia
	MemoResultados->Lines->Add("BARRAS DE LOCALIZA��O POR DIST�NCIA:\n");
   for(int i=0; i<listaBarrasSol_Dist->Count; i++)
   {
		VTBarra* barra = (VTBarra*) listaBarrasSol_Dist->Items[i];
      MemoResultados->Lines->Add(barra->Codigo);
   }

   MemoResultados->Lines->Add("");

   // Insere os c�digos das barras da �rea de busca
	MemoResultados->Lines->Add("BARRAS DA �REA DE BUSCA:");
   for(int i=0; i<lisAreaBusca->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) lisAreaBusca->Items[i];
    	// Insere o c�digo da primeira barra do bloco
		VTBarra* barra0 = (VTBarra*) bloco->LisBarra()->Items[0];
      MemoResultados->Lines->Add(barra0->Codigo);
   }

   delete lisAreaBusca;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::MostraResultadosTreeView_DMS3(TList* lisStrSolucaoDMS3)
{
	StrDadosGerais* strDadosGerais;
	String          Item_CodProcesso, Item_SE, Item_Alimentador, Item_Solucoes, Item_Solucao, Item_Algoritmo;
	String          Item_Coord_X, Item_Coord_Y, Item_ChaveMont;
	String          Item_IndiceErro, Item_Rfalta;
	strSolucao*     strSol;
	TTreeView*      tvProcLF = formFL->TreeViewProcessosLF;
	TTreeNode*      LFProcessNode;
	TTreeNode*      SolutionsNode;
	TTreeNode*      SolutionNode;

	strDadosGerais = GetDadosGerais_DMS3();
	Item_CodProcesso = strDadosGerais->Alimentador + " - " + strDadosGerais->TimeStamp;
	Item_SE = "Subesta��o: " + redeSE->Codigo;
	Item_Alimentador = "Alimentador: " + redeMT->Codigo;
	Item_Algoritmo = "M�todo de localiza��o: Algoritmo 3";
	Item_Solucoes = "Liga��es da solu��o";

	// Adiciona itens ao TreeView
	LFProcessNode = tvProcLF->Items->AddObjectFirst(NULL, Item_CodProcesso, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_SE, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Alimentador, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Algoritmo, NULL);
	SolutionsNode = tvProcLF->Items->AddChildObject(LFProcessNode, Item_Solucoes, lisSolucoes);

	// Adiciona as solu��es ao TreeView
	if(!lisStrSolucaoDMS3) return;

	for(int i=0; i<lisStrSolucaoDMS3->Count; i++)
	{
		StrSolucaoDMS3* solucaoDMS3 = (StrSolucaoDMS3*) lisStrSolucaoDMS3->Items[i];

		Item_Solucao = "Liga��o " + String(i+1);
		SolutionNode = tvProcLF->Items->AddChildObject(SolutionsNode, Item_Solucao, solucaoDMS3);
	}
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::MostraResultadosTreeView_DMS2(TList* lisStrSolucaoDMS2)
{
	StrDadosGerais* strDadosGerais;
	String          Item_CodProcesso, Item_SE, Item_Alimentador, Item_Solucoes, Item_Solucao, Item_Algoritmo;
	String          Item_Coord_X, Item_Coord_Y, Item_ChaveMont;
	String          Item_IndiceErro, Item_Rfalta;
	strSolucao*     strSol;
	TTreeView*      tvProcLF = formFL->TreeViewProcessosLF;
	TTreeNode*      LFProcessNode;
	TTreeNode*      SolutionsNode;
	TTreeNode*      SolutionNode;

   strDadosGerais = GetDadosGerais();
	Item_CodProcesso = strDadosGerais->Alimentador + " - " + strDadosGerais->TimeStamp;
	Item_SE = "Subesta��o: " + redeSE->Codigo;
	Item_Alimentador = "Alimentador: " + redeMT->Codigo;
	Item_Algoritmo = "M�todo de localiza��o: Algoritmo 2";
	Item_Solucoes = "Blocos da solu��o";

	// Adiciona itens ao TreeView
	LFProcessNode = tvProcLF->Items->AddObjectFirst(NULL, Item_CodProcesso, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_SE, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Alimentador, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Algoritmo, NULL);
	SolutionsNode = tvProcLF->Items->AddChildObject(LFProcessNode, Item_Solucoes, lisSolucoes);

	// Adiciona as solu��es ao TreeView
	if(!lisStrSolucaoDMS2) return;

	for(int i=0; i<lisStrSolucaoDMS2->Count; i++)
	{
		StrSolucaoDMS2* solucaoDMS2 = (StrSolucaoDMS2*) lisStrSolucaoDMS2->Items[i];

		Item_Solucao = "Bloco " + String(i+1);
		SolutionNode = tvProcLF->Items->AddChildObject(SolutionsNode, Item_Solucao, solucaoDMS2);
	}
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::MostraResultadosTreeView()
{
	StrDadosGerais* strDadosGerais;
	String          Item_CodProcesso, Item_SE, Item_Alimentador, Item_Solucoes, Item_Solucao, Item_Algoritmo;
	String          Item_Coord_X, Item_Coord_Y, Item_ChaveMont;
	String          Item_IndiceErro, Item_Rfalta;
	strSolucao*     strSol;
	TTreeView*      tvProcLF = formFL->TreeViewProcessosLF;
	TTreeNode*      LFProcessNode;
	TTreeNode*      SolutionsNode;
	TTreeNode*      SolutionNode;

   strDadosGerais = GetDadosGerais();
	Item_CodProcesso = strDadosGerais->Alimentador + " - " + strDadosGerais->TimeStamp;
	Item_SE = "Subesta��o: " + redeSE->Codigo;
	Item_Alimentador = "Alimentador: " + redeMT->Codigo;
	Item_Algoritmo = "M�todo de localiza��o: Algoritmo 1";
	Item_Solucoes = "Solu��es";

	// Adiciona itens ao TreeView
	LFProcessNode = tvProcLF->Items->AddObjectFirst(NULL, Item_CodProcesso, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_SE, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Alimentador, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Algoritmo, NULL);
	SolutionsNode = tvProcLF->Items->AddChildObject(LFProcessNode, Item_Solucoes, lisSolucoes);

	// Adiciona as solu��es ao TreeView
   if(lisSolucoes != NULL)
	{
      for(int i=0; i<lisSolucoes->Count; i++)
      {
         strSol = (strSolucao*) lisSolucoes->Items[i];

      	Item_Solucao = "Solu��o " + String(i+1);
         SolutionNode = tvProcLF->Items->AddChildObject(SolutionsNode, Item_Solucao, strSol);

         // Insere coordenadas X e Y
      	Item_Coord_X = "Coordenada Lat:" + strSol->DefLat;
      	Item_Coord_Y = "Coordenada Lon:" + strSol->DefLon;
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Coord_X, NULL);
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Coord_Y, NULL);

      	// Insere o c�digo da chave � montante
         Item_ChaveMont = "Chave montante:" + strSol->ChvMont;
			tvProcLF->Items->AddChildObject(SolutionNode, Item_ChaveMont, strSol);

			// Insere o �ndice de erro da solu��o
			Item_IndiceErro = "�ndice de Erro:" + String(Round(strSol->IndiceErro,4));
			tvProcLF->Items->AddChildObject(SolutionNode, Item_IndiceErro, strSol);

			// Insere a resist�ncia de falta estimada para a solu��o
			Item_Rfalta = "Resist�ncia de falta estimada:" + String(Round(strSol->Rfalta_estimado,2));
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Rfalta, strSol);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::MostraResultadosTreeView_DMS1()
{
	StrDadosGerais* strDadosGerais;
	String          Item_CodProcesso, Item_SE, Item_Alimentador, Item_Solucoes, Item_Solucao, Item_Algoritmo;
	String          Item_Coord_X, Item_Coord_Y, Item_ChaveMont;
	String          Item_IndiceErro, Item_Rfalta;
	strSolucao*     strSol;
	TTreeView*      tvProcLF = formFL->TreeViewProcessosLF;
	TTreeNode*      LFProcessNode;
	TTreeNode*      SolutionsNode;
	TTreeNode*      SolutionNode;

   strDadosGerais = GetDadosGerais();
	Item_CodProcesso = strDadosGerais->Alimentador + " - " + strDadosGerais->TimeStamp;
	Item_SE = "Subesta��o: " + redeSE->Codigo;
	Item_Alimentador = "Alimentador: " + redeMT->Codigo;
	Item_Algoritmo = "M�todo de localiza��o: Algoritmo 1";
	Item_Solucoes = "Solu��es";

	// Adiciona itens ao TreeView
	LFProcessNode = tvProcLF->Items->AddObjectFirst(NULL, Item_CodProcesso, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_SE, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Alimentador, NULL);
	tvProcLF->Items->AddChildObject(LFProcessNode, Item_Algoritmo, NULL);
	SolutionsNode = tvProcLF->Items->AddChildObject(LFProcessNode, Item_Solucoes, lisSolucoes);

	// Adiciona as solu��es ao TreeView
   if(lisSolucoes != NULL)
	{
      for(int i=0; i<lisSolucoes->Count; i++)
      {
         strSol = (strSolucao*) lisSolucoes->Items[i];

      	Item_Solucao = "Solu��o " + String(i+1);
         SolutionNode = tvProcLF->Items->AddChildObject(SolutionsNode, Item_Solucao, strSol);

         // Insere coordenadas X e Y
      	Item_Coord_X = "Coordenada Lat:" + strSol->DefLat;
      	Item_Coord_Y = "Coordenada Lon:" + strSol->DefLon;
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Coord_X, NULL);
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Coord_Y, NULL);

      	// Insere o c�digo da chave � montante
         Item_ChaveMont = "Chave montante:" + strSol->ChvMont;
			tvProcLF->Items->AddChildObject(SolutionNode, Item_ChaveMont, strSol);

			// Insere o �ndice de erro da solu��o
			Item_IndiceErro = "�ndice de Erro:" + String(Round(strSol->IndiceErro,4));
			tvProcLF->Items->AddChildObject(SolutionNode, Item_IndiceErro, strSol);

			// Insere a resist�ncia de falta estimada para a solu��o
			Item_Rfalta = "Resist�ncia de falta estimada:" + String(Round(strSol->Rfalta_estimado,2));
			tvProcLF->Items->AddChildObject(SolutionNode, Item_Rfalta, strSol);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::SalvaAlarme(TAlarme* alarme)
{
	if(listaAlarmes->IndexOf(alarme) < 0)
		listaAlarmes->Add(alarme);

   if(CodigoAlimentador == "")
		CodigoAlimentador = alarme->GetCodAlimentador();

   if(TimeStamp == "")
		TimeStamp = alarme->GetTimeStamp();

	if(CodigoAlimentador != "" && TimeStamp != "")
		CodigoEvento = CodigoAlimentador + " - " + TimeStamp;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::AjustaAlarmesEvento()
{
   // Se houver alarmes FREL de disjuntor E de religadora, desconsideramos o alarme FREL de disjuntor
	AjustaAlarmesFREL();

   // Verifica se o mesmo equipamento emitiu mais de um alarme, do mesmo tipo. Caso positivo,
   // elimina os repetidos.
	EliminaAlarmesRepetidos();
}
//---------------------------------------------------------------------------
// Verifica se o mesmo equipamento emitiu mais de um alarme, do mesmo tipo. Caso positivo,
// elimina os repetidos.
void __fastcall TThreadFaultLocation::EliminaAlarmesRepetidos()
{
	TAlarme *alarme, *alarmeAdicionado;

 	if(listaAlarmes->Count == 0)
		return;

	// Transfere alarmes para lista auxiliar
	TList* lisAux = new TList();
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
		lisAux->Add(alarme);
	}
	listaAlarmes->Clear();

	// Retorna os alarmes para a lista original, verificando se j� existe
	for(int i=lisAux->Count-1; i>=0; i--)
	{
		alarme = (TAlarme*) lisAux->Items[i];

		// Remove o alarme da lista auxiliar
		lisAux->Remove(alarme);

		// Se a lista alarmes est� vazia, adiciona alarme sem verificar
		if(listaAlarmes->Count == 0)
			listaAlarmes->Add(alarme);
		else
		// Se o alarme ainda n�o foi adicionado � lista de alarmes, adiciona-o
		{
			alarmeAdicionado = NULL;
			for(int j=0; j<listaAlarmes->Count; j++)
			{
				alarmeAdicionado = (TAlarme*) listaAlarmes->Items[j];
				if(alarmeAdicionado->GetCodEqpto() == alarme->GetCodEqpto() &&
					alarmeAdicionado->GetTipoAlarme() == alarme->GetTipoAlarme())
					break;
				else
					alarmeAdicionado = NULL;
			}
			if(alarmeAdicionado == NULL)
				listaAlarmes->Add(alarme);
			else
				delete alarme;
      }
	}
	delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
// Verifica os alarmes recebidos. Se houver alarmes FREL de disjuntor E de religadora,
// desconsideramos o alarme FREL de disjuntor, pois a religadora est� mais pr�xima
// do defeito, teoricamente.
void __fastcall TThreadFaultLocation::AjustaAlarmesFREL()
{
	TList* listaAlarmesFREL_DJ = NULL;
   TList* listaAlarmesFREL_RE = NULL;
   TAlarme* alarmeFREL_DJ = NULL;
	TAlarme* alarmeFREL_RE = NULL;

	if(listaAlarmes->Count == 0)
		return;

   listaAlarmesFREL_DJ = new TList();
	listaAlarmesFREL_RE = new TList();

   // Pega alarmes FREL de disjuntor
	for(int i=listaAlarmes->Count-1; i>=0; i--)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
      if(alarme->GetTipoEqpto() == 1 && alarme->GetTipoAlarme() == 1)
   	{
      	listaAlarmesFREL_DJ->Add(alarme);
      }
   }

   // Dentre os alarmes de FREL de disjuntor, considera apenas o mais antigo (o primeiro no tempo)
   if(listaAlarmesFREL_DJ->Count == 1)
   {
      alarmeFREL_DJ = (TAlarme*) listaAlarmesFREL_DJ->Items[0];
   }
   else if(listaAlarmesFREL_DJ->Count > 1)
   {
      // Pega o alarme FREL de DJ mais antigo
      alarmeFREL_DJ = (TAlarme*) listaAlarmesFREL_DJ->Items[0];
      for(int i=1; i<listaAlarmesFREL_DJ->Count; i++)
      {
         TAlarme* alarme = (TAlarme*) listaAlarmesFREL_DJ->Items[i];
         if(GetTimeStamp(alarme) < GetTimeStamp(alarmeFREL_DJ))
      		alarmeFREL_DJ = alarme;
      }

      // Apaga os demais alarmes FREL de DJ
      for(int i=listaAlarmes->Count-1; i>=0; i--)
      {
         TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
      	if(alarme->GetTipoEqpto() == 1 && alarme != alarmeFREL_DJ)
      		listaAlarmes->Remove(alarme);
      }
   }

   // neste ponto, temos o alarme alarmeFREL_DJ

   // Seleciona os alarmes FREL de religadora
   for(int i=listaAlarmes->Count-1; i>=0; i--)
   {
      TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
      if(alarme->GetTipoAlarme() == 1 && alarme->GetTipoEqpto() == 2)
   	{
      	listaAlarmesFREL_RE->Add(alarme);
      }
   }

   // Dentre os alarmes de FREL de religadora, considera apenas o mais antigo (o primeiro no tempo)
   if(listaAlarmesFREL_RE->Count == 1)
   {
      alarmeFREL_RE = (TAlarme*) listaAlarmesFREL_RE->Items[0];
   }
   else if(listaAlarmesFREL_RE->Count > 1)
   {
      // Pega o alarme FREL de RE mais antigo
      alarmeFREL_RE = (TAlarme*) listaAlarmesFREL_RE->Items[0];
      for(int i=1; i<listaAlarmesFREL_RE->Count; i++)
      {
         TAlarme* alarme = (TAlarme*) listaAlarmesFREL_RE->Items[i];
         if(GetTimeStamp(alarme) < GetTimeStamp(alarmeFREL_RE))
      		alarmeFREL_RE = alarme;
      }

      // Apaga os demais alarmes FREL de RE
      for(int i=listaAlarmes->Count-1; i>=0; i--)
      {
         TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
      	if(alarme->GetTipoEqpto() == 2 && alarme != alarmeFREL_RE)
      		listaAlarmes->Remove(alarme);
      }
   }

   // Se houver alarmes FREL de Religadora, desconsidera o alarme FREL de disjuntor
   if(alarmeFREL_DJ && alarmeFREL_RE)
   {
    	listaAlarmes->Remove(alarmeFREL_DJ);
   }

   // Destroi lista
	if(listaAlarmesFREL_DJ) {delete listaAlarmesFREL_DJ; listaAlarmesFREL_DJ = NULL;}
	if(listaAlarmesFREL_RE) {delete listaAlarmesFREL_RE; listaAlarmesFREL_RE = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::AtualizaCodigoEvento()
{
	if(listaAlarmes->Count == 0)
		return;

   TAlarme* alarmeReferencia = (TAlarme*) listaAlarmes->Items[0];

   CodigoAlimentador = alarmeReferencia->GetCodAlimentador();
   TimeStamp = alarmeReferencia->GetTimeStamp();
   CodigoEvento = CodigoAlimentador + " - " + TimeStamp;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::SetLViewMonitores(TListView* LViewMonitores)
{
	this->LViewMonitores = LViewMonitores;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::SetMemoProcessosLF(TMemo* MemoProcessosLF)
{
   this->MemoProcessosLF = MemoProcessosLF;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::SetMemoResultados(TMemo* MemoResultados)
{
   this->MemoResultados = MemoResultados;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::SetMaxJanelaDados(int MaxJanelaDados)
{
	this->MaxJanelaDados = MaxJanelaDados;
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::VerificaTipoLocalizacao()
{
	tipoLF = faultlocationINDEF;
	if(ExistemAlarmesProtecao() && !FaltaEnvolveTerra())
	{
		tipoLF = faultlocationDMS1;
	}
	else if(ExistemAlarmesProtecao() && FaltaEnvolveTerra())
	{
		tipoLF = faultlocationDMS2;
	}
	else if(!ExistemAlarmesProtecao() && ExistemAlarmesLastGasp())
	{
		tipoLF = faultlocationDMS3;
	}

//	if(ExistemAlarmesProtecao() && !ExistemAlarmesQualimetros(true))
//	{
//		tipoLF = faultlocationFS;
//		caboRompido = false;
//	}
//	else if(ExistemAlarmesProtecao() && ExistemAlarmesQualimetros(true))
//	{
//		tipoLF = faultlocationALGOFASORIAL;
//		caboRompido = false;
//	}
//	else if(!ExistemAlarmesProtecao() && ExistemAlarmesSubtensao() && !ExistemAlarmesSobrecorrenteQual())
//	{
//		tipoLF = faultlocationROMPCABO;
//		caboRompido = true;
//	}
//	else if(!ExistemAlarmesProtecao() && !ExistemAlarmesQualimetros(false))
//	{
//		tipoLF = faultlocationINDEF;
//		caboRompido = false;
//	}
}
//---------------------------------------------------------------------------
/***
 * M�todo que escreve um XML de solu��o, para ser lido pelo M�d. de Supervis�o
 */
void __fastcall TThreadFaultLocation::XMLGeraSolucoesDMS1(TList* lisSolucoes, StrDadosGerais* strDadosGerais)
{
	strSolucao* solucao;
	String pathFinal;
   TXMLComunicacao* xmlCom;

	// Verifica��o
	if(lisSolucoes == NULL) return;

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Composi��o do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += "SOL_" + CodigoAlimentador + "_" + TimeStamp;
	pathFinal += ".xml";

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Gera��o de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	// Cria objeto XML de solu��o, passando o caminho (path) e o conte�do
	xmlCom = new TXMLComunicacao(pathFinal, XMLsolution);

	// Adiciona dados gerais
	xmlCom->AddDadosGerais(strDadosGerais);

	// Adiciona informa��es de LOG
	xmlCom->AddDadosLog(strLog);

	// Adiciona solu��es
	xmlCom->AddSolutions(lisSolucoes);

   // Persiste o arquivo XML
	xmlCom->Salvar();

   // Cria n� para o processo de LF em TreeView do form
	MostraResultadosTreeView_DMS1();

	// Adiciona informa��o ao memo do form principal
	MemoProcessosLF->Lines->Add("   Evento " + CodigoEvento + " - Exportando solu��es");
   formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " com sucesso.");
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::XMLGeraSolucoesRompCabo(TList* lisSolucoes, StrDadosGerais* strDadosGerais)
{
	String pathFinal;

	// Verifica��o
	if(lisSolucoes == NULL || strDadosGerais == NULL) return;

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Composi��o do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += "SOL_" + CodigoAlimentador + "_" + TimeStamp;
	pathFinal += ".xml";

   // :::::::::::::::::::::::::::::::::::::::::::::::::
	// Gera��o de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	// Cria objeto XML de solu��o, passando o caminho (path) e o conte�do
	xmlCom = new TXMLComunicacao(pathFinal, XMLsolution);

	// Adiciona dados gerais
	xmlCom->AddDadosGerais(strDadosGerais);

	// Adiciona informa��es de LOG
	xmlCom->AddDadosLog(strLog);

	// Adiciona solu��es
	xmlCom->AddSolutions(lisSolucoes);

	// Persiste o arquivo XML
	xmlCom->Salvar();

	// Cria n� para o processo de LF em TreeView do form
	MostraResultadosTreeView();

	// Adiciona informa��o ao memo do form principal
	MemoProcessosLF->Lines->Add("   Evento " + CodigoEvento + " - Exportando solu��es");
	formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " com sucesso.");
}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::XMLGeraSolucoesDMS3(TList* lisStrSolucaoDMS3, StrDadosGerais* strDadosGerais)
{
	if(!lisStrSolucaoDMS3 || !strDadosGerais) return;

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Composi��o do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	String pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += "SOL_" + CodigoAlimentador + "_" + TimeStamp;
	pathFinal += ".xml";

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Gera��o de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	// Cria objeto XML de solu��o, passando o caminho (path) e o conte�do
	xmlCom = new TXMLComunicacao(pathFinal, XMLsolutionDMS3);

	// Adiciona dados gerais
	xmlCom->AddDadosGerais(strDadosGerais);

	// Adiciona informa��es de LOG
	xmlCom->AddDadosLog(strLog);

	// Adiciona solu��es geradas pelo algoritmo 3 (DMS3): liga��es que formam a solu��o
	TStringList* lisCodLigacoes_AreaBusca = new TStringList;
	for(int i=0; i<lisStrSolucaoDMS3->Count; i++)
	{
		StrSolucaoDMS3* solDMS3 = (StrSolucaoDMS3*) lisStrSolucaoDMS3->Items[i];
		lisCodLigacoes_AreaBusca->Add(solDMS3->ligacao->Codigo);
	}
	xmlCom->InsereLigacoesSolucoes(lisCodLigacoes_AreaBusca);
	delete lisCodLigacoes_AreaBusca;

	// Persiste o arquivo XML
	xmlCom->Salvar();

	// Cria n� para o processo de LF em TreeView do form
	MostraResultadosTreeView_DMS3(lisStrSolucaoDMS3);

	// Adiciona informa��o ao memo do form principal
	MemoProcessosLF->Lines->Add("   Evento " + CodigoEvento + " - Exportando solu��es");
	formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " com sucesso.");

}
//---------------------------------------------------------------------------
void __fastcall TThreadFaultLocation::XMLGeraSolucoesDMS2(TList* lisStrSolucaoDMS2, StrDadosGerais* strDadosGerais)
{
	if(!lisStrSolucaoDMS2 || !strDadosGerais) return;

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Composi��o do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	String pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += "SOL_" + CodigoAlimentador + "_" + TimeStamp;
	pathFinal += ".xml";

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Gera��o de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	// Cria objeto XML de solu��o, passando o caminho (path) e o conte�do
	xmlCom = new TXMLComunicacao(pathFinal, XMLsolutionDMS2);

	// Adiciona dados gerais
	xmlCom->AddDadosGerais(strDadosGerais);

	// Adiciona informa��es de LOG
	xmlCom->AddDadosLog(strLog);

	// Adiciona solu��es geradas pelo algoritmo 2 (DMS2): chaves dos blocos
	// que formam a solu��o
	TStringList* lisCodChaves_BlocosAreaBusca = new TStringList;
	for(int i=0; i<lisStrSolucaoDMS2->Count; i++)
	{
		StrSolucaoDMS2* solDMS2 = (StrSolucaoDMS2*) lisStrSolucaoDMS2->Items[i];
		lisCodChaves_BlocosAreaBusca->Add(solDMS2->chaveMontante->Codigo);
   }
	xmlCom->InsereChavesBlocosSolucoes(lisCodChaves_BlocosAreaBusca);
	delete lisCodChaves_BlocosAreaBusca;

	// Persiste o arquivo XML
	xmlCom->Salvar();

	// Cria n� para o processo de LF em TreeView do form
	MostraResultadosTreeView_DMS2(lisStrSolucaoDMS2);

	// Adiciona informa��o ao memo do form principal
	MemoProcessosLF->Lines->Add("   Evento " + CodigoEvento + " - Exportando solu��es");
	formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + CodigoEvento + " com sucesso.");
}
////---------------------------------------------------------------------------
//void __fastcall TThreadFaultLocation::XMLExporta_SH()
//{
//	String pathFinal;
//   TStringList* lisCodChaves = new TStringList();
//	TXMLComunicacao* xmlCom;
//   VTBarra* barraSol;
//   VTChave* chaveMont;
//
//	if(!areaBusca)
//		return;
//
//  	pathFinal = path->DirImporta() + "\\SelfHealing\\";
//	pathFinal += CodigoAlimentador + "_" + TimeStamp;
//	pathFinal += ".xml";
//
//   // Cria objeto XML de solu��o, passando o caminho (path) e o conte�do
//	xmlCom = new TXMLComunicacao(pathFinal, XML_SelfHealing);
//   xmlCom->SetLogErros(logErros);
//
//
//   // *********** INSERE OS BLOCOS DA �REA DE BUSCA ***********
//	lisCodChaves->Clear();
//	areaBusca->GetAreaBusca_CodChavesBlocos(lisCodChaves);
//	xmlCom->SH_InsereChavesBlocosAreaBusca(lisCodChaves);
//
//   // *********** INSERE AS SOLU��ES DE LOCALIZA��O ***********
//   lisCodChaves->Clear();
//	for(int i=0; i<lisSolucoes->Count; i++)
//	{
//    	strSolucao* sol = (strSolucao*) lisSolucoes->Items[i];
//
//      barraSol = sol->barraSolucao;
//      if(!barraSol) continue;
//		chaveMont = funcoesRede->GetChaveMontante(barraSol);
//		if(!chaveMont) continue;
//		lisCodChaves->Add(chaveMont->Codigo);
//   }
//   if(lisCodChaves->Count > 0)
//	{
////		xmlCom->SetLogErros(logErros);
//		xmlCom->SH_InsereChavesBlocosSolucoes(lisCodChaves);
//   }
//
//	xmlCom->Salvar();
//
//	// Destroi lista de c�digos
//	if(lisCodChaves) {delete lisCodChaves; lisCodChaves = NULL;}
//}
////---------------------------------------------------------------------------
///***
// * M�todo que escreve um XML de solicita��o (request) de medi��es ao M�d. de Supervis�o
// *   para o caso espec�fico de problema de cabo rompido
// */
//void __fastcall TThreadFaultLocation::XMLSolicitaMedicoesCaboRompido()
//{
//	int requestType;
//	String pathFinal, timeStamp;
//	TStringList* lisCadQualimetros;
//	VTRede* redeRef = NULL;
//
//	// :::::::::::::::::::::::::::::::::::::::::::::::::
//	// Composi��o do nome do arquivo XML
//	// :::::::::::::::::::::::::::::::::::::::::::::::::
//
//	pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
//	pathFinal += "REQ_" + CodigoAlimentador + "_" + TimeStamp;
//	pathFinal += ".xml";
//
//	// Cria objeto XML de request, passando o caminho (path) e o conte�do
//	xmlCom = new TXMLComunicacao(pathFinal, XMLrequest);
//
//	// Para cada tipo de alarme, solicita informa��es adicionais
//	for(int i=0; i<listaAlarmes->Count; i++)
//	{
//		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
//
//		if(alarme->GetTipoEqpto() == 6) // Qual�metro
//		{
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			if(alarme->GetTipoAlarme() == 5) // VSAGQ - Afundamento de tens�o registrado por qual�metro
//			{
//				// Solicita medi��es de tens�o do qual�metro
//				requestType = 7;
//				timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//				xmlCom->AddRequestQualimetro(requestType, alarme->GetCodEqpto(), timeStamp);
//			}
//
//		}
//		else if(alarme->GetTipoEqpto() == 7) // Transformador inteligente
//		{
//
//		}
//	}
//
//	// Persiste o arquivo XML
//	xmlCom->Salvar();
//}
//---------------------------------------------------------------------------
/***
 * Caso haja pelo menos um alarme de Medidor Inteligente, solicita medi��es adicionais.
 */
bool __fastcall TThreadFaultLocation::DecideSolicitacaoMedicoes()
{
	// Para cada tipo de alarme, solicita informa��es adicionais
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
		int tipoEqpto = alarme->GetTipoEqpto();
		if(tipoEqpto == 12 || tipoEqpto == 13 || tipoEqpto == 14) // Medidor inteligente
			return(true);
	}
	return(false);
}
//---------------------------------------------------------------------------
/***
 * M�todo que escreve um XML de solicita��o (request) de medi��es ao M�d. de Supervis�o
 */
void __fastcall TThreadFaultLocation::XMLSolicitaMedicoes()
{
	int requestType, numCargasPorBloco;
	String pathFinal, codCargaMT;
	TAlarme* alarme;
	TStringList *lisCargasMT, *lisCodEqptosCampo;
	TStringList *lisCadReligadoras, *lisCadDisjuntores, *lisCadMedBalanco;
	VTRede* redeRef = NULL;

	// Inicia listas
	lisCadDisjuntores = new TStringList();
	lisCadReligadoras = new TStringList();
	lisCodEqptosCampo = new TStringList();
	lisCadMedBalanco = new TStringList();

	// :::::::::::::::::::::::::::::::::::::::::::::::::
	// Composi��o do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += "REQ_" + CodigoAlimentador + "_" + TimeStamp;
	pathFinal += ".xml";

   // :::::::::::::::::::::::::::::::::::::::::::::::::
   // Gera��o de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

	// Cria objeto XML de request, passando o caminho (path) e o conte�do
	xmlCom = new TXMLComunicacao(pathFinal, XMLrequest);
	xmlCom->SetLogErros(logErros);

	// Para cada tipo de alarme, solicita informa��es adicionais
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		alarme = (TAlarme*) listaAlarmes->Items[i];

		if(alarme->GetTipoEqpto() == 10) // DJ
		{
			// Guarda o c�digo da chave monitorada
			lisCodEqptosCampo->Add(alarme->GetCodEqpto());

			if(redeRef == NULL)
			{
				String codAlim = alarme->GetCodAlimentador();
				redeRef = funcoesRede->GetRede_CodRede(codAlim);
			}

			if(alarme->GetTipoAlarme() != 20 && alarme->GetTipoAlarme() != 21)
				continue;

			lisCadDisjuntores->Clear();
			lisCadDisjuntores->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroDisjuntores.csv");
			String codChave = funcoesRede->GetCodChaveSinap(alarme->GetCodEqpto(), lisCadDisjuntores);

			if(codChave == "")
				continue;
		}
		else if(alarme->GetTipoEqpto() == 11) // Religadora
		{
			// Guarda o c�digo da chave monitorada
			lisCodEqptosCampo->Add(alarme->GetCodEqpto());

			if(redeRef == NULL)
			{
				String codAlim = alarme->GetCodAlimentador();
				redeRef = funcoesRede->GetRede_CodRede(codAlim);
			}

			if(alarme->GetTipoAlarme() != 20 && alarme->GetTipoAlarme() != 21)
				continue;

			lisCadReligadoras->Clear();
			lisCadReligadoras->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroReligadoras.csv");
			String codChave = funcoesRede->GetCodChaveSinap(alarme->GetCodEqpto(), lisCadReligadoras);

			if(codChave == "")
				continue;
		}
		else if(alarme->GetTipoEqpto() == 12) // Medidor Inteligente de balan�o (trafo MTBT)
		{
			// Guarda o c�digo da chave monitorada
			lisCodEqptosCampo->Add(alarme->GetCodEqpto());

			if(redeRef == NULL)
			{
				String codAlim = alarme->GetCodAlimentador();
				redeRef = funcoesRede->GetRede_CodRede(codAlim);
			}

			// Certifica se � alarme de Last Gasp
			if(alarme->GetTipoAlarme() != 22)
				continue;

			lisCadMedBalanco->Clear();
			lisCadMedBalanco->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroMedidoresBalanco.csv");
			String codCarga = funcoesRede->GetCodChaveSinap(alarme->GetCodEqpto(), lisCadMedBalanco);

			if(codCarga == "")
				continue;

			// Solicita medi��o instant�nea de tens�o, fases A, B e C
			requestType = 30;
			xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);

		}
//		else if(alarme->GetTipoEqpto() == 1) // DJ
//		{
//			// Guarda o c�digo da chave monitorada
//			lisCodEqptosCampo->Add(alarme->GetCodEqpto());
//
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			if(alarme->GetTipoAlarme() == 1) //FREL
//			{
//				TStringList* lisCadDisjuntores = new TStringList();
//				lisCadDisjuntores->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroDisjuntores.csv");
//				String codChave = funcoesRede->GetCodChaveSinap(alarme->GetCodEqpto(), lisCadDisjuntores);
//
//				if(codChave == "")
//				{
//					continue;
//				}
//
//				// Pede corrente de defeito pela chave (requestType = 1)
//				requestType = 1;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede alarmes de atua��o da chave (requestType = 2)
//				requestType = 2;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede dist�ncia de falta calculada pelo rel� (requestType = 3)
//				requestType = 3;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede disponibilidade (qualidade de dados) dos sensores � jusante da chave. Isso � feito atrav�s
//				// da solicita��o de leitura de corrente de regime para avaliar disponibilidade/qualidade de dados
//				lisCadSensores->Clear();
//				lisCadSensores->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroSensores.csv");
//				lisCodSensoresJusante->Clear();
//
//				funcoesRede->GetSensoresJusanteChave(codChave, lisCadSensores, lisCodSensoresJusante);
//
//				for(int j=0; j<lisCodSensoresJusante->Count; j++)
//				{
//					String codSensor = lisCodSensoresJusante->Strings[j];
//
//					// Pede medi��o qualquer (de corrente, p. ex.), para avaliar a disponibilidade do sensor (requestType = 4)
//					requestType = 4;
//					xmlCom->AddRequest(requestType, codSensor, alarme);
//				}
//			}
//		}
//		else if(alarme->GetTipoEqpto() == 2) // RE
//		{
//			// Guarda o c�digo da chave monitorada
//			lisCodEqptosCampo->Add(alarme->GetCodEqpto());
//
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			if(alarme->GetTipoAlarme() == 1) //FREL
//			{
//				// Pede corrente de defeito pela chave (requestType = 1)
//				requestType = 1;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede alarmes de atua��o da chave (requestType = 2)
//				requestType = 2;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede disponibilidade (qualidade de dados) dos sensores � jusante da chave. Isso � feito atrav�s
//				// da solicita��o de leitura de corrente de regime para avaliar disponibilidade/qualidade de dados
//				lisCadSensores->Clear();
//				lisCadSensores->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroSensores.csv");
//				lisCodSensoresJusante->Clear();
//
//				lisCadReligadoras->Clear();
//				lisCadReligadoras->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroReligadoras.csv");
//				String codChave = funcoesRede->GetCodChaveSinap(alarme->GetCodEqpto(), lisCadReligadoras);
//				funcoesRede->GetSensoresJusanteChave(codChave, lisCadSensores, lisCodSensoresJusante);
//
//				for(int j=0; j<lisCodSensoresJusante->Count; j++)
//				{
//					String codSensor = lisCodSensoresJusante->Strings[j];
//
//					// Pede medi��o qualquer (de corrente, p. ex.), para avaliar a disponibilidade do sensor (requestType = 4)
//					requestType = 4;
//					xmlCom->AddRequest(requestType, codSensor, alarme);
//				}
//			}
//		}
//		else if(alarme->GetTipoEqpto() == 3) // Seccionalizadora
//		{
//			// Guarda o c�digo da chave monitorada
//			lisCodEqptosCampo->Add(alarme->GetCodEqpto());
//
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			if(alarme->GetTipoAlarme() == 2) //RAUT
//			{
//				// buscar a chave de prote��o � montante da seccionalizadora
//				VTChave* chvMont = funcoesRede->GetChaveMontante(alarme->GetCodEqpto());
//
//				if(chvMont != NULL)
//				{
//					// Pede corrente de defeito (requestType = 1)
//					requestType = 1;
//					xmlCom->AddRequest(requestType, chvMont->Codigo, alarme);
//
//					// Pede alarmes de atua��o da chave (requestType = 2)
//					requestType = 2;
//					xmlCom->AddRequest(requestType, chvMont->Codigo, alarme);
//
//					// Pede dist�ncia de falta calculada pelo rel� (requestType = 3)
//					requestType = 3;
//					xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//					// Pede disponibilidade (qualidade de dados) dos sensores � jusante da chave. Isso � feito atrav�s
//					// da solicita��o de leitura de corrente de regime para avaliar disponibilidade/qualidade de dados
//					lisCadSensores->Clear();
//					lisCadSensores->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroSensores.csv");
//					lisCodSensoresJusante->Clear();
//					funcoesRede->GetSensoresJusanteChave(chvMont->Codigo, lisCadSensores, lisCodSensoresJusante);
//
//					for(int j=0; j<lisCodSensoresJusante->Count; j++)
//					{
//						String codSensor = lisCodSensoresJusante->Strings[j];
//
//						// Pede disponibilidade de sensor (requestType = 4)
//						requestType = 4;
//						xmlCom->AddRequest(requestType, codSensor, alarme);
//					}
//				}
//			}
//		}
//		else if(alarme->GetTipoEqpto() == 4) // Sensor
//		{
//			// Guarda o c�digo da chave monitorada
//			lisCodEqptosCampo->Add(alarme->GetCodEqpto());
//
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			if(alarme->GetTipoAlarme() == 3 || alarme->GetTipoAlarme() == 4) //FPE ou AIM
//			{
//				// aguardar X segundos (verificar aqui)
//				// (janela de tempo j� pode suprir esse deltaT necess�rio)
//
//				// pede corrente de defeito (requestType = 1)
//				requestType = 1;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//
//				// Pede disponibilidade de sensor (requestType = 4)
//				requestType = 4;
//				xmlCom->AddRequest(requestType, alarme->GetCodEqpto(), alarme);
//			}
//		}
//		else if(alarme->GetTipoEqpto() == 6) // Qual�metro
//		{
//			if(redeRef == NULL)
//			{
//				String codAlim = alarme->GetCodAlimentador();
//				redeRef = funcoesRede->GetRede_CodRede(codAlim);
//			}
//
//			// 5: VSAGQ - alarme de subtens�o
//			// 6: ICCQ - alarme de sobrecorrente
//			if(alarme->GetTipoAlarme() == 5 || alarme->GetTipoAlarme() == 6)
//			{
//				// Se for algoritmo fasorial e se o qual�metro estiver associado a
//            // uma chave de prote��o, solicita fasores de V e I (7, 8):
//				if(tipoLF == faultlocationALGOFASORIAL &&
//               QualimetroChaveProtecao(alarme->GetCodAlimentador(), alarme->GetCodEqpto()))
//				{
//					// Solicita medi��es de tens�o do qual�metro
//					requestType = 7;
//					String timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//					xmlCom->AddRequestQualimetro(requestType, alarme->GetCodEqpto(), timeStamp);
//
//					// Solicita medi��es de corrente do qual�metro
//					requestType = 8;
//					timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//					xmlCom->AddRequestQualimetro(requestType, alarme->GetCodEqpto(), timeStamp);
//				}
//				// Caso contr�rio, solicita apenas os fasores de tens�o
//				else
//				{
//					// Solicita medi��es de tens�o do qual�metro
//					requestType = 7;
//					String timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//					xmlCom->AddRequestQualimetro(requestType, alarme->GetCodEqpto(), timeStamp);
//            }
//			}
//
//		}
//		else if(alarme->GetTipoEqpto() == 7) // Transformador inteligente
//		{
//         if(tipoLF == faultlocationALGOFASORIAL || tipoLF == faultlocationROMPCABO)
//         {
//            // Solicita medi��es de tens�o do trafo inteligente
//            requestType = 7;
//            String timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//            xmlCom->AddRequestTrafoInteligente(requestType, alarme->GetCodEqpto(), timeStamp);
//         }
//		}
//		else if(alarme->GetTipoEqpto() == 20) // Medidor inteligente
//		{
//
//      }
	}

//	// Verifica se ser� considerado o algoritmo fasorial
//	String pathConfigGerais = path->DirDat() + "\\FaultLocation\\ConfigGerais.ini";
//	TIniFile* fileConfigGerais = new TIniFile(pathConfigGerais);
//	bool UtilizarFasores = fileConfigGerais->ReadBool("ALGO_FASORIAL", "UtilizarFasores", 0);
//   delete fileConfigGerais; fileConfigGerais = NULL;


// SOLICITA MEDI��ES DE TODOS OS QUAL�METROS CADASTRADOS
//	if(tipoLF == faultlocationALGOFASORIAL)
//	{
//		// Solicita medi��es de tens�o dos qual�metros da rede de refer�ncia (investigada no momento)
//		lisCadQualimetros->Clear();
//		lisCadQualimetros->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv");
//		String timeStamp = ((TAlarme*) listaAlarmes->Items[0])->GetTimeStamp();
//		for(int i=0; i<lisCadQualimetros->Count; i++)
//		{
//			String codRedeQualimetro = ReplaceStr(GetCampoCSV(lisCadQualimetros->Strings[i], 0, ";"), "-", "");
//			String codRedeRef = ReplaceStr(redeRef->Codigo, "-", "");
//			String codQualimetro = GetCampoCSV(lisCadQualimetros->Strings[i], 1, ";");
//
//         if(codRedeRef == codRedeQualimetro)
//         {
//				// Solicita medi��es de tens�o do qual�metro
//				requestType = 7;
//				xmlCom->AddRequestQualimetro(requestType, codQualimetro, timeStamp);
//
//				// Solicita medi��es de correntes do qual�metro
//				requestType = 8;
//				xmlCom->AddRequestQualimetro(requestType, codQualimetro, timeStamp);
//			}
//		}
//	}


//	// ::::::::::::::::::::::::::::::::::::::::::::::::
//   // Adiciona solicita��o do estado de consumidores (ON/OFF)
//	// ::::::::::::::::::::::::::::::::::::::::::::::::
//
//   // Pega lista de c�digos das cargas MT (da rede com problema)
//	lisCargasMT = new TStringList();
//   numCargasPorBloco = 1;   // numCargasPorBloco = -1  ==> todas as cargas do bloco
//   funcoesRede->GetCargasMT(lisCodEqptosCampo, numCargasPorBloco, lisCargasMT);
//
//   for(int i=0; i<lisCargasMT->Count; i++)
//   {
//		codCargaMT = lisCargasMT->Strings[i];
//
//      requestType = 5;  // Estado de consumidor
//      xmlCom->AddRequest(requestType, codCargaMT, "");
//   }

	// Persiste o arquivo XML
	xmlCom->Salvar();

	// Destroi objetos
	if(lisCadDisjuntores)     {delete lisCadDisjuntores; lisCadDisjuntores = NULL;}
	if(lisCadReligadoras)     {delete lisCadReligadoras; lisCadReligadoras = NULL;}
	if(lisCodEqptosCampo)     {delete lisCodEqptosCampo;    lisCodEqptosCampo = NULL;}
	if(lisCadMedBalanco)      {delete lisCadMedBalanco; lisCadMedBalanco = NULL;}
}
//---------------------------------------------------------------------------
bool __fastcall TThreadFaultLocation::QualimetroChaveProtecao(String codAlimentador, String codQualimetro)
{
	if(codAlimentador == "" || codQualimetro == "") return(false);

	String codChaveQualimetro;
	VTRede* redeMT = NULL;

	// Remove "QUALIMETRO_" do c�digo do qual�metro, para obter o c�digo da chave associada
	int CompTotal = codQualimetro.Length();
	codChaveQualimetro = GetCodigoLigacaoAssociadaQualimetro(codQualimetro);

	// Obt�m a rede MT condizente com "codAlimentador"
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		redeMT = (VTRede*) redes->LisRede()->Items[i];
		if(!redeMT->Carregada || redeMT->TipoRede->Segmento != redePRI) continue;

		String codRedeMT = ReplaceStr(redeMT->Codigo, "-", "");
		codAlimentador = ReplaceStr(codAlimentador, "-", "");
		if(codRedeMT == codAlimentador)
			break;
		else
			redeMT = NULL;
	}
	if(!redeMT) return(false);

	// Verifica se a rede MT cont�m a chave do qual�metro. Retorna TRUE se
	// essa chave associada for de prote��o
	TList* lisChaves = new TList();
	redeMT->LisEqpto(lisChaves, eqptoCHAVE);
	for(int i=0; i<lisChaves->Count; i++)
	{
		VTChave* chave = (VTChave*) lisChaves->Items[i];
		if(chave->Codigo == codChaveQualimetro)
		{
			if(chave->TipoDisjuntor || chave->TipoReligadora)
				return(true);
		}
	}
	return(false);
}
//---------------------------------------------------------------------------
String __fastcall TThreadFaultLocation::GetCodigoLigacaoAssociadaQualimetro(String cod_qualimetro)
{
	String pathArqCadastro = path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv";
	TStringList* lisArqCadQual = new TStringList;
	lisArqCadQual->LoadFromFile(pathArqCadastro);

	for(int i=0; i<lisArqCadQual->Count; i++)
	{
		String linha = lisArqCadQual->Strings[i];
		if(GetCampoCSV(linha, 1, ";") == cod_qualimetro)
		{
			delete lisArqCadQual;
			return(GetCampoCSV(linha, 2, ";"));
		}
	}
	delete lisArqCadQual;
	return("");
}
//---------------------------------------------------------------------------
