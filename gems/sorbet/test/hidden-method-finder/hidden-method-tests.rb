# frozen_string_literal: true
# typed: ignore

require 'json'
require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'tmpdir'

class Sorbet; end
module Sorbet::Private; end
module Sorbet::Private::HiddenMethodFinder; end
module Sorbet::Private::HiddenMethodFinder::Test; end

TEST_CASES = [
  "simple",
  "thorough",
]

class Sorbet::Private::HiddenMethodFinder::Test::Simple < MiniTest::Spec
  TEST_CASES.each do |path|

    it "works on the #{path} example" do
      Dir.mktmpdir do |dir|
        FileUtils.cp_r(File.join(__dir__, path), dir)
        olddir = __dir__
        Dir.chdir(File.join(dir, path, 'src')) do |_|

          # we'll swallow all the output unless there's an error, in
          # which case we'll print it out
          trace = IO.popen(
            olddir + '/../../lib/hidden-definition-finder.rb',
            err: [:child, :out]
          ) {|io| io.read}
          success = $?.success?

          unless success
            puts trace
          end
          assert_equal(true, success)

          # the hidden definitions file itself
          hidden = File.read('sorbet/rbi/hidden-definitions/hidden.rbi')

          # because we can't guarantee that this test is run in a
          # consistent Ruby or OS environment, we can't make
          # guarantees about the full file (which might have
          # e.g. different socket protocol constants) and so we
          # instead encode a set of substrings we expect to see
          expectations = JSON.parse(File.read(File.join(dir, path, "expectations.json")))
          expectations.each do |exp|
            # we might expect some substring to /definitely/ appear in hidden.rbi
            if exp["assertion"] == "contains"
              assert_match(exp["substring"], hidden)
            # we also might expect some substring to definitely /not/ appear in hidden.rbi
            elsif exp["assertion"] == "omits"
              refute_match(exp["substring"], hidden)
            # we should also fail loudly if someone mistyped one of the assertions
            else
              assert(false, "Unknown assertion: #{exp['assertion']}")
            end
          end
        end
      end
    end
  end
end
