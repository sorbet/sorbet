# typed: true

# We run checks to ensure that all optional arguments to a method appear after all required arguments. This means that
# when we synthesize the initalize method for this struct, the optional foo parameter must by synthesized to come after
# the required bar parameter, even though the order we wrote in the code was the opposite.
class ParamOrder < T::Struct
  prop :foo, Integer, default: 3
  prop :bar, Integer
end
