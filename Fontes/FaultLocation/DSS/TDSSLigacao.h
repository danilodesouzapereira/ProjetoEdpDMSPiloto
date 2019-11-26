//---------------------------------------------------------------------------
#ifndef TDSSLigacaoH
#define TDSSLigacaoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TDSSLigacao : public TObject
{
private:
	int    Fases;
	int    Indice_VetorBarras;
   int    NumFases;
   String Codigo;

public:
	__fastcall TDSSLigacao();

   // Métodos
	void   __fastcall AddFase(int iFase);
   String __fastcall GetCodigo();
	int    __fastcall GetIndice_VetorLigacoes();
   int    __fastcall GetFases();

   void   __fastcall SetCodigo(String Codigo);
	void   __fastcall SetFases(String strFasesBus1, String strFasesBus2);
	void   __fastcall SetIndice_VetorLigacoes(int Indice_VetorLigacoes);
};
//---------------------------------------------------------------------------
#endif
