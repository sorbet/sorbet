# typed: true

module Kernel
  alias sorbet_old_require require
  def require(name)
    if File.exist?(name + ".rb")
      name = name + ".rb"
    end
    if File.exist?(name)
      tmpdir = ENV['llvmir']
      raise "no 'llvmir' in ENV" unless tmpdir
      root_name = name
      if name.start_with?('./')
        root_name = root_name[2..-1]
      end
      # Our paths are terrible...
      root_name = root_name.gsub(%r{.*test/testdata}, 'test/testdata')

      case RUBY_PLATFORM
      when "x86_64-linux"
        suffix = '.so'
      when "x86_64-darwin18", "x86_64-darwin19", "x86_64-darwin20"
        suffix = '.bundle'
      else
        raise "unknown platform: #{RUBY_PLATFORM}"
      end
      cext = File.join(tmpdir, root_name + suffix)
      if File.exist?(cext)
        # NOTE: we cross-talk to the payload through this variable. The payload doesn't know what the real path of the
        # original file is, so we pass it through here. It's consumed in payload.c:sorbet_allocateRubyStackFrames.
        $__sorbet_ruby_realpath = File.realpath(name)
        $stderr.puts "SorbetLLVM using compiled: #{cext} for #{$__sorbet_ruby_realpath}"
        return sorbet_old_require(cext)
      end
      if ENV['force_compile'] && File.exist?(name) && File.read(name).include?('# compiled: true')
        raise "Missing compiled extension: #{cext}"
      end
      # $stderr.puts "SorbetLLVM interpreting: #{name}"
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
