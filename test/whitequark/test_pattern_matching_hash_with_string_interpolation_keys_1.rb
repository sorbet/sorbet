# typed: true

case a; in "#{a}": 1; end # parser-error: symbol literal with interpolation is not allowed
