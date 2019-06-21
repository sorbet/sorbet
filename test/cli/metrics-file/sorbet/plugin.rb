require 'optparse'

module SorbetPlugins
  def self.parse_args
    klass, method, source = nil
    OptionParser.new do |opts|
      opts.on('--class CLASS', 'Immediate outer class') do |v|
        klass = v
      end
      opts.on('--method METHOD', 'Method trigger') do |v|
        method = v
      end
      opts.on('--source SOURCE', 'Source code for the enter method call') do |v|
        source = v
      end
    end.parse!
    [klass, method, source]
  end

  _klass, _method, source = SorbetPlugins.parse_args

  class << self
    def delegate(*methods, to: nil, prefix: nil, allow_nil: nil, private: nil)
      puts "def foo; end"
    end
  end

  module_eval(source)
end
