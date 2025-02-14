# typed: true

 case 0; in "abc":a1, "a#{"b"}c":a2; end  # parser-error: duplicate hash pattern key abc
