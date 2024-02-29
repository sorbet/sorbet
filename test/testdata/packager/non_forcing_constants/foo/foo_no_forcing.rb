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
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", "Project::Bar")
      #                                                       ^^^^^^^^^^^^^^ error: Too many arguments
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_wrong_keyword_arg(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", pakige: "Project::Bar")
      #                                              ^^^^^^^ error: Unable to resolve constant
      #                                                       ^^^^^^^^^^^^^^^^^^^^^^ error: Too many arguments
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.bad_package(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Bar", package: 5)
      #                                              ^^^^^^^ error: Unable to resolve constant
      #                                                       ^^^^^^^^^^ error: Too many arguments
      true
    else
      false
    end
  end

  sig {params(arg: T.untyped).returns(T::Boolean)}
  def self.good_check_is_bar(arg)
    if T::NonForcingConstants.non_forcing_is_a?(arg, "::Project::Bar::Bar")
      true
    else
      false
    end
  end
end
