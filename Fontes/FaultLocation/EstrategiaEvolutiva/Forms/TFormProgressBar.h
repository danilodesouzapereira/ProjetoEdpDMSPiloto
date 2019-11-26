//---------------------------------------------------------------------------
#ifndef TFormProgressBarH
#define TFormProgressBarH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormProgressBar : public TForm
{
	__published:
	TGroupBox *GroupBox1;
	TProgressBar *ProgressBarGeracoes;

public:
   // Construtor
   __fastcall TFormProgressBar(TComponent* Owner);

   // Métodos
   void __fastcall AtualizaMelhorResultado(double ConsInterH, double ConsInter, double END, double Compensacao);
   void __fastcall InicializaProgressBarIndividuos(int Valor);
   void __fastcall InicializaProgressBarGeracoes(int Valor);
   void __fastcall StepItProgressBarGeracoes(void);
   void __fastcall StepItProgressBarIndividuos(void);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof