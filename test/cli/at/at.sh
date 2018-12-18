#!/bin/bash
main/sorbet --silence-dev-message @test/cli/at/at.input 2>&1

main/sorbet --silence-dev-message @does_not_exist 2>&1 || true
