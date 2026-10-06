This folder specifies the way Sorbet's buildfarm works.
Note that buildfarm machines verify signatures of there files
and thus changing any of them requires changes to buildfarm config

macOS tests and release builds run on `master` and on any branch whose name
contains `ci-macos` (for example, `jez/ci-macos-fix`). Include this substring
in a feature branch name to opt in to macOS CI.
