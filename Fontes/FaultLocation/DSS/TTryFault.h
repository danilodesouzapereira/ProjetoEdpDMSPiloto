//---------------------------------------------------------------------------
#ifndef TTryFaultH
#define TTryFaultH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <complex>
//---------------------------------------------------------------------------
struct medicaoV
{
	int IDbarra;
   int numFases;
   double V[3];
};
//---------------------------------------------------------------------------
struct medicaoI
{
	String codTrecho;
   int numFases;
   double I[3];
};
//---------------------------------------------------------------------------
class TConfiguracoes;
class TDSS;
//---------------------------------------------------------------------------
class TTryFault : public TObject
{
public:
	// Dados
	int patamar;
   String caminhoDSS;
   String dirDSS;
   TConfiguracoes* config;
	TDSS*  DSS;

   TStringList* lisIDBarrasMonitoradas;   //< lista para os IDs das barras monitoradas
   TStringList* lisCodTrechosMonitorados; //< lista para os códigos dos trechos monitorados

   TList* lisMedV;                  //< Lista para as structs de medições de tensão
   TList* lisMedI;                  //< Lista para as structs de medições de corrente
   TStringList* lisCalcV;           //< Lista com as tensões calculadas durante o curto
   TStringList* lisCalcI;           //< Lista com as correntes calculadas durante o curto

   // Dados de nível de curto-circuito 3F no secundário do trafo SE
   double Vnom;
   double IccMT;

   String strIDBarraMedV_QualimetroRef;
   String codLigacaoQualimetroRef;

public:
	// Construtor e destrutor
	__fastcall TTryFault();
   __fastcall ~TTryFault();

   // Métodos de ajustes/preparação
	String __fastcall GetComandoFalta(String faseDefeito, int idBarraCurto, double Rf, bool Inicial = false);
	String __fastcall GetComandoFalta_2FT(int idFalta, String faseDefeito, int idBarraCurto, double Rf, bool Inicial = false);
	void   __fastcall GetLisCalcI(TStringList* lisEXT);
   void   __fastcall GetLisCalcV(TStringList* lisEXT);
	void   __fastcall GetLisMedV(TStringList* lisEXT);
	void   __fastcall IniciaBarrasTrechos();
   void   __fastcall SetBarrasMonitoradas(String CSV_IDBarrasMon);    //< lista com os IDs das barras monitoradas
   void   __fastcall SetCaminhoDSS(String caminhoDSS);
   void   __fastcall SetConfig(TConfiguracoes* config);
   void   __fastcall SetMedicoesBarras(TStringList* lisMedV);
   void   __fastcall SetMedicoesTrechos(TStringList* lisMedI);
   void   __fastcall SetIDBarraMonitorada(String strIDBarraMedV_QualimetroRef);
   void   __fastcall SetCodLigacaoRefMonitorada(String codLigacaoQualimetroRef);
	void   __fastcall SetNivelCurtoMT();
   void   __fastcall SetTrechosMonitorados(String CSV_CodTrechosMon); //< lista com os códigos dos trechos monitoradas

   // Métodos
	double __fastcall CalculaErro();
	double __fastcall CalculaErro_AlgoFasorial();
   void   __fastcall CorrentePreFalta(std::complex<double> &Ia_pre, std::complex<double> &Ib_pre, std::complex<double> &Ic_pre);
   void   __fastcall ExecutaTeste(int patamar);
	int    __fastcall GetNumFases(String fasesDefeito);
//	void   __fastcall TestaCurto(int patamar, int barra_ID, TStringList* lisCalcV, TStringList* lisCalcI); //< Retorna os valores calculados de tensão e corrente
	void   __fastcall TestaCurtoRf(int patamar, int idBarraCurto, String faseDefeito, double Rf, bool Inicial = false);
	void   __fastcall TestaCurtoRf_GeraDefeitos(int patamar, int idBarraCurto, String faseDefeito, double Rf, bool Inicial = false);

   void   __fastcall TestaPreFalta(int patamar, std::complex<double> &Ia_pre, std::complex<double> &Ib_pre, std::complex<double> &Ic_pre);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
