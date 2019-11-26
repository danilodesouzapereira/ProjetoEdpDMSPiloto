//---------------------------------------------------------------------------
#ifndef VTConectaH
#define VTConectaH

//arquivos incluídos-----------------------------------------------------------
#include <Classes.hpp>

//---------------------------------------------------------------------------
class VTApl;

//---------------------------------------------------------------------------
class VTConecta : public TObject
   {
   public:
                   __fastcall  VTConecta(void) {};
      virtual      __fastcall ~VTConecta(void) {};
      virtual bool __fastcall  Executa(void) = 0;
   };

//---------------------------------------------------------------------------
VTConecta* __fastcall NewObjConecta(VTApl *apl_owner);

#endif
//---------------------------------------------------------------------------
//eof