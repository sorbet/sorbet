# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(str: String).returns(T.untyped)}
def regexp_ok?(str)
  case str
  when "squeamish", /ossifrage/
    :ok
  else
    :ko
  end
end

p regexp_ok?("magic")
p regexp_ok?("phrase")
p regexp_ok?("is")
p regexp_ok?("squeamish")
p regexp_ok?("ossifrage")

def integer_ok?(int)
  case int
  when 7, 42, 432
    :ok
  else
    :ko
  end
end

p integer_ok?(5)
p integer_ok?(7)
p integer_ok?(100)
p integer_ok?(432)
p integer_ok?(42)

    
