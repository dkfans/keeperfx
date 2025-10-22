Target Audience: Software Engineers

This is proof of concept; the goal is primarily for setting up scenarios to replicate bugs or test implementation of new features.

Added benefit is that if tests are setup in a continuous integration environment we can automatically know when we break behaviours when adding new features.

The functional tests should run fast, hence you will see `fastforward_speed = 8` is used by default for some tests. It can be specified per-test.
They also skip the trademark/cutscene by default for super fast launch.

# Quickstart

## Available Test Names

- `example_template_test`
- `bug_imp_tp_attack_door__claim`
- `bug_imp_tp_attack_door__prisoner`
- `bug_imp_tp_attack_door__deadbody`
- `bug_imp_goldseam_dig`
- `bug_pathing_stair_treasury`

## Run Existing Test

1. Enable Functional Testing
    - <b>CTRL+SHIFT+B</b> (VSCode shortcut for switching debug option)
    - select one of the FTEST_DEBUG options
        - FTEST_DEBUG=1    (debug symbols ON, functional tests ON)
		- FTEST_DEBUG=0    (debug symbols OFF, functional tests ON)
2. Update [launch.json](../../.vscode/launch.json)
    - add `-ftests` argument to run all tests *(optionally provide the name of an existing test to run)*

        | without test name | with test name |
        |----------|-------------|
        | `"args": [`<br/>`"-ftests"`<br/>`],` | `"args": [`<br/>`"-ftests", "bug_imp_teleport_attack_door"`<br/>`],` |

3. Run game - Watch test execute
4. View keeperfx.log for test results (lines prefixed with `FTest:`)
5. (optional) If using the `-exitonfailedtest` flag, keeperfx.exe will exit on first failure, exit code represents test results (used for automation)
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
    - [ftest_template_init.h](./tests/ftest_template.h#L17) example: `ftest_bug_warlock_cooks_chicken_init`
    - [ftest_template_init.c](./tests/ftest_template.c#L44) example: `ftest_bug_warlock_cooks_chicken_init`
3. Implement test actions in your .c file.
    - follow the naming convention provided `ftest_<test_name>__action###__<action_name>`
        - this is not required, but doing so prevents naming collisions from occuring
    - declare any test variables inside a struct like `ftest_template__variables` see [ftest_template_init.c](./tests/ftest_template.c#L21), eg: `ftest_bug_warlock_cooks_chicken__variables`
        - define instances of the variables struct and set their values, like `ftest_template__vars` see [ftest_template_init.c](./tests/ftest_template.c#L30), eg: `ftest_bug_warlock_cooks_chicken__vars`
        - again, not required, but highly recommended for preventing many naming collisions; tests usually have a lot of variables!
    - use the variables inside your test actions
        1. when appending test actions using `ftest_append_action`, pass a reference of your vars, see [ftest_template_init.c](./tests/ftest_template.c#L49)
        2. access the vars inside your actions by casting `args->data` to your variables type, see [ftest_template_init.c](./tests/ftest_template.c#L60)
            - doing this allows for more flexibility, the test actions can be re-used multiple times with different data in your test, or other tests can use your actions.
            - NOTE: you may access your glolbal variables `__vars` directly instead of using `args->data`, but this makes the actions not as flexible or re-usable.
4. Update your tests init function
    - for example, inside `ftest_bug_warlock_cooks_chicken.c` make sure all your actions are appended
        - `ftest_append_action( ftest_template_action001__spawn_warlock,            100, NULL );`
        - `ftest_append_action( ftest_template_action002__drop_chicken_on_warlock,  100, NULL );`
    - actions are executed:
        - **sequentially**: `action002` will not execute until the previous action returns `FTRs_Go_To_Next_Action`
        - **after the `game turn delay`**: provided to `ftest_append_action(<test_action>, <game_turn_delay>)`
        
         this means if an action always returns `FTRs_Repeat_Current_Action` it will never finish and the `test will run forever`
     
5. Add your test to the test list [ftest_list.c](./ftest_list.c)
    - add the include for your tests header file
        - example: `#include "tests/ftest_bug_warlock_cooks_chicken.h"`
    - update [tests_list](./ftest_list.c#L30)
        - add the `test_name` of your test, your tests `init_func`, the `level_file` and specify the `fastforward_speed` *(there are other advanced optional values not listed here)*
        - example: `{ .test_name="bug_warlock_cooks_chicken",  .init_func=ftest_bug_warlock_cooks_chicken_init,  .level_file="keeporig",  .level=1,  .fastforward_speed=0 },`
        - this `test_name` is what you pass to the `ftests` arg in [launch.json](../../.vscode/launch.json) if you only want to run that test
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

| .h | .c |
|----------------------------------------|----------------------------------------|
| [ftest_bug_imp_tp_job_attack_door.h](./tests/ftest_bug_imp_tp_job_attack_door.h) | [ftest_bug_imp_tp_job_attack_door.c](./tests/ftest_bug_imp_tp_job_attack_door.c) |

- This advanced test showcases defining multiple tests inside the same file, while re-using shared functionality between the tests
- Each test has an init func that is slightly different from the other. One also modifies the data slightly to change logic inside actions

### Explanation / Flow

1. The desired campaign/mappack is atuomatically loaded for the current active test
    - see [ftest_list.c](./ftest_list.c) for examples
2. Init function is called for the current active test, which creates a list of actions to perform accordingly.
    - NOTE: You can also call one-time logic here, like altering the map before the test actions are executed.
3. The game loop will call each action in the list after the desired GameTurn delay.
    - actions are counted as completed when they return `FTRs_Go_To_Next_Action`, if they return `FTRs_Repeat_Current_Action`, the action will be executed again next game turn.
    - by default, if a test fails, the next test will be ran, unless you use the `-exitonfailedtest` flag.
    - (optional) when using the `-exitonfailedtest` flag if there is a failure at any stage (you decide this with your test, using the [FTEST_FAIL_TEST](./ftest.h#L24) macro) the test exits immediately, closing the program.
        - exit code 0 == `test success`
        - exit code -1 == `test failure`
        - optionally view the keeperfx.log to view details on why the test failed
            - example failure message: `FTest: [20] ftest_template_action001__spawn_imp: Failed to level up imp`
            - the above message tells us that at game turn 20, the test failed at function `ftest_template_action001__spawn_imp` because `Failed to level up imp`
    - (optional) when using the `-includelongtests` flag, extra tests from `long_running_tests_list` will be included in the test search.


Seeds are overriden when using the functional tests. This means tests should perform the same every time for random events.

- (NOTE: There may still be some unaccounted for RNG, the bug_imp_tp_attack_door__deadbody test is an example of this. Sometimes the imps do not fly into the door after teleport and the door isn't attacked/broken...)
- (The only srand not wrapped yet is inside [crypt.h](../../deps/zlib/contrib/minizip/crypt.h#L113) but requires further investigation)
