//---------------------------------------------------------------------------
#ifndef TDSSBarraH
#define TDSSBarraH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TDSSBarra : public TObject
{
private:
	// Parâmetros
   String Codigo;
   int ID;
   int Indice_VetorBarras;
   int Fases;
   int NumFases;

public:
	__fastcall TDSSBarra();

   // Métodos
   void __fastcall SetCodigo(String Codigo);
   void __fastcall SetFases(int Fases);
   void __fastcall SetID(int ID);
   void __fastcall SetIndice_VetorBarras(int Indice_VetorBarras);

   String __fastcall GetCodigo();
   int    __fastcall GetFases();
   int    __fastcall GetID();
	int    __fastcall GetIndice_VetorBarras();
	int    __fastcall GetNumFases();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
