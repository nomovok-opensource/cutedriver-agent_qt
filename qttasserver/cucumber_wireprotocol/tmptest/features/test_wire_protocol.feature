Feature: QtTasServer with Cucumber Wire Protocol
  As a test scripter writer
  I want to use Cucumber Wire Protocol to talk to QtTasServer
  so that I can do tests with QtScript and C++

  @calc
  Scenario: Testing with Calculator

    Given I launch application one with command:
        | calculator |
        | -testability |
    And I do click on object fourButton
    And I do click on object plusButton
    And I do click on object fiveButton
    And I do click on object equalButton
    Then object display has property text value "9"

    And I launch application two with command calculator -testability
    And I do click on object threeButton
    And I do click on object plusButton
    And I do click on object fiveButton
    And I do click on object equalButton
    Then object display has property text value "8"

    And I select application one
    And I do click on object plusButton
    And I do click on object fiveButton
    And I do click on object equalButton
    Then object display has property text value "14"

  @qml
  Scenario: Testing QML
    Given I launch application foo with command:
      | calculator |
      | -testability |
    Then I do click on object fiveButton
    Given I launch application with command:
      | /home/arhyttin/test/simpleqmlapp-build-desktop/simpleqmlapp |
      |-testability |
    Then object objectName has property toString() value "mainwindow"
    And I do toString() on property objectName to get "mainwindow"
    Then I do click on object mainrect.quitarea

  @simple
  Scenario: Testing repeated app start and implicit close
    Given I launch application two with command calculator -testability
