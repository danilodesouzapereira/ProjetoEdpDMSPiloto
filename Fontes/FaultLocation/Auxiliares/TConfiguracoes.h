//---------------------------------------------------------------------------
#ifndef TConfiguracoesH
#define TConfiguracoesH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
// Definição de tipos
enum tipoCruzamento {mediaLocal = 0, mediaGlobal};
enum tipoSelecao {melhoresIndiv = 0, rodaFortuna};
//---------------------------------------------------------------------------
class TConfiguracoes : public TObject
{
private:
	// Parâmetros elementares
   String pathConfigGerais;    //< Caminho do arquivo INI com as configurações gerais

	// Parâmetros de simulação
	int NumMaxGeracoes;         //< Número máximo de gerações
	int MaxIdade;               //< Máxima idade de um indivíduo
	int MaxIndivPorGeracao;     //< Número máximo de indivíduos numa geração
	int NumFilhosMutacao;       //< Número máximo de filhos gerados por mutação
   int NumIndivIni;            //< Número inicial de indivíduos
	int TipoCruzamento;         //< 0: média local, 1: média global
	int TipoSelecao;            //< 0: melhores indivíduos, 1: roda da fortuna
	double Pm;                  //< Probabilidade de mutação
	double Pc;                  //< Probabilidade de cruzamento
   double MaxRfalta;           //< Máxima resistência de falta considerada
   double MinDeltaFAval;       //< Mínimo delta da função de avaliação para finalizar as gerações do AE
   double sigmaXini;           //< desv. padrão inicial do parâmetro X
   double sigmaRfini;          //< desv. padrão inicial do parâmetro Rf

	bool   MostrarLogDebug;     //< Flag para habilitar ou desabilitar a impressão de arquivos de log do AE
	bool   ConsiderarQualidadeSensor;

   double PesoV;               //< Peso das comparações de tensão, durante a avaliação dos indivíduos
   double PesoI;               //< Peso das comparações de corrente, durante a avaliação dos indivíduos

public:
	// Construtores
	__fastcall TConfiguracoes();
	__fastcall TConfiguracoes(String pathDirDat);

	// Métodos para obter as configurações de EE
	void __fastcall GetConfigEE();
	void __fastcall GetConfigGerais();

	// Métodos de consulta
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

	// Métodos de inserção
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
