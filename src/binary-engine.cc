#include <narwhal.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

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
/*
FUNCTION(B_DECODE, POBJ(bytes), PINT(offset), PINT(length), PSTR(codec))
{
    EXTERNAL(unsigned char*, src, bytes, 0);
    
    char *dst = (char *)malloc(length*4);
    if (!dst)
        THROW("B_DECODE: malloc fail");
    
    iconv_t cd = iconv_open("UTF-8", *codec);
    if (!cd)
        THROW("B_DECODE: iconv_open fail");
    
    char *src_buf = (char*)src + offset, *dst_buf = dst;
    size_t src_bytes_left = length, dst_bytes_left = length*4;
    
    size_t size = iconv(cd, &src_buf, &src_bytes_left, &dst_buf, &dst_bytes_left);
    if (size < 0)
        THROW("B_DECODE: iconv error");
    
    if (iconv_close(cd))
        THROW("B_DECODE: iconv_close error");
    
    if (src_bytes_left < length)
        THROW("B_DECODE: buffer not big enough");
    
    Handle<String> str = JS_str2(dst, length*4 - dst_bytes_left);
    
    free(dst);
    
    return str;
}
END
*/
FUNCTION(B_DECODE_DEFAULT, POBJ(bytes), PINT(offset), PINT(length))
{
    EXTERNAL(unsigned char*, buffer, bytes, 0);
    return JS_str2((char *)(buffer + offset), length);
}
END
/*
FUNCTION(B_ENCODE, PUTF8(string), PSTR(codec))
{
    int length = string.length();
    char *dst = (char *)malloc(length*4);
    if (!dst)
        THROW("B_ENCODE: malloc fail");

    iconv_t cd = iconv_open(*codec, "UTF-8");
    if (!cd)
        THROW("B_ENCODE: iconv_open fail");

    char *src_buf = *string, *dst_buf = dst;
    size_t src_bytes_left = length, dst_bytes_left = length*4;
    

    size_t size = iconv(cd, &src_buf, &src_bytes_left, &dst_buf, &dst_bytes_left);
    if (size < 0)
        THROW("B_ENCODE: iconv error");
    
    if (iconv_close(cd))
        THROW("B_ENCODE: iconv_close error");
        
    if (src_bytes_left > 0)
        THROW("B_ENCODE: buffer not big enough");

    Handle<Object> bytes = Persistent<Object>::New(BYTES((unsigned char *)dst, length*4 - dst_bytes_left));

    free(dst);

    return bytes;
}
END
*/
FUNCTION(B_ENCODE_DEFAULT, PSTR(string))
{
    int length = string.length();
    
    unsigned char* buffer = (unsigned char*)calloc(length, sizeof(char));
    if (!buffer)
        THROW("B_ENCODE_DEFAULT: Couldn't alloc buffer");
    
    memcpy(buffer, *string, length);

    return Persistent<Object>::New(BYTES(buffer, length));
}
END

FUNCTION(B_TRANSCODE, POBJ(bytes), PINT(offset), PINT(length), PSTR(sourceCodec), PSTR(targetCodec))
{
    EXTERNAL(char*, src, bytes, 0);
    
    printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\nsrc=%p, offset=%d, length=%d, sourceCodec=%s, targetCodec=%s\n", src, offset, length, *sourceCodec, *targetCodec);
    
    int dst_length = length*10;
    char *dst = (char*)calloc(dst_length, 1);
    if (!dst)
        THROW("B_TRANSCODE: malloc fail");

    iconv_t cd = iconv_open(*targetCodec, *sourceCodec);
    if (cd == (iconv_t)-1)
        THROW("B_TRANSCODE: iconv_open fail");

    char *src_buf = (char*)(src + offset), *dst_buf = dst;
    size_t src_bytes_left = (size_t)length, dst_bytes_left = (size_t)dst_length, converted=0;
    
    printf("src=[%s] %d\n", src_buf, src_bytes_left);
    
    while (dst_bytes_left > 0)
    {
        printf("src_buf=%p, src_bytes_left=%d, dst_buf=%p, dst_bytes_left=%d, converted=%d\n", src_buf, src_bytes_left, dst_buf, dst_bytes_left, converted);
        if (src_bytes_left == 0)
            break;
        if ((converted = iconv(cd, &src_buf, &src_bytes_left, &dst_buf, &dst_bytes_left)) == (size_t)-1)
        {
            if (errno != EINVAL)
            {
                perror("B_TRANSCODE");
                THROW("B_TRANSCODE: iconv error");
            }
        }
        printf("converted=%d\n", converted);
    }
    
    if (dst_bytes_left >= sizeof (wchar_t))
        *((wchar_t *) dst_buf) = L'\0';
    
    if (iconv_close(cd))
        THROW("B_TRANSCODE: iconv_close error");
    
    if (src_bytes_left > 0)
        THROW("B_TRANSCODE: buffer not big enough");
        
    printf("dst=[%s] %d\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", dst, dst_length - dst_bytes_left);

    Handle<Object> bytes = Persistent<Object>::New(BYTES((unsigned char *)dst, dst_length - dst_bytes_left));

    free(dst);

    return bytes;
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
    //EXPORTS("B_DECODE", JS_fn(B_DECODE));
    EXPORTS("B_DECODE_DEFAULT", JS_fn(B_DECODE_DEFAULT));
    //EXPORTS("B_ENCODE", JS_fn(B_ENCODE));
    EXPORTS("B_ENCODE_DEFAULT", JS_fn(B_ENCODE_DEFAULT));
    EXPORTS("B_TRANSCODE", JS_fn(B_TRANSCODE));
    
    //EXPORTS("B_DECODE", JS_fn(B_DECODE_DEFAULT));
    //EXPORTS("B_ENCODE", JS_fn(B_ENCODE_DEFAULT));

    Handle<Object> impljs = require("binary-platform.js");
    EXPORTS("B_DECODE", OBJECT_GET(impljs, "B_DECODE"));
    EXPORTS("B_ENCODE", OBJECT_GET(impljs, "B_ENCODE"));
}
END_NARWHAL_MODULE
