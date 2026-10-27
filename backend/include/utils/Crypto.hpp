#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>

namespace tradex::utils {

class Crypto {
public:
    /**
     * @brief Computes SHA-256 hexadecimal hash string.
     */
    static std::string sha256(const std::string& input) {
        uint32_t h[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bitLen = data.size() * 8;
        data.push_back(0x80);
        while ((data.size() % 64) != 56) {
            data.push_back(0x00);
        }
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
        }

        auto rotr = [](uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); };

        for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
            uint32_t w[64];
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<uint32_t>(data[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 3]));
            }
            for (int i = 16; i < 64; ++i) {
                uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            uint32_t e = h[4], f = h[5], g = h[6], h_val = h[7];

            for (int i = 0; i < 64; ++i) {
                uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h_val + S1 + ch + k[i] + w[i];
                uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;

                h_val = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
        }

        std::ostringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << std::setw(8) << std::setfill('0') << h[i];
        }
        return ss.str();
    }

    /**
     * @brief Computes SHA-1 hash for WebSocket RFC 6455 handshake.
     */
    static std::vector<uint8_t> sha1(const std::string& input) {
        uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bitLen = data.size() * 8;
        data.push_back(0x80);
        while ((data.size() % 64) != 56) data.push_back(0x00);
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
        }

        auto rol = [](uint32_t value, size_t bits) { return (value << bits) | (value >> (32 - bits)); };

        for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
            uint32_t w[80];
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<uint32_t>(data[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 3]));
            }
            for (int i = 16; i < 80; ++i) {
                w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
            for (int i = 0; i < 80; ++i) {
                uint32_t f, k;
                if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
                else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
                else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
                else { f = b ^ c ^ d; k = 0xCA62C1D6; }

                uint32_t temp = rol(a, 5) + f + e + k + w[i];
                e = d; d = c; c = rol(b, 30); b = a; a = temp;
            }
            h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
        }

        std::vector<uint8_t> digest(20);
        for (int i = 0; i < 5; ++i) {
            digest[i * 4] = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xFF);
        }
        return digest;
    }

    /**
     * @brief Encodes bytes into a Base64 string.
     */
    static std::string base64Encode(const std::vector<uint8_t>& data) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        for (uint8_t c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }

    static std::string base64Encode(const std::string& str) {
        std::vector<uint8_t> data(str.begin(), str.end());
        return base64Encode(data);
    }

    /**
     * @brief Computes WebSocket Accept key according to RFC 6455.
     */
    static std::string computeWebSocketAccept(const std::string& clientKey) {
        static const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string combined = clientKey + guid;
        auto hash = sha1(combined);
        return base64Encode(hash);
    }

    /**
     * @brief Hash password with salt.
     */
    static std::string hashPassword(const std::string& password) {
        static const std::string salt = "tradex_secure_salt_";
        return sha256(salt + password);
    }
};

} // namespace tradex::utils
