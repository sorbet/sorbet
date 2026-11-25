# typed: strict
# enable-experimental-rbs-comments: true

module RbsKeywordArgumentSignatures

  #: (Module[top] mod) -> void
  def self.take(mod); end

  #: (mod: Module[top]) -> void
  def self.take_kw(mod:); end

  take(Module.new do
    #: (String name) -> void
    def from_positional(name)
      T.reveal_type(name) # error: Revealed type: `String`
    end
  end)

  take_kw(mod: Module.new do
    #: (String name) -> void
    def from_keyword(name)
      T.reveal_type(name) # error: Revealed type: `String`
    end
  end)
end
