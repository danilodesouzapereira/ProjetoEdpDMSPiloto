//---------------------------------------------------------------------------
#ifndef TEventoCGPH
#define TEventoCGPH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TEventoCGP
{
	// Métodos
   void __fastcall InicializaEvento();

	// Dados
   String CodAlimentador;
   String RelayID;
	String EventTime;
   String EventDate;
   String EventType;
   double EventLocation;
   double IAFault;
   double IBFault;
   double ICFault;
   double INFault;


public:
	// Construtor e destrutor
   __fastcall TEventoCGP();
   __fastcall ~TEventoCGP();

   // Métodos Set
   void __fastcall SetRelayID(String RelayID);
   void __fastcall SetEventTime(String EventTime);
   void __fastcall SetEventDate(String EventDate);
   void __fastcall SetEventType(String EventType);
   void __fastcall SetEventLocation(String strEventLocation);
   void __fastcall SetIAFault(String strIAFault);
   void __fastcall SetIBFault(String strIBFault);
   void __fastcall SetICFault(String strICFault);
   void __fastcall SetINFault(String strINFault);


};
//---------------------------------------------------------------------------
#endif
