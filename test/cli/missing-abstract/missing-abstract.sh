#!/bin/bash
main/sorbet --silence-dev-message test/cli/missing-abstract/interface.rb test/cli/missing-abstract/implementation.rb test/cli/missing-abstract/reopen.rb 2>&1
main/sorbet --silence-dev-message test/cli/missing-abstract/implementation.rb test/cli/missing-abstract/reopen.rb test/cli/missing-abstract/interface.rb 2>&1
