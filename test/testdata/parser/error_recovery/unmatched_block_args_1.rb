# typed: false

module Opus::Log
  def self.info; end
end

class A
  def test_no_args
    1.times do | # parser-error: unmatched "|" in block argument list
    end
    1.times do | # parser-error: unmatched "|" in block argument list
      puts 'hello'
    end
    1.times do | # parser-error: unmatched "|" in block argument list
      puts('hello')
    end
    1.times do | # parser-error: unmatched "|" in block argument list
      x = nil
    end
    1.times do | # parser-error: unmatched "|" in block argument list
      Opus::Log.info
    end
  end

  def test_one_arg
    1.times do |x # parser-error: unmatched "|" in block argument list
    end
    1.times do |x # parser-error: unmatched "|" in block argument list
      puts 'hello'
    end
    1.times do |x # parser-error: unmatched "|" in block argument list
      puts('hello')
    end
    1.times do |x # parser-error: unmatched "|" in block argument list
      x = nil
    end
    1.times do |x # parser-error: unmatched "|" in block argument list
      Opus::Log.info
    end
  end

  def test_one_arg_comma
    1.times do |x, # parser-error: unmatched "|" in block argument list
    end
    # 1.times do |x,
    #   puts 'hello'
    # end
    1.times do |x, # parser-error: unmatched "|" in block argument list
      puts('hello') # parser-error: unexpected token tNL
    end
    1.times do |x, # parser-error: unmatched "|" in block argument list
      x = nil
    end
    1.times do |x, # parser-error: unmatched "|" in block argument list
      Opus::Log.info # parser-error: unexpected token tNL
    end
  end

  def test_two_args
    1.times do |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
    end
    1.times do |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      puts 'hello'
    end
    1.times do |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      puts('hello')
    end
    1.times do |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      x = nil
    end
    1.times do |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      Opus::Log.info
    end
  end
end
