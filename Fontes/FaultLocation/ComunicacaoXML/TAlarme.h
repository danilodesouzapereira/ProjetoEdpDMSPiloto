//---------------------------------------------------------------------------
#ifndef TAlarmeH
#define TAlarmeH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TAlarme
{
public:
	// TipoAlarme:
	//   20 - Alarme de tentativa de religamento
	//   21 - Alarme de autobloqueio (lock-out)
	//   22 - Alarme de Last Gasp


	// TipoEqpto:
	//   10 - Disjuntor (DJ)
	//   11 - Religadora (RE)
	//   12 - Medidor inteligente de Trafo MT/BT
	//   13 - Medidor inteligente de consumidor MT
	//   14 - Medidor inteligente de consumidor BT

	// Dados
	String TimeStamp;
	String CodAlimentador;
	String pathArquivoAlarme;
	String CodEqpto;
	int TipoEqpto;
	int TipoAlarme;
	bool funcao50A;
	bool funcao50B;
	bool funcao50C;
	bool funcao50N;
	bool funcao51A;
	bool funcao51B;
	bool funcao51C;
	bool funcao51N;
	double correnteFalta;

public:
	// Construtores e destrutor
   __fastcall TAlarme();
	__fastcall TAlarme(String TimeStamp, String CodAlimentador, int TipoAlarme,
							 int TipoEqpto, String CodEqpto, String pathArquivoAlarme,
							 bool funcao50A, bool funcao50B, bool funcao50C,
							 bool funcao50N, bool funcao51A, bool funcao51B,
							 bool funcao51C, bool funcao51N, double correnteFalta);
	__fastcall ~TAlarme();

   // Métodos

   // Métodos Get
	String __fastcall GetCodAlimentador();
	String __fastcall GetPathArquivoAlarme();
	String __fastcall GetCodEqpto();
	String __fastcall GetTimeStamp();
	int    __fastcall GetTipoAlarme();
	int    __fastcall GetTipoEqpto();

	// Métodos Set
	void   __fastcall SetCodAlimentador(String CodAlimentador);
	void   __fastcall SetCodEqpto(String CodEqpto);
	void   __fastcall SetPathArquivoAlarme(String pathArquivoAlarme);
	void   __fastcall SetTimeStamp(String TimeStamp);
	void   __fastcall SetTipoAlarme(int TipoAlarme);
	void   __fastcall SetTipoEqpto(int TipoEqpto);

};
//---------------------------------------------------------------------------
#endif
