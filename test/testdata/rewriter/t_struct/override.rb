# typed: true

class Override < T::Struct
  extend T::Sig

  # The second initialize makes Sorbet think that `foo` will be `Integer`
  # (because we'll mangle rename the old method symbol),
  # which means that we get a bad error message here unfortunately.
  prop :foo, String # error: Argument does not have asserted type `String`

  sig {params(foo: Integer).void}
  def initialize(foo:)
    puts "override"
  end
end

Override.new
#           ^ error: Missing required keyword argument `foo` for method `Override#initialize`
Override.new(foo: "no")
#                 ^^^^ error: Expected `Integer` but found `String("no")` for argument `foo`
Override.new(foo: 3, bar: 4)
#                    ^^^^^^ error: Unrecognized keyword argument `bar` passed for method `Override#initialize`
T.reveal_type(Override.new(foo: 3).foo) # error: Revealed type: `String`
