#pragma once
#ifdef __cpp_lib_modules
import std;
#else
#include<memory_resource>
#endif
#ifdef _WIN32
#include<Windows.h>
#endif
#include<boost/interprocess/file_mapping.hpp>
namespace 万能文件格式
{
#ifdef _WIN32
	struct 文件内存资源
	{
		const HANDLE 文件句柄;
		HANDLE 映射句柄;
		LPVOID 映射指针;
		//使用 Win32 API CreateFile 以从路径获取文件句柄
		文件内存资源(HANDLE 文件句柄);
		~文件内存资源();
	};
#endif
}