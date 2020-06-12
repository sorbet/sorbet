#ifndef RUBY_TYPER_JSON2MSGPACK_H
#define RUBY_TYPER_JSON2MSGPACK_H

#include "common/common.h"
#include "mpack/mpack.h"
#include "rapidjson/document.h"

namespace sorbet::json2msgpack {
void json2msgpack(rapidjson::Value &value, mpack_writer_t *writer);
}
#endif
