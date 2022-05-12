# frozen_string_literal: true
# typed: strict

module Project::Foo::FooNonForcing
  extend T::Sig

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.global_check_is_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar") # error: Unable to resolve constant `::Bar`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_not_keyword_arg(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", "Project::Bar") # error: Too many positional arguments
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_wrong_keyword_arg(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", pakige: "Project::Bar") # error: Unrecognized keyword argument
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_package_not_a_string(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", package: 5) # error: Expected `T.nilable(String)` but found `Integer(5)`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_check_for_global_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", package: "Project::Bar") # error: should not be an absolute constant reference
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_non_existent_package(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "Bar", package: "Project::Quux") # error: Unable to resolve constant `::Project::Quux`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.check_for_non_existing_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "Quux", package: "Project::Bar") # error: Unable to resolve constant `::Project::Bar::Quux`
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.good_check_is_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "Bar", package: "Project::Bar")
      true
    else
      false
    end
  end
end
