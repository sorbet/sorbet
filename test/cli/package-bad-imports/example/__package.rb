# typed: strict

class Some::Example < PackageSpec
  # this shouldn't find anything similar
  import NonexistentPackage
  # this should NOT try to suggest `DoNot::SuggestThis` can suggest `Suggestion` though
  import DoNot::SUggestThis
  # this should correctly suggest the package `Suggestion`
  import SUggestion
  # a name with three parts, for testing reasons
  import A::B::C
end
