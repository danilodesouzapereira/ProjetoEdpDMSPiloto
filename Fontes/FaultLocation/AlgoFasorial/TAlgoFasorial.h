//---------------------------------------------------------------------------
#ifndef TAlgoFasorialH
#define TAlgoFasorialH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <complex>
//---------------------------------------------------------------------------
class TAreaBusca;
class TChaveMonitorada;
class TClassificacao;
class TConfiguracoes;
class TDados;
class TFormFaultLocation;
class TFuncoesDeRede;
class TLog;
class TQualimetro;
class TTryFault;
class VTApl;
class VTBarra;
class VTChave;
class VTGrafico;
class VTRede;
class VTRedes;
struct StrFasor;
//---------------------------------------------------------------------------
struct strBarraAlgoFasorial
{
	VTBarra* barra;
	double Xtotal;
   double Rtotal;
   double Rtotal_1, Xtotal_1, Rtotal_0, Xtotal_0;
   double Rf;
   double indiceErro;
   bool maxAfundamentoCoincidente; //< indica que a solução gera máx. afund. coincidente com o máx. afund. medido

   //debug
   String vetor_calcV;
};
//---------------------------------------------------------------------------
class TAlgoFasorial
{
public:
   // Dados
	TFuncoesDeRede* funcoesRede;
   VTApl*          apl;
	VTGrafico*      graf;
   VTRede*         redeMT;
   VTRede*         redeSE;
   VTRedes*        redes;

   TAreaBusca*          areaBusca;
	TClassificacao*      classificacao;
	VTChave*             chaveRef;
   TConfiguracoes*      config;   //< Parâmetros de config do algo de EE
   TDados*              dados;    //< Conjunto dos dados acerca do defeito
   TFormFaultLocation*  formFL;   //< Referência para o form de Fault Location
   TTryFault*           tryFault; //< objeto para testar um curto-circuito específico
   TTryFault*           preFalta; //< objeto para rodar fluxo de potência pré-falta

   // Dados auxiliares
   String strTipoFalta;
   TLog* logFL;                   //< Log para externalizar observações, resultados para debug e erros

   // Configurações
   double Tolerancia;
   bool   AgruparSolucoes;

	std::complex<double> Ztotal;
	std::complex<double> Ztotal_1;
   std::complex<double> Ztotal_3F;
	std::complex<double> V0, V1, V2, I0, I1, I2;
	TList*               lisBarrasCandidatas;

	// Construtor e destrutor
	__fastcall TAlgoFasorial(VTApl* apl);
	__fastcall ~TAlgoFasorial();

	void    __fastcall AgrupaSolucoes(TList* lisEXT);
   std::complex<double> __fastcall AjustaCorrenteI012();

	void    __fastcall BarrasCandidatas_FT(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal, TList* lisEXT);
	void    __fastcall BarrasCandidatas_FT(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal, TList* lisEXT);
	void    __fastcall BarrasCandidatas_FT(String codRede, double Xtotal, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2FT(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2FT(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2FT(String codRede, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2F(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2F(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_2F(String codRede, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_3F(String codRede, TQualimetro* qualimetroEqptoRef, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_3F(String codRede, TChaveMonitorada* chaveMonit_referencia, double Xtotal_1, TList* lisEXT);
	void    __fastcall BarrasCandidatas_3F(String codRede, double Xtotal_1, TList* lisEXT);

	double  __fastcall CalculaRf_Solucao(strBarraAlgoFasorial* barraAlgoFasorial);
	void    __fastcall CalculaZtotal_FT(StrFasor* Vse, StrFasor* Ise);
	void    __fastcall CalculaZtotal_seq1_2FT(StrFasor* Vse, StrFasor* Ise);
	void    __fastcall CalculaZtotal_seq1_2F(StrFasor* Vse, StrFasor* Ise);
	void    __fastcall CalculaZtotal_seq1_3F(StrFasor* Vse, StrFasor* Ise);
	void    __fastcall CalculaZtotal(double mVa, double mVb, double mVc, double pVa, double pVb, double pVc,
   									  double mIa, double mIb, double mIc, double pIa, double pIb, double pIc);
	void    __fastcall CalculaZtotal_3F(double mVa, double mVb, double mVc, double pVa, double pVb, double pVc,
   									  double mIa, double mIb, double mIc, double pIa, double pIb, double pIc);
	void    __fastcall CalculaVI_Sequencias012(StrFasor* fasorVref, StrFasor* fasorIref);
//	void    __fastcall CarregaMedicoesVI(TFormFaultLocation* formFL);

	void    __fastcall Executa_FT(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal, double Xtotal, bool FLOffline = false);        // Execução do algoritmo para defeitos FT
	void    __fastcall Executa_FT(String codRede, double Rtotal, double Xtotal, bool FLOffline = false);        // Execução do algoritmo para defeitos FT
	void    __fastcall Executa_2FT(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal, double Xtotal, bool FLOffline = false);        // Execução do algoritmo para defeitos 2FT
	void    __fastcall Executa_2FT(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline = false);   // Execução do algoritmo para defeitos 2FT
	void    __fastcall Executa_2F(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal_1, double Xtotal_1, bool FLOffline = false);    // Execução do algoritmo para defeitos 2F
	void    __fastcall Executa_2F(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline = false);    // Execução do algoritmo para defeitos 2F
	void    __fastcall Executa_3F(String codRede, TQualimetro* qualimetroEqptoRef, double Rtotal_1, double Xtotal_1, bool FLOffline = false);    // Execução do algoritmo para defeitos 3F
	void    __fastcall Executa_3F(String codRede, double Rtotal_1, double Xtotal_1, bool FLOffline = false);    // Execução do algoritmo para defeitos 3F

	void    __fastcall Executa_FT_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal, double Xtotal);        // Exec. algo. FT  - FLOffline
	void    __fastcall Executa_2F_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1);    // Exec. algo. 2F  - FLOffline
	void    __fastcall Executa_2FT_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1);   // Exec. algo. 2FT - FLOffline
	void    __fastcall Executa_3F_FLOffline(String codRede, TChaveMonitorada* chaveMonit_referencia, double Rtotal_1, double Xtotal_1);    // Exec. algo. 3F  - FLOffline

	void    __fastcall ExecutaFiltroRf(double Rtotal);
	void    __fastcall FiltraBarrasCurtoCircuito();
	void    __fastcall FiltraBarrasPorAnaliseFases(TList* lisBarrasCandidatas, String strTipoFalta);
	void    __fastcall FiltraSolucoes_por_afundamento_coincidente();
	VTRede* __fastcall GetRedeMT(String codRede);
	VTRede* __fastcall GetRedeSE(VTRede* redeMT);
	void    __fastcall GetSolucoes(TFuncoesDeRede* funcoesRede, TList* lisEXT);
	double  __fastcall GetValorMedio(String tipoFalta, String medicaoV);
	void    __fastcall GetZTotal(double &reZtotal, double &imZtotal);
	void    __fastcall GetZTotal_1(double &reZtotal_1, double &imZtotal_1);
	void    __fastcall GetZTotal_3F(double &reZtotal, double &imZtotal);
	void    __fastcall InicializaConfiguracoes(String CaminhoDirFaultLocation);
	void    __fastcall IniciaTryFault(String caminhoDSS);
   void    __fastcall IniciaPreFalta(String caminhoDSS);
	void    __fastcall OrdenaBarrasCurtoCircuito();
   bool    __fastcall RefinarSolucoes();
	void    __fastcall SetParametros(TConfiguracoes* config,
                                   TClassificacao* classificacao,
                                   TAreaBusca* areaBusca,
                                   TDados* dados,
                                   TFormFaultLocation* formFL);
	void    __fastcall SetLogFL(TLog* logFL);
   bool    __fastcall TemRepeticaoDeMinimo(double* medias, int NumeroMedicoes, double desvio);
	void    __fastcall TestaCurtoCircuitos_FT(double Rtotal);
	void    __fastcall TestaCurtoCircuitos_2FT(String strTipoFalta);
   void    __fastcall TestaCurtoCircuitos_2F(String strTipoFalta);
   void    __fastcall TestaCurtoCircuitos_3F(String strTipoFalta);
	bool    __fastcall VerificaAfundCoincidente(String tipoFalta, TStringList* lisAux_medV, TStringList* lisAux_calcV);

	VTBarra* __fastcall BarraReferencia_ChaveMonitorada(VTRede* redeMT, TChaveMonitorada* chaveMonit_referencia);
	VTBarra* __fastcall BarraReferencia_QualimetroEqptoRef(VTRede* redeMT, TQualimetro* qualimetroEqptoRef);
};
//---------------------------------------------------------------------------
#endif

