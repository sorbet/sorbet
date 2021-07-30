# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

module MyModule
  def calls_nil_p
    T.unsafe(self).nil?
  end
end

class MyObject
  include ::MyModule
end

class MyBasicObject < BasicObject
  include ::MyModule
end

def soft_no_method_error(&blk)
  begin
    yield
  rescue NoMethodError => exn
    puts exn.message
  end
end

nil_ = T.unsafe(nil)
obj = T.unsafe(Object.new)
basic_obj = T.unsafe(BasicObject.new)

soft_no_method_error {p nil_.nil?}
soft_no_method_error {p obj.nil?}
soft_no_method_error {p basic_obj.nil?}

soft_no_method_error {p MyObject.new.nil?}
soft_no_method_error {p MyObject.new.calls_nil_p}

soft_no_method_error {p T.unsafe(MyBasicObject.new).nil?}
soft_no_method_error {p MyBasicObject.new.calls_nil_p}

T::Sig::WithoutRuntime.sig {params(x: MyModule).void}
def takes_my_module(x)
  p T.unsafe(x).nil?
end

soft_no_method_error {takes_my_module(MyObject.new)}
soft_no_method_error {takes_my_module(MyBasicObject.new)}

