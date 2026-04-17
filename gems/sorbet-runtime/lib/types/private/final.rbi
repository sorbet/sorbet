# typed: true

module T::Private::Final
  sig {params(mod: Module).returns(T::Boolean)}
  def self.final_module?(mod); end
end
