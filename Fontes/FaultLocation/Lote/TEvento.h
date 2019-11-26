//---------------------------------------------------------------------------
#ifndef TEventoH
#define TEventoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TEvento
{
private:
   // Métodos
   void __fastcall InicializaEvento();

	// Dados
	String DtInicio;
   String DtAcionamento;
   String DtChegada;
   String DtConclusao;
   String PDF;
   String CodAlimentador;
   int CausaID;
   String RefDef;
   int TipoCC;
   double LocSEL;
   double LocOMS;
   double LocFLIISR;
   double Icc;



public:
	// Construtor e destrutor
   __fastcall TEvento();
   __fastcall ~TEvento();

   // Métodos Set
   void __fastcall SetDtInicio(String DtInicio);
   void __fastcall SetDtAcionamento(String DtAcionamento);
   void __fastcall SetDtChegada(String DtChegada);
   void __fastcall SetDtConclusao(String DtConclusao);
   void __fastcall SetPDF(String PDF);
   void __fastcall SetCodAlimentador(String CodAlimentador);
   void __fastcall SetCausaID(int CausaID);
   void __fastcall SetRefDef(String RefDef);
   void __fastcall SetTipoCC(int TipoCC);
   void __fastcall SetLocSel(double LocSEL);
   void __fastcall SetLocOMS(double LocOMS);
   void __fastcall SetLocFLIISR(double LocFLIISR);
   void __fastcall SetIcc(double Icc);

};
//---------------------------------------------------------------------------
#endif

