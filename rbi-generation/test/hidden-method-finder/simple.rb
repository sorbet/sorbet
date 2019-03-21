# frozen_string_literal: true
# typed: ignore

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'tmpdir'

module SorbetRBIGeneration; end
module SorbetRBIGeneration::HiddenMethodFinder; end
module SorbetRBIGeneration::HiddenMethodFinder::Test; end

class SorbetRBIGeneration::HiddenMethodFinder::Test::Simple < MiniTest::Spec
  it 'works on a simple example' do

    Dir.mktmpdir do |dir|
      FileUtils.cp_r(__dir__ + '/simple/', dir)
      olddir = __dir__
      Dir.chdir dir

      IO.popen(
        olddir + '/../../lib/hidden-method-finder.rb'
      ) {|io| io.read}

      assert_equal(true, $?.success?)
      # Some day these can be snapshot tests, but this isn't stable enough for
      # that yet
      # assert_equal(File.read(olddir + '/simple.errors.txt'), File.read('rbi/hidden-methods/errors.txt'))
      # assert_equal(File.read(olddir + '/simple.hidden.rbi'), File.read('rbi/hidden-methods/hidden.rbi'))
      assert_match("class Foo\n  def bar()", File.read('rbi/hidden-methods/hidden.rbi'))
    end
  end
end
