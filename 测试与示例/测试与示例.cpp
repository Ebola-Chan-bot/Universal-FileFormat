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
			//此方法在磁盘上创建一个文件。为0号块分配空间并写入数据。自动分配一个块，并写入数据。
			万能文件格式::独占文件内存资源 文件("test.bin");
		}
	};
}
