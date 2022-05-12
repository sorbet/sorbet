# typed: strict
# packaged: true

class RBI::Bar
  extend T::Sig

  sig {returns(::Integer)}
  def self.one(); end
end

class ::GlobalBar
  extend T::Sig

  sig {returns(RBI::Bar)}
  def self.foo(); end
end
