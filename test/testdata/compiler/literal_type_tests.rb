# frozen_string_literal: true
# typed: true
# compiled: true

def literal_symbol(obj)
  case obj
  when :matching
    true
  else
    false
  end
end

def literal_string(obj)
  case obj
  when "matching"
    true
  else
    false
  end
end

def literal_double(obj)
  3.14 <= obj
end

def literal_integer(obj)
  15 < obj
end
