#include <CppUnitTest.h>
#include <万能文件格式.hpp>
#ifndef __cpp_lib_modules
#include <queue>
#include <fstream>
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace 测试与示例
{
	TEST_CLASS(测试与示例)
	{
	public:
		//使用文件（指针）分配器在磁盘上创建文件并写出数据
		TEST_METHOD(创建磁盘文件)
		{
			const HANDLE 文件句柄 = CreateFileA("文件.bin", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			万能文件格式::文件分配器 分配器(文件句柄);
			分配器.初始化();
			分配器.分配(0, sizeof(int));
			*(int*)分配器.取指针(0) = 42;

			//可以在分配器析构之前关闭文件句柄，不影响分配器正常析构。
			CloseHandle(文件句柄);
		}
		//使用流式分配器读入磁盘上的文件
		TEST_METHOD(读取磁盘文件)
		{
			std::fstream 文件("文件.bin", std::ios::in | std::ios::out | std::ios::binary);
			万能文件格式::流式分配器 分配器(文件);
			int 缓冲区;
			分配器.读入(0, &缓冲区, sizeof(int));
			Assert::AreEqual(42, 缓冲区);
		}
	};
}
