//---------------------------------------------------------------------------
#ifndef FuncoesFLH
#define FuncoesFLH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
int __fastcall ContemSubstr(String strTotal, String substr);
bool __fastcall ContemSubstring(String strTotal, String substr);
int __fastcall CSVCamposCount(String linha, String separador);
String __fastcall DoubleToString(double val, int casasDec);
String __fastcall GetCampoCSV(String linha, int indiceCampo, String separador);
String __fastcall GetCampo(String linha, int indiceCampo, String separador);
String __fastcall GetSubstr(String strInicial, String strPos1, String strPos2);
int __fastcall GetPosSubstr(String strTotal, String substr);
int __fastcall NumeroCampos(String linha, String separador);
double __fastcall Round(double valIni, int ndigitos);
String __fastcall StrReplace(String strIni, int indice, String valor);
String __fastcall StrReplace(String strIni, String valorAnt, String valorNovo);
void get_all_files_names_within_folder(String folder, TStringList* listaArquivos);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
