# typed: false

class A
  include T::Props

  const :'foo-bar', Integer # error: Bad attribute name `foo-bar`
  prop :'left-right', String # error: Bad attribute name `left-right`
end
