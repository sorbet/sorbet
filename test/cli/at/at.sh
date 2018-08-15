#!/bin/bash
main/sorbet @test/cli/at/at.input 2>&1

main/sorbet @does_not_exist 2>&1 || true
