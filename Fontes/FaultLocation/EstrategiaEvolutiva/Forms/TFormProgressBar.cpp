//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormProgressBar.h"
#pragma package(smart_init)
#pragma resource "*.dfm"
#include <PlataformaSinap\DLL_Inc\Funcao.h>
//---------------------------------------------------------------------------
__fastcall TFormProgressBar::TFormProgressBar(TComponent* Owner) : TForm(Owner)
{
//	AtualizaMelhorResultado(0., 0., 0., 0.);
}
//---------------------------------------------------------------------------
void __fastcall TFormProgressBar::AtualizaMelhorResultado(double ConsInterH, double ConsInter, double END, double Compensacao)
{
//   LabelConsInterH->Caption  = DoubleToStr("%5.4f", ConsInterH) + " Horas";
//   LabelConsInter->Caption   = DoubleToStr("%5.4f", ConsInter)  + " Consumidores";
//   LabelEND->Caption         = DoubleToStr("%5.4f", END)        + " kWh";
//   LabelCompensacao->Caption = "R$" + DoubleToStr("%6.2f", Compensacao);
}
//---------------------------------------------------------------------------
void __fastcall TFormProgressBar::InicializaProgressBarGeracoes(int Valor)
{
   ProgressBarGeracoes->Max      = Valor;
   ProgressBarGeracoes->Position = 0;
}
//---------------------------------------------------------------------------
void __fastcall TFormProgressBar::InicializaProgressBarIndividuos(int Valor)
{
//   ProgressBarIndividuos->Max      = Valor;
//   ProgressBarIndividuos->Position = 0;
}
//---------------------------------------------------------------------------
void __fastcall TFormProgressBar::StepItProgressBarGeracoes(void)
{
   ProgressBarGeracoes->StepIt();
   Refresh();
   Show();
}
//---------------------------------------------------------------------------
void __fastcall TFormProgressBar::StepItProgressBarIndividuos(void)
{
//   ProgressBarIndividuos->StepIt();
//   Refresh();
//   Show();
}
//---------------------------------------------------------------------------
//eof
