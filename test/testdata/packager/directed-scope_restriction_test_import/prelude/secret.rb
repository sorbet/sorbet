# typed: true

# A root-level constant with the same name as `A::B::Secret`. Ruby's lexical lookup never reaches this from
# inside `module A::B`, because it finds `A::B::Secret` first.
module ::Secret
end
