Feature: QtTasServer with Cucumber Wire Protocol
  As a test scripter writer
  I want to use Cucumber Wire Protocol to talk to QtTasServer
  so that I can do tests with QtScript and C++

  Scenario: Testing with Calculator

    Given I launch application one with command:
        | calculator |
        | -testability |
    And I call click on application object fourButton
    And I call click on application object plusButton
    And I call click on application object fiveButton
    And I call click on application object equalButton
    Then application object display has text "9"

    And I launch application two with command calculator -testability
    And I call click on application object threeButton
    And I call click on application object plusButton
    And I call click on application object fiveButton
    And I call click on application object equalButton
    Then application object display has text "8"

    And I select application one
    And I call click on application object threeButton
    And I call click on application object plusButton
    And I call click on application object fiveButton
    And I call click on application object equalButton
    Then application object display has text "81"

  @ignore
  Scenario: Testing QML
    Given I launch application foo with command:
      | calculator |
      | -testability |
    Then I call click on application object fiveButton
    Given I launch application with command:
     | /home/arhyttin/test/simpleqmlapp-build-desktop/simpleqmlapp |
     |-testability |
     Then application object objectName has toString() "mainwindow"
     And application object objectName has toString() "mainwindow"
     Then I call click on application object mainrect.quitarea
