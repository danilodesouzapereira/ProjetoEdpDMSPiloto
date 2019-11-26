//---------------------------------------------------------------------------
#ifndef TTryRompCaboH
#define TTryRompCaboH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
struct medicaoV
{
	int IDbarra;
   int numFases;
   double V[3];
};
//---------------------------------------------------------------------------
class TDSS;
//---------------------------------------------------------------------------
class TTryRompCabo : public TObject
{
public:
	// Dados
	int patamar;
	String caminhoDSS;
	String dirDSS;
	TDSS*  DSS;

	TStringList* lisIDBarrasMonitoradas;   //< lista para os IDs das barras monitoradas
	TList* lisMedV;                  //< Lista para as structs de medições de tensão
	TStringList* lisCalcV;           //< Lista com as tensões calculadas durante o curto

	// Para romp de cabo
	String backup_fasesBarra1;
	String backup_fasesBarra2;
	String backup_codBarra1;
	String backup_codBarra2;
	String backup_codTrecho;
	String backup_codArranjo;

	// Construtor e destrutor
	__fastcall TTryRompCabo();
	__fastcall ~TTryRompCabo();

	// Métodos
	String __fastcall Bus1Trecho(String codTrecho);
	String __fastcall Bus2Trecho(String codTrecho);
	double __fastcall CalculaErro_RompCabo();
	String __fastcall CodigoArranjo(String codTrecho);
	String __fastcall Trecho(int idBarraRompimento);
	String __fastcall GetComandoRompimento_1fase(String faseRompimento, String codigoTrecho, String idBarraRomp1, String idBarraRomp2, String codigoArranjo);
	void   __fastcall IniciaBarras();
	void   __fastcall RestauraRompCabo();
	void   __fastcall SetCaminhoDSS(String caminhoDSS);
	void   __fastcall SetMedicoesBarras(TStringList* lisMedV);
	void   __fastcall TestaRompCabo(int patamar, int idBarraRompimento, String faseRompimento, bool Inicial = false);
};
//---------------------------------------------------------------------------
#endif

