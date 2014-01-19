#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <cstring>
using namespace node;
using namespace v8;

namespace
{

enum ByteOrder
{
    kNative,
    kFlip
};

template <typename t>
t SwapBytes(const t& in)
{
    t out;
    const char* in_p = reinterpret_cast<const char*>(&in);
    char* out_p = reinterpret_cast<char*>(&out) + sizeof(t) - 1;

    for(; out_p >= reinterpret_cast<char*>(&out); --out_p, ++in_p)
    {
        *out_p = *in_p;
    }

    return out;
}

bool IsPlatformLittleEndian()
{
    int32_t one = 1;
    char* one_p = reinterpret_cast<char*>(&one);
    if(*one_p == 1)
        return true;
    return false;
}

ByteOrder GetByteOrder(const FunctionCallbackInfo<Value>& args)
{
    // default to native.
    if(!(args.Length() > 1))
        return kNative;

    Local<Value> arg = args[1];
    if(arg->IsString())
    {
        char utf8[12];
        arg->ToString()->WriteUtf8(utf8, 10);
        if(!std::strncmp(utf8, "big", 10))
            return IsPlatformLittleEndian() ? kFlip : kNative;
        if(!std::strncmp(utf8, "little", 10))
            return IsPlatformLittleEndian() ? kNative : kFlip;
    }

    return kNative;
}

template<typename t>
void unpackBuffer(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if(args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "You must provide at least one argument.")));
        return;
    }

    if(!Buffer::HasInstance(args[0]->ToObject())) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "The first argument must be a buffer")));
        return;
    }

    if(Buffer::Length(args[0]->ToObject()) != sizeof(t)) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Buffer is the incorrect length")));
        return;
    }

    ByteOrder order = GetByteOrder(args);

    t nativeType = *reinterpret_cast<t*>(Buffer::Data(args[0]->ToObject()));

    if(order == kFlip)
        nativeType = SwapBytes(nativeType);

    Local<Number> num = Number::New(nativeType);
    args.GetReturnValue().Set(num);
}

template<typename t>
void packBuffer(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if(args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "You must provide at least one argument.")));
        return;
    }

    if(!args[0]->IsNumber ()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "The first argument must be a number")));
        return;
    }

    ByteOrder order = GetByteOrder(args);

    Local<Number> num = args[0]->ToNumber();
    t nativeType = num->Value();

    if(order == kFlip)
        nativeType = SwapBytes(nativeType);

    Local<Object> buff = Buffer::New(reinterpret_cast<char*>(&nativeType), sizeof(nativeType));

    args.GetReturnValue().Set(buff);
}

}// private namespace

extern "C"
{
    static void init(Handle<Object> target)
    {
        NODE_SET_METHOD(target, "unpackFloat32", unpackBuffer<float>);
        NODE_SET_METHOD(target, "unpackFloat64", unpackBuffer<double>);
        NODE_SET_METHOD(target, "unpackInt8", unpackBuffer<int8_t>);
        NODE_SET_METHOD(target, "unpackInt16", unpackBuffer<int16_t>);
        NODE_SET_METHOD(target, "unpackInt32", unpackBuffer<int32_t>);
        NODE_SET_METHOD(target, "unpackInt64", unpackBuffer<int64_t>);
        NODE_SET_METHOD(target, "unpackUInt8", unpackBuffer<uint8_t>);
        NODE_SET_METHOD(target, "unpackUInt16", unpackBuffer<uint16_t>);
        NODE_SET_METHOD(target, "unpackUInt32", unpackBuffer<uint32_t>);
        NODE_SET_METHOD(target, "unpackUInt64", unpackBuffer<uint64_t>);
        NODE_SET_METHOD(target, "packFloat32", packBuffer<float>);
        NODE_SET_METHOD(target, "packFloat64", packBuffer<double>);
        NODE_SET_METHOD(target, "packInt8",  packBuffer<int8_t>);
        NODE_SET_METHOD(target, "packInt16", packBuffer<int16_t>);
        NODE_SET_METHOD(target, "packInt32", packBuffer<int32_t>);
        NODE_SET_METHOD(target, "packInt64", packBuffer<int64_t>);
        NODE_SET_METHOD(target, "packUInt8",  packBuffer<uint8_t>);
        NODE_SET_METHOD(target, "packUInt16", packBuffer<uint16_t>);
        NODE_SET_METHOD(target, "packUInt32", packBuffer<uint32_t>);
        NODE_SET_METHOD(target, "packUInt64", packBuffer<uint64_t>);
    }

    NODE_MODULE(binpack, init);
}