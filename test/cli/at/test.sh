#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message @test/cli/at/at.input 2>&1

main/sorbet --censor-for-snapshot-tests --silence-dev-message @does_not_exist 2>&1 || true
