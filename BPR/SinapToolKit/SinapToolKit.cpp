//---------------------------------------------------------------------------
#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Licenca\VTEmpresa.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\SinapToolKit.h>
#include <ProjetoEdpDMSPiloto\Fontes\SinapToolKit\TFormStkChild.h>
#include <ProjetoEdpDMSPiloto\Fontes\SinapToolKit\TFormStkMdi.h>
#pragma argsused
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
   {
   return 1;
   }
//-----------------------------------------------------------------------------
EXPORT bool __fastcall DLL_AtualizaTelas(TComponent *Owner)
	{
	//variáveis locais
	TForm *form;

	try{//verifica se existe TFormStkChild
		if ((form = ExisteForm("TFormStkChild", Owner)) == NULL) return(false);
			{//atualiza telas
			((TFormStkChild*)form)->Atualiza();
         }
      }catch(Exception &e)
         {
         return(false);
         }
   return(true);
	}
//-----------------------------------------------------------------------------
EXPORT int __fastcall DLL_Empresa_ID(void)
	{
	return(COELBA);
	}
//-----------------------------------------------------------------------------
EXPORT AnsiString __fastcall DLL_Empresa_Codigo(void)
	{
	return("COELBA");
	}
//-----------------------------------------------------------------------------
EXPORT bool __fastcall DLL_FormChildStart(TComponent *Owner, VTApl *apl_owner, int stk_option)
   {
	try{//verifica se existe TFormStkChild
		if (! ExisteForm("TFormStkChild", Owner))
			{//cria TFormStkChild sem exibir
			new TFormStkChild(Owner, apl_owner, stk_option);
			}
      }catch(Exception &e)
         {
         return(false);
         }
   return(true);
	}
//-----------------------------------------------------------------------------
EXPORT bool __fastcall DLL_FormChildStop(TComponent *Owner)
	{
	//variáveis locais
	TForm *form;

	try{//verifica se existe TFormStkChild
		if ((form = ExisteForm("TFormStkChild", Owner)) != NULL)
			{//destró Form
			delete form;
			}
		}catch(Exception &e)
			{
			return(false);
         }
   return(true);
	}
//-----------------------------------------------------------------------------
EXPORT bool __fastcall DLL_FormMdiStart(TComponent *Owner, VTApl *apl_owner)
	{
	try{//verifica se existe TFormStkMdi
		if (! ExisteForm("TFormStkMdi", Owner))
			{//cria TFormStkMdi sem exibir
			new TFormStkMdi(Owner, apl_owner);
			}
		}catch(Exception &e)
			{
			return(false);
			}
	return(true);
	}
//-----------------------------------------------------------------------------
EXPORT bool __fastcall DLL_FormMdiStop(TComponent *Owner)
	{
	//variáveis locais
	TForm *form;

	try{//verifica se existe TFormStkMdi
		if ((form = ExisteForm("TFormStkMdi", Owner)) != NULL)
			{//destró Form
			delete form;
			}
		}catch(Exception &e)
			{
			return(false);
			}
	return(true);
	}
//---------------------------------------------------------------------------
//eof


