//---------------------------------------------------------------------------
#ifndef VTConectaMaeFilhaH
#define VTConectaMaeFilhaH

//arquivos incluídos-----------------------------------------------------------
#include <Classes.hpp>

//---------------------------------------------------------------------------
class VTApl;

//---------------------------------------------------------------------------
class VTConectaMaeFilha : public TObject
   {
   public:
				   __fastcall  VTConectaMaeFilha(void) {};
	  virtual      __fastcall ~VTConectaMaeFilha(void) {};
	  virtual bool __fastcall  Executa(void) = 0;
   };

//---------------------------------------------------------------------------
VTConectaMaeFilha* __fastcall NewObjConectaMaeFilha(VTApl *apl_owner);

#endif
//---------------------------------------------------------------------------
//eof