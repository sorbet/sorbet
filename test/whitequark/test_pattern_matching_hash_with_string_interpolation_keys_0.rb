# typed: true

case a; in "#{a}": 1; end # error: symbol literal with interpolation is not allowed
