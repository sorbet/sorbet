# frozen_string_literal: true
# typed: ignore

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'tmpdir'

class Sorbet; end
module Sorbet::Private; end
module Sorbet::Private::Test; end
class Sorbet::Private::Test::Empty < MiniTest::Spec
  it 'works on an empty example' do
    Dir.mktmpdir do |dir|
      Dir.foreach(__dir__ + '/empty/') do |item|
        FileUtils.cp_r(__dir__ + '/empty/' + item, dir)
      end

      olddir = __dir__
      Dir.chdir dir

      out = IO.popen(
        {'SRB_YES' => '1'},
        olddir + '/../../../bin/srb-rbi',
        'r+', &:read
      )
      out = out.gsub(/with [0-9]+ modules and [0-9]+ aliases/, 'with %d modules and %d aliases')

      if ENV['RECORD']
        File.write(olddir + '/empty.out', out)
      end
      assert_equal(File.read(olddir + '/empty.out'), out)
      assert_equal(true, $?.success?)

      assert_dirs_equal(olddir + '/sorbet', dir + '/sorbet')
    end
  end

  def assert_dirs_equal(expdir, dir)
    seen = []
    Dir.foreach(dir) do |item|
      seen << item
      next if (item == ".") || (item == "..")
      expfile = expdir + '/' + item
      file = dir + '/' + item
      if File.directory?(file)
        if ENV['RECORD'] && !Dir.exist?(expfile)
          Dir.mkdir(expfile)
        end
        assert_dirs_equal(expfile, file)
      else
        if ENV['RECORD']
          File.write(expfile, File.read(file))
        end
        puts "#{expfile} != #{file}"
        puts File.read(expfile) if File.exist?(expfile)
        puts File.read(file) if File.exist?(file)
        # assert_equal(File.read(expfile), File.read(file), "#{expfile} != #{file}")
      end
    end
    Dir.foreach(expdir) do |item|
      next if seen.include?(item)
      expfile = expdir + '/' + item
      flunk("#{expfile} is missing in output")
    end
  end
end
