object FormStkChild: TFormStkChild
  Left = 0
  Top = 0
  Align = alRight
  BorderIcons = []
  BorderStyle = bsSizeToolWin
  Caption = 'Sinap Tool Kit: Neoenergia'
  ClientHeight = 266
  ClientWidth = 273
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
    Left = 80
    Top = 40
    object ActionImportaRedeGIS: TAction
      Caption = 'Importar Redes do GIS'
      OnExecute = ActionImportaRedeGISExecute
    end
    object ActionOcorrencias: TAction
      Caption = 'Ocorr'#234'ncias'
      Hint = 'Gerenciamento de ocorr'#234'ncias de interrup'#231#227'o.'
      ImageIndex = 6
    end
    object ActionEnumeracao: TAction
      Caption = 'Enumera Religadores'
      Hint = 
        'Executa enumera'#231#227'o de alternativas para a aloca'#231#227'o de religadore' +
        's.'
      ImageIndex = 7
    end
    object ActionAlocaChaves: TAction
      Caption = 'Aloca Religadores'
      Hint = 'Executa aloca'#231#227'o otimizada de religadores.'
      ImageIndex = 1
    end
    object ActionCompensacoes: TAction
      Caption = 'C'#225'lculo de Compensa'#231#245'es'
      Hint = 
        'Executa estimativa anual de pagamentos despendidos por meio de c' +
        'ompensa'#231#245'es.'
      ImageIndex = 2
    end
    object ActionPiscadas: TAction
      Caption = 'Interrup'#231#245'es Tempor'#225'rias'
      Hint = 'Avalia'#231#227'o de interrup'#231#245'es tempor'#225'rias.'
      ImageIndex = 4
    end
    object ActionSimTurmas: TAction
      Caption = 'Aloca'#231#227'o de Turmas'
    end
    object ActionPerdasComerciais: TAction
      Caption = 'Perdas Comerciais'
    end
    object ActionImportaAccess: TAction
      Caption = 'Importar redes Access'
      OnExecute = ActionImportaAccessExecute
    end
    object ActionAlocaIdentificadorFalta: TAction
      Caption = 'Aloca Identificador de Falta'
    end
    object ActionCalcIndContTelecom: TAction
      Caption = 'Cosimula'#231#227'o'
      Visible = False
    end
    object ActionGerenciadorCenarioTelecom: TAction
      Caption = 'Gerenciador de cen'#225'rios de Telecom'
      Visible = False
    end
  end
  object ActionList1: TActionList
    Left = 24
    Top = 104
    object ActionRedeCarregada: TAction
      Caption = 'ActionRedeCarregada'
      OnExecute = ActionRedeCarregadaExecute
    end
  end
end
