# @typed

# This file tests the ::M implicit alias
class M::MyModel
end

Opus::DB::Model::MyModel

M = Opus::DB::Model
