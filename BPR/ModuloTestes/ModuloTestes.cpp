#include <vcl.h>
#include <windows.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include <ProjetoEdpDMSPiloto\Fontes\ModuloTestes\TFormModuloTestes.h>
#include <ProjetoEdpDMSPiloto\Dll_Inc\FaultLocationDMSPiloto.h>
#pragma argsused
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
EXPORT TForm* __fastcall DLL_NewFormModuloTestes(TComponent *Owner, VTApl *apl_owner, TWinControl *parent)
{
	TForm *form;
	form = new TFormModuloTestes(Owner, apl_owner, parent);
	return(form);
}
//---------------------------------------------------------------------------
