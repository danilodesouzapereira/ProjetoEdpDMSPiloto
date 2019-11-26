//---------------------------------------------------------------------------
#ifndef TDadosH
#define TDadosH
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
class TAlarme;
class TChaveMonitorada;
class TEqptoCampo;
class TFuncoesDeRede;
class TMedidorInteligente;
class VTApl;
class VTLigacao;
class VTPath;
class VTRedes;
class VTTrecho;
//---------------------------------------------------------------------------
struct StrFasor
{
   std::complex<double> faseA;
   std::complex<double> faseB;
   std::complex<double> faseC;
};
//---------------------------------------------------------------------------
class TDados : public TObject
{
private:
	// Parâmetros elementares
   String          CodigoAlimentador;
   String          CodigoEvento;
   TFuncoesDeRede* funcoesRede;
   VTApl*          apl;
   VTPath*         path;
   VTRedes*        redes;

	// Dados
	TList*  lisEqptosCampo; //< lista com eqptos (sensor, iTrafo ...)
	String  pathDados;      //< path do diretório com os arquivos de entrada
	String  pathEvento;     //< path com o diretório do evento


   int patamar;           //< patamar horário = 0, 1, 2, ..., 23.

public:
	// Construtor e destrutor
   __fastcall TDados(VTApl* apl, TList* listaAlarmes);
   __fastcall TDados(VTApl* apl, String TimeStamp, String CodAlimentador);
	__fastcall ~TDados();

	// Métodos principais
	void __fastcall AtualizaMedicoesMedidorInteligente(TMedidorInteligente* medidorInteligente);
	void __fastcall AtualizaMedicoesDisjuntor(TChaveMonitorada* chvDJ);
	void __fastcall AtualizaMedicoesDisjuntor(TChaveMonitorada* chvDJ, TAlarme* alarmeDisjuntor);
	void __fastcall AtualizaMedicoesReligadora(TChaveMonitorada* chvRE);
	void __fastcall AtualizaMedicoesReligadora(TChaveMonitorada* chvRE, TAlarme* alarmeReligadora);
	void __fastcall LeDadosFormatados(void);

	// Métodos auxiliares
	void   __fastcall AcrescentaDadosMedicoes(TList* listaAlarmes);
   VTTrecho* __fastcall DeterminaTrechoJusante(VTLigacao* ligacaoRef);
	bool   __fastcall ExisteEqptoCampo(int Tipo, String Codigo);
//	bool   __fastcall FaltamDados();
	String __fastcall GetCodigoAlimentador();
	String __fastcall GetCodCargaMIbalanco(String cod_medidor_inteligente_balanco);
	String __fastcall GetCodChaveDisjuntor(String CodDisjuntorSage);
	String __fastcall GetCodChaveSensor(String CodSensor);
	String __fastcall GetCodChaveQualimetro(String CodQualimetro);
	String __fastcall GetCodChaveReligadora(String CodReligadoraSage);
	String __fastcall GetCodigoLigacaoAssociadaQualimetro(String cod_qualimetro);
   String __fastcall GetCodigoEvento();
   TList* __fastcall GetEqptosCampo();
	TEqptoCampo* __fastcall GetEqptoCampo(int Tipo, String Codigo);
	String __fastcall GetPathEvento();
	String __fastcall GetCodTrechosMonitorados();
	TStringList* __fastcall GetMedicoesBarras();
	void __fastcall GetMedicoesBarras_AlgoFasorial(TStringList* lisEXT);
	TStringList* __fastcall GetMedicoesTrechos();
	void __fastcall GetMedicoesTrechos_AlgoFasorial(TStringList* lisEXT);
	TStringList* __fastcall GetMedicoesTrechos_EE();
	int    __fastcall GetPatamarHorario();
	void   __fastcall SetPatamarHorario(String ev_timestamp);
	void   __fastcall GetFasoresVI_SE_FLOffline(StrFasor* Vse, StrFasor* Ise);
	TChaveMonitorada* __fastcall GetFasoresVI_Referencia_FLOffline(StrFasor* fasorVref, StrFasor* fasorIref);
	void   __fastcall InsereDadosDisjuntor(String CodDisjuntor, String linhaDados);
	void   __fastcall InsereDadosDisjuntor(String CodDisjuntor, String linhaV, String linhaI);
	void   __fastcall InsereDadosReligadora(String CodReligadora, String linhaV, String linhaI);
	bool   __fastcall MedicoesTrafoIntelBT();
	void   __fastcall TensoesNominais(String codTrafo, double &V_linha_MT_nom, double &V_linha_BT_nom);

   // Para os dados do FL offline
	void __fastcall GeraEqptoDisjuntor();
	void __fastcall GeraEqptoReligadora();
};
//---------------------------------------------------------------------------
#endif
