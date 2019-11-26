//---------------------------------------------------------------------------
#ifndef TXMLParserH
#define TXMLParserH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <XMLDoc.hpp>
//---------------------------------------------------------------------------
class TAlarme;
class TLog;
class TXMLComunicacao;
class VTApl;
class VTPath;
//---------------------------------------------------------------------------
struct StrCadSensor
{
	String CodSensor;
   String CodChave;
   String Alimentador;
};
//---------------------------------------------------------------------------
struct StrReqResponse
{
	String RequestID;
   String CodEqpto;
   String TipoEqpto;  //< "DJ", "RE", "SR"
	// Dados de corrente
   String IA;
   String IB;
	String IC;
   String pIA;
   String pIB;
	String pIC;
	// Dados de tensão
   String VA;
   String VB;
	String VC;
   String pVA;
   String pVB;
	String pVC;
	// Dados de funções de atuantes
   String TimeStampEvento;
   String Funcao50;
   String Funcao51;
   String Funcao50N;
   String Funcao51N;
   String TipoAtuacao;  //< "FASE", "NEUTRO"
	// Dados de distância de falta calculada pelo relé
   String DistFaltaRele;
   // Dados de disponibilidade/qualidade dos dados do sensor
   String Qualidade;

   // Resposta da aquisição da informação
   String Resposta_erro;
   String Resposta_mensagem;
};
//---------------------------------------------------------------------------
struct StrAlarme
{
   String TimeStamp;
   String CodAlimentador;
   String CodEqpto;
   int TipoEqpto;
   int TipoAlarme;
};
//---------------------------------------------------------------------------
class TXMLParser
{
private:
	// Dados elementares
   TForm*  formFL;
   VTApl*  apl;
   VTPath* path;

   // Dados
   String dirAlarmes;
   String dirMsgEntrada;
   TLog*  logErros;

public:
	// Construtor e destrutor
   __fastcall TXMLParser(VTApl* apl, TForm* formFL);
	__fastcall ~TXMLParser();

   // Métodos
	TAlarme* __fastcall GetAlarme(String CodArquivoAlarme);
//	void     __fastcall GetAtualizacoesAlarmes(String pathArqAlarme, TList* lisEXT);
	void     __fastcall GetCadSensores(String CodArquivoCadSensores, TList* lisEXT);
	void     __fastcall GetReqResponse(String CodArquivoResponse, TXMLComunicacao* xmlRequest, TList* lisEXT);
	void     __fastcall SetLogErros(TLog* logErros);

private:
	// Métodos internos
	String __fastcall GetCodEqpto(TXMLComunicacao* xmlRequest, String responseID);
	String __fastcall GetResponseErro(_di_IXMLNodeList nodeResponse, String &Mensagem);
	String __fastcall GetResponseID(_di_IXMLNodeList nodeResponse);
	int    __fastcall GetRequestType(TXMLComunicacao* xmlRequest, String responseID);
	int    __fastcall GetTipoEqpto(TXMLComunicacao* xmlRequest, String responseID);
   String __fastcall GetTimeStampEvento(TXMLComunicacao* xmlRequest, String responseID);

	void            __fastcall GetReqResponse_LeXML(String CodArquivoResponse, TXMLComunicacao* xmlRequest, TList* lisEXT);
	StrReqResponse* __fastcall FindRespGeral(TList* lisEXT, String CodEqpto, String TipoEqpto);
	void            __fastcall PreencheResp(StrReqResponse* respIni, StrReqResponse* respFin, String TipoEqpto);
	void            __fastcall DefineEqpto(TList* lisEXT);
	String          __fastcall GetCodChaveQualimetro(String CodQualimetro);
	String          __fastcall GetCodChaveSensor(String CodSensor);
};
//---------------------------------------------------------------------------
#endif

