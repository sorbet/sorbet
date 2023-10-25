# typed: true

module Opus::Log
  def self.info(msg); end
end

class MethodCompletion
  def self.test1(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when MethodCompletion.
    #                     ^ completion: x, test1, test2, ...
    puts 'hello' # error: unexpected token tSTRING
  end

  def self.test2(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when MethodCompletion.
    #                     ^ completion: test1, test2, test3, test4, ...
    puts('hello')
  end

  def self.test3(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when MethodCompletion. # error: Dynamic constant references are unsupported
    #                     ^ completion: test1, test2, test3, test4, ...
    Opus::Log.info('hello') # error: Method `Opus` does not exist on `T.class_of(MethodCompletion)`
  end

  def self.test4(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when MethodCompletion.
    #                     ^ completion: test1, test2, test3, test4, ...
    y = nil # error: Setter method `y=` does not exist on `T.class_of(MethodCompletion)`
  end
end # error: unexpected token "end of file"
