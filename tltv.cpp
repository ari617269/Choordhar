#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

int main() {
    // --- Constants ---
    const uint8_t TYPE_INT    = 0x01;
    const uint8_t TYPE_BOOL   = 0x02;
    const uint8_t TYPE_STRING = 0x04;
    const uint8_t TYPE_ARRAY  = 0x05;

    const uint16_t TAG_KEY0 = 0x0001;
    const uint16_t TAG_KEY1 = 0x0002;
    const uint16_t TAG_KEY2 = 0x0003;
    const uint16_t TAG_KEY3 = 0x0004; // array of INT

    const uint16_t ARR_ELEM_TAG = 0x0001; // elements inside array

    // --- Basic helpers as lambdas (big-endian) ---
    auto writeUint16Be = [](std::vector<uint8_t>& out, uint16_t v){
        out.push_back((uint8_t)((v >> 8) & 0xFF));
        out.push_back((uint8_t)(v & 0xFF));
    };
    auto writeUint32Be = [](std::vector<uint8_t>& out, uint32_t v){
        out.push_back((uint8_t)((v >> 24) & 0xFF));
        out.push_back((uint8_t)((v >> 16) & 0xFF));
        out.push_back((uint8_t)((v >> 8) & 0xFF));
        out.push_back((uint8_t)(v & 0xFF));
    };
    auto writeInt64Be = [](std::vector<uint8_t>& out, int64_t v){
        uint64_t uv = (uint64_t)v;
        out.push_back((uint8_t)((uv >> 56) & 0xFF));
        out.push_back((uint8_t)((uv >> 48) & 0xFF));
        out.push_back((uint8_t)((uv >> 40) & 0xFF));
        out.push_back((uint8_t)((uv >> 32) & 0xFF));
        out.push_back((uint8_t)((uv >> 24) & 0xFF));
        out.push_back((uint8_t)((uv >> 16) & 0xFF));
        out.push_back((uint8_t)((uv >> 8) & 0xFF));
        out.push_back((uint8_t)(uv & 0xFF));
    };

    auto readUint16Be = [](const std::vector<uint8_t>& b, size_t pos)->uint16_t{
        return (uint16_t(b[pos]) << 8) | uint16_t(b[pos+1]);
    };
    auto readUint32Be = [](const std::vector<uint8_t>& b, size_t pos)->uint32_t{
        return (uint32_t(b[pos]) << 24) | (uint32_t(b[pos+1]) << 16) | (uint32_t(b[pos+2]) << 8) | uint32_t(b[pos+3]);
    };
    auto readInt64Be = [](const std::vector<uint8_t>& b, size_t pos)->int64_t{
        uint64_t uv = ((uint64_t)b[pos] << 56) | ((uint64_t)b[pos+1] << 48) | ((uint64_t)b[pos+2] << 40) | ((uint64_t)b[pos+3] << 32)
                    | ((uint64_t)b[pos+4] << 24) | ((uint64_t)b[pos+5] << 16) | ((uint64_t)b[pos+6] << 8)  | (uint64_t)b[pos+7];
        return (int64_t)uv;
    };

    // --- Example data to serialize ---
    bool key0 = true;
    int64_t key1 = 123456789;
    std::string key2 = "hello, TLTV minimal";
    std::vector<int64_t> key3 = {10, 20, 30};

    // --- Serialize into buffer ---
    std::vector<uint8_t> buf;

    auto writeTLTV = [&](uint16_t tag, uint8_t type, const std::vector<uint8_t>& value){
        writeUint16Be(buf, tag);
        buf.push_back(type);
        writeUint32Be(buf, (uint32_t)value.size());
        buf.insert(buf.end(), value.begin(), value.end());
    };

    // key0 (bool)
    {
        std::vector<uint8_t> v;
        v.push_back(key0 ? 0x01 : 0x00);
        writeTLTV(TAG_KEY0, TYPE_BOOL, v);
    }

    // key1 (int64)
    {
        std::vector<uint8_t> v;
        writeInt64Be(v, key1);
        writeTLTV(TAG_KEY1, TYPE_INT, v);
    }

    // key2 (string)
    {
        std::vector<uint8_t> v(key2.begin(), key2.end());
        writeTLTV(TAG_KEY2, TYPE_STRING, v);
    }

    // key3 (array of int64) - represent as nested TLTVs using ARR_ELEM_TAG
    {
        std::vector<uint8_t> inner;
        for (size_t i = 0; i < key3.size(); ++i) {
            // write element TLTV inside inner
            writeUint16Be(inner, ARR_ELEM_TAG);
            inner.push_back(TYPE_INT);
            writeUint32Be(inner, 8);
            writeInt64Be(inner, key3[i]);
        }
        writeTLTV(TAG_KEY3, TYPE_ARRAY, inner);
    }

    // --- Hex dump ---
    printf("Hex dump (%zu bytes):\n", buf.size());
    for (size_t off = 0; off < buf.size(); off += 16) {
        printf("%08zx: ", off);
        for (size_t i = 0; i < 16; ++i) {
            if (off + i < buf.size()) printf("%02X ", buf[off + i]); else printf("   ");
        }
        printf(" ");
        for (size_t i = 0; i < 16 && off + i < buf.size(); ++i) {
            unsigned char c = buf[off + i];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");
    }

    // --- Parse TLTVs ---
    bool got_key0 = false;
    bool got_key0_val = false;
    bool got_key1 = false;
    int64_t got_key1_val = 0;
    bool got_key2 = false;
    std::string got_key2_val;
    std::vector<int64_t> got_key3_vals;

    size_t pos = 0;
    while (pos + 7 <= buf.size()) { // header minimal: 2+1+4 = 7
        uint16_t tag = readUint16Be(buf, pos);
        uint8_t type = buf[pos + 2];
        uint32_t length = readUint32Be(buf, pos + 3);
        size_t valuePos = pos + 7;
        if (valuePos + length > buf.size()) { printf("truncated entry"); break; }

        if (tag == TAG_KEY0 && type == TYPE_BOOL && length == 1) {
            got_key0 = true;
            got_key0_val = (buf[valuePos] != 0x00);
        } else if (tag == TAG_KEY1 && type == TYPE_INT && length == 8) {
            got_key1 = true;
            got_key1_val = readInt64Be(buf, valuePos);
        } else if (tag == TAG_KEY2 && type == TYPE_STRING) {
            got_key2 = true;
            got_key2_val.assign((const char*)(&buf[valuePos]), length);
        } else if (tag == TAG_KEY3 && type == TYPE_ARRAY) {
            // parse nested TLTVs inside this array region
            size_t ip = valuePos;
            size_t endp = valuePos + length;
            while (ip + 7 <= endp) {
                uint16_t etag = readUint16Be(buf, ip);
                uint8_t etype = buf[ip + 2];
                uint32_t elen = readUint32Be(buf, ip + 3);
                size_t evalue = ip + 7;
                if (evalue + elen > endp) { printf("truncated inner entry"); break; }
                if (etag == ARR_ELEM_TAG && etype == TYPE_INT && elen == 8) {
                    int64_t v = readInt64Be(buf, evalue);
                    got_key3_vals.push_back(v);
                }
                ip = evalue + elen;
            }
        }

        pos = valuePos + length;
    }

    // --- Print deserialized values ---
    printf("Deserialized MyData:");
    if (got_key0) printf("key0: %s\n", got_key0_val ? "true" : "false"); else printf("key0: (missing)");
    if (got_key1) printf("key1: %lld\n", (long long)got_key1_val); else printf("key1: (missing)");
    if (got_key2) printf("key2: \"%s\"\n", got_key2_val.c_str()); else printf("key2: (missing)");
    printf("key3: [");
    for (size_t i = 0; i < got_key3_vals.size(); ++i) {
        if (i) printf(", ");
        printf("%lld", (long long)got_key3_vals[i]);
    }
    printf("]\n");

    return 0;
}

