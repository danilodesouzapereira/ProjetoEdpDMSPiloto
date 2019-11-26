//---------------------------------------------------------------------------
#ifndef TOpcoesGraficasH
#define TOpcoesGraficasH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class VTApl;
class VTGrafico;
//---------------------------------------------------------------------------
class TOpcoesGraficas
{
private:
	// Par�metros elementares
   VTApl* apl;
	VTGrafico* graf;

public:
	// Contrutor e destrutor
   __fastcall TOpcoesGraficas(VTApl* apl);
	__fastcall ~TOpcoesGraficas();


   // M�todos
   void __fastcall InsereMoldurasBarras(TList* lisBarras);
	void __fastcall InsereMoldurasBarrasFaultStudy(TList* lisSolucoes);
};
// ---------------------------------------------------------------------------
#endif
// ---------------------------------------------------------------------------
//eof
