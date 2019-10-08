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
        Dir.chdir(File.join(dir, path)) do |_|

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

          # first we compare against a generated version

          # we encode the expects contents into a JSON file in the
          # test directory: while this is redundant with the full
          # generated file, this also keeps us robust against
          # accidentally committing a changed expectation file which
          # has removed something relevant we cared about (as expected
          # hidden.rbi files are rather large even for trivial
          # examples!)
          assert_equal(hidden, File.read(File.join(olddir, path, 'hidden.rbi.exp')))

          expectations = JSON.parse(File.read(File.join(dir, path, "expectations.json")))
          expectations.each do |exp|
            # we might expect some substring to /definitely/ appear in hidden.rbi
            if exp["assertion"] == "contains"
              assert_match(exp["substring"], hidden)
            # we also might expect some substring to definitely /not/ appear in hidden.rbi
            elsif exp["assertion"] == "omits"
              refute_match(exp["substring"], hidden)
            # we should also fail loudly if someone mistypeed one of the assertions
            else
              assert(false, "Unknown assertion: #{exp['assertion']}")
            end
          end
        end
      end
    end
  end
end
