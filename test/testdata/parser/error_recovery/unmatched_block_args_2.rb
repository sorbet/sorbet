# typed: false

module Opus::Log
  def self.info; end
end

class A
  def test_no_args
    1.times { |
    }
    1.times { |
      puts 'hello'
    }
    1.times { |
      puts('hello')
    }
    1.times { |
      x = nil
    }
    1.times { |
      Opus::Log.info
    }
  end

  def test_one_arg
    1.times { |x
    }
    1.times { |x
      puts 'hello'
    }
    1.times { |x
      puts('hello')
    }
    1.times { |x
      x = nil
    }
    1.times { |x
      Opus::Log.info
    }
  end

  def test_one_arg_comma
    1.times { |x,
    }
    1.times { |x,
      puts 'hello'
    }
    1.times { |x,
      puts('hello')
    }
    1.times { |x,
      x = nil
    }
    1.times { |x,
      Opus::Log.info
    }
  end

  def test_two_args
    1.times { |x, y
    }
    1.times { |x, y
      puts 'hello'
    }
    1.times { |x, y
      puts('hello')
    }
    1.times { |x, y
      x = nil
    }
    1.times { |x, y
      Opus::Log.info
    }
  end
end
