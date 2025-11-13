# typed: true

class Main
  extend T::Sig

  sig {returns(String)}
  def implicit_return_via_else
    if T.unsafe(nil); return 'yep'; end
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected `String` but found `NilClass` for method result type
  end

  sig {returns(String)}
  def implicit_return_non_empty_cont_block
    puts nil
  # ^^^^^^^^ error: Expected `String` but found `NilClass` for method result type
  end

  sig {returns(String)}
  def double_return
    return puts nil
  # ^^^^^^^^^^^^^^^ error: Expected `String` but found `NilClass` for method result type
  end

  sig {params(x: T::Module[T.anything]).returns(String)}
  def initialized_twice(x)
    if T.unsafe(nil)
      res = x.name
    else
      res = x.name
    end
    return res
  # ^^^^^^^^^^ error: Expected `String` but found `T.nilable(String)` for method result type
  end
end

module M
  extend T::Sig
  include Kernel

  sig {returns(String)}
  def display_name
    if self.is_a?(Module)
      return self.name
    # ^^^^^^^^^^^^^^^^ error: Expected `String` but found `T.nilable(String)` for method result type
    else
      return self.class.name
    end
  end
end

