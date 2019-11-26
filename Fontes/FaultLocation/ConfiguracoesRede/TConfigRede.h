//---------------------------------------------------------------------------
#ifndef TConfigRedeH
#define TConfigRedeH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class VTApl;
class VTPath;
class VTRedes;
class VTSuprimento;
class VTTrafo;
//---------------------------------------------------------------------------
struct StrPotCurtoCircuito
{
	VTSuprimento* sup;     //< ponteiro para o suprimento em questão
	String CodigoSE;       //< 3 caracteres
   String NivelTensao;    //< AT, MT, ...

   double pcc_3f;         //< potência (MW) de CC 3f
   double qcc_3f;         //< potência (MVAr) de CC 3f
   double pcc_ft;         //< potência (MW) de CC ft
   double qcc_ft;         //< potência (MVAr) de CC ft

   double r0;             //< resist. seq. 0 do equiv. Thevenin do suprimento
   double x0;             //< reatância. seq. 0 do equiv. Thevenin do suprimento
   double r1;             //< resist. seq. 1 do equiv. Thevenin do suprimento
   double x1;             //< reatância. seq. 1 do equiv. Thevenin do suprimento
};
//---------------------------------------------------------------------------
struct StrParamTrafosSE
{
	VTTrafo* trafoSE;        //< ponteiro para o trafo em questão

	String   CodigoSE;       //< 3 caracteres
   String   CodigoTrafoSE;  //< Código do trafo de SE

   double   Snom;           //< potência nominal do trafo SE (MVA)
   double   r0;             //< resist. seq. 0 do equiv. Thevenin do suprimento
   double   x0;             //< reatância. seq. 0 do equiv. Thevenin do suprimento
   double   r1;             //< resist. seq. 1 do equiv. Thevenin do suprimento
   double   x1;             //< reatância. seq. 1 do equiv. Thevenin do suprimento

   double   r0_baseTrafo;   //< resist. seq. 0 do equiv. Thevenin do suprimento
   double   x0_baseTrafo;   //< reatância. seq. 0 do equiv. Thevenin do suprimento
   double   r1_baseTrafo;   //< resist. seq. 1 do equiv. Thevenin do suprimento
   double   x1_baseTrafo;   //< reatância. seq. 1 do equiv. Thevenin do suprimento
};
//---------------------------------------------------------------------------
class TConfigRede
{
private:
	// Parâmetros elementares
	VTApl*   apl;
   VTPath*  path;
   VTRedes* redes;

   // Dados
   TList* lisPotenciasCurtoCircuito;
   TList* lisParamTrafosSE;

public:
	// Construtor e destrutor
   __fastcall TConfigRede(VTApl* apl);
	__fastcall ~TConfigRede();


   // Métodos
   bool          __fastcall AbreArquivoPotCurtoCircuito(String pathArquivoSuprimento = "");
	bool          __fastcall AbreArquivoParamTrafosSE(String pathArquivoParamTrafosSE = "");
	void          __fastcall AlteraPotenciasCurtoCircuito();
	void          __fastcall AlteraParametrosTrafosSE();
	void          __fastcall PopulaListViewPotCurto(TListView* listview, String tipoExibicao);
	void          __fastcall PopulaListViewParamTrafosSE(TListView* lvConfigParamTrafosSE, String opcao);
   VTSuprimento* __fastcall GetSuprimento(String NivelTensao, String CodigoRede);
	VTTrafo*      __fastcall GetTrafoSE(String CodigoSE, String CodigoTrafoSE);
   void          __fastcall Resseta();

};
//---------------------------------------------------------------------------
#endif
