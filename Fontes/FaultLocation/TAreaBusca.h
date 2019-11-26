//---------------------------------------------------------------------------
#ifndef TAreaBuscaH
#define TAreaBuscaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TBlocosRedeRadial;
class TCluster;
class TConfiguracoes;
class TDados;
class TEqptoCampo;
class TFuncoesDeRede;
class VTApl;
class VTBarra;
class VTBloco;
class VTCarga;
class VTChave;
class VTLigacao;
class VTPath;
class VTRede;
class VTRedes;
class VTTrecho;
//---------------------------------------------------------------------------
class TTrechoDiscret
{
public:
	// Métodos
	__fastcall TTrechoDiscret(VTTrecho* trecho);
	void __fastcall SetXiniXfin(double xIni, double xFin);

   // Dados
   VTTrecho* trecho;
   double xIni;
   double xFin;
};
//---------------------------------------------------------------------------
class TAreaBusca : public TObject
{
public:
	// Dados elementares
	TConfiguracoes* configuracoes;
   TDados* dados;
   TFuncoesDeRede* funcoesRede;
   VTApl* apl;
   VTPath* path;
   VTRede* redeMT;
   VTRede* redeSE;
   VTRedes* redes;

	// Dados
	bool ConsiderarCoordenacaoProtecao;
	TList* lisBlocosPesquisa;        //< para os blocos que serão efetivamente pesquisados
	TList* lisClusteres;             //< lista de clusteres para FL com EE
	TList* listaEqptosCampo;         //< para os eqptos de campo (de medições)
//   TList* listaCadastroEqptosCampo; //< lista com todos os eqptos de campo
	TList* lisTrechosDiscret;        //< para os trechos discretizados da rede
	TEqptoCampo* eqptoCampo_DeterminaAreaBusca;
	TList* lisBlocosRedeRadial; //< lista para os blocos de rede radial
	VTChave* chaveRef_FLOffline;

	// Para Área de Busca do problema de rompimento de cabo
	VTBarra* barraInicioAreaBusca;
	TList* lisLigacoesAreaBusca_RompCabo;
	VTLigacao* ligacaoFinalAreaBusca_RompCabo;

   // Métodos
//	void __fastcall GeraCadastroEqptosCampo();
	void __fastcall RefinamentoBlocosJusante_Sensores();
	void __fastcall RemoveSubconjunto(TList* listaMaior, TList* listaMenor);
	bool __fastcall Subconjunto(TList* listaMaior, TList* listaMenor);

	// Construtor e destrutor
	__fastcall TAreaBusca(VTApl* apl, TDados* dados, TFuncoesDeRede* funcoesRede);
	__fastcall ~TAreaBusca(void);


	// Métodos
	void __fastcall AjustaAreaBusca_RompCabo();
	void __fastcall AtualizaBlocosPesquisa_LastGaspAposFusivel();
	void __fastcall AtualizaBlocosPesquisa_LastGaspAntesFusivel();
	void __fastcall DefineAlarmeQualimetroTrafoMontante();
   void __fastcall DefineClusteres();
	void __fastcall DefineBlocosMaisJusante();
	void __fastcall DeterminaInicioAreaBusca();
	void __fastcall DeterminaLigacaoFinal_RompCabo();
   void __fastcall DeterminaLigacoesAreaBuscaRompCabo();
	void __fastcall DiscretizaAreaBusca(TList* lisBlocos = NULL);
	void __fastcall DiscretizaAreaBusca_EE(VTChave* chvMontante, TList* lisBlocos = NULL);
	void __fastcall Executa();
	void __fastcall Executa_DMS2();
	void __fastcall Executa_RompCabo();
	void __fastcall ExecutaClusteres();
	bool __fastcall ExisteAlarme(VTChave* chave);
   bool __fastcall ExisteAlarme(VTCarga* carga);
	bool __fastcall ExisteEqptoJusanteSemAlarme(VTBarra* barraRef);
	void __fastcall FiltrarBlocos_Religadoras();
   void __fastcall FiltrarBlocos_Fusiveis();
   void __fastcall GetAreaBusca_Blocos(TList* listaExt);
	void __fastcall GetAreaBusca_Barras(TList* listaExt);
   void __fastcall GetAreaBuscaRompCabo_Trechos(TList* listaExt);
	void __fastcall GetAreaBusca_CodChavesBlocos(TStringList* listaExt);
	void __fastcall GetAreaBusca_DMS2(TList* listaExt);
	VTBarra* __fastcall GetBarra_ChaveTrafoMontante(VTBarra* barraRef);
	VTLigacao* __fastcall GetLigaMontante(TEqptoCampo* eqptoCampo);
	void __fastcall Inicializa();
	void __fastcall IniciaConfigs();

	// Auxiliares
	void     __fastcall AjustaListaChavesJusante(TList* lisChaves, TList* lisChavesQualimetros);
	void     __fastcall AjustaListaCargasJusante(TList* lisCargas, TList* lisCargasTrafosIntel);
	bool     __fastcall ExisteEqptoCampo(int Tipo, String Codigo, TList* lista);
	VTBarra* __fastcall GetBarra(int Id);
	VTBarra* __fastcall GetBarraMontante();
	void     __fastcall GetCargasAssociadasTrafosIntel(TList* lisEXT);
	void     __fastcall GetChavesAssociadasQualimetros(TList* lisEXT);
	void     __fastcall GetCargasAssociadasQualimetros(TList* lisEXT);
	TBlocosRedeRadial* __fastcall GetBlocosRedeRadial(VTRede* rede);
	TList*   __fastcall GetLisClusteres();
	VTRede*  __fastcall GetRedeMT();
	VTRede*  __fastcall GetRedeSE();
	bool     __fastcall LigacaoComum(TList* caminhos, VTLigacao* ligacao);
	TList*   __fastcall MaiorCaminho(TList* caminhos);
	void     __fastcall MantemBlocosJusanteFusiveis(TList* lisBlocosPesquisa, TList* lisChaves);
	VTRede*  __fastcall RedeBarra(VTBarra* barra);
	void     __fastcall RemoveBlocosJusanteFusiveis(TList* lisBlocosPesquisa, TList* lisChaves);
	void     __fastcall SetConfiguracoes(TConfiguracoes* configuracoes);

   // Conversões entre trechos e suas discretizações
   String  __fastcall GetTrechoDiscret(double xPorc);  //< Retorna o código do trecho que contém um valor de x (%)
	String  __fastcall GetTrechoLocalDiscret(double xPorc); //< Retorna CSV: códTrecho;xLocal(%);
	String  __fastcall GetTrechoBarra1Barra2LocalDiscret(double xPorc); //< Retorna CSV: códTrecho;IDbarra1;IDbarra2;xLocal(%);
   VTRede* __fastcall RedeCarga(VTCarga* carga);
};
//---------------------------------------------------------------------------
#endif
