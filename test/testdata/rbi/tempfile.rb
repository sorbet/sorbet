# typed: true

require 'tempfile'

Tempfile.new('hello')
Tempfile.new(['hello', '.jpg'])
Tempfile.new('hello', '/tmp')
Tempfile.new('hello', '/tmp', encoding: 'ascii-8bit')
Tempfile.new('hello', '/tmp', mode: 0)
Tempfile.new('hello', '/tmp', mode: 0, encoding: 'ascii-8bit')

Tempfile.create('hello')
Tempfile.create(['hello', '.jpg'])
Tempfile.create('hello', '/tmp')
Tempfile.create('hello', '/tmp', encoding: 'ascii-8bit')
Tempfile.create('hello', '/tmp', mode: 0)
Tempfile.create('hello', '/tmp', mode: 0, encoding: 'ascii-8bit')

Tempfile.create('hello') do; end
Tempfile.create(['hello', '.jpg']) do; end
Tempfile.create('hello', '/tmp') do; end
Tempfile.create('hello', '/tmp', encoding: 'ascii-8bit') do; end
Tempfile.create('hello', '/tmp', mode: 0) do; end
Tempfile.create('hello', '/tmp', mode: 0, encoding: 'ascii-8bit') do; end

Tempfile.open('hello')
Tempfile.open(['hello', '.jpg'])
Tempfile.open('hello', '/tmp')
Tempfile.open('hello', '/tmp', encoding: 'ascii-8bit')
Tempfile.open('hello', '/tmp', mode: 0)
Tempfile.open('hello', '/tmp', mode: 0, encoding: 'ascii-8bit')

Tempfile.open('hello') do; end
Tempfile.open(['hello', '.jpg']) do; end
Tempfile.open('hello', '/tmp') do; end
Tempfile.open('hello', '/tmp', encoding: 'ascii-8bit') do; end
Tempfile.open('hello', '/tmp', mode: 0) do; end
Tempfile.open('hello', '/tmp', mode: 0, encoding: 'ascii-8bit') do; end

# returns the correct type from block
i = Tempfile.create('hello') { 1 }
T.assert_type!(i, Integer)

i = Tempfile.open('hello') { 1 }
T.assert_type!(i, Integer)

f = Tempfile.create('hello')
T.assert_type!(f, File)

tf = Tempfile.open('hello')
T.assert_type!(tf, Tempfile)
