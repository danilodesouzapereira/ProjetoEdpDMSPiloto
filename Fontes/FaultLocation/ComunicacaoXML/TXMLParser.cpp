//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlarme.h"
#include "TXMLComunicacao.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TLog.h"
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include "TXMLParser.h"
#include <XMLDoc.hpp>
#include <StrUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TXMLParser::TXMLParser(VTApl* apl, TForm* formFL)
{
	// Inicializações
	this->apl = apl;
   path = (VTPath*) apl->GetObject(__classid(VTPath));
   this->formFL = formFL;
   logErros = NULL;

   // Seta diretório com os XML de alarme
   dirAlarmes = path->DirImporta() + "\\FaultLocation\\Alarmes";
   dirMsgEntrada = path->DirImporta() + "\\FaultLocation\\Mensagens";
}
//---------------------------------------------------------------------------
__fastcall TXMLParser::~TXMLParser()
{

}
//---------------------------------------------------------------------------
void __fastcall TXMLParser::DefineEqpto(TList* lisEXT)
{
   StrReqResponse* resp = NULL;

   // Se há resposta para os timestamps das funções de proteção, trata-se de um disjuntor (DJ ou RE)
   // Se há resposta para o campo "Qualidade", trata-se de um sensor (SR)

	for(int i=0; i<lisEXT->Count; i++)
   {
   	resp = (StrReqResponse*) lisEXT->Items[i];

      if(resp->TipoEqpto != "") continue;

      if(resp->Qualidade != "")
      {
         resp->TipoEqpto = "SR";
      }
      else if((resp->Funcao50 != "") ||
              (resp->Funcao51 != "") ||
              (resp->Funcao50N != "") ||
              (resp->Funcao51N != ""))
      {
         resp->TipoEqpto = "DJ";
      }
      else if(resp->VA != "" || resp->VB != "" || resp->VC != "")
      {
			resp->TipoEqpto = "QUALIM";
      }
      else
	      resp->TipoEqpto = "";
   }
}
//---------------------------------------------------------------------------
StrReqResponse* __fastcall TXMLParser::FindRespGeral(TList* lisEXT, String CodEqpto, String TipoEqpto)
{
   StrReqResponse* resp = NULL;

   for(int j=0; j<lisEXT->Count; j++)
   {
      resp = (StrReqResponse*) lisEXT->Items[j];
      if(resp->CodEqpto == CodEqpto && resp->TipoEqpto == TipoEqpto)
         break;
      else
			resp = NULL;
   }

   return resp;
}
////---------------------------------------------------------------------------
//// Pegando as atualizações de um determinado arquivo de alarme.
//void __fastcall TXMLParser::GetAtualizacoesAlarmes(String pathArqAlarme, TList* lisEXT)
//{
//	if(pathArqAlarme == "" || !lisEXT) return;
//
//	TXMLDocument* docXMLAlarmes;
//	int TipoEqpto, TipoAlarme;
//	String TimeStamp, CodAlimentador, CodEqpto, pathArquivoAlarme;
//	TAlarme* alarme;
//
//	// Carrega arquivo XML
//	docXMLAlarmes = new TXMLDocument(formFL);
//	docXMLAlarmes->LoadFromFile(pathArqAlarme);
//
//	// Nó raiz
//	IXMLNode *rootNode = docXMLAlarmes->DocumentElement;
//	IXMLNodeList *listaAlarmes = rootNode->ChildNodes;
//
//	for(int i=0; i<listaAlarmes->GetCount(); i++)
//	{
//		// Percorrendo os nós de alarmes
//		_di_IXMLNodeList nodeAlarme = listaAlarmes[0].Get(i)->ChildNodes;
//
//		for(int j=0; j<nodeAlarme->Count; j++)
//		{
//			_di_IXMLNode atributo = nodeAlarme->Get(j);
//
//			if(atributo->NodeName == "TimeStamp")
//			{
//				TimeStamp = atributo->Text;
//			}
//			else if(atributo->NodeName == "CodAlimentador")
//			{
//				CodAlimentador = atributo->Text;
//			}
//			else if(atributo->NodeName == "TipoEqpto")
//			{
//				TipoEqpto = atributo->Text.ToInt();
//			}
//			else if(atributo->NodeName == "CodEqpto")
//			{
//				CodEqpto = atributo->Text;
//			}
//			else if(atributo->NodeName == "TipoAlarme")
//			{
//				TipoAlarme = atributo->Text.ToInt();
//			}
//		}
//
//		// Verificação
//		if(TimeStamp == "" || CodAlimentador == "" || TipoEqpto == -1 || TipoAlarme == -1 || CodEqpto == "")
//			continue;
//
//		// Cria obj de TAlarme e o insere na lista externa
//		alarme = new TAlarme(TimeStamp, CodAlimentador, TipoAlarme, TipoEqpto, CodEqpto, pathArqAlarme);
//		lisEXT->Add(alarme);
//	}
//   delete docXMLAlarmes;
//}
//---------------------------------------------------------------------------
TAlarme* __fastcall TXMLParser::GetAlarme(String CodArquivoAlarme)
{
	bool funcao50A, funcao50B, funcao50C, funcao50N;
	bool funcao51A, funcao51B, funcao51C, funcao51N;
	double correnteFalta = 0.;
   int TipoEqpto, TipoAlarme;
	String TimeStamp, CodAlimentador, CodEqpto, pathArquivoAlarme;
	TAlarme* alarme;
	TXMLDocument* docXMLAlarmes;

   // Inicializações
   TipoEqpto = -1;
   TipoAlarme = -1;

	// Carrega arquivo XML
	docXMLAlarmes = new TXMLDocument(formFL);
	pathArquivoAlarme = dirAlarmes + "\\" + CodArquivoAlarme + ".xml";
	docXMLAlarmes->LoadFromFile(pathArquivoAlarme);

	// Nó raiz
	IXMLNode *rootNode = docXMLAlarmes->DocumentElement;
	IXMLNodeList *listaAlarmes = rootNode->ChildNodes;

	// Supondo que sempre teremos apenas 1 nó:
	int i = 0;
	_di_IXMLNodeList nodeAlarme = listaAlarmes[0].Get(i)->ChildNodes;

	for(int j=0; j<nodeAlarme->Count; j++)
	{
		_di_IXMLNode atributo = nodeAlarme->Get(j);

		if(atributo->NodeName == "TimeStamp")
		{
			TimeStamp = atributo->Text;
		}
      else if(atributo->NodeName == "CodAlimentador")
      {
         CodAlimentador = atributo->Text;
      }
      else if(atributo->NodeName == "TipoEqpto")
      {
         TipoEqpto = atributo->Text.ToInt();
      }
      else if(atributo->NodeName == "CodEqpto")
      {
         CodEqpto = atributo->Text;
		}
		else if(atributo->NodeName == "TipoAlarme")
		{
			TipoAlarme = atributo->Text.ToInt();
		}
		else if(atributo->NodeName == "Funcao50A")
		{
			funcao50A = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao50B")
		{
			funcao50B = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao50C")
		{
			funcao50C = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao50N")
		{
			funcao50N = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao51A")
		{
			funcao51A = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao51B")
		{
			funcao51B = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao51C")
		{
			funcao51C = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "Funcao51N")
		{
			funcao51N = (atributo->Text == "True" || atributo->Text == "true" || atributo->Text == "TRUE");
		}
		else if(atributo->NodeName == "CorrenteFalta")
		{
			correnteFalta = atributo->Text.ToDouble();
		}
	}

   // Verificação
	if(TimeStamp == "" || CodAlimentador == "" || TipoEqpto == -1 || TipoAlarme == -1 || CodEqpto == "")
		return NULL;

	if((TipoEqpto == 10 || TipoEqpto == 11) && (!funcao50A && !funcao50B && !funcao50C && !funcao50N &&
															  !funcao51A && !funcao51B && !funcao51C && !funcao51N))
		return NULL;

	if((TipoEqpto == 10 || TipoEqpto == 11) && correnteFalta <= 0.)
		return NULL;

	// Cria obj de TAlarme, com as informações que chegaram via XML
	alarme = new TAlarme(TimeStamp, CodAlimentador, TipoAlarme, TipoEqpto, CodEqpto, pathArquivoAlarme,
								funcao50A, funcao50B, funcao50C, funcao50N, funcao51A, funcao51B, funcao51C, funcao51N, correnteFalta);

	return alarme;
}
//---------------------------------------------------------------------------
void __fastcall TXMLParser::GetCadSensores(String CodArquivoCadSensores, TList* lisEXT)
{
	String codAlimentador, nomeXMLCadastro;
	StrCadSensor* str_cad;
   TList* lisCadSensores;
	TXMLDocument* docXMLCadastro;

   // Verificação
   if(lisEXT == NULL) return;
   lisEXT->Clear();

   // Carrega arquivo XML
   docXMLCadastro = new TXMLDocument(formFL);
   nomeXMLCadastro = dirMsgEntrada + "\\" + CodArquivoCadSensores + ".xml";
   docXMLCadastro->LoadFromFile(nomeXMLCadastro);

   // Nó raiz
   IXMLNode *rootNode = docXMLCadastro->DocumentElement;
   IXMLNodeList *listaResponses = rootNode->ChildNodes;

   // Para cada response, gera objetos da struct "StrCadSensor" e guarda na lista de saída
   for(int i=0; i<listaResponses->Count; i++)
   {
    	_di_IXMLNodeList nodeResponse = listaResponses[0].Get(i)->ChildNodes;

		codAlimentador = "";
      for(int j=0; j<nodeResponse->Count; j++)
      {
      	_di_IXMLNode atributo = nodeResponse->Get(j);

      	if(atributo->NodeName == "Alimentador")
            codAlimentador = atributo->Text;
         else if(atributo->NodeName == "Sensors")
         {
          	_di_IXMLNodeList listaSensores = atributo->ChildNodes;
            for(int k=0; k<listaSensores->Count; k++)
            {
					_di_IXMLNode sensor = listaSensores->Get(k);

               // cria struct de cadastro de sensor
               str_cad = new StrCadSensor();
               str_cad->Alimentador = codAlimentador;

               // pega os atributos do sensor (seu código e código da chave associada)
               for(int m=0; m<sensor->ChildNodes->Count; m++)
               {
                  _di_IXMLNode atributoSensor = sensor->ChildNodes->Get(m);

                  if(sensor->ChildNodes->Get(m)->NodeName == "codSensor")
                     str_cad->CodSensor = sensor->ChildNodes->Get(m)->Text;
                  else if(sensor->ChildNodes->Get(m)->NodeName == "codChave")
                     str_cad->CodChave = sensor->ChildNodes->Get(m)->Text;
               }

               // salva a struct de cadastro na lista externa
               lisEXT->Add(str_cad);
            }
         }
      }
   }

   //Tenta apagar o arquivo XML com o cadastro de sensores
   DeleteFile(nomeXMLCadastro);



//   // Para cada sensor cadastrado, gera um obj da struct "StrCadSensor" e guarda na lista de saída
//	for(int i=0; i<listaSensores->Count; i++)
//   {
//   	_di_IXMLNodeList nodeSensor = listaSensores[0].Get(i)->ChildNodes;
//
//      str_cad = new StrCadSensor();
//
//      for(int j=0; j<nodeSensor->Count; j++)
//      {
//      	_di_IXMLNode atributo = nodeSensor->Get(j);
//
//         if(atributo->NodeName == "Alimentador")
//            str_cad->Alimentador = atributo->Text;
//
//         else if(atributo->NodeName == "CodSensor")
//         	str_cad->CodSensor = atributo->Text;
//
//         else if(atributo->NodeName == "CodChave")
//         	str_cad->CodChave = atributo->Text;
//      }
//
//      lisEXT->Add(str_cad);
//   }
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetCodEqpto(TXMLComunicacao* xmlRequest, String responseID)
{
	_di_IXMLNode rootNode;
	_di_IXMLNodeList req;
   _di_IXMLNode atributoID;
	_di_IXMLNode atributoCodEqpto;
	_di_IXMLNode atributoTipoEqpto;
   IXMLNodeList *listaRequests;
   String CodEqpto, TipoEqpto;

   rootNode = xmlRequest->GetRootNode();
	listaRequests = rootNode->ChildNodes;

   CodEqpto = "";

	// Se encontrar o request com ID = reponseID, pega o RequestType
	for(int i=0; i<listaRequests->Count; i++)
   {
   	req = listaRequests[0].Get(i)->ChildNodes;
      atributoID = req->FindNode("ID");
      if(atributoID->Text != responseID) continue;

      atributoCodEqpto = req->FindNode("CodEqpto");
      CodEqpto = atributoCodEqpto->Text;

      atributoTipoEqpto = req->FindNode("TipoEqpto");
   	TipoEqpto = atributoTipoEqpto->Text;
      break;
   }

	return CodEqpto;
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetCodChaveQualimetro(String CodQualimetro)
{

	String pathCadQualimetros = path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv";
   TStringList* linhas = new TStringList();
   linhas->LoadFromFile(pathCadQualimetros);

   for(int i=0; i<linhas->Count; i++)
   {
		String linha = linhas->Strings[i];
      String qualimetro = GetCampoCSV(linha, 1, ";");
      if(qualimetro == CodQualimetro)
      	return GetCampoCSV(linha, 2, ";");
   }

   return "";
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetCodChaveSensor(String CodSensor)
{

	String pathCadSensores = path->DirDat() + "\\FaultLocation\\CadastroSensores.csv";
   TStringList* linhas = new TStringList();
   linhas->LoadFromFile(pathCadSensores);

   for(int i=0; i<linhas->Count; i++)
   {
		String linha = linhas->Strings[i];
      String sensor = GetCampoCSV(linha, 1, ";");
      if(sensor == CodSensor)
      	return GetCampoCSV(linha, 2, ";");
   }

   return "";
}
//---------------------------------------------------------------------------
/***
 * Faz o parse do XML de respostas a solicitações de medições e alarmes
 */
void __fastcall TXMLParser::GetReqResponse(String CodArquivoResponse, TXMLComunicacao* xmlRequest, TList* lisEXT)
{
   TList* lisRespIni = new TList();
   StrReqResponse *resp, *respAux;

   // Obtém as responses do XML de resposta
	GetReqResponse_LeXML(CodArquivoResponse, xmlRequest, lisRespIni);

   // Para cada resp da lista inicial, gera um resp agregado / geral
   lisEXT->Clear();
   for(int i=0; i<lisRespIni->Count; i++)
   {
		resp = (StrReqResponse*) lisRespIni->Items[i];

      respAux = FindRespGeral(lisEXT, resp->CodEqpto, resp->TipoEqpto);
      // Se ainda não houver um resp geral, cria e guarda na lista externa
      if(respAux == NULL)
      {
         respAux = new StrReqResponse();
         // Inicializa parâmetros
         respAux->Resposta_erro = "False";
         respAux->Resposta_mensagem = "";
			lisEXT->Add(respAux);

         // Passa os parâmetros do resp inicial para o resp geral
         PreencheResp(resp, respAux, resp->TipoEqpto);
      }
      else
      {
         // Passa os parâmetros do resp inicial para o resp geral
         PreencheResp(resp, respAux, "");
      }
   }

   for(int i=lisRespIni->Count-1; i>=0; i--)
   {
		resp = (StrReqResponse*) lisRespIni->Items[i];
      delete resp;
   }
	delete lisRespIni; lisRespIni = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TXMLParser::GetReqResponse_LeXML(String CodArquivoResponse, TXMLComunicacao* xmlRequest, TList* lisEXT)
{
	String Resposta_erro;
	int requestType, tipoEqpto;
	String responseID, codEqpto, TimeStampEvento, Resposta_mensagem;
   StrReqResponse* strResp;
	TXMLDocument* docXMLresponse;
	_di_IXMLNode nodeI, nodeV, nodeAlarme, nodeDistFalta, nodeQualidade;


   // Verificação
	if(lisEXT == NULL) return;
   lisEXT->Clear();

   // Carrega arquivo XML de responses
   docXMLresponse = new TXMLDocument(formFL);
   docXMLresponse->LoadFromFile(dirMsgEntrada + "\\" + CodArquivoResponse);

   // Nó raiz de responses
   IXMLNode *rootNode = docXMLresponse->DocumentElement;
   IXMLNodeList *listResponses = rootNode->ChildNodes;

   // Para cada response, gera um obj da struct "StrReqResponse" e guarda na lista de saída
	for(int i=0; i<listResponses->Count; i++)
   {
   	_di_IXMLNodeList nodeResponse = listResponses[0].Get(i)->ChildNodes;

      // Procura pelo ID da response
      responseID = GetResponseID(nodeResponse);

      // A partir do xml de Request e o identificador do request, pega: tipo de request e tipo de eqpto
      requestType = GetRequestType(xmlRequest, responseID);
      tipoEqpto = GetTipoEqpto(xmlRequest, responseID);
      if(requestType == 0 || tipoEqpto == 0) continue;

      codEqpto = GetCodEqpto(xmlRequest, responseID);
      TimeStampEvento = GetTimeStampEvento(xmlRequest, responseID);

		strResp = new StrReqResponse();
      strResp->RequestID = responseID;
      strResp->CodEqpto = codEqpto;

      // Pega a "resposta", para verificar se houve problema na aquisição da informação solicitada
		Resposta_mensagem = "";
      Resposta_erro = GetResponseErro(nodeResponse, Resposta_mensagem);
      // Salva a resposta da aquisição
      strResp->Resposta_erro = Resposta_erro;
      strResp->Resposta_mensagem = Resposta_mensagem;

      // Se não houve erro, obtém os responses de acordo com o tipo de request
      if(Resposta_erro == "False")
		{
			if(requestType == 30)
         {
            strResp->TimeStampEvento = TimeStampEvento;

            try
            {
					nodeV = nodeResponse->FindNode("VA"); strResp->VA = nodeV->Text;
					nodeV = nodeResponse->FindNode("VB"); strResp->VB = nodeV->Text;
					nodeV = nodeResponse->FindNode("VC"); strResp->VC = nodeV->Text;
            }
				catch(Exception &e)
				{
					strResp->VA = "0";
					strResp->VB = "0";
					strResp->VC = "0";
               if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
						linhaLog += ", Request ID: " + responseID;
						linhaLog += " - Problema de ausência de tensões de fase";
						if(tipoEqpto == 12) linhaLog += " de Medidor Inteligente de balanço.";
						else if(tipoEqpto == 13) linhaLog += " de Medidor Inteligente de consumidor MT.";
						else if(tipoEqpto == 14) linhaLog += " de Medidor Inteligente de consumidor BT.";
                  logErros->AddLinha(linhaLog);
               }
            }

				// Verifica o eqpto solicitado
				if(tipoEqpto == 10)
				{
					strResp->TipoEqpto = "DJ";
				}
				else if(tipoEqpto == 11)
				{
					strResp->TipoEqpto = "RE";
				}
				else if(tipoEqpto == 12 || tipoEqpto == 13 || tipoEqpto == 14)
				{
					strResp->TipoEqpto = "MI";
				}

				// Troca "." por ","
				strResp->VA = ReplaceStr(strResp->VA, ".", ",");
				strResp->VB = ReplaceStr(strResp->VB, ".", ",");
				strResp->VC = ReplaceStr(strResp->VC, ".", ",");
			}
			else if(requestType == 1)
         {
            strResp->TimeStampEvento = TimeStampEvento;

            try
            {
               nodeI = nodeResponse->FindNode("IA"); strResp->IA = nodeI->Text;
               nodeI = nodeResponse->FindNode("IB"); strResp->IB = nodeI->Text;
               nodeI = nodeResponse->FindNode("IC"); strResp->IC = nodeI->Text;
            }
            catch(Exception &e)
            {
               strResp->IA = "0";
               strResp->IB = "0";
               strResp->IC = "0";
               if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema de ausência de correntes de falta";
                  if(tipoEqpto == 1) linhaLog += " de Disjuntor.";
                  else if(tipoEqpto == 2) linhaLog += " de Religadora.";
                  else if(tipoEqpto == 4) linhaLog += " de Sensor.";
                  logErros->AddLinha(linhaLog);
               }
            }

            // Verifica se o eqpto solicitado é SENSOR OU DJ/RE
            if(tipoEqpto == 1)
            {
            	strResp->TipoEqpto = "DJ";
            }
            else if(tipoEqpto == 2)
            {
            	strResp->TipoEqpto = "RE";
            }
            else if(tipoEqpto == 4)
            {
               strResp->TipoEqpto = "SR";
            }
            else
            {
               if((strResp->IA.ToDouble() > 0.) && (strResp->IB.ToDouble() > 0.) && (strResp->IC.ToDouble() > 0.))
               {
                  strResp->TipoEqpto = "SR";
               }
               // Verifica se é DR/RE
               else if((strResp->IA.ToDouble() > 0.) && (strResp->IB.ToDouble() == 0.) && (strResp->IC.ToDouble() == 0.))
               {
                  strResp->TipoEqpto = "DJ";
               }
            }

            strResp->IA = ReplaceStr(strResp->IA, ".", ",");
            strResp->IB = ReplaceStr(strResp->IB, ".", ",");
            strResp->IC = ReplaceStr(strResp->IC, ".", ",");
         }
         else if(requestType == 2)
         {
            try
            {
               nodeAlarme = nodeResponse->FindNode("TimeStampEvento"); strResp->TimeStampEvento = nodeAlarme->Text;
               nodeAlarme = nodeResponse->FindNode("Funcao50"); strResp->Funcao50 = nodeAlarme->Text;
               nodeAlarme = nodeResponse->FindNode("Funcao51"); strResp->Funcao51 = nodeAlarme->Text;
               nodeAlarme = nodeResponse->FindNode("Funcao50N"); strResp->Funcao50N = nodeAlarme->Text;
               nodeAlarme = nodeResponse->FindNode("Funcao51N"); strResp->Funcao51N = nodeAlarme->Text;
            }
            catch(Exception &e)
            {
               strResp->TimeStampEvento = "00000000000000";
               strResp->Funcao50 = "False";
               strResp->Funcao51 = "False";
               strResp->Funcao50N = "False";
               strResp->Funcao51N = "False";
               if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema na consulta das funções de proteção atuantes.";
                  logErros->AddLinha(linhaLog);
               }
            }
            // Seta tipo de equipamento
            if(tipoEqpto == 1)
            {
            	strResp->TipoEqpto = "DJ";
            }
            else if(tipoEqpto == 2)
            {
            	strResp->TipoEqpto = "RE";
            }
         }
         else if(requestType == 3)
         {
            try
            {
	            nodeDistFalta = nodeResponse->FindNode("DistFaltaRele"); strResp->DistFaltaRele = ReplaceStr(nodeDistFalta->Text, ".", ",");
            }
            catch(Exception &e)
            {
             	strResp->DistFaltaRele = "0";
					if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema na consulta da distância de falta do relé.";
                  logErros->AddLinha(linhaLog);
               }
            }
            // Seta DJ
            strResp->TipoEqpto = "DJ";
         }
         else if(requestType == 4)
         {
            try
            {
	            nodeQualidade = nodeResponse->FindNode("Qualidade"); strResp->Qualidade = nodeQualidade->Text;
            }
            catch(Exception &e)
            {
            	strResp->Qualidade = "BAD";
					if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema na avaliação da disponibilidade de sensor.";
                  logErros->AddLinha(linhaLog);
               }
            }
            // Seta SENSOR
            strResp->TipoEqpto = "SR";
         }
         else if(requestType == 7)
         {
            try
            {
	            strResp->TimeStampEvento = TimeStampEvento;
               nodeV = nodeResponse->FindNode("VA"); strResp->VA = ReplaceStr(nodeV->Text, ".", ",");
               nodeV = nodeResponse->FindNode("VB"); strResp->VB = ReplaceStr(nodeV->Text, ".", ",");
               nodeV = nodeResponse->FindNode("VC"); strResp->VC = ReplaceStr(nodeV->Text, ".", ",");
               nodeV = nodeResponse->FindNode("pVA"); strResp->pVA = ReplaceStr(nodeV->Text, ".", ",");
               nodeV = nodeResponse->FindNode("pVB"); strResp->pVB = ReplaceStr(nodeV->Text, ".", ",");
               nodeV = nodeResponse->FindNode("pVC"); strResp->pVC = ReplaceStr(nodeV->Text, ".", ",");
            }
            catch(Exception &e)
            {
	            strResp->TimeStampEvento = "00000000000000";
               strResp->VA = "0";
               strResp->VB = "0";
               strResp->VC = "0";
               strResp->pVA = "0";
               strResp->pVB = "0";
               strResp->pVC = "0";
					if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema na consulta dos fasores de tensão.";
                  logErros->AddLinha(linhaLog);
               }
            }

            // Verifica tipo de equipamento
            if(tipoEqpto == 6)
            {
               // Seta QUALÍMETRO
               strResp->TipoEqpto = "QUALIM";
            }
            else if(tipoEqpto == 7)
            {
               // Seta TRAFOINTELIGENTE
               strResp->TipoEqpto = "TRAFOINTELIGENTE";
            }
         }
         else if(requestType == 8)
         {
            try
            {
               strResp->TimeStampEvento = TimeStampEvento;
               nodeI = nodeResponse->FindNode("IA"); strResp->IA = ReplaceStr(nodeI->Text, ".", ",");
               nodeI = nodeResponse->FindNode("IB"); strResp->IB = ReplaceStr(nodeI->Text, ".", ",");
               nodeI = nodeResponse->FindNode("IC"); strResp->IC = ReplaceStr(nodeI->Text, ".", ",");
               nodeI = nodeResponse->FindNode("pIA"); strResp->pIA = ReplaceStr(nodeI->Text, ".", ",");
               nodeI = nodeResponse->FindNode("pIB"); strResp->pIB = ReplaceStr(nodeI->Text, ".", ",");
               nodeI = nodeResponse->FindNode("pIC"); strResp->pIC = ReplaceStr(nodeI->Text, ".", ",");
            }
            catch(Exception &e)
            {
               strResp->TimeStampEvento = "00000000000000";
               strResp->IA = "0";
               strResp->IB = "0";
               strResp->IC = "0";
               strResp->pIA = "0";
               strResp->pIB = "0";
               strResp->pIC = "0";
					if(logErros)
               {
                  String linhaLog = "";
                  linhaLog = "Arquivo: " + CodArquivoResponse;
                  linhaLog += ", Request ID: " + responseID;
                  linhaLog += " - Problema na consulta dos fasores de corrente.";
                  logErros->AddLinha(linhaLog);
               }
            }

            // Seta QUALÍMETRO
            strResp->TipoEqpto = "QUALIM";
         }
      }
      // Se houve algum erro na aquisição da informação, anula os campos
      else
      {
      	if(requestType == 1)
         {
            strResp->TimeStampEvento = TimeStampEvento;
            strResp->IA = "0";
            strResp->IB = "0";
            strResp->IC = "0";
         }
         else if(requestType == 2)
         {
            nodeAlarme = nodeResponse->FindNode("TimeStampEvento");
            strResp->TimeStampEvento = nodeAlarme->Text;

            strResp->Funcao50 = strResp->TimeStampEvento;
            strResp->Funcao51 = strResp->TimeStampEvento;
            strResp->Funcao50N = strResp->TimeStampEvento;
            strResp->Funcao51N = strResp->TimeStampEvento;
         }
         else if(requestType == 3)
         {
            strResp->DistFaltaRele = "0";
         }
         else if(requestType == 4)
         {
            strResp->Qualidade = "BAD";
         }
      	else if(requestType == 7)
         {
            strResp->TimeStampEvento = TimeStampEvento;
            strResp->VA = "0";
            strResp->VB = "0";
            strResp->VC = "0";
         }
      	else if(requestType == 8)
         {
            strResp->TimeStampEvento = TimeStampEvento;
            strResp->VA = "0";
            strResp->VB = "0";
            strResp->VC = "0";
            strResp->pVA = "0";
            strResp->pVB = "0";
            strResp->pVC = "0";
         }
      	else if(requestType == 9)
         {
            strResp->TimeStampEvento = TimeStampEvento;
            strResp->IA = "0";
            strResp->IB = "0";
            strResp->IC = "0";
            strResp->pIA = "0";
            strResp->pIB = "0";
            strResp->pIC = "0";
         }
      }

      // Salva a struct de response na lista externa
      lisEXT->Add(strResp);
   }
}
//---------------------------------------------------------------------------
int __fastcall TXMLParser::GetRequestType(TXMLComunicacao* xmlRequest, String responseID)
{
	_di_IXMLNode rootNode = xmlRequest->GetRootNode();
	_di_IXMLNodeList req;
   _di_IXMLNode atributoID;
	_di_IXMLNode atributoReqType;
   IXMLNodeList *listaRequests = rootNode->ChildNodes;
   int requestType;

   requestType = 0;

	// Se encontrar o request com ID = reponseID, pega o RequestType
	for(int i=0; i<listaRequests->Count; i++)
   {
   	req = listaRequests[0].Get(i)->ChildNodes;
      atributoID = req->FindNode("ID");
      if(atributoID->Text != responseID) continue;

      atributoReqType = req->FindNode("RequestType");
      requestType = atributoReqType->Text.ToInt();
      break;
   }

	return requestType;
}
//---------------------------------------------------------------------------
int __fastcall TXMLParser::GetTipoEqpto(TXMLComunicacao* xmlRequest, String responseID)
{
	_di_IXMLNode rootNode = xmlRequest->GetRootNode();
	_di_IXMLNodeList req;
   _di_IXMLNode atributoID;
	_di_IXMLNode atributoTipoEqpto;
   IXMLNodeList *listaRequests = rootNode->ChildNodes;
   int tipoEqpto;

   tipoEqpto = 0;

	// Se encontrar o request com ID = reponseID, pega o RequestType
	for(int i=0; i<listaRequests->Count; i++)
   {
   	req = listaRequests[0].Get(i)->ChildNodes;
      atributoID = req->FindNode("ID");
      if(atributoID->Text != responseID) continue;

      atributoTipoEqpto = req->FindNode("TipoEqpto");
      tipoEqpto = atributoTipoEqpto->Text.ToInt();
      break;
   }

	return tipoEqpto;
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetResponseErro(_di_IXMLNodeList nodeResponse, String &Mensagem)
{
	String Resposta = "False";

   for(int i=0; i<nodeResponse->Count; i++)
   {
		_di_IXMLNode atributo = nodeResponse->Get(i);
      if(atributo->NodeName == "Error")
      {
      	Resposta = atributo->Text;
      }
      else if(atributo->NodeName == "ErrorMessage")
      {
      	Mensagem = atributo->Text;
      }
   }


//   for(int i=0; i<nodeResponse->Count; i++)
//   {
//      _di_IXMLNode atributo = nodeResponse->Get(i);
//
//      // Procura o nó "Resposta"
//      if(atributo->NodeName == "Resposta")
//      {
//         nodeRespostaAquisicao = nodeResponse[0].Get(i)->GetChildNodes();
//
//         // Obtém os campos Erro e Mensagem
//         for(int j=0; j<nodeRespostaAquisicao->Count; j++)
//         {
//         	_di_IXMLNode atributoResposta = nodeRespostaAquisicao->Get(j);
//         	if(atributoResposta->NodeName == "Erro")
//      		{
//					Resposta = atributoResposta->Text;
//            }
//         	else if(atributoResposta->NodeName == "Mensagem")
//      		{
//               Mensagem = atributoResposta->Text;
//            }
//         }
//         break;
//      }
//   }

   return Resposta;
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetResponseID(_di_IXMLNodeList nodeResponse)
{
	String responseID;

   for(int j=0; j<nodeResponse->Count; j++)
   {
      _di_IXMLNode atributo = nodeResponse->Get(j);

      if(atributo->NodeName == "ID")
      {
         responseID = atributo->Text;
         break;
      }
   }

   return responseID;
}
//---------------------------------------------------------------------------
String __fastcall TXMLParser::GetTimeStampEvento(TXMLComunicacao* xmlRequest, String responseID)
{
	_di_IXMLNode rootNode;
	_di_IXMLNodeList req;
   _di_IXMLNode atributoID;
	_di_IXMLNode atributoTimeStampEvento;
   IXMLNodeList *listaRequests;
   String TimeStamp;

   rootNode = xmlRequest->GetRootNode();
	listaRequests = rootNode->ChildNodes;

   TimeStamp = "";

	// Se encontrar o request com ID = reponseID, pega o TimeStamp
	for(int i=0; i<listaRequests->Count; i++)
   {
   	req = listaRequests[0].Get(i)->ChildNodes;
      atributoID = req->FindNode("ID");
      if(atributoID->Text != responseID) continue;

      atributoTimeStampEvento = req->FindNode("TimeStamp");
      TimeStamp = atributoTimeStampEvento->Text;
      break;
   }

	return TimeStamp;
}
//---------------------------------------------------------------------------
void __fastcall TXMLParser::PreencheResp(StrReqResponse* respIni, StrReqResponse* respFin, String TipoEqpto)
{
	if(respIni == NULL || respFin == NULL) return;

   if(respFin->CodEqpto == "") respFin->CodEqpto = respIni->CodEqpto;

   if(respFin->IA == "") respFin->IA = respIni->IA;
   if(respFin->IB == "") respFin->IB = respIni->IB;
   if(respFin->IC == "") respFin->IC = respIni->IC;

   if(respFin->pIA == "") respFin->pIA = respIni->pIA;
   if(respFin->pIB == "") respFin->pIB = respIni->pIB;
   if(respFin->pIC == "") respFin->pIC = respIni->pIC;

   if(respFin->VA == "") respFin->VA = respIni->VA;
   if(respFin->VB == "") respFin->VB = respIni->VB;
   if(respFin->VC == "") respFin->VC = respIni->VC;

   if(respFin->pVA == "") respFin->pVA = respIni->pVA;
   if(respFin->pVB == "") respFin->pVB = respIni->pVB;
   if(respFin->pVC == "") respFin->pVC = respIni->pVC;

   if(respFin->TimeStampEvento == "") respFin->TimeStampEvento = respIni->TimeStampEvento;
   if(respFin->Funcao50 == "") respFin->Funcao50 = respIni->Funcao50;
   if(respFin->Funcao51 == "") respFin->Funcao51 = respIni->Funcao51;
   if(respFin->Funcao50N == "") respFin->Funcao50N = respIni->Funcao50N;
   if(respFin->Funcao51N == "") respFin->Funcao51N = respIni->Funcao51N;
   if(respFin->DistFaltaRele == "") respFin->DistFaltaRele = respIni->DistFaltaRele;
   if(respFin->Qualidade == "") respFin->Qualidade = respIni->Qualidade;
   if(respFin->Resposta_erro == "") respFin->Resposta_erro = "False";
   if(respFin->Resposta_mensagem == "") respFin->Resposta_mensagem = "Ok";

   //debug
   if(TipoEqpto != "")
   {
    	respFin->TipoEqpto = TipoEqpto;
   }

   // Informações sobre a aquisição de dados
   if(respFin->Resposta_erro == "False" && respIni->Resposta_erro == "True")
   {
		respFin->Resposta_erro = "True";
      respFin->Resposta_mensagem += respIni->Resposta_mensagem + ";";
   }
}
//---------------------------------------------------------------------------
void __fastcall TXMLParser::SetLogErros(TLog* logErros)
{
	this->logErros = logErros;
}
//---------------------------------------------------------------------------
