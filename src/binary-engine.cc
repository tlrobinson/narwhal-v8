#include <narwhal.h>
#include <stdlib.h>
#include <string.h>

// FIXME: leaks forevvvverrrrrrr

OBJECT(BYTES, 2, unsigned char* buffer, unsigned int length)
{
    unsigned int *len = (unsigned int*)malloc(sizeof(unsigned int));
    *len = length;
    
    INTERNAL(0, buffer);
    INTERNAL(1, len);
    
    return self;
}
END

FUNCTION(B_ALLOC, PINT(length))
{
    unsigned char* buffer = (unsigned char*)calloc(length, sizeof(char));
    if (!buffer)
        THROW("B_ALLOC: Couldn't alloc buffer");
    
    return Persistent<Object>::New(BYTES(buffer, length));
}
END

FUNCTION(B_LENGTH, POBJ(bytes))
{
    EXTERNAL(unsigned int*, len, bytes, 1);
    unsigned int length = *len;

    return JS_int(length);
}
END

FUNCTION(B_FILL, POBJ(bytes), PINT(from), PINT(to), PINT(value))
{
    EXTERNAL(unsigned char*, buffer, bytes, 0);
    EXTERNAL(unsigned int*, len, bytes, 1);
    unsigned int length = *len;
    
    if (from > to || from < 0 || to >= length)
        THROW("B_FILL: tried to fill beyond bounds");
    
    memset(buffer + from, value, to - from);
    
    return JS_undefined;
}
END

FUNCTION(B_COPY, POBJ(src), PINT(srcOffset), POBJ(dst), PINT(dstOffset), PINT(length))
{
    EXTERNAL(unsigned char*, src_buffer, src, 0);
    EXTERNAL(unsigned char*, dst_buffer, dst, 0);
    EXTERNAL(unsigned int*, src_len, src, 1);
    EXTERNAL(unsigned int*, dst_len, dst, 1);
    unsigned int src_length = *src_len;
    unsigned int dst_length = *dst_len;
    
    if (srcOffset + length > src_length || dstOffset + length > dst_length)
        THROW("B_COPY: tried to copy beyond bounds");

    memcpy(dst_buffer + dstOffset, src_buffer + srcOffset, length);
    
    return JS_undefined;
}
END

FUNCTION(B_GET, POBJ(bytes), PINT(index))
{
    EXTERNAL(unsigned char*, buffer, bytes, 0);
    EXTERNAL(unsigned int*, len, bytes, 1);
    unsigned int length = *len;
    
    if (index >= length)
        THROW("B_GET: tried get beyond bounds");
    
    return JS_int(buffer[index]);
}
END

FUNCTION(B_SET, POBJ(bytes), PINT(index), PINT(value))
{
    EXTERNAL(unsigned char*, buffer, bytes, 0);
    EXTERNAL(unsigned int*, len, bytes, 1);
    unsigned int length = *len;
    
    if (index >= length) 
        THROW("B_SET: tried set beyond bounds");
    if (value < 0 || value >= 256)
        THROW("B_SET: tried to set out of byte range");
    
    buffer[index] = (unsigned char)value;
    
    return JS_undefined;
}
END

FUNCTION(B_DECODE, POBJ(bytes), PINT(offset), PINT(length), PSTR(codec))
{
    THROW("NYI");
}
END

FUNCTION(B_DECODE_DEFAULT, POBJ(bytes), PINT(offset), PINT(length))
{
    THROW("NYI");
}
END

FUNCTION(B_ENCODE, PSTR(string), PSTR(codec))
{
    THROW("NYI");
}
END

FUNCTION(B_ENCODE_DEFAULT, PSTR(string))
{
    THROW("NYI");
}
END

FUNCTION(B_TRANSCODE, POBJ(bytes), PINT(offset), PINT(length), PSTR(sourceCodec), PSTR(targetCodec))
{
    THROW("NYI");
}
END

NARWHAL_MODULE(binary_platform)
{
    EXPORTS("B_LENGTH", JS_fn(B_LENGTH));
    EXPORTS("B_ALLOC", JS_fn(B_ALLOC));
    EXPORTS("B_FILL", JS_fn(B_FILL));
    EXPORTS("B_COPY", JS_fn(B_COPY));
    EXPORTS("B_GET", JS_fn(B_GET));
    EXPORTS("B_SET", JS_fn(B_SET));
    EXPORTS("B_DECODE", JS_fn(B_DECODE));
    EXPORTS("B_DECODE_DEFAULT", JS_fn(B_DECODE_DEFAULT));
    EXPORTS("B_ENCODE", JS_fn(B_ENCODE));
    EXPORTS("B_ENCODE_DEFAULT", JS_fn(B_ENCODE_DEFAULT));
    EXPORTS("B_TRANSCODE", JS_fn(B_TRANSCODE));
}
END_NARWHAL_MODULE
