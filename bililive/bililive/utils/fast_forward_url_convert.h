#ifndef BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H
#define BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H

#include <string>

namespace bililive
{
	// 只判断前面https://***/ 的域名,后面的参数不做判断
	// 域名只拦截bilibili.com 和bilibili.co, 其他的域名不处理
	std::string FastForwardChangeEnv(const std::string& url);
	std::wstring FastForwardChangeEnv(const std::wstring& url);
}

#endif //BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H