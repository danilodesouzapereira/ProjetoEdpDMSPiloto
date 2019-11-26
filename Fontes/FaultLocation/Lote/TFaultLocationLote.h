//---------------------------------------------------------------------------
#ifndef TFaultLocationLoteH
#define TFaultLocationLoteH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TEvento;
class VTApl;
class VTPath;
//---------------------------------------------------------------------------
class TFaultLocationLote
{
private:
	// Dados elementares
	VTApl* apl;
   VTPath* path;

	// Dados
   TList* lisEventos;
   TList* lisEventosCGP;
   TList* lisEventosOMS;
   TStringList* lisLinhasEventosOMS;
   TStringList* lisLinhasEventosCGP;

public:
   // Construtor e destrutor
   __fastcall TFaultLocationLote(VTApl* apl);
   __fastcall ~TFaultLocationLote();

   // Métodos
   void __fastcall CarregarArquivos();
	void __fastcall CarregarDados();
   void __fastcall ParseEventos();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
