#include "bililive/secret/utils/bililive_crypto.h"

#include <array>
#include <cstdint>
#include <stdio.h>
#include <stdint.h>

#include "base/base64.h"
#include "base/logging.h"

namespace {

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

const int kBlockSize = 8;
const int KDataSize = 2;

using DataBlock = uint32_t[2];
using TeaKey = std::array<uint32_t, 4>;

const TeaKey kChosenKey = { 0x24ff2b1c, 0x5a768e08, 0x645a9bc7, 0x8e53a4c9 };

/// <summary>
/// XXTEA加解密
/// </summary>
/// <param name="data">要加密的数据是两个32位无符号整数</param>
/// <param name="n">n的绝对值表示data的长度，取正表示加密，取负表示解密</param>
/// <param name="key">key为加密解密密钥，为4个32位无符号整数，即密钥长度为128位</param>
void XXTEA(DataBlock data, int n, const TeaKey& key)
{
    uint32_t y, z, sum;
    unsigned p, rounds, e;
    if (n > 1)            /* EnCoding Part */
    {
        rounds = 6 + 52 / n;
        sum = 0;
        z = data[n - 1];
        do
        {
            sum += DELTA;
            e = (sum >> 2) & 3;
            for (p = 0; p < n - 1; p++)
            {
                y = data[p + 1];
                z = data[p] += MX;
            }
            y = data[0];
            z = data[n - 1] += MX;
        } while (--rounds);
    }
    else if (n < -1)      /* Decoding Part */
    {
        n = -n;
        rounds = 6 + 52 / n;
        sum = rounds * DELTA;
        y = data[0];
        do
        {
            e = (sum >> 2) & 3;
            for (p = n - 1; p > 0; p--)
            {
                z = data[p - 1];
                y = data[p] -= MX;
            }
            z = data[n - 1];
            y = data[0] -= MX;
            sum -= DELTA;
        } while (--rounds);
    }
}

}   // namespace

namespace secret {

std::string BililiveEncrypt(const std::string& plaintext)
{
    if (plaintext.empty()) {
        return std::string();
    }

    int round = (plaintext.size() + kBlockSize - 1) / kBlockSize;

    // Use null-terminator as padding.
    std::vector<char> data(round * kBlockSize, 0);
    std::copy(plaintext.cbegin(), plaintext.cend(), data.begin());

    auto data_blocks = reinterpret_cast<DataBlock*>(data.data());
    for (int i = 0; i < round; ++i) {
        XXTEA(*(data_blocks + i), KDataSize, kChosenKey);
    }

    std::string ciphertext;
    base::Base64Encode(base::StringPiece(data.data(), data.size()), &ciphertext);

    return ciphertext;
}

std::string BililiveDecrypt(const std::string& ciphertext)
{
    if (ciphertext.empty()) {
        return std::string();
    }

    std::string data;
    if (!base::Base64Decode(ciphertext, &data)) {
        NOTREACHED() << "Cipher text should be in base64 encoding!";
        return std::string();
    }

    if (data.size() % kBlockSize != 0) {
        NOTREACHED() << "Corrupted cipher data!";
        return std::string();
    }

    int round = data.size() / kBlockSize;

    auto data_blocks = reinterpret_cast<DataBlock*>(&data[0]);
    for (int i = 0; i < round; ++i) {
        XXTEA(*(data_blocks + i), -KDataSize, kChosenKey);
    }

    // Eliminate paddings if necessary.
    auto it = std::find_if_not(data.rbegin(), data.rend(), [](const char& value) { return value == 0; });
    if (it != data.rbegin()) {
        data.erase(it.base(), data.end());
    }

    return data;
}

}   // namespace secret
