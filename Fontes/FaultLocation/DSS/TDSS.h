//---------------------------------------------------------------------------
#ifndef TDSSH
#define TDSSH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TDSSBarra;
class TDSSLigacao;
//---------------------------------------------------------------------------
class TDSS : public TObject
{
private:
  	// dados
	HINSTANCE OpenDSSDirect;
	typedef int(__cdecl*pfDSSI)(int, int);
	typedef char*(__cdecl*pfDSSPut_Command)(char*);
	typedef int(__cdecl*pfSolutionI)(int, int);
	typedef char*(__cdecl*pfCircuitS)(int, char*);
	typedef void(__cdecl*pfCircuitV)(int, Variant*, int);
	typedef int(__cdecl*pfCircuitI)(int, int);
   typedef void(__cdecl*pfLinesV)(int, Variant*);
   typedef char*(__cdecl*pfLinesS)(int, char*);
   typedef int(__cdecl*pfLinesI)(int, int);
	typedef void(__cdecl*pfCktElementV)(int, Variant*, int);

  	pfDSSI DSSI;
	pfDSSPut_Command DSSPut_Command;
	pfSolutionI SolutionI;
	pfCircuitS CircuitS;
	pfCircuitV CircuitV;
	pfCircuitI CircuitI;
   pfLinesV LinesV;
   pfLinesS LinesS;
	pfLinesI LinesI;
	pfCktElementV CktElementV;


   // Dados
   TList* lisDSSBarras;
   TList* lisDSSLigacoes;

public:
	// Construtor e destrutor
	__fastcall TDSS();
   __fastcall ~TDSS();

	// Métodos
  	void __fastcall ClearAll();
	int __fastcall Solve();
	int __fastcall Start();
	void __fastcall GetDSSBarras();
   void __fastcall GetDSSBarras_Codigos(TStringList* lisLinhas);
   void __fastcall GetDSSBarras_IDs(TStringList* lisLinhas);
	void __fastcall GetDSSLigacoes_Codigos(TStringList* lisLinhas);
   void __fastcall GetFases(TStringList* lisNodeNames);
   TList* __fastcall GetLisDSSBarras();
   TList* __fastcall GetLisDSSLigacoes();
	void __fastcall GetNodeNamesArray(Variant* allNodeNames, TStringList* lisNodeNames);
   void __fastcall GetVoltages(TStringList* calcV);
	void __fastcall GetVoltages_GeraDefeitos(TStringList* lisCodBarras, TStringList* calcV);
	AnsiString __fastcall WriteCommand(AnsiString comando);

   int __fastcall GetCircuitNumNodes();
   void __fastcall GetVoltagesDoubleArray(int NumValores, Variant* allBusVolts, TStringList* lisV);
	void __fastcall GetVoltagesDoubleArray_Monitoradas(TList* lisDSSBarrasMonitoradas, Variant* allBusVolts, TStringList* lisV);
	TDSSBarra* __fastcall ProcuraDSSBarra(IDbarra);
   TDSSLigacao* __fastcall ProcuraDSSLigacao(String codLiga);
	void __fastcall GeraDSSLigacoes(TStringList* lisLinesNames);
	void __fastcall GetDSSLigacoes();
	void __fastcall GetLinesNamesArray(Variant* allNames, TStringList* lisLinesNames);
	int __fastcall SetActiveLine(int iLine);
	char* __fastcall GetLineBus(int iBarra);
	void __fastcall GetFasesLigacoes();
   void __fastcall GetCurrents(TStringList* calcI);
   char* __fastcall GetLineCode();
	void __fastcall GetLineCurrents(TStringList* lisI);
	void __fastcall GetCurrentsDoubleArray(int NumValores, Variant* allCurrents, TStringList* lisI);
	void __fastcall GetCurrents_GeraDefeitos(TStringList* lisCodLigacoes, TStringList* calcI);
	void __fastcall GetCurrents(TStringList* lisCodLigacoes, TStringList* calcI);
   void __fastcall GetCurrents_QualimetroRef(String codLigacaoQualimetroRef, TStringList* lisCalcI_qualRef);
	void __fastcall GetVoltages(TStringList* lisCodBarras, TStringList* calcV);
	TList* __fastcall GetDSSBarras_Monitoradas(TStringList* lisCodBarras);

	void __fastcall ExecutaFaultStudy();

   void __fastcall AjustaTensoes(TStringList* calcV);
   void __fastcall AjustaTensoes_ModFase(TStringList* calcV);
	void __fastcall AjustaCorrentes(TStringList* lisEXTcalcI);
	void __fastcall AjustaCorrentes_ModFase(TStringList* calcI);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
