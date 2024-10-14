# typed: true
extend T::Sig

sig { returns(String) }
def returns_string = ""

sig { returns(T.nilable(String)) }
def returns_nilable_string = nil

begin
  x = returns_string
rescue ArgumentError
  begin
    x = returns_string
  rescue ArgumentError
    raise
  end
end

T.reveal_type(x) # error: `String`

x&.chars || []
#^^ error: can never be nil
#           ^^ error: This code is unreachable
