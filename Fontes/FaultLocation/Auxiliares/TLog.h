//---------------------------------------------------------------------------
#ifndef TLogH
#define TLogH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TLog
{
private:
	// Dados
   TStringList* lisConteudo;
   String caminho;

public:
	// Construtores e destrutor
   __fastcall TLog();
   __fastcall TLog(String caminho);
   __fastcall ~TLog();

   // Métodos
	void __fastcall AddLinha(String linha, bool pulaLinha = false);
   void __fastcall ImprimeLog();
   void __fastcall Reinicia();
	void __fastcall SetCaminho(String caminho);
   bool __fastcall Vazio();

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
