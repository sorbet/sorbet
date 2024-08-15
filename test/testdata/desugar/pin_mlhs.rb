# typed: true
extend T::Sig

sig { returns([Integer, String]) }
def tuple_int_str = [0, '']

sig { returns([String, Integer]) }
def tuple_str_int = ['', 0]

def mlhs_example
  x, _ = tuple_str_int
  2.times do
    y, _ = tuple_int_str
  # ^^^^^^^^^^^^^^^^^^^^ error: Changing the type of a variable is not permitted
  end
end
