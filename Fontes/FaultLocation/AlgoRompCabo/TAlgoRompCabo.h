//---------------------------------------------------------------------------
#ifndef TAlgoRompCaboH
#define TAlgoRompCaboH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <complex>
//---------------------------------------------------------------------------
class TAreaBusca;
class TClassificacao;
class TDados;
class TFuncoesDeRede;
class TLog;
class TTryRompCabo;
class VTApl;
class VTBarra;
class VTPath;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
struct strBarraSolucaoRompCabo
{
	VTBarra* barra;
	double indiceErro;
	String faseAfetada;
};
//---------------------------------------------------------------------------
class TAlgoRompCabo
{
public:
	// Dados elementares
	TFuncoesDeRede* funcoesRede;
	TAreaBusca*     areaBusca;
	TClassificacao* classificacao;
	TDados*         dados;    //< Conjunto dos dados acerca do defeito
	VTApl*          apl;
	VTPath*         path;
	VTRedes*        redes;

	// Dados
	TLog*           logRompCabo;   //< Log para externalizar observações, resultados para debug e erros
	TTryRompCabo*   tryRompCabo;
	TList*          lisBarrasCandidatas;

	// Construtor e destrutor
	__fastcall TAlgoRompCabo(VTApl* apl);
	__fastcall ~TAlgoRompCabo();


	// Métodos
	void __fastcall ExecutaLocRompCabo();
	void __fastcall GetSolucoes(TFuncoesDeRede* funcoesRede, TList* lisEXT);
	void __fastcall IniciaTryRompCabo(String caminhoDSS);
	void __fastcall OrdenaBarrasSolucao();
	void __fastcall SetLogRompCabo(TLog* logRompCabo);
	void __fastcall SetParametros(TAreaBusca* areaBusca, TClassificacao* classificacao, TDados* dados);
};
//---------------------------------------------------------------------------
#endif
