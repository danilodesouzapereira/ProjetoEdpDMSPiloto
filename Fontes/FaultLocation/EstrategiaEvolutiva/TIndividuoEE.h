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
	// Parâmetros básicos
   int ID;
   int IDpaiMutacao;
   int IDpaisCruzamento[2];

private:

	// Parâmetros auxiliares
	TSorteio* sorteio;           //< objeto para geração de números aleatórios
	TConfiguracoes* config;      //< obj de configurações da estratégia evolutiva

	// Parâmetros do indivíduo
	double X;
	double Rf;
	double sigmaX;
	double sigmaRf;

   // De-para entre X e o local da rede
   int IDbarra1;     //< ID da barra 1 de localização
   int IDbarra2;     //< ID da barra 2 de localização
   double xPorc;     //< x(%) de localização, entre barra 1 e barra 2
   String codigoTrecho; //< código do trecho onde é dado o curto-circuito

   // Avaliação
   double fAval;       //< função de avaliação (0 a 100%)
   double indiceErro;  //< indice de erro (entre valores medidos e calculados)
   int idade;

	TEstrategiaEvolutiva* estrEvol;

public:
	// Construtores
	__fastcall TIndividuoEE();
	__fastcall TIndividuoEE(TEstrategiaEvolutiva* estrEvol, double X, double Rf, double sigmaX, double sigmaRf);

	// Métodos de operação
	void          __fastcall AjustaLocalizacaoIndividuo();
	TIndividuoEE* __fastcall Cruzamento(TIndividuoEE* indiv2, double pc);
	TList*        __fastcall Mutacao(double pm);
	double        __fastcall GetAvaliacao();
	double        __fastcall GetIndiceErro();
   void          __fastcall SetAvaliacao(double indiceErro);
	void          __fastcall SetIndiceErro(double indiceErro);
   void          __fastcall SetLocalizacao(int IDbarra1, int IDbarra2, double xPorc);

	// Métodos de parâmetros
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

	// Outros métodos
	void __fastcall SetConfig(TConfiguracoes* config);
	void __fastcall SetSorteio(TSorteio* sorteio);
};
//---------------------------------------------------------------------------
#endif
