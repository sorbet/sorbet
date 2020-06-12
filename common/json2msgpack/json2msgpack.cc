#include "json2msgpack.h"

using namespace rapidjson;

namespace sorbet::json2msgpack {
void json2msgpack(rapidjson::Value &value, mpack_writer_t *writer) {
    switch (value.GetType()) {
        case kTrueType:
            mpack_write_true(writer);
            break;
        case kFalseType:
            mpack_write_false(writer);
            break;
        case kNullType:
            mpack_write_nil(writer);
            break;

        case kNumberType:
            if (value.IsDouble()) {
                mpack_write_double(writer, value.GetDouble());
            } else if (value.IsUint64()) {
                mpack_write_u64(writer, value.GetUint64());
            } else {
                mpack_write_i64(writer, value.GetInt64());
            }
            break;

        case kStringType:
            mpack_write_str(writer, value.GetString(), value.GetStringLength());
            break;

        case kArrayType: {
            mpack_start_array(writer, value.Size());
            Value::ValueIterator it = value.Begin(), end = value.End();
            for (; it != end; ++it) {
                json2msgpack(*it, writer);
            }
            mpack_finish_array(writer);
            break;
        }

        case kObjectType: {
            mpack_start_map(writer, value.MemberCount());
            Value::MemberIterator it = value.MemberBegin(), end = value.MemberEnd();
            for (; it != end; ++it) {
                mpack_write_str(writer, it->name.GetString(), it->name.GetStringLength());
                json2msgpack(it->value, writer);
            }
            mpack_finish_map(writer);
            break;
        }

        default:
            Exception::notImplemented();
    }

    return;
}
} // namespace sorbet::json2msgpack
