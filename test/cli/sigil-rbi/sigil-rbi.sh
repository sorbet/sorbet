#!/bin/bash
main/sorbet --silence-dev-message \
    test/cli/pragma-rbi/no_type.rbi \
    test/cli/pragma-rbi/typed.rbi \
    test/cli/pragma-rbi/strict.rbi \
    test/cli/pragma-rbi/abstract.rbi \
    test/cli/pragma-rbi/multiple_definition.rbi \
    2>&1
