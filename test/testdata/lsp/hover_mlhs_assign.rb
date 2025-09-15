# typed: true
extend T::Sig

sig { returns([Integer, Integer]) }
def returns_tuple = [0, 0]

arg0, arg1 = returns_tuple
#^ hover: Integer
T.reveal_type(arg0) # error: `Integer`

sig { returns([Integer, Integer, Integer]) }
def returns_3tuple = [0, 0, 0]

arg0, *arg1 = returns_3tuple
# ^ hover: Integer
#        ^ hover: T::Array[Integer]

sig { returns(T::Array[String]) }
def returns_string_array = ["a", "b", "c", "d"]

arg0, arg1 = returns_string_array
# ^ hover: T.nilable(String)
#       ^ hover: T.nilable(String)
