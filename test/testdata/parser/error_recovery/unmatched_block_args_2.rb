# typed: false

module Opus::Log
  def self.info; end
end

class A
  def test_no_args
    1.times { | # parser-error: unmatched "|" in block argument list
    }
    1.times { | # parser-error: unmatched "|" in block argument list
      puts 'hello'
    }
    1.times { | # parser-error: unmatched "|" in block argument list
      puts('hello')
    }
    1.times { | # parser-error: unmatched "|" in block argument list
      x = nil
    }
    1.times { | # parser-error: unmatched "|" in block argument list
      Opus::Log.info
    }
  end

  def test_one_arg
    1.times { |x # parser-error: unmatched "|" in block argument list
    }
    1.times { |x # parser-error: unmatched "|" in block argument list
      puts 'hello'
    }
    1.times { |x # parser-error: unmatched "|" in block argument list
      puts('hello')
    }
    1.times { |x # parser-error: unmatched "|" in block argument list
      x = nil
    }
    1.times { |x # parser-error: unmatched "|" in block argument list
      Opus::Log.info
    }
  end

  def test_one_arg_comma
    1.times { |x, # parser-error: unmatched "|" in block argument list
    }
    1.times { |x, # parser-error: unmatched "|" in block argument list
      puts 'hello' # parser-error: unexpected token tSTRING
    }
    1.times { |x, # parser-error: unmatched "|" in block argument list
      puts('hello') # parser-error: unexpected token tNL
    }
    1.times { |x, # parser-error: unmatched "|" in block argument list
      x = nil
    }
    1.times { |x, # parser-error: unmatched "|" in block argument list
      Opus::Log.info # parser-error: unexpected token tNL
    }
  end

  def test_two_args
    1.times { |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
    }
    1.times { |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      puts 'hello'
    }
    1.times { |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      puts('hello')
    }
    1.times { |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      x = nil
    }
    1.times { |x, y # parser-error: unmatched "|" in block argument list
      # parser-error: unexpected token tNL
      Opus::Log.info
    }
  end
end
