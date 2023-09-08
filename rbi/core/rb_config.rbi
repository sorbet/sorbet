# typed: __STDLIB_INTERNAL

# The module storing Ruby interpreter configurations on building.
#
# This file was created by mkconfig.rb when ruby was built. It contains build
# information for ruby which is used e.g. by mkmf to build compatible native
# extensions. Any changes made to this file will be lost the next time ruby is
# built.
module RbConfig
  # The hash configurations stored.
  CONFIG = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # [`DESTDIR`](https://docs.ruby-lang.org/en/2.7.0/RbConfig.html#DESTDIR) on
  # make install.
  DESTDIR = T.let(T.unsafe(nil), String)
  # Almost same with
  # [`CONFIG`](https://docs.ruby-lang.org/en/2.7.0/RbConfig.html#CONFIG).
  # [`MAKEFILE_CONFIG`](https://docs.ruby-lang.org/en/2.7.0/RbConfig.html#MAKEFILE_CONFIG)
  # has other variable reference like below.
  #
  # ```ruby
  # MAKEFILE_CONFIG["bindir"] = "$(exec_prefix)/bin"
  # ```
  #
  # The values of this constant is used for creating Makefile.
  #
  # ```
  # require 'rbconfig'
  #
  # print <<-END_OF_MAKEFILE
  # prefix = #{Config::MAKEFILE_CONFIG['prefix']}
  # exec_prefix = #{Config::MAKEFILE_CONFIG['exec_prefix']}
  # bindir = #{Config::MAKEFILE_CONFIG['bindir']}
  # END_OF_MAKEFILE
  #
  # => prefix = /usr/local
  #    exec_prefix = $(prefix)
  #    bindir = $(exec_prefix)/bin  MAKEFILE_CONFIG = {}
  # ```
  #
  # [`RbConfig.expand`](https://docs.ruby-lang.org/en/2.7.0/RbConfig.html#method-c-expand)
  # is used for resolving references like above in rbconfig.
  #
  # ```ruby
  # require 'rbconfig'
  # p Config.expand(Config::MAKEFILE_CONFIG["bindir"])
  # # => "/usr/local/bin"
  # ```
  MAKEFILE_CONFIG = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # Ruby installed directory.
  TOPDIR = T.let(T.unsafe(nil), String)

  sig {returns(String)}
  def self.ruby; end
end
