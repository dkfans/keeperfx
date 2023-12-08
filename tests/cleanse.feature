Feature Cleanse Ability

    Scenario: cleanse effect
        When a creature uses cleanse
        Then I should see that the creature has the cleanse effect
        And the duration should last for the time specified in the configuration

    Scenario Outline: applying <Spell> and creature cleansing
        Given a creature in a normal state
        When the <Spell> spell is cast on the creature
        And the creature has the ability to cleanse
        And the cleanse is off cooldown
        Then the creature uses cleanse
        And the creature is no longer affected by <Spell>
        
        # Note: These spells can be applied via magic, creature abilities, or commands
        Examples:
            | Spell   |
            | Chicken |
            | Disease |
            | Slow    |
            | Freeze  |
		
    Scenario Outline: applying <Spell> with cleanse activated
        Given a creature has cleanse activated
        When the <Spell> spell is cast on the creature
        Then the creature is not affected by the <Spell>
        
        # Note: These spells can be applied via magic, creature abilities, or commands
        Examples:
            | Spell   |
            | Chicken |
            | Disease |
            | Slow    |
            | Freeze  |
		
    Scenario Outline: applying <Spell> and creature not cleansing
        Given a creature in a normal state
        When the <Spell> spell is cast on the creature
        And the creature has the ability to cleanse
        And the cleanse is on cooldown
        Then the creature does not cleanse
        And the creature is still affected by <Spell>

        # Note: These spells can be applied via magic, creature abilities, or commands
        Examples:
            | Spell   |
            | Chicken |
            | Disease |
            | Slow    |
            | Freeze  |
			
    Scenario Outline: cleansing in first-person
        Given I have a creature possessed
        And the <Spell> spell is cast on me
        And I have the ability to cleanse
        And the cleanse is off cooldown
        When I use the cleanse ability
        Then I am no longer affected by the <Spell>

        Examples:
            | Spell   |
            | Chicken |
            | Disease |
            | Slow    |
            | Freeze  |

    Scenario: applying cleanse via command
        Given I have a queried creature
        When I use the cleanse command
        Then the creature should have cleanse activated
        And I should see that the creature has the cleanse effect
            
    Scenario: cleanse deactivated when imprisoned
        Given a creature has cleanse activated
        When a creature has been put in prison
        Then the cleanse should be deactivated

    Scenario: cleanse deactivated when tortured
        Given a creature has cleanse activated
        When they are selected for torture
        Then the cleanse should be deactivated
		
    Scenario: creature stopped by hand
        Given a creature is being stopped by hand
        And the creature has the ability to cleanse
        And the cleanse is off cooldown
        And the creature is frozen
        When the hand is no longer stopping the creature
        Then the creature should use cleanse
		
    Scenario: applying cleanse via scripting
        Given a creature in a normal state
        When USE_SPELL_ON_PLAYERS_CREATURES executes with cleanse on the creature type
        Then that creature type should have cleanse activated

    Scenario: diggers not cleansing on the job
        Given I have an Imp
        And this Imp has the ability to cleanse
        And the cleanse is off cooldown
        When I give a job for the Imp to perform
        Then the Imp should not use cleanse

    Scenario: cleanse removed on death
        Given a creature has cleanse active
        When this creature dies
        Then I should no longer see the cleanse effect active
