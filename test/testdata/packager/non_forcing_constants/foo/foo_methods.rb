# typed: strict

module FooMethods
  extend T::Sig

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def global_check_is_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar") # error: Unable to resolve constant `::Bar`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def bad_check_for_global_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", "Project::Bar") # error: should not be an absolute constant reference
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def check_for_non_existing_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "Quux", "Project::Bar") # error: Unable to resolve constant `::<PackageRegistry>::Project_Bar_Package::Quux`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def good_check_is_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "Bar", "Project::Bar")
      true
    else
      false
    end
  end
end
