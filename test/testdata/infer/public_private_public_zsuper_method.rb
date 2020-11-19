# typed: true

class GrandParent
  def grand_parent_method; puts 'in grand_parent_method'; end
  def inspect; '#<GrandParent:...>'; end
end

class Parent < GrandParent
  private :grand_parent_method
end

class Child < Parent
  public :grand_parent_method
end

GrandParent.new.grand_parent_method

begin
  Parent.new.grand_parent_method # error: Non-private call to private method `grand_parent_method`
rescue NoMethodError => exn
  p exn
end

# This is ok, because Ruby dispatches straight to GrandParent#grand_parent_method
Child.new.grand_parent_method
