@qt_linux @qt_windows @qt_symbian @qt_meego

Feature: QtTasServer with Cucumber Wire Protocol
  As a test scripter writer
  I want to use Cucumber Wire Protocol to talk to QtTasServer
  so that I can do tests with QtScript and C++

  Scenario: Testing stuff    
    Given I launch application two with command:
        | calculator |
        | -testability |
    And I call click on application object threeButton
    And I call click on application object plusButton
    And I call click on application object fiveButton
    And I call click on application object equalButton
    Then application object display has text "8"
    And I launch application four with command:
        | calculator |
        | -testability |
    And I find application three running with name calculator
    And I select application three
    And I select application two	
    And I select application threex
