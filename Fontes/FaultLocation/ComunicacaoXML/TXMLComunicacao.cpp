//---------------------------------------------------------------------------
#pragma hdrstop
#include "TXMLComunicacao.h"
#include "TAlarme.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TLog.h"
#include "..\DSS\TFaultStudyFL.h"
#include "..\TThreadFaultLocation.h"
#pragma package(smart_init)
#include <XMLDoc.hpp>
//---------------------------------------------------------------------------
__fastcall TXMLComunicacao::TXMLComunicacao(String pathFinal, int tipoXML)
{
   // Cria documento do tipo XML
	xmlFile = interface_cast<Xmlintf::IXMLDocument>(new TXMLDocument(NULL));
	xmlFile->Active = true;
   // Configura vers�o e codifica��o
   xmlFile->Version = "1.0";
   xmlFile->Encoding = "UTF-8";

   // Path para salvamento do arquivo XML
   this->pathFinal = pathFinal;

	// Define o n� raiz do XML
   switch(tipoXML)
   {
   case XMLcadSensores:
      rootNode = xmlFile->CreateNode("Requests");
   	break;

   case XMLrequest:
   	rootNode = xmlFile->CreateNode("Requests");
      break;

   case XMLsolution:
   	rootNode = xmlFile->CreateNode("FaultLocationResults");
      break;

   case XMLFaultLocationOffline:
      rootNode = xmlFile->CreateNode("FaultLocationOffline");
      break;

   case XMLrespCadSensoresFake:
   	rootNode =  xmlFile->CreateNode("CadastroSensores");
		break;

   case XMLareaBuscaSH:
   	rootNode = xmlFile->CreateNode("BlocosAreaBusca");
      break;

   case XMLsolutionSH:
   	rootNode = xmlFile->CreateNode("BlocosSolucao");
      break;

   case XML_SelfHealing:
   	rootNode = xmlFile->CreateNode("LF");
		break;

   case XMLsolutionDMS2:
   	rootNode = xmlFile->CreateNode("BlocosAreaBusca");
		break;

	case XMLsolutionDMS3:
   	rootNode = xmlFile->CreateNode("TrechosAreaBusca");
		break;

   default:
   	rootNode = xmlFile->CreateNode("RootNode");
		break;
   }

	xmlFile->DocumentElement = rootNode;

   // Inicializa��es
   contador = 1;
   contItens = 0;
   lisStrSolicitacoes = new TList();
   logErros = NULL;
}
//---------------------------------------------------------------------------
__fastcall TXMLComunicacao::~TXMLComunicacao()
{
	// Destroi objetos
   if(lisStrSolicitacoes)
   {
      for(int i=lisStrSolicitacoes->Count-1; i>=0; i--) delete lisStrSolicitacoes->Items[i];
      delete lisStrSolicitacoes; lisStrSolicitacoes = NULL;
   }
   delete xmlFile; xmlFile = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddRequest(int requestType, String codEqpto, TAlarme* alarme)
{
	_di_IXMLNode contentNode;
	_di_IXMLNode requestNode;
   String timeStamp, tipoEqpto;

   // Obt�m as informa��es do alarme
   timeStamp = alarme->GetTimeStamp();
   if(requestType == 4)
   {
      tipoEqpto = 4;   //qualidade do dado do sensor
   }
   else
   {
	   tipoEqpto = String(alarme->GetTipoEqpto());
   }

   // Verifica se a solicita��o j� foi feita
   if(ExisteSolicitacao(codEqpto, requestType)) return;

   // Cria n� para um request
   requestNode = xmlFile->CreateElement("Request", "");
   rootNode->ChildNodes->Add(requestNode);

   // Cria n�s de conte�do
   contentNode = xmlFile->CreateElement("ID", "");
   contentNode->Text = String(contador++);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("RequestType", "");
   contentNode->Text = String(requestType);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TipoEqpto", "");
   contentNode->Text = String(tipoEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CodEqpto", "");
   contentNode->Text = String(codEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TimeStamp", "");
   contentNode->Text = String(timeStamp);
   requestNode->ChildNodes->Add(contentNode);

   // Incrementa contador de itens
   contItens += 1;

   // Salva para controle apenas
   strSolicitacao* solicit = new strSolicitacao();
   solicit->CodEqpto = String(codEqpto);
   solicit->requestType = String(requestType);
   lisStrSolicitacoes->Add(solicit);
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddRequestTrafoInteligente(int requestType, String codEqpto, String timeStamp)
{
	_di_IXMLNode contentNode;
	_di_IXMLNode requestNode;
   String tipoEqpto;

   // Verifica se a solicita��o j� foi feita
   if(ExisteSolicitacao(codEqpto, requestType)) return;

   // Tipo de Equipamento (Trafo inteligente = 7)
   tipoEqpto = 7;

   // Cria n� para um request
   requestNode = xmlFile->CreateElement("Request", "");
   rootNode->ChildNodes->Add(requestNode);

   // Cria n�s de conte�do
   contentNode = xmlFile->CreateElement("ID", "");
   contentNode->Text = String(contador++);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("RequestType", "");
   contentNode->Text = String(requestType);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TipoEqpto", "");
   contentNode->Text = String(tipoEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CodEqpto", "");
   contentNode->Text = String(codEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TimeStamp", "");
   contentNode->Text = String(timeStamp);
   requestNode->ChildNodes->Add(contentNode);

   // Incrementa contador de itens
   contItens += 1;

   // Salva para controle apenas
   strSolicitacao* solicit = new strSolicitacao();
   solicit->CodEqpto = String(codEqpto);
   solicit->requestType = String(requestType);
   lisStrSolicitacoes->Add(solicit);
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddRequestQualimetro(int requestType, String codEqpto, String timeStamp)
{
	_di_IXMLNode contentNode;
	_di_IXMLNode requestNode;
   String tipoEqpto;

   // Verifica se a solicita��o j� foi feita
   if(ExisteSolicitacao(codEqpto, requestType)) return;

   // Tipo de Equipamento (Qual�metro = 6)
   tipoEqpto = 6;

   // Cria n� para um request
   requestNode = xmlFile->CreateElement("Request", "");
   rootNode->ChildNodes->Add(requestNode);

   // Cria n�s de conte�do
   contentNode = xmlFile->CreateElement("ID", "");
   contentNode->Text = String(contador++);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("RequestType", "");
   contentNode->Text = String(requestType);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TipoEqpto", "");
   contentNode->Text = String(tipoEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CodEqpto", "");
   contentNode->Text = String(codEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TimeStamp", "");
   contentNode->Text = String(timeStamp);
   requestNode->ChildNodes->Add(contentNode);

   // Incrementa contador de itens
   contItens += 1;

   // Salva para controle apenas
   strSolicitacao* solicit = new strSolicitacao();
   solicit->CodEqpto = String(codEqpto);
   solicit->requestType = String(requestType);
   lisStrSolicitacoes->Add(solicit);
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddAlimentador(String CodAlimentador)
{
	_di_IXMLNode contentNode;
	_di_IXMLNode requestNode;
	int requestType;
   String tipoEqpto;

	requestType = 5;
   tipoEqpto = "Alimentador";

   // Cria n� para um request
   requestNode = xmlFile->CreateElement("Request", "");
   rootNode->ChildNodes->Add(requestNode);

   // Cria n�s de conte�do
   contentNode = xmlFile->CreateElement("ID", "");
   contentNode->Text = String(contador++);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("RequestType", "");
   contentNode->Text = String(requestType);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TipoEqpto", "");
   contentNode->Text = String(tipoEqpto);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CodEqpto", "");
   contentNode->Text = String(CodAlimentador);
   requestNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("TimeStamp", "");
   contentNode->Text = TimeStampAtual();
   requestNode->ChildNodes->Add(contentNode);

   // Incrementa contador de itens
   contItens += 1;
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddChavesBlocosSolucoes(TStringList* lisCodChaves)
{
	if(!lisCodChaves)
   	return;

	_di_IXMLNode contentNode;
	_di_IXMLNode blocoNode;
   _di_IXMLNode blocosNode;


   blocosNode = xmlFile->CreateElement("Blocos", "");
   rootNode->ChildNodes->Add(blocosNode);

   for(int i=0; i<lisCodChaves->Count; i++)
   {

      String codChave = lisCodChaves->Strings[i];

      // Cria n� "Bloco", dentro do n� "Blocos"
      blocoNode = xmlFile->CreateElement("Bloco", "");
      blocosNode->ChildNodes->Add(blocoNode);

     	// Cria os par�metros de um bloco
      contentNode = xmlFile->CreateElement("CodigoChave", "");
      contentNode->Text = codChave;
      blocoNode->ChildNodes->Add(contentNode);

      // Incrementa contador de itens
      contItens += 1;
   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::InsereLigacoesSolucoes(TStringList* lisCodLigacoes)
{
	if(!lisCodLigacoes) return;

	_di_IXMLNode contentNode;
	_di_IXMLNode ligacaoNode;
	_di_IXMLNode ligacoesNode;
	_di_IXMLNode nodeLigacoesSolucao;

	// Cria n� "BlocosSolucao"
	nodeLigacoesSolucao = xmlFile->CreateElement("LigacoesSolucoes", "");
	rootNode->ChildNodes->Add(nodeLigacoesSolucao);

	// Cria n� para as liga��es
	ligacoesNode = xmlFile->CreateElement("Ligacoes", "");
	nodeLigacoesSolucao->ChildNodes->Add(ligacoesNode);

	for(int i=0; i<lisCodLigacoes->Count; i++)
   {
		String codLigacao = lisCodLigacoes->Strings[i];

		// Cria n� "Liga��o", dentro do n� "Liga��es"
		ligacaoNode = xmlFile->CreateElement("Ligacao", "");
		ligacoesNode->ChildNodes->Add(ligacaoNode);

		// Cria os par�metros de um bloco
		contentNode = xmlFile->CreateElement("CodigoLigacao", "");
		contentNode->Text = codLigacao;
		ligacaoNode->ChildNodes->Add(contentNode);

		// Incrementa contador de itens
		contItens += 1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::InsereChavesBlocosSolucoes(TStringList* lisCodChaves)
{
	if(!lisCodChaves) return;

	_di_IXMLNode contentNode;
	_di_IXMLNode blocoNode;
	_di_IXMLNode blocosNode;
	_di_IXMLNode nodeBlocosSolucao;

	// Cria n� "BlocosSolucao"
	nodeBlocosSolucao = xmlFile->CreateElement("BlocosSolucao", "");
	rootNode->ChildNodes->Add(nodeBlocosSolucao);

	// Cria n� para os blocos
   blocosNode = xmlFile->CreateElement("Blocos", "");
   nodeBlocosSolucao->ChildNodes->Add(blocosNode);

   for(int i=0; i<lisCodChaves->Count; i++)
   {

      String codChave = lisCodChaves->Strings[i];

      // Cria n� "Bloco", dentro do n� "Blocos"
      blocoNode = xmlFile->CreateElement("Bloco", "");
      blocosNode->ChildNodes->Add(blocoNode);

		// Cria os par�metros de um bloco
		contentNode = xmlFile->CreateElement("CodigoChave", "");
		contentNode->Text = codChave;
		blocoNode->ChildNodes->Add(contentNode);

		// Incrementa contador de itens
		contItens += 1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::SH_InsereChavesBlocosSolucoes(TStringList* lisCodChaves)
{
	if(!lisCodChaves)
   	return;

	_di_IXMLNode contentNode;
	_di_IXMLNode blocoNode;
   _di_IXMLNode blocosNode;
   _di_IXMLNode nodeBlocosSolucao;

 	// Cria n� "BlocosSolucao"
   nodeBlocosSolucao = xmlFile->CreateElement("BlocosSolucao", "");
   rootNode->ChildNodes->Add(nodeBlocosSolucao);

   // Cria n� para os blocos
   blocosNode = xmlFile->CreateElement("Blocos", "");
   nodeBlocosSolucao->ChildNodes->Add(blocosNode);

   for(int i=0; i<lisCodChaves->Count; i++)
   {

      String codChave = lisCodChaves->Strings[i];

      // Cria n� "Bloco", dentro do n� "Blocos"
      blocoNode = xmlFile->CreateElement("Bloco", "");
      blocosNode->ChildNodes->Add(blocoNode);

     	// Cria os par�metros de um bloco
      contentNode = xmlFile->CreateElement("CodigoChave", "");
      contentNode->Text = codChave;
      blocoNode->ChildNodes->Add(contentNode);

      // Incrementa contador de itens
      contItens += 1;
   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddChavesBlocosAreaBusca(TStringList* lisCodChaves)
{
	if(!lisCodChaves)
   	return;

	_di_IXMLNode contentNode;
	_di_IXMLNode blocoNode;
   _di_IXMLNode blocosNode;

	// Cria n� de blocos
   blocosNode = xmlFile->CreateElement("Blocos", "");
   rootNode->ChildNodes->Add(blocosNode);

   for(int i=0; i<lisCodChaves->Count; i++)
   {
      String codChave = lisCodChaves->Strings[i];

      // Cria n� "Bloco", dentro do n� "Blocos"
      blocoNode = xmlFile->CreateElement("Bloco", "");
      blocosNode->ChildNodes->Add(blocoNode);

     	// Cria os par�metros de um bloco
      contentNode = xmlFile->CreateElement("CodigoChave", "");
      contentNode->Text = codChave;
      blocoNode->ChildNodes->Add(contentNode);

      // Incrementa contador de itens
      contItens += 1;
   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::SH_InsereChavesBlocosAreaBusca(TStringList* lisCodChaves)
{
	if(!lisCodChaves)
   	return;

	_di_IXMLNode contentNode;
	_di_IXMLNode blocoNode;
   _di_IXMLNode blocosNode;
   _di_IXMLNode nodeBlocosAreaBusca;

	// Cria n� "BlocosAreaBusca"
   nodeBlocosAreaBusca = xmlFile->CreateElement("BlocosAreaBusca", "");
   rootNode->ChildNodes->Add(nodeBlocosAreaBusca);

	// Cria n� para os blocos
   blocosNode = xmlFile->CreateElement("Blocos", "");
   nodeBlocosAreaBusca->ChildNodes->Add(blocosNode);

   for(int i=0; i<lisCodChaves->Count; i++)
   {
      String codChave = lisCodChaves->Strings[i];

      // Cria n� "Bloco", dentro do n� "Blocos"
      blocoNode = xmlFile->CreateElement("Bloco", "");
      blocosNode->ChildNodes->Add(blocoNode);

     	// Cria os par�metros de um bloco
      contentNode = xmlFile->CreateElement("CodigoChave", "");
      contentNode->Text = codChave;
      blocoNode->ChildNodes->Add(contentNode);

      // Incrementa contador de itens
      contItens += 1;
   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddDadosLog(StrDadosLog* strLog)
{
	_di_IXMLNode contentNode;
   _di_IXMLNode FLProcessNode;
	_di_IXMLNode MeasurementNode;
	_di_IXMLNode Node;
   _di_IXMLNode unavailableMeasurementsNode;
   _di_IXMLNode deviceNode;
	_di_IXMLNode aquisitionErrors;
   _di_IXMLNode additionalData;
   _di_IXMLNode relayFaultDistance;
	String codSensor, codEqptoMedicao, tipoMedicao, measurements;

   _di_IXMLNode InputDataNode;
   _di_IXMLNode BadDataNode;
   _di_IXMLNode BadDataDeviceNode;
   _di_IXMLNode InputNode;

   // Cria n� para dados de log
	Node = xmlFile->CreateElement("LogData", "");
   rootNode->ChildNodes->Add(Node);

   // :::::::::::::::::::::::::::::::::::::::::
	// PROCESSO DE FL
   // :::::::::::::::::::::::::::::::::::::::::

   // Cria n� para o processo de FL
   FLProcessNode = xmlFile->CreateElement("FaultLocationProcess", "");
   Node->ChildNodes->Add(FLProcessNode);

   // Cria n� para m�todo de FL
   contentNode = xmlFile->CreateElement("FaultLocationMethod", "");
   contentNode->Text = strLog->FLMethod;
   FLProcessNode->ChildNodes->Add(contentNode);
   // Cria n� para toler�ncia do m�todo
   contentNode = xmlFile->CreateElement("FaultLocationTolerance", "");
   contentNode->Text = String(strLog->FLTolerance);
   FLProcessNode->ChildNodes->Add(contentNode);
	// Cria n� para a corrente de curto-circuito medida e utilizada pela metodologia
	contentNode = xmlFile->CreateElement("MeasuredFaultCurrent", "");
	contentNode->Text = String((int)strLog->MeasuredFaultCurrent);
	FLProcessNode->ChildNodes->Add(contentNode);
	// Cria n� para a resposta do processo
	contentNode = xmlFile->CreateElement("ProcessResponse", "");
	contentNode->Text = ProcessResponse(strLog->solutionsAmount);
	FLProcessNode->ChildNodes->Add(contentNode);
 	// Cria n� para oposi��o dos �ngulos das fases s�s
	contentNode = xmlFile->CreateElement("OpposedSoundPhases", "");
   if(strLog->fasesSasOpostas) contentNode->Text = "True"; else contentNode->Text = "False";
	FLProcessNode->ChildNodes->Add(contentNode);

   // :::::::::::::::::::::::::::::::::::::::::
	// AGRUPAMENTO DE ALARMES
	// :::::::::::::::::::::::::::::::::::::::::

	// Cria n� para indicar o grupo de alarmes considerado
	FLProcessNode = xmlFile->CreateElement("Alarms", "");
	Node->ChildNodes->Add(FLProcessNode);

	TList* lisAlarmes = strLog->lisAlarmes;

	for(int i=0; i<lisAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) lisAlarmes->Items[i];

		// Cria n� para alarme
		InputDataNode = xmlFile->CreateElement("Alarm", "");
		FLProcessNode->ChildNodes->Add(InputDataNode);

		// Insere informa��es do alarme
		contentNode = xmlFile->CreateElement("EquipmentType", "");
		contentNode->Text = StringTipoEqpto(alarme->GetTipoEqpto());
		InputDataNode->ChildNodes->Add(contentNode);
		contentNode = xmlFile->CreateElement("EquipmentCode", "");
		contentNode->Text = alarme->GetCodEqpto();
		InputDataNode->ChildNodes->Add(contentNode);
		contentNode = xmlFile->CreateElement("AlarmTimestamp", "");
		contentNode->Text = alarme->GetTimeStamp();
		InputDataNode->ChildNodes->Add(contentNode);

//		// Cria n� para o n�mero total de alarmes
//		contentNode = xmlFile->CreateElement("AlarmsTotalNumber", "");
//		contentNode->Text = strLog->AlarmsTotalNumber;
//		FLProcessNode->ChildNodes->Add(contentNode);
//		// Cria n� para toler�ncia do m�todo
//		contentNode = xmlFile->CreateElement("FaultLocationTolerance", "");
//		contentNode->Text = String(strLog->FLTolerance);
//		FLProcessNode->ChildNodes->Add(contentNode);
//		// Cria n� para a corrente de curto-circuito medida e utilizada pela metodologia
//		contentNode = xmlFile->CreateElement("MeasuredFaultCurrent", "");
//		contentNode->Text = String(strLog->MeasuredFaultCurrent);
//		FLProcessNode->ChildNodes->Add(contentNode);
	}

   // :::::::::::::::::::::::::::::::::::::::::
	// INSUMOS DE DADOS (INPUT DATA)
   // :::::::::::::::::::::::::::::::::::::::::

	// Cria n� para Data Input
	InputDataNode = xmlFile->CreateElement("InputData", "");
	Node->ChildNodes->Add(InputDataNode);

   // Para cada insumo de dado, gera um n�
	for(int i=0; i<strLog->lisInputData->Count; i++)
   {
		StrInsumoDado* dataInput = (StrInsumoDado*) strLog->lisInputData->Items[i];

      // Cria n� para Input
      InputNode = xmlFile->CreateElement("Input", "");
      InputDataNode->ChildNodes->Add(InputNode);
      // Cria n� para tipo de Input
      contentNode = xmlFile->CreateElement("Type", "");
      contentNode->Text = dataInput->tipoEqpto;
      InputNode->ChildNodes->Add(contentNode);
      // Cria n� para o c�digo do equipamento
      contentNode = xmlFile->CreateElement("Equipment", "");
      contentNode->Text = dataInput->codEqpto;
      InputNode->ChildNodes->Add(contentNode);

      // Insere as medi��es que foram utilizadas
      measurements = "0";
      if(dataInput->tipoEqpto == "Relay")
		{
      	measurements = dataInput->Ifalta_DJ_RE;
      }
      else if(dataInput->tipoEqpto == "Sensor")
      {
         measurements = dataInput->IA_falta_sensor;
         measurements += ";" + dataInput->IB_falta_sensor;
			measurements += ";" + dataInput->IC_falta_sensor;
      }
      else if(dataInput->tipoEqpto == "Qualimetro")
      {
         measurements = dataInput->VA_falta_qualimetro;
         measurements += ";" + dataInput->VB_falta_qualimetro;
			measurements += ";" + dataInput->VC_falta_qualimetro;
         measurements += ";" + dataInput->IA_falta_qualimetro;
         measurements += ";" + dataInput->IB_falta_qualimetro;
			measurements += ";" + dataInput->IC_falta_qualimetro;
      }
      contentNode = xmlFile->CreateElement("Measurements", "");
      contentNode->Text = measurements;
      InputNode->ChildNodes->Add(contentNode);
   }

   // :::::::::::::::::::::::::::::::::::::::::
	// DADOS RUINS (BAD DATA)
   // :::::::::::::::::::::::::::::::::::::::::

   if(strLog->lisBadData->Count == 0)
   {
   	// Cria n� para Bad Data
      BadDataNode = xmlFile->CreateElement("BadData", "");
      BadDataNode->Text = "";
      Node->ChildNodes->Add(BadDataNode);
   }
   else
   {
      // Cria n� para Bad Data
      BadDataNode = xmlFile->CreateElement("BadData", "");
      Node->ChildNodes->Add(BadDataNode);

      // Para cada dado ruim, gera um n�
      for(int i=0; i<strLog->lisBadData->Count; i++)
      {
         StrDadoRuim* badData = (StrDadoRuim*) strLog->lisBadData->Items[i];

         // Cria n� para Device
         BadDataDeviceNode = xmlFile->CreateElement("Device", "");
         BadDataNode->ChildNodes->Add(BadDataDeviceNode);
         // Cria n� para tipo de Input
         contentNode = xmlFile->CreateElement("Type", "");
         contentNode->Text = badData->tipoEqpto;
         BadDataDeviceNode->ChildNodes->Add(contentNode);
         // Cria n� para o c�digo do equipamento
         contentNode = xmlFile->CreateElement("Equipment", "");
         contentNode->Text = badData->codEqpto;
         BadDataDeviceNode->ChildNodes->Add(contentNode);
      }
   }

   // :::::::::::::::::::::::::::::::::::::::::
	// ERROS DE AQUISI��O
   // :::::::::::::::::::::::::::::::::::::::::
   if(strLog->lisEqptosErrosAquisicao->Count == 0)
   {
   	// Cria n� para Erros de Aquisi��o
      contentNode = xmlFile->CreateElement("AquisitionErrors", "");
      contentNode->Text = "";
      Node->ChildNodes->Add(contentNode);
   }
   else
   {
      // Cria n� para Erros de Aquisi��o
   	aquisitionErrors = xmlFile->CreateElement("AquisitionErrors", "");
      Node->ChildNodes->Add(aquisitionErrors);

      for(int i=0; i<strLog->lisEqptosErrosAquisicao->Count; i++)
      {
         String erroAquisicao = strLog->lisEqptosErrosAquisicao->Strings[i];

         // Cria n� para eqptos indispon�veis
         deviceNode = xmlFile->CreateElement("Device", "");
         aquisitionErrors->ChildNodes->Add(deviceNode);

         contentNode = xmlFile->CreateElement("Type", "");
         contentNode->Text = GetCampoCSV(erroAquisicao, 0, ";");
         deviceNode->ChildNodes->Add(contentNode);
         contentNode = xmlFile->CreateElement("Code", "");
         contentNode->Text = GetCampoCSV(erroAquisicao, 1, ";");
         deviceNode->ChildNodes->Add(contentNode);
      }
   }


   // :::::::::::::::::::::::::::::::::::::::::
	// DADOS ADICIONAIS (ADDITIONAL DATA)
   // :::::::::::::::::::::::::::::::::::::::::
   if(strLog->DistFaltaRele == "")
   {
   	// Cria n� para Dados Adicionais
      contentNode = xmlFile->CreateElement("AdditionalData", "");
      contentNode->Text = "";
      Node->ChildNodes->Add(contentNode);
   }
   else
   {
      // Cria n� para Dados Adicionais
   	additionalData = xmlFile->CreateElement("AdditionalData", "");
      Node->ChildNodes->Add(additionalData);

      // Cria n� para informa��o adicional do tipo RelayFaultDistance
      relayFaultDistance = xmlFile->CreateElement("RelayFaultDistance", "");
		additionalData->ChildNodes->Add(relayFaultDistance);

		contentNode = xmlFile->CreateElement("Equipment", "");
      contentNode->Text = strLog->CodigoReleSubestacao;
      relayFaultDistance->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("FaultDistance", "");
      contentNode->Text = strLog->DistFaltaRele;
      relayFaultDistance->ChildNodes->Add(contentNode);
	}

//   // :::::::::::::::::::::::::::::::::::::::::
//	// SENSORES INDISPON�VEIS
//   // :::::::::::::::::::::::::::::::::::::::::
//
//   if(strLog->lisUnavailableSensors->Count == 0)
//   {
//   	// Cria n� para sensores indispon�veis
//      contentNode = xmlFile->CreateElement("UnavailableSensors", "");
//      contentNode->Text = "";
//      Node->ChildNodes->Add(contentNode);
//   }
//   else
//   {
//    	// Cria n� para sensores indispon�veis
//      unavailableSensorNode = xmlFile->CreateElement("UnavailableSensors", "");
//      Node->ChildNodes->Add(unavailableSensorNode);
//      for(int i=0; i<strLog->lisUnavailableSensors->Count; i++)
//      {
//         codSensor = strLog->lisUnavailableSensors->Strings[i];
//
//         contentNode = xmlFile->CreateElement("SensorCode", "");
//         contentNode->Text = codSensor;
//         unavailableSensorNode->ChildNodes->Add(contentNode);
//      }
//   }
//
//   // :::::::::::::::::::::::::::::::::::::::::
//	// MEDI��ES INDISPON�VEIS
//   // :::::::::::::::::::::::::::::::::::::::::
//
//   if(strLog->lisUnavailableMeasurements->Count == 0)
//   {
//      // Cria n� para medi��o indispon�vel
//      contentNode = xmlFile->CreateElement("UnavailableMeasurements", "");
//      contentNode->Text = "";
//      Node->ChildNodes->Add(contentNode);
//   }
//   else
//   {
//      // Cria n� para medi��es indispon�veis
//      unavailableMeasurementsNode = xmlFile->CreateElement("UnavailableMeasurements", "");
//      Node->ChildNodes->Add(unavailableMeasurementsNode);
//
//      // Cria n� para medi��o indispon�vel
//      for(int i=0; i<strLog->lisUnavailableMeasurements->Count; i++)
//      {
//      	String linha = strLog->lisUnavailableMeasurements->Strings[i];
//
//         // Adiciona n� de medi��o
//         MeasurementNode = xmlFile->CreateElement("Measurement", "");
//         unavailableMeasurementsNode->ChildNodes->Add(MeasurementNode);
//
//         // Obt�m os valores da linha
//         codEqptoMedicao = GetCampoCSV(linha, 0, ";");
//         tipoMedicao = GetCampoCSV(linha, 1, ";");
//
//         // Conte�do da medi��o indispon�vel
//         contentNode = xmlFile->CreateElement("MeasurementType", "");
//         contentNode->Text = tipoMedicao;
//         MeasurementNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("EquipmentCode", "");
//         contentNode->Text = codEqptoMedicao;
//         MeasurementNode->ChildNodes->Add(contentNode);
//      }
//   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddDadosGerais(StrDadosGerais* strGer)
{
	TAlarme* alarme;
	_di_IXMLNode contentNode;
	_di_IXMLNode Node;
	_di_IXMLNode triggerNode;

   // :::::::::::::::::::::::::::::::::::::::::
	// TriggerData
   // :::::::::::::::::::::::::::::::::::::::::

   // Cria n� para dados de trigger - gatilhos que iniciaram o processo
	Node = xmlFile->CreateElement("TriggerData", "");
   rootNode->ChildNodes->Add(Node);
   for(int i=0; i<strGer->lisTriggers->Count; i++)
   {
      alarme = (TAlarme*) strGer->lisTriggers->Items[i];

      triggerNode = xmlFile->CreateElement("Trigger", "");
      Node->ChildNodes->Add(triggerNode);

		// Insere o timestamp do alarme
      contentNode = xmlFile->CreateElement("TimeStamp", "");
      contentNode->Text = alarme->GetTimeStamp();
      triggerNode->ChildNodes->Add(contentNode);
		// Insere o c�digo do eqpto que gerou o alarme
      contentNode = xmlFile->CreateElement("TriggeredEquipment", "");
      contentNode->Text = alarme->GetCodEqpto();
      triggerNode->ChildNodes->Add(contentNode);
		// Insere o c�digo/tipo do alarme
      contentNode = xmlFile->CreateElement("AlarmType", "");
      contentNode->Text = TraduzTipoAlarme(alarme->GetTipoAlarme());
      triggerNode->ChildNodes->Add(contentNode);
   }
//   // Cria n�
//   Node = xmlFile->CreateElement("TriggerData", "");
//   rootNode->ChildNodes->Add(Node);
//
//   contentNode = xmlFile->CreateElement("TimeStamp", "");
//   contentNode->Text = strGer->TimeStamp;
//   Node->ChildNodes->Add(contentNode);
//   contentNode = xmlFile->CreateElement("TriggeredEquipment", "");
//   contentNode->Text = strGer->EqptoTrigger;
//   Node->ChildNodes->Add(contentNode);

   // :::::::::::::::::::::::::::::::::::::::::
	// Contingency
   // :::::::::::::::::::::::::::::::::::::::::

   // Cria n�
   Node = xmlFile->CreateElement("Contingency", "");
   rootNode->ChildNodes->Add(Node);

   contentNode = xmlFile->CreateElement("TriggeredEquipment", "");
   contentNode->Text = strGer->EqptoTrigger;
   Node->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("AffectedCustomers", "");
   contentNode->Text = String(strGer->ClientesAfetados);
	Node->ChildNodes->Add(contentNode);
//   contentNode = xmlFile->CreateElement("DownedConductor", "");
//   if(strGer->rompCabo) contentNode->Text = "True"; else contentNode->Text = "False";
//	Node->ChildNodes->Add(contentNode);

   // :::::::::::::::::::::::::::::::::::::::::
	// Topology
   // :::::::::::::::::::::::::::::::::::::::::

   // Cria n�
   Node = xmlFile->CreateElement("Topology", "");
   rootNode->ChildNodes->Add(Node);

   contentNode = xmlFile->CreateElement("Substation", "");
   contentNode->Text = strGer->SE;
   Node->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("Feeder", "");
   contentNode->Text = strGer->Alimentador;
   Node->ChildNodes->Add(contentNode);

}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddSolution(strSolucao* strSol)
{
	_di_IXMLNode contentNode;
	_di_IXMLNode solutionNode;

   // Cria n� para uma solu��o
   solutionNode = xmlFile->CreateElement("Solution", "");
   rootNode->ChildNodes->Add(solutionNode);

   // Cria n�s de conte�do
//   contentNode = xmlFile->CreateElement("SE", "");
//   contentNode->Text = SE;
//   solutionNode->ChildNodes->Add(contentNode);
//   contentNode = xmlFile->CreateElement("Alimentador", "");
//   contentNode->Text = Alimentador;
//   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("Likelyhood", "");
   contentNode->Text = strSol->Probabilidade;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("BusCode", "");
   contentNode->Text = strSol->CodBarra;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("BusId", "");
   contentNode->Text = String(strSol->IdBarra);
   solutionNode->ChildNodes->Add(contentNode);
//   contentNode = xmlFile->CreateElement("FaultType", "");
//   contentNode->Text = strSol->DefTipo;
//   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CoordLat", "");
   contentNode->Text = strSol->DefLat;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CoordLon", "");
   contentNode->Text = strSol->DefLon;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("UpstreamSwitch", "");
   contentNode->Text = strSol->ChvMont;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("UpstreamSwitchDistance", "");
   contentNode->Text = strSol->DistChvMont;
   solutionNode->ChildNodes->Add(contentNode);
   contentNode = xmlFile->CreateElement("CustomersAfterSwitch", "");
   contentNode->Text = strSol->ClientesDepoisChvMont;
   solutionNode->ChildNodes->Add(contentNode);

   // Incrementa contador de itens
   contItens += 1;
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::AddSolutions(TList* lisSolucoes)
{
	strSolucao* strSol;
	_di_IXMLNode contentNode;
	_di_IXMLNode solutionNode;
   _di_IXMLNode solutionsNode;


   solutionsNode = xmlFile->CreateElement("Solutions", "");
   rootNode->ChildNodes->Add(solutionsNode);


   for(int i=0; i<lisSolucoes->Count; i++)
   {

      strSol = (strSolucao*) lisSolucoes->Items[i];

      // Cria n� "Solution", dentro do n� "Solutions"
      solutionNode = xmlFile->CreateElement("Solution", "");
      solutionsNode->ChildNodes->Add(solutionNode);

     	// Cria os n�s/par�metros de uma solution
      contentNode = xmlFile->CreateElement("LikelyhoodOrder", "");
      contentNode->Text = strSol->Probabilidade;
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("BusCode", "");
      contentNode->Text = strSol->CodBarra;
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("BusId", "");
      contentNode->Text = String(strSol->IdBarra);
		solutionNode->ChildNodes->Add(contentNode);

		contentNode = xmlFile->CreateElement("FaultType", "");
		if(strSol->DefTipo == "ABABC")
			contentNode->Text = strSol->DefTipo_detalhe;
		else
			contentNode->Text = strSol->DefTipo;

      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("CoordLat", "");
      contentNode->Text = strSol->DefLat;
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("CoordLon", "");
      contentNode->Text = strSol->DefLon;
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("DistanceFromSubstation", "");
		contentNode->Text = String(Round(strSol->DistRele.ToDouble(),3));
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("UpstreamSwitch", "");
      contentNode->Text = strSol->ChvMont;
      solutionNode->ChildNodes->Add(contentNode);
      contentNode = xmlFile->CreateElement("UpstreamSwitchDistance", "");
      contentNode->Text = strSol->DistChvMont;
		solutionNode->ChildNodes->Add(contentNode);
		contentNode = xmlFile->CreateElement("CustomersAfterSwitch", "");
		contentNode->Text = strSol->ClientesDepoisChvMont;
		solutionNode->ChildNodes->Add(contentNode);
		contentNode = xmlFile->CreateElement("EstimatedRfault", "");
		contentNode->Text = String(Round(strSol->Rfalta_estimado,3));
		solutionNode->ChildNodes->Add(contentNode);

      // Incrementa contador de itens
      contItens += 1;
   }
}
//---------------------------------------------------------------------------
bool __fastcall TXMLComunicacao::ExisteSolicitacao(String codEqpto, String requestType)
{
	strSolicitacao* solicit;

	for(int i=0; i<lisStrSolicitacoes->Count; i++)
   {
    	solicit = (strSolicitacao*) lisStrSolicitacoes->Items[i];
      if(solicit->CodEqpto == codEqpto && solicit->requestType == requestType)
      	return true;
   }

   return false;
}
////---------------------------------------------------------------------------
///***
// * M�todo que escreve um XML com os resultados dos processos de LF
// */
//void __fastcall TXMLComunicacao::InsereProcessosLF(StrDadosGerais* dadosGerais, TList* lisSolucoes)
//{
//	strSolucao* solucao;
//  	String pathFinal;
//   TXMLComunicacao* xmlCom;
//   _di_IXMLNode FLProcessNode;
//   _di_IXMLNode solutionNode;
//   _di_IXMLNode solutionsNode;
//
//	// Verifica��o
//	if(lisStrProcessosLF == NULL) return;
//
////   // Adiciona dados gerais
////	xmlCom->AddDadosGerais(strDadosGerais);
////
//////	// Adiciona informa��es de LOG
//////	xmlCom->AddDadosLog(strLog);
////
////   // Adiciona solu��es
////   xmlCom->AddSolutions(lisSolucoes);
//
//
//	for(int i=0; i<lisStrProcessosLF->Count; i++)
//   {
//      FLProcessNode = xmlFile->CreateElement("FaultLocationProcess", "");
//      rootNode->ChildNodes->Add(FLProcessNode);
//      StrProcessoLF* processoLF = (StrProcessoLF*) lisStrProcessosLF->Items[i];
//
//      // Insere no XML os dados gerais
//
//      // Insere no XML os dados das solu��es
//      solutionsNode = xmlFile->CreateElement("Solutions", "");
//      FLProcessNode->ChildNodes->Add(solutionsNode);
//
//      for(int j=0; j<processoLF->lisSolucoes->Count; j++)
//      {
//      	strSolucao* strSol = (strSolucao*) processoLF->lisSolucoes->Items[j];
//
//         // Cria n� "Solution", dentro do n� "Solutions"
//         solutionNode = xmlFile->CreateElement("Solution", "");
//         solutionsNode->ChildNodes->Add(solutionNode);
//
//         // Cria os n�s/par�metros de uma solution
//         contentNode = xmlFile->CreateElement("LikelyhoodOrder", "");
//         contentNode->Text = strSol->Probabilidade;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("BusCode", "");
//         contentNode->Text = strSol->CodBarra;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("BusId", "");
//         contentNode->Text = String(strSol->IdBarra);
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("FaultType", "");
//         contentNode->Text = strSol->DefTipo;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("CoordLat", "");
//         contentNode->Text = strSol->DefLat;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("CoordLon", "");
//         contentNode->Text = strSol->DefLon;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("DistanceFromSubstation", "");
//         contentNode->Text = String(Round(strSol->DistRele.ToDouble(),3));
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("UpstreamSwitch", "");
//         contentNode->Text = strSol->ChvMont;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("UpstreamSwitchDistance", "");
//         contentNode->Text = strSol->DistChvMont;
//         solutionNode->ChildNodes->Add(contentNode);
//         contentNode = xmlFile->CreateElement("CustomersAfterSwitch", "");
//         contentNode->Text = strSol->ClientesDepoisChvMont;
//         solutionNode->ChildNodes->Add(contentNode);
//      }
//   }
//}
//---------------------------------------------------------------------------
int __fastcall TXMLComunicacao::GetContItens()
{
	return contItens;
}
//---------------------------------------------------------------------------
_di_IXMLNode __fastcall TXMLComunicacao::GetRootNode()
{
   return rootNode;
}
//---------------------------------------------------------------------------
String __fastcall TXMLComunicacao::ProcessResponse(int solutionsAmount)
{
	String resp;
	if(solutionsAmount <= 0)
		resp = "Response with no solutions";
	else if(solutionsAmount > 0)
		resp = "Response with solutions";

	return(resp);
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::Salvar()
{
	try
   {
      // Salva o arquivo XML
      xmlFile->SaveToFile(pathFinal);
   }
   catch(Exception &e)
   {
		if(logErros)
      {
      	String linhaErros = "";
         linhaErros = "N�o foi poss�vel encontrar o caminho: " + pathFinal + ".";
         logErros->AddLinha(linhaErros);
      }
   }
}
//---------------------------------------------------------------------------
String __fastcall TXMLComunicacao::TraduzTipoAlarme(int tipoAlarme)
{
	// Obs: posteriormente, trazer essas informa��es no TConfiguracoes
   // que pode obter a rela��o DE-PARA a partir de arquivo externo

   String strTipoAlarme = "";

	switch(tipoAlarme)
   {
   case 1:
   	strTipoAlarme = "FREL";
      break;

   case 2:
   	strTipoAlarme = "RAUT";
      break;

   case 3:
		strTipoAlarme = "FPE";
      break;

   case 4:
		strTipoAlarme = "IAM";
      break;

   case 5:
      strTipoAlarme = "VSAGQ";
      break;

   case 6:
      strTipoAlarme = "ICCQ";
      break;

   case 7:
      strTipoAlarme = "VSAGTR";
      break;

   default:
   	strTipoAlarme = "";
      break;
   }

   return strTipoAlarme;
}
//---------------------------------------------------------------------------
String __fastcall TXMLComunicacao::TimeStampAtual()
{
   String timestamp = "";
   TDateTime agora = Now();
	unsigned short hora, min, seg, mseg, ano, mes, dia;

   agora.DecodeDate(&ano, &mes, &dia);
   agora.DecodeTime(&hora, &min, &seg, &mseg);

	timestamp = String(ano);
   if(mes < 10) timestamp += "0";
	timestamp += String(mes);
   if(dia < 10) timestamp += "0";
	timestamp += String(dia);
   if(hora < 10) timestamp += "0";
	timestamp += String(hora);
   if(min < 10) timestamp += "0";
	timestamp += String(min);
   if(seg < 10) timestamp += "0";
	timestamp += String(seg);

   return timestamp;
}
//---------------------------------------------------------------------------
void __fastcall TXMLComunicacao::SetLogErros(TLog* logErros)
{
	this->logErros = logErros;
}
//---------------------------------------------------------------------------
String __fastcall TXMLComunicacao::StringTipoEqpto(int tipoEqpto)
{
	String stringTipoEqpto = "";
	switch(tipoEqpto)
	{
	case 1:
		stringTipoEqpto = "Circuit breaker";
		break;

	case 2:
		stringTipoEqpto = "Recloser";
		break;

	case 3:
		stringTipoEqpto = "Switch";
		break;

	case 4:
		stringTipoEqpto = "Sensor";
		break;

	case 6:
		stringTipoEqpto = "Power quality meter";
		break;

	case 7:
		stringTipoEqpto = "Intelligent transformer";
		break;

	default:
		break;
	}
	return(stringTipoEqpto);
}
//---------------------------------------------------------------------------
