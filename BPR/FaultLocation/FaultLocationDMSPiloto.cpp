#include <vcl.h>
#include <windows.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include <ProjetoEdpDMSPiloto\Dll_Inc\FaultLocationDMSPiloto.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\TFormFaultLocation.h>
#pragma argsused
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
EXPORT TForm* __fastcall DLL_NewFormFaultLocation(TComponent *Owner, VTApl *apl_owner, TWinControl *parent)
{
	TForm *form;
	form = new TFormFaultLocation(Owner, apl_owner, parent);
	return(form);
}
//---------------------------------------------------------------------------
//eof
