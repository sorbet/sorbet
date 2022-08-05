#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml test/cli/override-typed-bad/override-typed-bad.rb 2>&1
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed=false --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml test/cli/override-typed-bad/override-typed-bad.rb 2>&1
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed=strong --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml test/cli/override-typed-bad/override-typed-bad.rb 2>&1
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=file-that-does-not-exist 2>&1 -e ''
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed-bad/bad-top-level.yaml 2>&1 -e ''
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed-bad/bad-strictness.yaml 2>&1 -e ''
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed-bad/bad-list.yaml 2>&1 -e ''
echo ----
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed-bad/bad-filename.yaml 2>&1 -e ''
