//---------------------------------------------------------------------------
#ifndef TFormFaultLocationH
#define TFormFaultLocationH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.Menus.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.Dialogs.hpp>
#include <System.ImageList.hpp>
//---------------------------------------------------------------------------
class TAlgoFasorial;
class TConfigRede;
class TFaultLocationLote;
class TFormProgressBar;
class TFuncoesDeRede;
class TGeraDefeito;
class TGerenciadorEventos;
class VTApl;
class VTChave;
class VTGrafico;
class VTPath;
//---------------------------------------------------------------------------
class TFormFaultLocation : public TForm
{
__published:	// IDE-managed Components
	TImageList *ImageList;
	TPopupMenu *PopupMenu;
	TMenuItem *mnuAlignLeft;
	TMenuItem *mnuAlignRight;
	TMenuItem *mnuAlignDown;
	TMenuItem *mnuAlignUp;
	TMenuItem *mnuAlignNode;
	TStatusBar *StatusBarCount;
	TTimer *Timer;
	TPageControl *PageControl;
	TTabSheet *TabSheetLFTempoReal;
	TTabSheet *TabSheetLFLote;
	TGroupBox *gbResultados;
	TListView *lvResultados;
	TToolBar *tbFLLote;
	TToolButton *tbCarregarArquivos;
	TTabSheet *tsAuxiliar;
	TGroupBox *GroupBox1;
	TEdit *edtModVa;
	TEdit *edtModVb;
	TEdit *edtModVc;
	TEdit *edtFaseVa;
	TEdit *edtFaseVb;
	TEdit *edtFaseVc;
	TEdit *edtModIa;
	TEdit *edtFaseIa;
	TEdit *edtModIb;
	TEdit *edtFaseIb;
	TEdit *edtModIc;
	TEdit *edtFaseIc;
	TLabel *Label1;
	TLabel *Label2;
	TButton *btnCalcZtotal;
	TEdit *edtRtotal;
	TButton *btnCalcBarras;
	TEdit *edtCodigoRede;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *edtZtotal;
	TLabel *Label5;
	TEdit *edtXtotal;
	TButton *btnFiltroRtotal;
	TButton *Button1;
	TEdit *edtZtotal_3F;
	TButton *Button2;
	TButton *Button3;
	TGroupBox *GroupBox2;
	TEdit *edtPathArquivo;
	TButton *Button4;
	TLabel *Label6;
	TGroupBox *GroupBox3;
	TLabel *Label7;
	TEdit *edtPathArquivoChaves;
	TButton *Button5;
	TGroupBox *GroupBox4;
	TLabel *Label8;
	TEdit *edtCodTrechos;
	TButton *btnCalculaCompTotal;
	TLabel *lblCompTotal;
	TButton *Button6;
	TButton *Button7;
	TEdit *edtZtotal_2F;
	TOpenDialog *OpenDialog1;
	TTabSheet *tsGeraDefeito;
	TGroupBox *gbConfigDefeito;
	TLabel *Label9;
	TEdit *edtCodigoBarraDefeito;
	TLabel *Label10;
	TEdit *edtTipoDefeito;
	TLabel *Label11;
	TEdit *edtRfalta;
	TLabel *Label12;
	TEdit *edtCodigoAlimentador;
	TButton *btnGeraDefeito;
	TLabel *Label13;
	TEdit *edtCaminhoDSS;
	TLabel *Label14;
	TGroupBox *GroupBox5;
	TMemo *memoResultadosGeraDefeito;
	TButton *Button8;
	TEdit *edtPathTxtResultados;
	TToolBar *tbFLTempoReal;
	TToolButton *ToolButton1;
	TGroupBox *GroupBox6;
	TLabel *Label15;
	TEdit *edtPathSaidaExpBlocos;
	TButton *Button9;
	TLabel *Label16;
	TEdit *edtLigaRef;
   TPanel *pnlFLTempoReal;
   TSplitter *Splitter1;
   TGroupBox *GBoxEventos;
   TMemo *MemoProcessosLF;
   TGroupBox *GBoxProcessosLF;
   TTreeView *TreeViewProcessosLF;
	void __fastcall butAlignClick(TObject *Sender);
	void __fastcall mnuAlignLeftClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall ToolButtonDistanciaClick(TObject *Sender);
	void __fastcall ToolButtonExpBlocosClick(TObject *Sender);
	void __fastcall ToolButtonExpLigacoesClick(TObject *Sender);
	void __fastcall mnuAlignRightClick(TObject *Sender);
	void __fastcall mnuAlignDownClick(TObject *Sender);
	void __fastcall mnuAlignUpClick(TObject *Sender);
	void __fastcall mnuAlignNodeClick(TObject *Sender);
	void __fastcall tbIniciaLFClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall TreeViewProcessosLFMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall ToolButton1Click(TObject *Sender);
	void __fastcall tbCarregarArquivosClick(TObject *Sender);
	void __fastcall btnCalcBarrasClick(TObject *Sender);
	void __fastcall btnCalcZtotalClick(TObject *Sender);
	void __fastcall btnFiltroRtotalClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall Button7Click(TObject *Sender);
	void __fastcall btnGeraDefeitoClick(TObject *Sender);
	void __fastcall Button8Click(TObject *Sender);
	void __fastcall Button9Click(TObject *Sender);



private:	// User declarations

	// Parâmetros elementares
	VTApl*          apl;
	VTPath*         path;
   VTGrafico*      graf;
   TFuncoesDeRede* funcoesRede;
	TGeraDefeito*   geraDefeito;

   // Parâmetros
   int                  tempo;                 //< Tempo, em segundos
   double               tempoTotal;            //< Acumula o tempo total de execução da solução, em horas
   int                  PassoMonitDirImporta;  //< Passo de tempo (segundos) para monitoramento da pasta Importa/FaultLocation/Alarmes
   TGerenciadorEventos* gerEventos;            //< Objeto gerenciador dos eventos/processos de LF
   TFaultLocationLote*  FLLote;
   TFormProgressBar*    FormPG;

   TAlgoFasorial*       AlgoFasorial;
   TConfigRede*       configRede;          //< Objeto para configuração de Pot. de curto e param. de trafos SE

public:		// User declarations

	// Construtor e destrutor
	__fastcall TFormFaultLocation(TComponent *Owner, VTApl *apl_owner, TWinControl *parent);
  	__fastcall ~TFormFaultLocation(void);

   // Métodos
   TFormProgressBar* __fastcall CriaFormProgressBar();
	void     __fastcall SetPassoMonitDirImporta(int PassoMonitDirImporta);

	int      __fastcall CodigoAlgoritmo(TTreeNode* node);
	void     __fastcall DestacarGrafico_DMS1(TTreeNode* node);
	void     __fastcall DestacarGrafico_DMS2(TTreeNode* node);
	void     __fastcall DestacarGrafico_DMS3(TTreeNode* node);
	VTChave* __fastcall GetChave(String codChave);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormFaultLocation *FormFaultLocation;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
