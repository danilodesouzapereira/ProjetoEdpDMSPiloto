//---------------------------------------------------------------------------
#ifndef TConversorSinapDSSH
#define TConversorSinapDSSH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TChaveMonitorada;
class TDados;
class TQualimetro;
class VTApl;
class VTCarga;
class VTPath;
class VTRadial;
class VTRede;
class VTRedes;
class VTSuprimento;
class VTTrafo;
class VTTrecho;
//---------------------------------------------------------------------------
struct StrEquivalenteThevenin
{
	// Resistências e reatâncias em pu
	double r0;
	double x0;
	double r1;
	double x1;

	// Tensões (em pu) em ângulos de fase (em graus)
	double Va_pu;
	double Vb_pu;
	double Vc_pu;
	double pVa;
	double pVb;
	double pVc;

	VTBarra* barra;
};
//---------------------------------------------------------------------------
class TConversorSinapDSS : public TObject
{
public:
	// Parâmetros elementares
	String   pathSaida;
	TDados*  dados;          //< referência ao obj "TDados", para obter as informações do eqptoRef
	VTApl*   apl;
	VTPath*  path;
	VTRedes* redes;

	// Dados
	TList*       lisArranjos;
	TList*       lisTrafosSEsFilhas;
	TList*       lisRedesFilhas;
	VTRadial*    radial;
	VTRede*      redeMT;
	VTRede*      redeSE;

	// Dados
	String            posicaoSuprimento;  //< "AT" (alta tensão) ou "MT" (média tensão)
	TQualimetro*      qualimetroEqptoRef; //< Qualímetro de referência, que contém os fasores V, I durante a falta
	TChaveMonitorada* chaveMonitoradaEqptoRef; //< Chave monitorada de referência, que contém os fasores V, I durante a falta
	TFuncoesDeRede*   funcoesRede;
	TList*            lisLigacoesConsideradas;  //< lista com as ligações consideradas, à jusante do eqpto de referência
	TList*            lisBarrasConsideradas;    //< lista com as barras consideradas, à jusante do eqpto de referência

	// Construtor
	__fastcall TConversorSinapDSS(VTApl* apl);
	__fastcall ~TConversorSinapDSS(void);

   // Métodos principais
	void      __fastcall AjustaArranjos();
	void      __fastcall DeterminaAreaJusanteEqptoRef(VTLigacao* ligacaoReferencia);
	void      __fastcall Executa(bool locRompCabo = false);
   void      __fastcall Executa_FLOffline(bool locRompCabo = false);
	VTTrecho* __fastcall FindTrechoPai(VTCarga* carga);
	VTRede*   __fastcall GetRedeMT();
	VTRede*   __fastcall GetRedeSE();
	void      __fastcall Inicializa(String pathSaida, VTRede* redeMT, VTRede* redeSE);

	// Exportação dos arquivos DSS
	void __fastcall ExportaArranjos(bool InsereArranjosAuxiliares = false);
	void __fastcall ExportaCargas();
	void __fastcall ExportaLigacoes();
	void __fastcall ExportaMaster();
   void __fastcall ExportaMaster_FLOffline();
	void __fastcall ExportaMaster_SuprimentoMT();       //< master.dss com o suprimento na MT (à jusante do trafo SE)
	void __fastcall ExportaMaster_SuprimentoAT();       //< master.dss com o suprimento na AT (à montante do trafo SE)
	void __fastcall ExportaMaster_SuprimentoEqptoRef(); //< master.dss com o suprimento na posição do eqptoRef (med. V,I)
   void __fastcall ExportaMaster_SuprimentoEqptoRef_FLOffline();
	void __fastcall ExportaTrafos();
	void __fastcall ExportaDadosPotCurto(VTSuprimento* sup, VTTrafo* trafoSE);
	void __fastcall ExportaDadosPotCurto_suprimentoMT(StrEquivalenteThevenin* strEqThv, VTTrafo* trafoSE);

	// Métodos auxiliares
	void __fastcall FiltraBarrasAreaConsiderada(TList* lisBarrasTodas);
	void __fastcall FiltraCargasAreaConsiderada(TList* lisCargasTodas);
	void __fastcall FiltraLigacoesAreaConsiderada(TList* lisLigacoesTodas);

	// Pesquisa e edição de linhas
	void   __fastcall EquivalenteTheveninMT(VTRede* redeSE, VTRede* redeMT, StrEquivalenteThevenin* strEqThv);
	void   __fastcall EquivalenteTheveninEqptoRef(double baseKV, StrEquivalenteThevenin* strEqThv);
   void   __fastcall EquivalenteTheveninEqptoRef_FLOffline(double baseKV, StrEquivalenteThevenin* strEqThv);
	String __fastcall GetConnCarga(int enumFases);
	String __fastcall GetIDbus(String linha, int idBus);
	String __fastcall GetIndiceFases(int enumFases);
	String __fastcall GetIndiceFases(String linhaTrecho);
	int    __fastcall GetNumFases(int enumFases);
	int    __fastcall GetNumPhasesCarga(int enumFases);
	void   __fastcall InsereIndicesFases(TStringList* linhasDSSChaves, int indiceLinha, String indiceFases);
	void   __fastcall InsereInfoFasesChaves(TStringList* linhasDSStrechos, TStringList* linhasDSSChaves);
	int    __fastcall InsereInfoFasesTrecho(String indiceFases);
	void   __fastcall InsereOrdenado(TStringList* lista, String valor);
	String __fastcall ProcuraFasesTrecho(TStringList* linhasDSStrechos, String idBus1, String idBus2);
	void   __fastcall RedesFilhas(VTRede* redeMT, TList* lisTrafosSEsFilhas, TList* lisEXT);
	void   __fastcall TrafosSEsFilhas(TList* lisEXT);
	String __fastcall VoltageBases(VTTrafo* trafoSE, TList* lisTrafosSEsFilhas);
	String __fastcall VoltageBases_SuprimentoMT(VTTrafo* trafoSE, TList* lisTrafosSEsFilhas);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
