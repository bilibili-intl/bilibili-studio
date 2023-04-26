#ifndef BILILIVE_SECRET_UTILS_BILILIVE_CRYTPO_H_
#define BILILIVE_SECRET_UTILS_BILILIVE_CRYTPO_H_

#include <string>

namespace secret {

std::string BililiveEncrypt(const std::string& plaintext);

std::string BililiveDecrypt(const std::string& ciphertext);

}   // namespace secret

#endif  // BILILIVE_SECRET_UTILS_BILILIVE_CRYTPO_H_
