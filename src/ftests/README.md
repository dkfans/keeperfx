Target Audience: Software Engineers

This is proof of concept; the goal is primarily for setting up scenarios to replicate bugs or test implementation of new features.

Added benefit is that if tests are setup in a continuous integration environment we can automatically know when we break behaviours when adding new features.

The functional tests should run fast, hence `frame_skip = 8` is used by default.
They also skip the trademark/cutscene by default for super fast launch.

# <span style="color:yellow">WIP Issues</span>

- <span style="color:orange">You must manually copy the [ftests campaign folder](../../ftests/) to your keeperfx directory</span>
    - this means placing it adjacent to the `levels folder` and `keeperfx.exe`
    - this is not ideal; it will potentially be automated in the future

# Quickstart

## Available Test Names

- `example_template_test`
- `bug_imp_teleport_attack_door`
- `bug_pathing_pillar_circling`
- `bug_imp_goldseam_dig`

## Run Existing Test

1. Enable Functional Testing
    - <b>CTRL+SHIFT+B</b> (VSCode shortcut for switching debug option)
    - select one of the FTEST_DEBUG options
        - FTEST_DEBUG=1 for debugging info
        - FTEST_DEBUG=0 for without
2. Update [launch.json](../../.vscode/launch.json)
    - add `-ftests` argument *(optionally provide the name of an existing test to run)*

        | without test name | with test name |
        |----------|-------------|
        | `"args": [`<br/>`"-ftests"`<br/>`],` | `"args": [`<br/>`"-ftests", "bug_imp_teleport_attack_door"`<br/>`],` |

3. Run game - Watch test execute
    - it will be x8 speed by default, and exit to desktop very quickly
        - use
4. View keeperfx exit code for test result
    - exit code 0 == `test success`
    - exit code -1 == `test failure`
    - optionally view the keeperfx.log to view details on why the test failed
        - example failure message: `FTest: [20] ftest_template_action001__spawn_imp: Failed to level up imp`
        - the above message tells us that at game turn 20, the test failed at function `ftest_template_action001__spawn_imp` because `Failed to level up imp`

## Create New Test

1. Copy example test files and rename them to reflect your test
    - [ftest_template.h](./tests/ftest_template.h) example: `ftest_bug_warlock_cooks_chicken.h`
    - [ftest_template.c](./tests/ftest_template.c) example: `ftest_bug_warlock_cooks_chicken.c`
2. Update the name of the init function inside your new files to also reflect the name of your test.
    - [ftest_template_init](./tests/ftest_template.h#L17) example: `ftest_bug_warlock_cooks_chicken_init`
3. Implement test actions in your .c file.
    - follow the naming convention provided `ftest_<test_name>__action###__<action_name>`
    - this is not required, but doing so prevents naming collisions from occuring
4. Update your tests init function
    - for example, inside `ftest_bug_warlock_cooks_chicken.c` make sure all your actions are appended
        - `ftest_append_action( ftest_template_action001__spawn_warlock,            100, NULL );`
        - `ftest_append_action( ftest_template_action002__drop_chicken_on_warlock,  100, NULL );`
    - actions are executed:
        - *sequentially*: `action002` will not execute until the previous action `returns true`
        - *after the `game turn delay`*: provided to `ftest_append_action(<test_action>, <game_turn_delay>)`
        
         this means if an action always `returns false` it will never finish and the `test will run forever`
     
5. Add your test to the test list [ftest_list.c](./ftest_list.c)
    - add the include for your tests header file
        - example: `#include "tests/ftest_bug_warlock_cooks_chicken.h"`
    - update [tests_list](./ftest_list.c#L29)
        - add the `name` of your test and then your tests `init function`
        - example: `{ "bug_warlock_cooks_chicken", ftest_bug_warlock_cooks_chicken_init }`
        - this `name` is what you pass to the `ftests` arg in [launch.json](../../.vscode/launch.json)
6. [Run Your Test](#run-existing-test)

###  See Example Tests:

| .h | .c |
|----------------------------------------|----------------------------------------|
| [ftest_template.h](./tests/ftest_template.h) | [ftest_template.c](./tests/ftest_template.c) |
| [ftest_bug_imp_tp_job_attack_door.h](./tests/ftest_bug_imp_tp_job_attack_door.h) | [ftest_bug_imp_tp_job_attack_door.c](./tests/ftest_bug_imp_tp_job_attack_door.c) |
| [ftest_bug_imp_goldseam_dig.h](./tests/ftest_bug_imp_goldseam_dig.h) | [ftest_bug_imp_goldseam_dig.c](./tests/ftest_bug_imp_goldseam_dig.c) |

### See Advanced Example Tests:

| .h | .c |
|----------------------------------------|----------------------------------------|
| [ftest_bug_pathing_pillar_circling.h](./tests/ftest_bug_pathing_pillar_circling.h) | [ftest_bug_pathing_pillar_circling.c](./tests/ftest_bug_pathing_pillar_circling.c) |

- This advanced test showcases using a single action to perform multiple stages, using custom data via void*
- It allows for re-usable test actions that can be shared between tests. (Think of a setup function that creates a base with different rooms and doors, then fills them with different creatures for each test)
- In the example itself `ftest_bug_pathing_pillar_circling` you can see that it creates a tunneler that digs towards the dungeon heart.
- The second test action does the same thing, but from different map coordinates and with a different base block type, showcasing the differences of data passed to the action.

### Explanation / Flow

1. Template map is loaded (This is always [ftests/campaign.cfg](../../ftests/campaign.cfg) - map00001)
    - in the future we might have multiple base maps, but for now there is only one
    - it contains a dungeon heart for each player, with all research unlocked
2. Init function is called for the current active test, which creates a list of actions to perform accordingly.
3. The game loop will call each action in the list after the desired GameTurn delay.
    - actions are counted as completed when they `return true`, if they `return false`, the action will be executed again next game turn.
    - if there is a failure at any stage (you decide this with your test, using the [FTEST_FAIL_TEST](./ftest.h#L24) macro) the test exits immediately, closing the program.
        - exit code 0 == `test success`
        - exit code -1 == `test failure`
        - optionally view the keeperfx.log to view details on why the test failed
            - example failure message: `FTest: [20] ftest_template_action001__spawn_imp: Failed to level up imp`
            - the above message tells us that at game turn 20, the test failed at function `ftest_template_action001__spawn_imp` because `Failed to level up imp`

Seeds have been overridden similar to the existing AUTOTESTING functionality. This means tests should perform the same every time for random events.