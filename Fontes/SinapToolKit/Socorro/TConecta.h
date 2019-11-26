//---------------------------------------------------------------------------
#ifndef TConectaH
#define TConectaH

//arquivos incluídos-----------------------------------------------------------
#include <Classes.hpp>
#include "VTConecta.h"

//---------------------------------------------------------------------------
class VTBarra;
class VTChave;
//---------------------------------------------------------------------------
class TConecta : public VTConecta
   {
   public:
              __fastcall  TConecta(VTApl *apl_owner);
              __fastcall ~TConecta(void);
      bool    __fastcall  Executa(void);

   private: //métodos
	  VTBarra* __fastcall ExisteBarraDeMesmaCoordenada(VTBarra *bar_ref, TList *lisBAR, VTChave *chave);
	  void	   __fastcall TransfereEqptosDeAparaB(VTBarra *barraA, VTBarra *barraB);

   private: //objetos externos
      VTApl *apl;

   private: //dados locais
      TList  *lisCHV;
	  TList  *lisBAR_ISO;
   };

#endif
//---------------------------------------------------------------------------
//eof