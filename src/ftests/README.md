Target Audience: Software Engineers

This is proof of concept; the goal is primarily for setting up scenarios to replicate bugs or test implementation of new features.

Added benefit is that if tests are setup in a continuous integration environment we can automatically know when we break behaviours when adding new features.

The functional tests should run fast, hence frame_skip is used by default.
They also skip the trademark/cutscene by default for super fast launch.



# Quickstart

## Run Existing Test

1. Enable Functional Testing - <b>CTRL+SHIFT+B</b>, select one of the FTEST_DEBUG options, FTEST_DEBUG=1 for debugging info, FTEST_DEBUG=0 for without
2. Update launch.json with -ftests argument. Optionally provide the name of an existing test to run.
3. Run game - Watch test execute

## Create New Test

1. Copy [ftest_template.h](./src/ftest_template.h) and [ftest_template.c](./src/ftest_template.c) and rename them to reflect your test.
2. Update the name of the init function inside your new files to also reflect the name of your test.
3. Implement test actions in your .c file.
4. Update the init function inside your .c file to add the actions in the order you want, with the turn delays you prefer.
5. Add the name of your test and the init function to [src/ftest_list.c](https://github.com/demonds1/keeperfx/blob/functional_tests__bugfixing/src/ftest_list.c) so it can be found by name.
6. [Run Your Test](#run-existing-test)

###  See Example test:
- [ftest_bug_imp_tp_job_attack_door.h](https://github.com/demonds1/keeperfx/blob/functional_tests__bugfixing/src/ftest_bug_imp_tp_job_attack_door.h)
- [ftest_bug_imp_tp_job_attack_door.c](https://github.com/demonds1/keeperfx/blob/functional_tests__bugfixing/src/ftest_bug_imp_tp_job_attack_door.c)

### launch.json example:
```
"args": [
    "-ftests", "bug_imp_teleport_attack_door"
],
```

### Explanation / Flow

1. Template map is loaded (This is always ../ftests/campaign - map00001, it will never change).
2. Init function is called for the current active test, which creates a list of actions to perform accordingly.
3. The game loop will call each action in the list after the desired GameTurn delay.
    - Actions are counted as completed when they return true, if they return false, the action will be executed again next game turn.
    - If there is a failure at any stage (you decide this with your test, using the FTEST_FAIL_TEST macro) the test exits immediately, closing the program.
    - Test failure results in keeperfx.exe returning -1 exit code.
    - Test success results in exit code 0.

Seeds have been overridden similar to the existing AUTOTESTING functionality.

# PR Checklist

- [x] demonds1/keeperfx#2
- [x] demonds1/keeperfx#1
- [x] demonds1/keeperfx#3
- [ ] demonds1/keeperfx#4
- [ ] demonds1/keeperfx#5
- [ ] demonds1/keeperfx#6
- [x] demonds1/keeperfx#7