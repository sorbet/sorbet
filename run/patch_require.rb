require 'open3'
require 'tmpdir'

module Kernel
  alias sorbet_old_require require
  def require(name)
    if File.exists?(name + ".rb")
      name = name + ".rb"
    end
    if File.exists?(name)
      tmpdir = ENV['llvmir']
      name = ENV['llvmir'] + '/' + name.gsub('/', '_') + '.bundle'
      return sorbet_old_require(name)
    end

    sorbet_old_require(name)
  end

  def require_relative(feature)
    locations = Kernel.caller_locations(1, 2)
    file = case locations[0]
           when Thread::Backtrace::Location
             locations[0].absolute_path
           when Array
             # For runtime require_relatives
             locations[1][0]
           end

    absolute = File.expand_path(feature, File.dirname(file))
    require(absolute)
  end
end
