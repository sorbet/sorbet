# Running the CLI tests

```
% bazel test //test/cli
```

# Adding a new CLI test

Run the `new_test.sh` script to add a new test skeleton:

```
% ./test/cli/new_test.sh foo
[ .. ] Creating ./test/cli/foo
[ .. ] Adding expected.out
[ .. ] Adding test.sh
[ OK ] Done
% cat test/cli/foo/test.sh
#!/usr/bin/env bash

set -euo pipefail

# NOTE: Sorbet and Sorbet Ruby are already in the PATH
```

# Updating exp files

You can update exp files by running the `update_cli_exp_files.sh` script:

```
% ./tools/scripts/remote-script test/cli/update_cli_exp_files.sh
[ .. ] Building the exp test outputs
[ .. ] Updating exp files
### BEGIN SYNCBACK ###
test/cli/help/expected.out
### END SYNCBACK ###
[ OK ] Done
```
