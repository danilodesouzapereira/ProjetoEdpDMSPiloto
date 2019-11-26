//---------------------------------------------------------------------------
#ifndef TClusterH
#define TClusterH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTBloco;
class VTChave;
class VTLigacao;
//---------------------------------------------------------------------------
class TCluster
{
public:
   // Construtor e destrutor
	__fastcall TCluster(VTChave* chaveMontante);
	__fastcall ~TCluster();

   // Métodos
   void __fastcall ImprimeLigacoes(String pathArquivo);
	void __fastcall SetBloco(VTBloco* bloco);
	void __fastcall SetBlocos(TList* lisBlocos);
   TList* __fastcall GetBlocos();


   // Dados
//   TList* lisLigacoes;
   TList* lisBlocos;
   VTChave* chaveMontante;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
