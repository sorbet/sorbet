build: FORCE
	bazel build //emscripten:sorbet-wasm.tar --config=webasm-darwin \
	&& tar -xvf ./bazel-bin/emscripten/sorbet-wasm.tar sorbet-wasm.wasm sorbet-wasm.js

FORCE: ;
