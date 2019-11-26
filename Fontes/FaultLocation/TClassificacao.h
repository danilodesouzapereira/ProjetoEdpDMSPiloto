//---------------------------------------------------------------------------
#ifndef TClassificacaoH
#define TClassificacaoH
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TChaveMonitorada;
class TConfiguracoes;
class TDados;
class TEqptoCampo;
class TQualimetro;
class TSensor;
//---------------------------------------------------------------------------
class TClassificacao : public TObject
{
public:
	// Dados
	TDados* dados;
	TConfiguracoes* configGerais;

	struct
	{
		double maxPorc_3f;
		double minPorc_1f;
		double minRelI0I1_2ft;
	}ParametrosComparacao;

	struct
	{
		double minPorc_1f;
	}ParametrosComparacaoRompCabo;

	// Dados para classificação de localização de curto-circuito
	std::complex<double> I[3];       //< Valores complexos das correntes das fases
	std::complex<double> V_afund[3]; //< Valores complexos das tensões de afundamento
	int TipoFalta;                   //< Tipo de falta (enum), com base nas correntes medidas
	int FonteClassificacao;          //< Enum para a fonte da classificação da falta
	int NumFasesFalta;               //< Número de fases da falta (1, 2 ou 3)
	int NumFasesRompCabo;            //< Número de fases do rompimento de cabo (1, 2 ou 3)
   bool CorrentesOpostasFasesSas;   //< As fases sãs têm correntes opostas, indicando rompimento de cabo

	// Dados para classificação de rompimento de cabo
	int TipoRompCabo;               //< Tipo de rompimento de cabo, indicando as fases afetadas

   double ToleranciaFasesSasOk;    //< Ângulo limite para verificar se as fases sãs são opostas.

	// Construtor e destrutor
   __fastcall TClassificacao(TDados* dados, double* vetorParamComparacao);
	__fastcall ~TClassificacao(void);

   // Métodos
   void         __fastcall ComparaCorrentesFase(std::complex<double>* I, TChaveMonitorada* eqptoProtecao);
	TEqptoCampo* __fastcall DeterminaMonitorProximo();
	TChaveMonitorada* __fastcall DeterminaProtecaoProxima();
	TChaveMonitorada* __fastcall DeterminaProtecaoProxima_FLOffline();
//	TQualimetro* __fastcall DeterminaQualimetroSE();
	TQualimetro* __fastcall QualimetroEqptoRef();
	TQualimetro* __fastcall DeterminaQualimetroProximo_RompCabo();
	TSensor*     __fastcall DeterminaSensorProximo();
	void         __fastcall Executa();
	void         __fastcall ExecutaRompCabo();
	void         __fastcall Executa_FLOffline();
	void         __fastcall GetCorrentes(TEqptoCampo* eqptoCampoProximo, std::complex<double>* I);
	int          __fastcall GetNumFasesFalta();
	String       __fastcall GetStrTipoFalta();
	String       __fastcall GetStrTipoRompCabo();
   void         __fastcall InicializaConfiguracoes(String CaminhoDirFaultLocation);
	void         __fastcall SetConfigGerais(TConfiguracoes* configGerais);
	void         __fastcall SetParamComparacao(double* vetorParamComparacao);

	void         __fastcall ClassificaRompCaboComQualimetro(TQualimetro* qualimetro);
	void         __fastcall ClassificaComEqptoProt(TChaveMonitorada* chvProt);
	void         __fastcall ClassificaComEqptoProtecao(TChaveMonitorada* chvProt);
   void         __fastcall ClassificaComEqptoProt_FLOffline(TChaveMonitorada* chvDJ_RE);
	void         __fastcall ClassificaComQualimetroSE(TQualimetro* qualimetroSE, TChaveMonitorada* eqptoProtecaoProx);
	void         __fastcall ClassificaComQualimetroEqptoRef(TQualimetro* qualimetroEqptoRef, TChaveMonitorada* eqptoProtecaoProx);
	void         __fastcall ClassificaComSensor(TSensor* sensor, TChaveMonitorada* eqptoProtecao);
   void         __fastcall GetSensoresDefeito(TList* lisSensoresDef);
   void         __fastcall VerificaCorrentesFasesSasOpostas(TChaveMonitorada* chaveMonitorada);
   void         __fastcall VerificaCorrentesFasesSasOpostas(TQualimetro* qualimetroEqptoRef);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
