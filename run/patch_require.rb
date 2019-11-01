# typed: true
require 'open3'
require 'tmpdir'

module Kernel
  alias sorbet_old_require require
  def require(name)
    if File.exist?(name + ".rb")
      name = name + ".rb"
    end
    if File.exist?(name)
      tmpdir = ENV['llvmir']
      raise "no 'llvmir' in ENV" unless tmpdir
      name = tmpdir + '/' + name.gsub('/', '_') + '.bundle'
      return sorbet_old_require(name)
    end

    sorbet_old_require(name)
  end

  def require_relative(feature)
    locations = Kernel.caller_locations(1, 2)
    file = T.must(T.must(locations)[0]).absolute_path
    absolute = File.expand_path(feature, File.dirname(T.must(file)))
    require(absolute)
  end
end
