# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that T::Configuration state changes
# are isolated from the main test process.

alias_t = T.type_alias { Integer }.checked(:tests)
alias_t.valid?(1)

begin
  T::Configuration.enable_checking_for_sigs_marked_checked_tests
  puts "FAIL: expected RuntimeError about toggling too late"
rescue RuntimeError => e
  unless e.message.include?("Toggle `:tests`-level runtime type checking earlier")
    puts "FAIL: wrong error: #{e.message}"
  end
end
