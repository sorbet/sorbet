This folder specifies the way Sorbet's buildfarm works.
Note that buildfarm machines verify signatures of there files
and thus changing any of them requires changes to buildfarm config

macOS tests and release builds run on `master` and on any branch whose name
contains `MACOS_CI` or `MACOS-CI` at word boundaries (for example,
`jez/MACOS_CI-fix` or `jez/MACOS-CI-fix`). Include either marker in a feature
branch name to opt in to macOS CI. Letters, digits, and underscores adjoining
the marker prevent a match; slashes and hyphens can delimit it.
