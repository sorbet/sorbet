
require_relative './lib/sorbet-runtime'

class Module
  include T::Sig
end

module Opus; end
module Opus::Repro
  autoload :A, './a.rb'
  autoload :B, './b.rb'
  autoload :C, './c.rb'
end

Opus::Repro::C.duplex(Opus::Repro::A, nil)
