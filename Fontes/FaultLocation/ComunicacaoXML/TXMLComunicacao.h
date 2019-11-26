//---------------------------------------------------------------------------
#ifndef TXMLComunicacaoH
#define TXMLComunicacaoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <XMLDoc.hpp>
//---------------------------------------------------------------------------
enum tipoXML
{
	XMLindef = -1,
   XMLcadSensores,
   XMLrequest,
   XMLsolution,
   XMLFaultLocationOffline,
   XMLareaBuscaSH,
   XMLsolutionSH,
   XML_SelfHealing,
	XMLrespCadSensoresFake,
	XMLsolutionDMS2,
	XMLsolutionDMS3
};
//---------------------------------------------------------------------------
struct strSolicitacao    // struct para controle das solicitações
{
   String requestType;
   String CodEqpto;
};
//---------------------------------------------------------------------------
struct strSolucao;      // Definido em TFaultStudyFL.h
struct StrDadosGerais;  // Definido em TThreadFaultLocation.h
struct StrDadosLog;     // Definido em TThreadFaultLocation.h
//---------------------------------------------------------------------------
class TAlarme;
class TLog;
//---------------------------------------------------------------------------
class TXMLComunicacao : public TObject
{
private:
	// Dados
   _di_IXMLDocument xmlFile;
	_di_IXMLNode rootNode;
   int contador;
	String pathFinal;
   int contItens;
   TLog* logErros;

   // Para controle
   TList* lisStrSolicitacoes;

public:
	// Construtor e destrutor
   __fastcall TXMLComunicacao(String pathFinal, int tipoXML);
	__fastcall ~TXMLComunicacao();

   // Métodos
   void __fastcall AddRequest(int requestType, String codEqpto, TAlarme* alarme);
   void __fastcall AddAlimentador(String CodAlimentador);

	void __fastcall AddChavesBlocosAreaBusca(TStringList* lisCodChaves);
   void __fastcall SH_InsereChavesBlocosAreaBusca(TStringList* lisCodChaves);

	void __fastcall AddChavesBlocosSolucoes(TStringList* lisCodChaves);
   void __fastcall SH_InsereChavesBlocosSolucoes(TStringList* lisCodChaves);

   void __fastcall AddDadosGerais(StrDadosGerais* strGer);
   void __fastcall AddDadosLog(StrDadosLog* strLog);
	void __fastcall AddRequestQualimetro(int requestType, String codEqpto, String timeStamp);
   void __fastcall AddRequestTrafoInteligente(int requestType, String codEqpto, String timeStamp);
   void __fastcall AddSolution(strSolucao* strSol);
   void __fastcall AddSolutions(TList* lisSolucoes);
	void __fastcall InsereChavesBlocosSolucoes(TStringList* lisCodChaves);
	void __fastcall InsereLigacoesSolucoes(TStringList* lisCodLigacoes);
	int  __fastcall  GetContItens();
	String __fastcall ProcessResponse(int solutionsAmount);
	void __fastcall Salvar();
	void __fastcall SetLogErros(TLog* logErros);
	String __fastcall StringTipoEqpto(int tipoEqpto);

   // Métodos auxiliares
   _di_IXMLNode __fastcall GetRootNode();


private:
	// Métodos chamados internamente
	bool   __fastcall ExisteSolicitacao(String codEqpto, String requestType);
   String __fastcall TimeStampAtual();
   String __fastcall TraduzTipoAlarme(int tipoAlarme);
};
//---------------------------------------------------------------------------
#endif
