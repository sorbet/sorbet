# typed: strict

# This is baked in
# M = Opus::DB::Model

# This file tests the ::M implicit alias
class M::MyModel
end

Opus::DB::Model::MyModel
