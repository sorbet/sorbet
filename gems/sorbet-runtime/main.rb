require_relative './lib/sorbet-runtime'

class Module
  include T::Sig
end

module Opus; end
module Opus::Repro
  autoload :M, './m.rb'
  autoload :D1, './d1.rb'
  autoload :D2, './d2.rb'
  autoload :F, './f.rb'
  autoload :HC, './hc.rb'
  autoload :I, './i.rb'
end

Opus::Repro::M.from_thing(nil)
