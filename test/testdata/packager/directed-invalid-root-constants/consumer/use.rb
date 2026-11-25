# typed: strict

module Consumer
  FromRuby.new # error: Unable to resolve constant `FromRuby`
  FromRBI.new # error: Unable to resolve constant `FromRBI`
end
