//---------------------------------------------------------------------------
#ifndef TConfiguracoesH
#define TConfiguracoesH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
// Defini��o de tipos
enum tipoCruzamento {mediaLocal = 0, mediaGlobal};
enum tipoSelecao {melhoresIndiv = 0, rodaFortuna};
//---------------------------------------------------------------------------
class TConfiguracoes : public TObject
{
private:
	// Par�metros elementares
   String pathConfigGerais;    //< Caminho do arquivo INI com as configura��es gerais

	// Par�metros de simula��o
	int NumMaxGeracoes;         //< N�mero m�ximo de gera��es
	int MaxIdade;               //< M�xima idade de um indiv�duo
	int MaxIndivPorGeracao;     //< N�mero m�ximo de indiv�duos numa gera��o
	int NumFilhosMutacao;       //< N�mero m�ximo de filhos gerados por muta��o
   int NumIndivIni;            //< N�mero inicial de indiv�duos
	int TipoCruzamento;         //< 0: m�dia local, 1: m�dia global
	int TipoSelecao;            //< 0: melhores indiv�duos, 1: roda da fortuna
	double Pm;                  //< Probabilidade de muta��o
	double Pc;                  //< Probabilidade de cruzamento
   double MaxRfalta;           //< M�xima resist�ncia de falta considerada
   double MinDeltaFAval;       //< M�nimo delta da fun��o de avalia��o para finalizar as gera��es do AE
   double sigmaXini;           //< desv. padr�o inicial do par�metro X
   double sigmaRfini;          //< desv. padr�o inicial do par�metro Rf

	bool   MostrarLogDebug;     //< Flag para habilitar ou desabilitar a impress�o de arquivos de log do AE
	bool   ConsiderarQualidadeSensor;

   double PesoV;               //< Peso das compara��es de tens�o, durante a avalia��o dos indiv�duos
   double PesoI;               //< Peso das compara��es de corrente, durante a avalia��o dos indiv�duos

public:
	// Construtores
	__fastcall TConfiguracoes();
	__fastcall TConfiguracoes(String pathDirDat);

	// M�todos para obter as configura��es de EE
	void __fastcall GetConfigEE();
	void __fastcall GetConfigGerais();

	// M�todos de consulta
	bool   __fastcall GetConsiderarQualidadeSensor();
	int    __fastcall GetNumMaxGeracoes();
	int    __fastcall GetMaxIdade();
	int    __fastcall GetMaxIndivPorGeracao();
	double __fastcall GetMaxRfalta();
   bool   __fastcall GetMostrarLogDebug();
   double __fastcall GetMinDeltaFAval();
	int    __fastcall GetNumFilhosMutacao();
	int    __fastcall GetNumIndivIni();
	double __fastcall GetSigmaXini();
	double __fastcall GetSigmaRfini();
	int    __fastcall GetTipoCruzamento();
	int    __fastcall GetTipoSelecao();
	double __fastcall GetPm();
	double __fastcall GetPc();
	double __fastcall GetPesoV();
	double __fastcall GetPesoI();

	// M�todos de inser��o
	void __fastcall   SetNumMaxGeracoes(int NumMaxGeracoes);
	void __fastcall   SetMaxIdade(int MaxIdade);
	void __fastcall   SetMaxIndivPorGeracao(int MaxIndivPorGeracao);
	void __fastcall   SetMaxRfalta(double MaxRfalta);
	void __fastcall   SetNumFilhosMutacao(int NumFilhosMutacao);
	void __fastcall   SetTipoCruzamento(int TipoCruzamento);
	void __fastcall   SetTipoSelecao(int TipoSelecao);
	void __fastcall   SetPm(double Pm);
	void __fastcall   SetPc(double Pc);
	void __fastcall   SetPesoV(double PesoV);
	void __fastcall   SetPesoI(double PesoI);
};
//---------------------------------------------------------------------------
#endif
