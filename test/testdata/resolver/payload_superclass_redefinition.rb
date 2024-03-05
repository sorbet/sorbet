# typed: true

class Parent
end

class IRB::RelineInputMethod < Parent
                             # ^^^^^^ error: Parent of class `IRB::RelineInputMethod` redefined from `IRB::InputMethod` to `Parent`
end
