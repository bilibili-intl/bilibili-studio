#ifndef BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H
#define BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H

#include <string>

namespace bililive
{
	// ֻ�ж�ǰ��https://***/ ������,����Ĳ��������ж�
	// ����ֻ����bilibili.com ��bilibili.co, ����������������
	std::string FastForwardChangeEnv(const std::string& url);
	std::wstring FastForwardChangeEnv(const std::wstring& url);
}

#endif //BILILIVE_BILILIVE_UTILS_FAST_FORWARD_URL_CONVERT_H