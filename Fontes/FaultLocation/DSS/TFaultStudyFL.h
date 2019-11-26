//---------------------------------------------------------------------------
#ifndef TFaultStudyFLH
#define TFaultStudyFLH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TAreaBusca;
class TClassificacao;
class TConfiguracoes;
class TDados;
class TDSS;
class TFuncoesDeRede;
class VTBarra;
//---------------------------------------------------------------------------
struct StrDadosLog;
//---------------------------------------------------------------------------
struct resCurto
{
   int IDbarra;
   VTBarra* barraCurto;

   double Icc3f;
   double Icc1f;
   double Icc2f;
	double desvio;  //< Desvio (em %) em relação à corrente medida
	String detalheTipoFalta;    //< campo para detalhar o tipo de falta (AB ou ABC, para os casos ABABC)
};
//---------------------------------------------------------------------------
class TFaultStudyFL
{
private:
   // Dados
	double MaxDesvioIporc;    //< máximo desvio porcentual admissível, em relação à medição de corrente
	double Ifalta;            //< corrente de falta medida e que norteia a LF por Fault Study
   int    NumMaxSolucoes;    //< número máximo de soluções a serem fornecidas
	bool   FiltrarSolucoesPorBloco;
	TConfiguracoes* configGerais;

	String CaminhoDSS;
   String CodigoAlimentador;
	String strTipoFalta;
   TAreaBusca* areaBusca;  //< Referência para o obj de área de busca
	TDSS*  DSS;
   TStringList* listaLinhasFaultStudy; //< linhas do arquivo de fault study (sem espaços e com ";")
   TList* lisResCurto;
   TList* lisBarrasAreaBusca;          //< lista com VTBarras da área de busca
   TList* listaBarrasSolucao;            // Lista com as barras (VTBarra) que são soluções

public:
	// Construtor e destrutor
   __fastcall TFaultStudyFL(TAreaBusca* areaBusca);
   __fastcall ~TFaultStudyFL();

   // Métodos principais
	void __fastcall ExecutaFaultStudy();
   void __fastcall ExecutaLocalizFaultStudy(TClassificacao* classificacao, TDados* dados);
   void __fastcall Inicializa(String CaminhoDSS, String CaminhoDirFaultLocation, String CodigoAlimentador);
	void __fastcall LocalizaDefeito(int numFases, double Icc);
	void __fastcall LocalizaDefeitoAB_ABC(double Icc);
	void __fastcall LocalizaDefeito(TClassificacao* classificacao, TDados* dados);

   // Métodos
	String       __fastcall AjustaLinha(String linha);
	void         __fastcall ConsideraAreaBusca();
	VTBarra*     __fastcall GetBarra(int ID);
	double       __fastcall GetCorrenteFaltaMedida();
	double       __fastcall GetDesvioResultado(resCurto* res, double Icc, int numFases);
	void         __fastcall GetDesvioResultadoAB_ABC(resCurto* res, double Icc, double &ext_desvio, String &detalheTipoFalta);
   double       __fastcall GetMaxDesvioIporc();
	resCurto*    __fastcall GetResCurto(String linhaAjustada);
	void         __fastcall GetSolucoes(TFuncoesDeRede* funcoesRede, StrDadosLog* strLog, TList* lisEXT);
	void         __fastcall OrdenaResultados();
	bool         __fastcall ProblemaLinha(String linha);
	void         __fastcall SetAreaBusca(TList* lisBarrasAreaBusca);
	void         __fastcall SetCodigoAlimentador(String CodigoAlimentador);
	void         __fastcall SetConfiguracoes(TConfiguracoes* configGerais);


private:
	void __fastcall FiltraSolucoesBloco(TFuncoesDeRede* funcoesRede, TList* lisResCurto);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
