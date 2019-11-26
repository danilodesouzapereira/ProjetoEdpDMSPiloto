//---------------------------------------------------------------------------
#ifndef TGeraDefeitoH
#define TGeraDefeitoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <complex>
//---------------------------------------------------------------------------
class TTryFault;
class VTApl;
class VTBarra;
class VTLigacao;
class VTPath;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
struct StrGeraDefeito
{
	String caminhoDSS;
	String CodigoAlimentador;
	String CodigoBarra;
	String TipoDefeito;
	double Rfalta;
};
//---------------------------------------------------------------------------
class TGeraDefeito
{

private:
	// Parâmetros elementares
	VTApl* apl;
   VTPath* path;
   VTRedes* redes;
   TMemo* memoResultadosGeraDefeito;

   // Parâmetros
	StrGeraDefeito* strGeraDefeito;
   TTryFault* tryFault;
   VTBarra* barra;
   VTRede* rede;

   TStringList* lisMedicoesBarras;
   TStringList* lisMedicoesTrechos;

public:
	// Construtor
	__fastcall TGeraDefeito(VTApl* apl);

   // Métodos
   void __fastcall SetConfiguracoes(StrGeraDefeito* strGeraDefeito);
   void __fastcall Executa();
	void __fastcall ExportarResultados(String pathTxtResultados);
	int  __fastcall GetIDbarra(String CodigoChaveRef);
   VTLigacao* __fastcall GetLigacaoInicial(VTRede* rede);
   void __fastcall InicializaParametros();
	void __fastcall IniciaTryFault(String caminhoDSS);
   void __fastcall MostraResultadosMemo(TStringList* lisAux_calcV, TStringList* lisAux_calcI);
	void __fastcall SetMemo(TMemo* memoResultadosGeraDefeito);
};
//---------------------------------------------------------------------------
#endif

