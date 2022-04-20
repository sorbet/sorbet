# typed: true

class GrandParent1; end
class Parent1 < GrandParent1; end
class Child1 < Parent1; end

class Parent2
  extend T::Generic

  A1 = type_member {{fixed: Parent1}}
  A2 = type_member {{fixed: Parent1}}

  B = type_member {{lower: Parent1}}

  C = type_member {{upper: Parent1}}

  D1 = type_template {{fixed: Parent1}}
  D2 = type_template {{fixed: Parent1}}
end

class Child2 < Parent2
  extend T::Generic

  A1 = type_member {{fixed: Child1}}
  A2 = type_member {{fixed: GrandParent1}}

  B = type_member {{lower: Child1}}

  C = type_member {{upper: GrandParent1}}

  D1 = type_template {{lower: Child1, upper: Parent1}}
  D2 = type_template {{lower: Parent1, upper: GrandParent1}}
end
