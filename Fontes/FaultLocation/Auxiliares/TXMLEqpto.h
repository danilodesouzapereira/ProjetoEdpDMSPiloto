//---------------------------------------------------------------------------
#ifndef TXMLEqptoH
#define TXMLEqptoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TXMLEqpto
{

private:

   // Parâmetros

   String timeStamp;          // Exemplo: 20171016114800  (ano, mês, dia, hora, min, seg).
   String codigoAlimentador; // Exemplo: CMC01P1.

   int tipoEqpto;            // 1- Disjuntor, 2- Religadora, 3- Seccionalizadora, 4- Sensor.

	String codigoEqpto;       // Código do equipamento que deu origem ao alarme

	int tipoAlarme;           // 1- Lock-out (para tipoEqpto = 1 e 2)
   	                       // 2- Raut (para tipoEqpto = 3)
   				              // 3- Falta permanente FPE (para tipoEqpto=4)
   					           // 4- Falta a jusante ou surto AIM (para tipoEqpto=4)

   double DfaltaRele;        // Distância de falta estimada pelo relé.


public:
	// Construtor
	__fastcall TXMLEqpto();
   __fastcall TXMLEqpto(String timeStamp);

   // Métodos
   void __fastcall SetTimeStamp(String timeStamp);
   void __fastcall SetCodigoAlimentador(String codigoAlimentador);
   void __fastcall SetCodigoEqpto(String codigoEqpto);
   void __fastcall SetTipoEqpto(int tipoEqpto);
   void __fastcall SetTipoAlarme(int tipoAlarme);
   void __fastcall SetDfaltaRele(double DfaltaRele);

   String  __fastcall GetTimeStamp();
   String  __fastcall GetCodigoAlimentador();
   String  __fastcall GetCodigoEqpto();
   int     __fastcall GetTipoEqpto();
   int     __fastcall GetTipoAlarme();
   double  __fastcall GetDfaltaRele();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
