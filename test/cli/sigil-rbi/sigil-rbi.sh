#!/bin/bash
main/sorbet \
    test/cli/sigil-rbi/no_type.rbi \
    test/cli/sigil-rbi/typed.rbi \
    test/cli/sigil-rbi/strict.rbi \
    test/cli/sigil-rbi/abstract.rbi \
    test/cli/sigil-rbi/multiple_definition.rbi \
    2>&1
