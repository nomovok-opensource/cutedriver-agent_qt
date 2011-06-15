@qt_linux @qt_windows @qt_symbian @qt_meego

Feature: QtTasServer with Cucumber Wire Protocol
  As a test scripter writer
  I want to use Cucumber Wire Protocol to talk to QtTasServer
  so that I can do tests with QtScript and C++

  Scenario: Testing stuff
    Given I launch application with command calculator -testability
    Given I launch application two with command:
        | calculator |
        | -testability |
    And I launch application four with command:
        | calculator |
        | -testability |
    And I perform pairs:
	| col1 | col 2 |
	| 11   | 12    |
	| 21   | 22    |
	| 31   | 32    |
    And I find application three running with name calculator
    And I select application
    Then current application is calculator
    And I select application two	
    Then current application is calculator
    And I select application three	
    Then current application is calculator
    And I select application nocando
    Then I click button foobar