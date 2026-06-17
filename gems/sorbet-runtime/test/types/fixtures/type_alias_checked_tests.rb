# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that T::Configuration state changes
# are isolated from the main test process.

T::Configuration.enable_checking_for_sigs_marked_checked_tests

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

checked_alias = T.type_alias { Integer }.checked(:tests)

check("checked(:tests) validates Integer") { checked_alias.valid?(1) }
check("checked(:tests) rejects String") { !checked_alias.valid?("hello") }
check("checked(:tests) name preserved") { checked_alias.name == "Integer" }

# a rejected proposal made it so that once one `.checked(:tests)` type alias
# was evaluated, no new ones could be defined.
another_one = T.type_alias { Integer }.checked(:tests)
check("another type alias defined after the initial load") {
  another_one.valid?(1) && !another_one.valid?("not an int")
}
