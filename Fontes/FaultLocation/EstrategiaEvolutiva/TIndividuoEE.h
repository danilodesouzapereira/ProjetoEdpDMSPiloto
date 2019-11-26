//---------------------------------------------------------------------------
#ifndef TIndividuoEEH
#define TIndividuoEEH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TAreaBusca;
class TConfiguracoes;
class TEstrategiaEvolutiva;
class TSorteio;
//---------------------------------------------------------------------------
class TIndividuoEE
{
public:
	// Par�metros b�sicos
   int ID;
   int IDpaiMutacao;
   int IDpaisCruzamento[2];

private:

	// Par�metros auxiliares
	TSorteio* sorteio;           //< objeto para gera��o de n�meros aleat�rios
	TConfiguracoes* config;      //< obj de configura��es da estrat�gia evolutiva

	// Par�metros do indiv�duo
	double X;
	double Rf;
	double sigmaX;
	double sigmaRf;

   // De-para entre X e o local da rede
   int IDbarra1;     //< ID da barra 1 de localiza��o
   int IDbarra2;     //< ID da barra 2 de localiza��o
   double xPorc;     //< x(%) de localiza��o, entre barra 1 e barra 2
   String codigoTrecho; //< c�digo do trecho onde � dado o curto-circuito

   // Avalia��o
   double fAval;       //< fun��o de avalia��o (0 a 100%)
   double indiceErro;  //< indice de erro (entre valores medidos e calculados)
   int idade;

	TEstrategiaEvolutiva* estrEvol;

public:
	// Construtores
	__fastcall TIndividuoEE();
	__fastcall TIndividuoEE(TEstrategiaEvolutiva* estrEvol, double X, double Rf, double sigmaX, double sigmaRf);

	// M�todos de opera��o
	void          __fastcall AjustaLocalizacaoIndividuo();
	TIndividuoEE* __fastcall Cruzamento(TIndividuoEE* indiv2, double pc);
	TList*        __fastcall Mutacao(double pm);
	double        __fastcall GetAvaliacao();
	double        __fastcall GetIndiceErro();
   void          __fastcall SetAvaliacao(double indiceErro);
	void          __fastcall SetIndiceErro(double indiceErro);
   void          __fastcall SetLocalizacao(int IDbarra1, int IDbarra2, double xPorc);

	// M�todos de par�metros
   int    __fastcall GetIDbarra1();
   int    __fastcall GetIDbarra2();
   double __fastcall GetXporc();
   int    __fastcall GetIdade();
	double __fastcall GetX();
	double __fastcall GetRf();
	double __fastcall GetSigmaX();
	double __fastcall GetSigmaRf();
	void   __fastcall IncrementaIdade();
	void   __fastcall SetX(double X);
	void   __fastcall SetRf(double Rf);
	void   __fastcall SetSigmaX(double sigmaX);
	void   __fastcall SetSigmaRf(double sigmaRf);

	// Outros m�todos
	void __fastcall SetConfig(TConfiguracoes* config);
	void __fastcall SetSorteio(TSorteio* sorteio);
};
//---------------------------------------------------------------------------
#endif
