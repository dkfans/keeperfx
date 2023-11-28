# [pre-commit](https://pre-commit.com/)

`pre-commit` is a framework and tool that helps developers automate and enforce code formatting, linting, and other
`pre-commit` checks before code is committed to version control repositories. It allows for the configuration of various
`pre-commit` hooks to maintain consistent code quality and style across a codebase.

To install `pre-commit`, install it via your package manager (APT, PIP, etc.), e.g.

```shell
sudo apt update && sudo apt install pre-commit
# alternatively
pip install --user pre-commit
```

Change your directory to the cloned project and run

```shell
pre-commit install
```

When `pre-commit` runs, it looks for a configuration file named `.pre-commit-config.yaml` in the project.
This file defines the tools and checks to be run as `pre-commit` hooks.

The command installs the `git` hook scripts specified in the configuration file. These hooks are executed automatically
when you attempt to commit changes. Each hook corresponds to a tool or check defined in the configuration, such as code
linters, formatters, or other pre-commit checks.

This helps ensure that your code complies with the defined standards and passes pre-commit checks
before being committed to the version control system.

To run `pre-commit` independently outside of `git commit`

```shell
pre-commit run # runs against only changed files
pre-commit run --all-files # runs against all files
pre-commit run <hook_id> # run a specific hook given its ID defined in .pre-commit-config.yaml
```

If you have any issues with pre-commit, try upgrading and/or force reinstalling to the latest version, e.g.

```shell
pip install --user --force-reinstall --upgrade pre-commit
```

# [clang-format](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)

`clang-format` is a tool provided by the Clang compiler suite that automatically formats C, C++, and Objective-C code
according to a specified coding style. It helps maintain consistent code formatting within a project by enforcing a
defined set of style rules. clang-format is installed as part of `pre-commit`, and is set up to automatically fix the
code formatting (if needed) when `git commit` is run.

When `clang-format` runs, it looks for a configuration file named `.clang-format` in the project.
This file defines the coding style rules used such as indentation, line breaks, and other formatting preferences
for C, C++, and Objective-C code. It will automatically fix any formatting issues as defined by these rules.

Make sure you have enabled `clang-format` over your default code formatter for your IDE.
