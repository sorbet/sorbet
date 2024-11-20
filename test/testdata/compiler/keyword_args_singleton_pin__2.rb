# frozen_string_literal: true
# typed: true
# compiled: true

# This file is compiled, which means we will create a keywordArgsSingleton for
# it. This will be allocated on Ruby's heap. The Init_ function will have a
# reference to this on the C stack as a local (depending on how the
# optimizations shake out), but there will be no naturally-occurring reference
# to this Hash otherwise because the Ruby GC does not look for pointers into
# its heap in shared objects' memory spaces.
#
# So once this compiled file loads (i.e., the require finishes), there will be
# no reference to the keywordArgsSingleton, unless we mark it for the GC.

class Main
  def self.main
    Helper.requires_keyword_args(x: [0], y: 548)
  end
end
