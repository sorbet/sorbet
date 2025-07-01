# typed: true
require_relative 'lib/sorbet-runtime'

class Model
  def self.upsert; end
end

class MyModel < Model
  extend T::Sig

  sig { override.params(x: Integer).void }
  def self.upsert(x)
  end
end

T::Utils.run_all_sig_blocks
