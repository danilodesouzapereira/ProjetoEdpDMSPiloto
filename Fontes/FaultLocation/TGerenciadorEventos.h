//---------------------------------------------------------------------------
#ifndef TGerenciadorEventosH
#define TGerenciadorEventosH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TAlarme;
class TFormFaultLocation;
class TThreadFaultLocation;
class TXMLParser;
class VTApl;
class VTChave;
class VTPath;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
class TGerenciadorEventos
{
public:
   // Configurações
   bool       CadSensoresInicio;
   int        PassoMonitDirImporta;
   int        MaxJanelaDados;
   int        TimeoutSegundos;
   int        AgrupamentoAlarmesSegundos;
   TThreadFaultLocation* FL_Executando;   //< Thread em execução no momento

private:
	// Param. elementares
   TFormFaultLocation* formFL;
   TXMLParser*         xmlParser;
   VTApl*              apl;
   VTPath*             path;
   VTRedes*            redes;

   // Auxiliares
   bool       AguardandoSensores;
   TListView* LViewMonitores;
   TMemo*     MemoResultados;
   TMemo*     MemoProcessosLF;

  	// Dados
   TList*       listaAlarmes;
   TList*       listaFL;                  //< Lista de instâncias de localizadores de falta
   TStringList* listaArquivosAdicionados; //< Lista com os códigos dos arquivos do barramento
   TStringList* listaArquivosNovos;       //< Lista auxiliar para os arquivos novos do diretório

   // Cadastro de sensores
   TStringList* listaXMLCadSensores;      //< Lista com os códigos dos XML de solicitação de cadastro
   String       xmlRespCadSensores;       //< Nome do XML com o cadastro de sensores solicitado

public:
	// Construtor e destrutor
   __fastcall TGerenciadorEventos(VTApl* apl, TFormFaultLocation* formFL);
   __fastcall ~TGerenciadorEventos(void);

 	// Métodos
   bool __fastcall   AguardandoCadastroSensores();
   bool __fastcall   AlarmeConsistente(TAlarme* alarme);
   void __fastcall   AtualizaCadastroDisjuntores(TStringList* lisLinhas);
   void __fastcall   AtualizaCadastroReligadoras(TStringList* lisLinhas);
   void __fastcall   AtualizaTimers();   //< Atualiza os timers dos objs TFaultLocation
   bool __fastcall   CadastroContemReligadora(TStringList* lisLinhas, VTChave* chaveReligadora);
   bool __fastcall   CadastroContemRede(TStringList* lisLinhas, VTRede* rede);
   void __fastcall   CadSensores_Fake(String TimeStamp);
   VTChave* __fastcall DisjuntorRede(VTRede* rede);
	bool __fastcall   ExisteRespostaCadSensores(); //< Verifica se o mod. de superv. retornou o XML com o cadastro de sensores
   bool __fastcall   ExisteAlarmeNovo();  //< DirEntrada: XMLs do barramento
   String __fastcall GetStrTimeStamp();  //< Retorna o timestamp no formato: 20171116150800 (<ano><mes><dia><hora><minuto><segundo>)
   void __fastcall   InicializaArquivosApoio();
	void __fastcall   LeConfigGerais();
	void __fastcall   ListaMonitores(TListView* LViewMonitores);
	void __fastcall   ListaObjFaultLocation(TListView* LViewEventos);
	void __fastcall   MostrarAreaBusca(void* objFL);
   void __fastcall   VerificaNovosDados();
   TThreadFaultLocation* __fastcall ProcuraObjFaultLocation(String CodigoEvento);
	TThreadFaultLocation* __fastcall ProcuraObjFaultLocation(TAlarme* alarme);
   void __fastcall   ReligadorasRede(VTRede* rede, TList* lisEXT);
	void __fastcall   SalvaArquivoCadastroSensores();
	void __fastcall   SetLViewMonitores(TListView* LViewMonitores);
	void __fastcall   SetMemoResultados(TMemo* MemoResultados);
   void __fastcall   SetMemoProcessosLF(TMemo* MemoProcessosLF);
   bool __fastcall   Sincronismo(TAlarme* alarmeFL, TAlarme* alarme);
   bool __fastcall   Sincronismo(TThreadFaultLocation* FL, TAlarme* alarme);
	void __fastcall   VerificaCadastroSensores();
   void __fastcall   VerificaDiretoriosAuxiliares();
	void __fastcall   XMLCadastroSensores();

private:
	void __fastcall   ResetDir();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
