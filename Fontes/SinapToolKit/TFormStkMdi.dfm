object FormStkMdi: TFormStkMdi
  Left = 0
  Top = 0
  Align = alRight
  BorderIcons = []
  BorderStyle = bsSizeToolWin
  Caption = 'Sinap Tool Kit: AES Eletropaulo'
  ClientHeight = 266
  ClientWidth = 330
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poDesigned
  PixelsPerInch = 96
  TextHeight = 13
  object ActionList: TActionList
    Left = 16
    Top = 8
    object ActionImportaGis: TAction
      Caption = 'Importar rede do sistema GIS'
      OnExecute = ActionImportaGisExecute
    end
    object ActionImportaANEEL: TAction
      Caption = 'Importar rede de arquivos ANEEL'
      OnExecute = ActionImportaANEELExecute
    end
  end
end
