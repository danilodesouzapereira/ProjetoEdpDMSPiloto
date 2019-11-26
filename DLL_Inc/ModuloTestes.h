//---------------------------------------------------------------------------
#ifndef ModuloTestesH
#define ModuloTestesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#define EXPORT extern "C++" __declspec(dllexport)
//-----------------------------------------------------------------------------
class VTApl;
//-----------------------------------------------------------------------------
EXPORT TForm* __fastcall DLL_NewFormModuloTestes(TComponent *Owner, VTApl *apl_owner, TWinControl *parent);
//---------------------------------------------------------------------------
#endif
//eof