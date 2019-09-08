# typed: __STDLIB_INTERNAL

# Some versions of the
# [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html) 1.1 RC series
# introduced corrupted lockfiles. There were two major problems:
#
# *   multiple copies of the same GIT section appeared in the lockfile
# *   when this happened, those sections got multiple copies of gems in those
#     sections.
#
#
# As a result, [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html) 1.1
# contains code that fixes the earlier corruption. We will remove this fix-up
# code in [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html) 1.2.
# Ruby 1.9.3 and old RubyGems don't play nice with frozen version strings
# rubocop:disable MutableConstant
module Bundler
  sig {returns(::T.untyped)}
  def self.load(); end
end
