//---------------------------------------------------------------------------
#ifndef TThreadFaultLocationH
#define TThreadFaultLocationH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TAlarme;
class TAlgoFasorial;
class TAlgoRompCabo;
class TAreaBusca;
class TClassificacao;
class TConfiguracoes;
class TConversorSinapDSS;
class TDados;
class TEstrategiaEvolutiva;
class TFaultStudyFL;
class TFormFaultLocation;
class TFuncoesDeRede;
class TGerenciadorEventos;
class TImportaXMLs;
class TLog;
class TOpcoesGraficas;
class	TXMLComunicacao;
//---------------------------------------------------------------------------
class VTApl;
class VTBarra;
class VTBloco;
class VTChave;
class VTLigacao;
class VTPath;
class VTRede;
class VTRedes;
struct StrFasor;
//---------------------------------------------------------------------------
struct strSolucao
{
	String   DefTipo;
	String   DefTipo_detalhe;
   String   DefX;
   String   DefY;
   String   DefLat;
   String   DefLon;
   String   ChvMont;
   String   DistChvMont;
   String   DistRele;
   String   ClientesDepoisChvMont;
   VTBarra* barraSolucao;

   String   CodBarra;
   int      IdBarra;
   int      Probabilidade;
	double   IndiceErro;
	double   Rfalta_estimado;
};
//---------------------------------------------------------------------------
struct StrSolucaoDMS2 : public TObject
{
	VTBloco* bloco;
	VTChave* chaveMontante;
	TList* lisTrechosBloco;

	__fastcall StrSolucaoDMS2()
	{
      lisTrechosBloco = new TList;
	}
	__fastcall ~StrSolucaoDMS2()
	{
      delete lisTrechosBloco;
	}
};
//---------------------------------------------------------------------------
struct StrSolucaoDMS3 : public TObject
{
	VTLigacao* ligacao;
};
//---------------------------------------------------------------------------
struct StrDadosGerais
{
	TList*   lisTriggers;
   int      ClientesAfetados;
   String   Alimentador;
   String   EqptoTrigger;
   String   SE;
   String   TimeStamp;
	String   TipoFalta;
	double   Ifalta_medida;
   bool     rompCabo;

   ~StrDadosGerais()
   {
      if(lisTriggers)
      {
         delete lisTriggers;
         lisTriggers = NULL;
      }
   }
};
//---------------------------------------------------------------------------
struct StrDadosLog
{
   String FLMethod;
	double FLTolerance;
	double MeasuredFaultCurrent;   //< corrente de curto-circuito medida e utilizada pela metodologia
	int    solutionsAmount;        //< n�mero de solu��es
	TList* lisInputData;
	TList* lisBadData;
	TList* lisAlarmes;
   TStringList* lisEqptosErrosAquisicao;
   String DistFaltaRele;          //< dist�ncia de falta calculada pelo rel� do disjuntor (na SE)
   String CodigoReleSubestacao;   //< c�digo do rel� do disjuntor (na SE)
   bool   fasesSasOpostas;        //< indica se as fases s�s t�m �ngulos opostos
};
//---------------------------------------------------------------------------
struct StrInsumoDado
{
   String tipoEqpto;
   String codEqpto;
   String Ifalta_DJ_RE;
   String IA_falta_sensor;
   String IB_falta_sensor;
   String IC_falta_sensor;
   String IA_falta_qualimetro;
   String IB_falta_qualimetro;
   String IC_falta_qualimetro;
   String VA_falta_qualimetro;
   String VB_falta_qualimetro;
   String VC_falta_qualimetro;
   String VA_falta_trafo_inteligente;
   String VB_falta_trafo_inteligente;
   String VC_falta_trafo_inteligente;
	String DistRele;               //< dist�ncia de falta indicada pelo rel�
	String VA_posfalta_medidor_inteligente;
	String VB_posfalta_medidor_inteligente;
	String VC_posfalta_medidor_inteligente;
};
//---------------------------------------------------------------------------
struct StrDadoRuim
{
   String tipoEqpto;
   String codEqpto;
};
//---------------------------------------------------------------------------
class TThreadFaultLocation : public TThread
{

public:
	int  tipoLF;
	bool caboRompido;
   bool iniciado;
   bool FLfinalizado;
   int  timerTimeout;

private:

	// Par�metros elementares
   TConfiguracoes*      config;
   TFormFaultLocation*  formFL;
   TGerenciadorEventos* gerenciador;
   TImportaXMLs*        importaXMLs;
   TLog*                logErros;
	VTApl*               apl;
   VTPath*              path;
   VTRedes*             redes;
   VTRede*              redeMT;
   VTRede*              redeSE;

   // Auxiliares
   StrDadosLog* strLog;
   TListView*   LViewMonitores;
	TMemo*       MemoResultados;
   TMemo*       MemoProcessosLF;

   // Identifica��o do evento
   AnsiString caminhoDSS;
   String     CodigoAlimentador;
	String     NomeArquivo;               //< c�digo (timestamp) que identifica um evento
   String     CodigoEvento;              //< c�digo do evento (Alimentador + timestamp)
	TList*     listaAlarmes;              //< lista de alarmes (objs da classe TAlarme)
   String     TimeStamp;                 //< timestamp do evento
	TAlarme*   alarmeGatilho;           //< aponta para o alarme que foi gatilho do processo (timestamp mais antigo)
	bool       solicitacaoMedicoesAdicionais;   //< flag para indicar se houve ou n�o solicita��o de medi��es adicionais

   // Dados externos
   TStringList*     lisNomesXMLMsg;  //< lista com os nomes dos arquivos XML de resposta do M�d. de Supervis�o
 	String           nomeXMLreq;      //< nome do XML com a resposta do M�d. de Supervis�o
   bool             janelaAberta;    //< indica se a janela de inser��o de dados ainda est� aberta
   int              timerTempo;
   int              MaxJanelaDados;  //< m�xima janela de tempo para aguardar inser��o de dados do barramento
   TXMLComunicacao* xmlCom;          //< Arquivo XML para solicitar informa��es ao M�d. Supervis�o

   // Dados
   TAreaBusca*           areaBusca;      //< para determina��o da �rea de busca
   TClassificacao*       classificacao;  //< para a classifica��o da falta (fases)
   TDados*               dados;          //< objeto com o conjunto de dados do defeito
   TEstrategiaEvolutiva* estrEvol;       //< objeto para executar o algoritmo evolutivo
   TFuncoesDeRede*       funcoesRede;    //< objeto para opera��es com os objs de redes do Sinap
	TAlgoFasorial*        AlgoFasorial;   //< objeto de localiza��o de faltas com base em fasores V,I
	TAlgoRompCabo*        AlgoRompCabo;   //< objeto de localiza��o de rompimento de cabo

   // Dados
   TList* lisBarrasAreaBusca;         //< lista com as barras da �rea de busca j� definida
   TList* lisBarrasDefeitoFaultStudy; //< lista com as barras de solu��o por Fault Study

   // Dados para solu��o com a dist�ncia de falta do rel�
   VTBarra* barraSol_Dist;            //< barra do defeito para a solu��o de dist�ncia
   TList*   listaBarrasSol_Dist;      //< lista de barras para solu��o de dist�ncia

   // Dados para solu��o com o FaultStudy do OpenDSS
   TFaultStudyFL* FS;
   TOpcoesGraficas* OpGraf;           //< classe para mostrar resultados no gr�fico do Sinap

   // Dados para solu��o com o Algoritmo Evolutivo
   TConversorSinapDSS* conversor;

	// Solu��es finais
   TList* lisSolucoes;


protected:
	void __fastcall Execute();

public:

   // Construtor
	__fastcall TThreadFaultLocation(bool CreateSuspended, VTApl* apl, TFormFaultLocation* formFL, TGerenciadorEventos* gerenciador, TAlarme* alarme);
	__fastcall ~TThreadFaultLocation();

	// M�todos principais
	void __fastcall ExecutaFaultLocation_DMS1();        // Algoritmo para localizar faltas que n�o envolvem a terra (3F e 2F)
	void __fastcall ExecutaFaultLocation_DMS2();        // Algoritmo para localizar faltas que envolvem a terra (2FT e FT)
	void __fastcall ExecutaFaultLocation_DMS3();        // Algoritmo para localizar rompimentos de cabo (algoritmo 3)
	void __fastcall ExecutaLocRompCabo();
	void __fastcall GetBarras_ResLocalizacao(TList* lisBarrasAreaBusca, TStringList* lisIDsBarrasDefeito, TList* lisEXT);
	void __fastcall MostraResultadosTreeView();
	void __fastcall MostraResultadosTreeView_DMS1();
	void __fastcall MostraResultadosTreeView_DMS2(TList* lisStrSolucaoDMS2);
	void __fastcall MostraResultadosTreeView_DMS3(TList* lisStrSolucaoDMS3);
	void __fastcall MostraResultados();
	void __fastcall VerificaTipoLocalizacao();
	void __fastcall XMLGeraSolucoesRompCabo(TList* lisSolucoes, StrDadosGerais* strDadosGerais);
	void __fastcall XMLGeraSolucoesDMS1(TList* lisSolucoes, StrDadosGerais* strDadosGerais);
	void __fastcall XMLGeraSolucoesDMS2(TList* lisStrSolucaoDMS2, StrDadosGerais* strDadosGerais);
	void __fastcall XMLGeraSolucoesDMS3(TList* lisStrSolucaoDMS3, StrDadosGerais* strDadosGerais);
//	void __fastcall XMLExporta_SH();
	void __fastcall XMLSolicitaMedicoes();

   // M�todos de monitoramento
	void            __fastcall AddAlarme(TAlarme* alarme);
	void            __fastcall AtualizaTimer(int indexProcesso);
	bool            __fastcall DecideSolicitacaoMedicoes();
	bool            __fastcall ExistemMedicoes();
	TList*          __fastcall GetAlarmes();
	TAlarme*        __fastcall GetAlarmeGatilho();
	StrDadosGerais* __fastcall GetDadosGerais();
	StrDadosGerais* __fastcall GetDadosGeraisRompCabo();
	StrDadosGerais* __fastcall GetDadosGerais_DMS3();
   StrDadosLog*    __fastcall GetDadosLog();
   TList*          __fastcall GetListaAlarmes();
   bool            __fastcall JanelaAberta();
   void            __fastcall SalvaAlarme(TAlarme* alarme);

	// M�todos auxiliares
	void   __fastcall AjustaAlarmesEvento();
   void   __fastcall AjustaAlarmesFREL();
	void   __fastcall AtualizaCodigoEvento();
	bool   __fastcall CaboRompido();
	double __fastcall CorrenteFaltaMedida(StrFasor* fasorCorrente);
//   bool   __fastcall Eh_AtualizacaoAlarme(TAlarme* novoAlarme, TList* listaAlarmes);
	void   __fastcall EliminaAlarmesRepetidos();
	bool   __fastcall ExistemAlarmesQualimetros(bool qualimetroEmChaveProtecao);
   bool   __fastcall ExistemAlarmesTrafoInteligente();
	bool   __fastcall ExistemAlarmesProtecao();
   bool   __fastcall ExistemAlarmesSobrecorrenteQual();
	bool   __fastcall ExistemAlarmesSubtensao();
	bool   __fastcall ExistemAlarmesLastGasp();
	bool   __fastcall FaltaEnvolveTerra();   // DMS
	String __fastcall GetCodigoAlimentador();
	String __fastcall GetCodigoLigacaoAssociadaQualimetro(String cod_qualimetro);
	String __fastcall GetNomeArquivo();
	String __fastcall GetPathRootDir();
	TDateTime __fastcall GetTimeStamp(TAlarme* alarme);
	void   __fastcall InformacoesAgrupamentoAlarmes(TList* lisEXT);
	void   __fastcall InicializaDadosLog();
	void   __fastcall IniciaFlags();
	void   __fastcall OrdenaListaAlarmes();
	bool   __fastcall QualimetroChaveProtecao(String codAlimentador, String codQualimetro);
	void   __fastcall SetLViewMonitores(TListView* LViewMonitores);
	void   __fastcall SetMemoProcessosLF(TMemo* MemoProcessosLF);
	void   __fastcall SetMemoResultados(TMemo* MemoResultados);
	void   __fastcall SetMaxJanelaDados(int MaxJanelaDados);
};
//---------------------------------------------------------------------------
#endif
