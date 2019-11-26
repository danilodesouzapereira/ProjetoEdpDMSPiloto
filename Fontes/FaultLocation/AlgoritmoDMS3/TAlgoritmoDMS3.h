//---------------------------------------------------------------------------
#ifndef TAlgoritmoDMS3H
#define TAlgoritmoDMS3H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TDados;
class TFuncoesDeRede;
//---------------------------------------------------------------------------
class VTApl;
class VTBarra;
class VTPath;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
struct StrCaminho : public TObject
{
	VTBarra* barraIni;
	VTBarra* barraFim;
	TList* lisLigacoes;
	TList* lisBarras;

	__fastcall StrCaminho()  {lisLigacoes = new TList; lisBarras = new TList;}
	__fastcall ~StrCaminho() {delete lisLigacoes; delete lisBarras;}
};
//---------------------------------------------------------------------------
class TAlgoritmoDMS3 : public TObject
{
public:
	// Dados elementares
	VTApl*   apl;
	VTPath*  path;
	VTRedes* redes;

	// Dados
	StrCaminho*     caminhoAteBarraFinal;
	TDados*         dados;
	TFuncoesDeRede* funcoesRede;
	VTRede*         redeMT;
	VTRede*         redeSE;

	// Métodos
					__fastcall TAlgoritmoDMS3(VTApl* apl, TDados* dados);
					__fastcall ~TAlgoritmoDMS3();

	StrCaminho* __fastcall CaminhoDesdeSubestacao(VTBarra* barraFinal);
	StrCaminho* __fastcall DefineCaminhoAteBarraFinalAreaBusca();
	StrCaminho* __fastcall DeterminaCaminhoComum(TList* lisCaminhos);
	void        __fastcall DeterminaCaminhosAteBarrasMedidoresSemLastGasp(TList* lisBarrasMI_SemLastGasp, TList* lisEXT);
	void        __fastcall DeterminaBarrasMedidoresInteligentesSemLastGasp(TList* lisEXT);
	void        __fastcall EliminaCaminhosComuns(StrCaminho* caminhoAteBarraFinal, TList* lisCaminhosAteMedSemLG);
	void        __fastcall ExecutaLocRompCabo();
	void        __fastcall GetAreaBusca(TList* lisEXT);
	void        __fastcall Inicializa();
	bool        __fastcall TodosCaminhosMesmaBarra(TList* lisCaminhos, int indicePosicao);
};
//---------------------------------------------------------------------------
#endif
