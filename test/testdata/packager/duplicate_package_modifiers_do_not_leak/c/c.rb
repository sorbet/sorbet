# typed: strict

module C
  A::NotExported # error: `A::NotExported` resolves but is not exported from `A`
  B::Exported
end
