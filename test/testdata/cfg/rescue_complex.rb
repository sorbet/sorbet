# typed: true
class TestRescue
  extend T::Sig

  def meth; 0; end
  def foo; 1; end
  def bar; 2; end
  def baz; 3; end
  def take_arg(x); x; end
  def untyped_exceptions(); [Exception]; end
  sig {returns(T::Array[T.class_of(Exception)])}
  def typed_exceptions(); [Exception]; end

  sig {returns([T.class_of(TypeError), T.class_of(ArgumentError)])}
  def tuple_exceptions
    [TypeError, ArgumentError]
  end

  def initialize
    @ex = T.let(nil, T.nilable(StandardError))
  end

  def multiple_rescue()
    begin
      meth
    rescue
      baz
    rescue
      bar
    end
  end

  def multiple_rescue_classes()
    begin
      meth
    rescue Foo, Bar => baz
         # ^^^ error: Unable to resolve constant `Foo`
              # ^^^ error: Unable to resolve constant `Bar`
      baz
    end
  end

  def multiple_rescue_classes_varuse()
    begin
      meth
    rescue LoadError, SocketError => baz
      baz
    end

    T.reveal_type(baz) # error: Revealed type: `T.nilable(T.any(LoadError, SocketError))`
  end

  def rescue_loop()
    ex = T.let(nil, T.nilable(StandardError))

    loop do
      ex = nil
      begin
        meth
      rescue => ex
      end
    end
  end

  def rescue_untyped_splat()
    begin
      meth
    rescue *untyped_exceptions => e
      T.reveal_type(e) # error: Revealed type: `Exception`
    end
  end

  def rescue_typed_splat()
    begin
      meth
    rescue *typed_exceptions => e
      T.reveal_type(e) # error: Revealed type: `Exception`
    end
  end

  def rescue_typed_splat()
    begin
      meth
    rescue *tuple_exceptions => e
      T.reveal_type(e) # error: Revealed type: `Exception`
    end
  end

  def parse_rescue_ensure()
    begin; meth; rescue; baz; ensure; bar; end
  end

  def parse_bug_rescue_empty_else()
    begin; rescue LoadError; else; end
  end

  def parse_ruby_bug_12686()
    take_arg (bar rescue nil)
  end

  def parse_rescue_mod()
    meth rescue bar
  end

  def parse_resbody_list_var()
    begin; meth; rescue foo => ex; bar; end
  end

  def parse_rescue_else_ensure()
    begin; meth; rescue; baz; else foo; ensure; bar end
  end

  def parse_rescue()
    begin; meth; rescue; foo; end
  end

  def parse_resbody_var()
    begin; meth; rescue => ex; bar; end
  end

  def parse_resbody_var_1()
    begin; meth; rescue => @ex; bar; end
  end

  def parse_rescue_mod_op_assign()
    foo += meth rescue bar # error: Method `+` does not exist on `NilClass`
  end

  def parse_ruby_bug_12402()
    foo = raise(bar) rescue nil
  end

  def parse_ruby_bug_12402_1()
    foo += raise(bar) rescue nil # error: Method `+` does not exist on `NilClass`
  end

  def parse_ruby_bug_12402_2()
    foo[0] += raise(bar) rescue nil
  end
end
