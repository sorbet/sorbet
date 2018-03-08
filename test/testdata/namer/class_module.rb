# @typed
class TestClass
end

module TestClass # error: `TestClass` was previously defined as a `class`
end

class TestClass
end

module TestMod
end

class TestMod # error: `TestMod` was previously defined as a `module`
end
