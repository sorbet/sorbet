# frozen_string_literal: true
# typed: true
# compiled: true

def case_ok?(x, cases)
  case x
  when *cases
    :ok
  else
    :ko
  end
end

cases = [Integer, String]

p case_ok?(19, cases)
p case_ok?(22.4, cases)
p case_ok?("yes", cases)
p case_ok?(:no, cases)
