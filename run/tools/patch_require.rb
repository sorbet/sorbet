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
      root_name = name.gsub('/', '_').gsub('.', '_').gsub('_rb', '.rb')
      if name.start_with?('./')
        root_name = root_name[2..-1]
      end
      if RUBY_PLATFORM == "x86_64-linux"
        suffix = '.so'
      elsif RUBY_PLATFORM == "x86_64-darwin18"
        suffix = '.bundle'
      else
        raise "unknown platform: #{RUBY_PLATFORM}"
      end
      cext = tmpdir + '/' + root_name + suffix
      if File.exist?(cext)
        $stderr.puts "SorbetLLVM using compiled: #{cext}"
        return sorbet_old_require(cext)
      end
      if ENV['force_compile']
        raise "No compiled extension: #{cext}"
      end
      $stderr.puts "SorbetLLVM interpreting: #{name}"
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
