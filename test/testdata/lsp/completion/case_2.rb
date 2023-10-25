# typed: true

module Opus::Log
  def self.info(msg); end
end

class ConstantCompletion
  A = nil
  B = nil

  def self.test1(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when ConstantCompletion:: # error: expected constant name following "::"
    #                        ^ completion: A, B
    puts 'hello'
  end

  def self.test2(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when ConstantCompletion::
    #                        ^ completion: A, B, ...
    puts('hello')
  end

  def self.test3(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when ConstantCompletion:: # error: Unable to resolve constant `Opus`
    #                        ^ completion: A, B, ...
    Opus::Log.info('hello')
  end

  def self.test4(x)
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when ConstantCompletion::
    #                        ^ completion: A, B, ...
    y = nil # error: Setter method `y=` does not exist on `T.class_of(ConstantCompletion)`
  end
end # error: unexpected token "end of file"
