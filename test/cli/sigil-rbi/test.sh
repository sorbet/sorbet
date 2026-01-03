#!/bin/bash
main/sorbet --silence-dev-message \
    --max-threads=0 \
    test/cli/sigil-rbi/no_type.rbi \
    test/cli/sigil-rbi/typed.rbi \
    test/cli/sigil-rbi/strict.rbi \
    test/cli/sigil-rbi/abstract.rbi \
    test/cli/sigil-rbi/multiple_definition.rbi \
    test/cli/sigil-rbi/overrides.rbi \
    2>&1
