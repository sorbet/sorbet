# compiled: true
# typed: true
# frozen_string_literal: true

# When parsing keyword arguments that have a kwsplat present, the code that we
# emit will iteratively remove parsed arguments from the kwsplat hash to
# determine what's left over. The effect of this is that if something else had a
# reference to the hash that was used for the keyword args, they will see that
# hash as having fewer elements after the compiled function was called.

class Util
  extend T::Sig

  sig {params(x: Integer, args: T.untyped).void}
  def self.main(x:, **args)
    puts x
    puts args
  end
end

class Inspector

  def self.setup
    return unless @old.nil?

    @old = Util.method(:main)
    old = @old

    Util.define_singleton_method(:main) do |args|
      Inspector.instance_variable_set(:@args, args)
      old.call(args)
    end
  end

  def self.teardown
    # In the compiled version, this puts will differ as the `@args` hash has
    # been mutated by the compiled function.
    puts "args = #{@args}"
    Util.define_singleton_method(:main, @old)
  end

end

class Test
  def self.test_stub
    args = {msg: 'hi', x: 20}
    Util.main(x: 10, **args)
  end
end

# Matches ruby
Test.test_stub

# Doesn't match ruby, as the `@args` hash has been mutated
Inspector.setup
Test.test_stub
Inspector.teardown
