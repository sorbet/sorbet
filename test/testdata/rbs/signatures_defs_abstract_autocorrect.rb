# typed: strict
# enable-experimental-rbs-comments: true

# @abstract
class Abstract
  # @abstract
  #: -> void
  def foo; end # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call

  # @abstract
  #: -> void
  def bar # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end

  # @abstract
  #: -> void
  def baz # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    puts # error: Abstract methods must not contain any code in their body
  end

  # @abstract
  #: -> void
  def qux # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    puts # error: Abstract methods must not contain any code in their body
    puts
  end

  # @abstract
  #: -> void
  def self.foo; end # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call

  # @abstract
  #: -> void
  def self.bar # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end

  # @abstract
  #: -> void
  def self.baz # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    puts # error: Abstract methods must not contain any code in their body
  end

  # @abstract
  #: -> void
  def self.qux # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    puts # error: Abstract methods must not contain any code in their body
    puts
  end

  # @abstract
  #: (Integer) -> void
  def positional(value); end # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call

  # @abstract
  #: (i: Integer) -> void
  def keyword(i:) # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end

  # @abstract
  #: { -> void } -> void
  def self.with_block(&block); end # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call

  # @abstract
  #: (Integer, i: Integer) { -> void } -> void
  def unparenthesized value, i:, &block # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end

  # @abstract
  #: (Integer, i: Integer) { -> void } -> void
  def multiline_parameters( # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    value,
    i:,
    &block
  )
  end

  # @abstract
  #: -> void
  def empty_parentheses() # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end

  # @abstract
  #: (Integer, i: Integer) { -> void } -> void
  def existing_body(value, i:, &block) # error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
    puts value # error: Abstract methods must not contain any code in their body
  end

  # @abstract
  #: (i: Integer) -> void
  def fooBar(i:) # rubocop:disable Naming/MethodName
  #   ^^^^^^ error: Methods declared @abstract with an RBS comment must contain only a forwarding super call
  end
end
