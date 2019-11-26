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
  ExplicitWidth = 320
  ExplicitHeight = 240
  PixelsPerInch = 96
  TextHeight = 13
  object ActionList: TActionList
    Left = 16
    Top = 8
    object ActionImportaOracle: TAction
      Caption = 'Importar redes GIS/Oracle'
      OnExecute = ActionImportaOracleExecute
    end
    object ActionImportaAccess: TAction
      Caption = 'Importar redes base Access'
      Visible = False
      OnExecute = ActionImportaAccessExecute
    end
  end
end
