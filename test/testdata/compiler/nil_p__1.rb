# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './nil_p__2'

extend T::Sig

module MyModule
  def calls_nil_p
    nil?
  end
end

class MyObject
  include ::MyModule
end

class MyBasicObject < BasicObject
  include ::MyModule
end

sig {params(blk: T.proc.void).void}
def soft_no_method_error(&blk)
  begin
    yield
  rescue NoMethodError => exn
    p exn.message
  end
end

obj_nil       = T.let(nil, Object)
obj_obj       = Object.new
basic_obj_nil = T.let(nil, BasicObject)
basic_obj_obj = BasicObject.new

soft_no_method_error {p obj_nil.nil?}
soft_no_method_error {p obj_obj.nil?}

soft_no_method_error {p basic_obj_nil.nil?}
soft_no_method_error {p basic_obj_obj.nil?}

soft_no_method_error {p MyObject.new.nil?}
soft_no_method_error {p MyObject.new.calls_nil_p}

soft_no_method_error {p MyBasicObject.new.nil?}
soft_no_method_error {p MyBasicObject.new.calls_nil_p}

def takes_my_module(x)
  p x.nil?
end

soft_no_method_error {takes_my_module(MyObject.new)}
soft_no_method_error {takes_my_module(MyBasicObject.new)}

