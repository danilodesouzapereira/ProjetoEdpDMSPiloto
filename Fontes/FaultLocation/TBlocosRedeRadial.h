//---------------------------------------------------------------------------
#ifndef TBlocosRedeRadialH
#define TBlocosRedeRadialH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class VTApl;
class VTBarra;
class VTBloco;
class VTBlocos;
class VTChave;
class VTLigacao;
class VTOrdena;
class VTPath;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
struct StrBloco
{
	VTBloco* bloco;
	VTChave* chaveMont;
	StrBloco* strBlocoMontante;
	TList* lisStrBlocosJusante;
};
//---------------------------------------------------------------------------
class TBlocosRedeRadial
{
private:
	// Parâmetros básicos
	VTApl*    apl;
	VTBlocos* blocos;
   VTOrdena* ordena;
	VTPath*   path;
	VTRedes*  redes;

public:
	VTRede*   rede;
	TList* lisStrBlocos;

public:
	__fastcall TBlocosRedeRadial(VTApl* apl, VTRedes* redes, VTRede* rede);
	__fastcall ~TBlocosRedeRadial();

	void __fastcall AtualizaStrBlocos(StrBloco* strBlocoMont, StrBloco* strBlocoJus);
	bool __fastcall ExisteStrBlocoJusante(StrBloco* strBlocoMont, StrBloco* strBlocoJus);
	bool __fastcall ExisteStrBloco(StrBloco* strBlocoRef);
	void __fastcall Inicializa();
	void __fastcall GetBlocosJusanteLigacao(VTLigacao* ligacao, TList* lisEXT);
	StrBloco* __fastcall GetStrBloco(VTBloco* bloco);
	StrBloco* __fastcall GetStrBloco(VTLigacao* ligacao);
	StrBloco* __fastcall GetStrBlocoMontante(VTBloco* bloco);
	void __fastcall InserirStrBlocoJusante(StrBloco* strBlocoMontante, StrBloco* strBlocoJusante);
};
//---------------------------------------------------------------------------
#endif
