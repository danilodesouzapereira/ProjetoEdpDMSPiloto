//---------------------------------------------------------------------------
#ifndef FaultLocationDMSPilotoH
#define FaultLocationDMSPilotoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#define EXPORT extern "C++" __declspec(dllexport)
//-----------------------------------------------------------------------------
class VTApl;
//-----------------------------------------------------------------------------
EXPORT TForm* __fastcall DLL_NewFormFaultLocation(TComponent *Owner, VTApl *apl_owner, TWinControl *parent);
//---------------------------------------------------------------------------
#endif
//eof