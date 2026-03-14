# typed: false
# disable-parser-comparison: true

lambda do |; block_local1| end
lambda do |; block_local1, block_local2| end
lambda do |; block_local1, block_local2, block_local3| end
lambda do |param1; block_local1| end
lambda do |param1; block_local1, block_local2| end
lambda do |param1; block_local1, block_local2, block_local3| end
lambda do |param1, param2; block_local1| end
lambda do |param1, param2; block_local1, block_local2| end
lambda do |param1, param2; block_local1, block_local2, block_local3| end

lambda { |; block_local1| }
lambda { |; block_local1, block_local2| }
lambda { |; block_local1, block_local2, block_local3| }
lambda { |param1; block_local1| }
lambda { |param1; block_local1, block_local2| }
lambda { |param1; block_local1, block_local2, block_local3| }
lambda { |param1, param2; block_local1| }
lambda { |param1, param2; block_local1, block_local2| }
lambda { |param1, param2; block_local1, block_local2, block_local3| }

-> (; block_local1) {}
-> (; block_local1, block_local2) {}
-> (; block_local1, block_local2, block_local3) {}
-> (param1; block_local1) {}
-> (param1; block_local1, block_local2) {}
-> (param1; block_local1, block_local2, block_local3) {}
-> (param1, param2; block_local1) {}
-> (param1, param2; block_local1, block_local2) {}
-> (param1, param2; block_local1, block_local2, block_local3) {}
