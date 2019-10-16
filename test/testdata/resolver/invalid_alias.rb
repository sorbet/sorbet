# typed: true

class BadAliasingClassMethod1
  class << self
    def hello
      'HELLO WORLD'
    end

    alias_method :hello, :hello
  end
end

def main
  BadAliasingClassMethod1.hello
end
