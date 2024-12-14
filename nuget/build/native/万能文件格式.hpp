#pragma once
#ifdef __cpp_lib_modules
import std;
#else
#include<memory_resource>
#endif
namespace 万能文件格式
{
	//用于标识文件头的魔数，确保不可能存在任何与此重复的有效数据。
	constexpr uint64_t 魔数[2] = { 15264649811907286980ull,2192407666978137515ull };
	struct 文件内存资源:std::pmr::memory_resource
	{
		void* const 文件句柄;
		void* 映射句柄;
		void* 映射指针;
		//文件句柄来自 Win32 CreateFile API。此对象不拥有文件句柄，用户负责关闭文件句柄。如果不希望手动管理文件句柄，请使用`独占文件内存资源`。
		文件内存资源(void* const 文件句柄);
		~文件内存资源();
	protected:
		void* do_allocate(size_t bytes, size_t alignment) override;
		void do_deallocate(void* p, size_t bytes, size_t alignment) override;
		bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
	};
	//此对象析构时会自动关闭文件
	struct 独占文件内存资源 :文件内存资源
	{
		独占文件内存资源(const char* 文件路径);
		独占文件内存资源(const wchar_t* 文件路径);
		~独占文件内存资源();
	};
}