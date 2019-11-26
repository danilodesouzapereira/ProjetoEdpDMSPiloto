object FormExportaBlocos: TFormExportaBlocos
  Left = 0
  Top = 0
  Caption = 'Exportar blocos da rede'
  ClientHeight = 233
  ClientWidth = 476
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 40
    Top = 72
    Width = 105
    Height = 49
    Action = ActionExportaBlocos
    TabOrder = 0
  end
  object ActionList1: TActionList
    Left = 200
    Top = 80
    object ActionExportaBlocos: TAction
      Caption = 'Exporta blocos'
      OnExecute = ActionExportaBlocosExecute
    end
  end
end
