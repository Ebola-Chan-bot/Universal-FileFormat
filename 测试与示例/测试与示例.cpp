#include <CppUnitTest.h>
#include <万能文件格式.hpp>
#ifdef __cpp_lib_modules
import std;
#else
#include <queue>
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace 测试与示例
{
	TEST_CLASS(测试与示例)
	{
	public:
		//目前仅支持Windows
		TEST_METHOD(在磁盘上创建文件)
		{
			//此方法在磁盘上创建一个文件，向0号块写入一个std::unordered_map<std::string,自定义结构体>。然后自动分配一个空闲块，写入一个std::vector。关闭文件后，再尝试读入，看是否与写入的相同。
			struct 自定义结构体
			{
				char 字符串[10];
				std::queue<int>队列;
			};
		}
	};
}
