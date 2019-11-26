//---------------------------------------------------------------------------
#ifndef TEqptoCampoH
#define TEqptoCampoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTEqpto;
//---------------------------------------------------------------------------
/***
 * Enumeradores para os tipos de eqptos
 **/
enum TipoEqptoCampo
{
	tipoINDEF = -1,
	chaveDJ,               //< Disjuntor
	chaveRE,               //< Religadora
	chaveSC,               //< Seccionadora
	chaveFU,               //< Fus�vel
	eqptoSENSOR,           //< Sensor
	eqptoQUALIMETRO,       //< Qual�metro
	eqptoITRAFO,           //< ITrafo
	eqptoBARRA_SEM_TENSAO, //< Barra MT agregando a informa��o de falta de tens�o
	eqptoMI                //< Medidor inteligente
};
//---------------------------------------------------------------------------
/***
 * Enumeradores para os tipos de medidores inteligentes
 **/
enum TipoMedidorInteligente
{
	tipoMIINDEF = -1,
	miBALANCO,       //< MI instalado em tranformador MT/BT
	miCARGAMT,       //< MI instalado em carga MT
	miCARGABT        //< MI instalado em carga BT
};
//---------------------------------------------------------------------------
enum EstadoChave
{
	estadoINDEF = -1, estadoABERTO, estadoFECAHDO
};
//---------------------------------------------------------------------------
class TEqptoCampo : public TObject
{
public:
   // Par�metros elementares
   int      Tipo;
	String   Codigo;

   // Dados
	TList*   lisBlocosJusante;  //< Lista com os blocos � jusante do eqpto
   TList*   lisAlarmes;        //< Lista com os alarmes emitidos pelo equipamento

	// Construtor e destrutor
   __fastcall TEqptoCampo(String Codigo, int Tipo);
   __fastcall ~TEqptoCampo(void);

   // M�todos
   TList*   __fastcall GetBlocosJusante();
   String   __fastcall GetCodigo();
	int      __fastcall GetTipo();
   void     __fastcall SetBlocosJusante(TList* lisBlocosJusante);
	void     __fastcall SetTipo(int Tipo);
};
//---------------------------------------------------------------------------
#endif
