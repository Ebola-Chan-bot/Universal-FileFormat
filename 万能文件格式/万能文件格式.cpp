#include"万能文件格式.hpp"
#ifndef __cpp_lib_modules
#include<sstream>
#endif
#undef min
static constexpr uint64_t 魔数 = 15264649811907286980ull;
struct 分配块
{
	uint64_t 上块;
	uint64_t 偏移;
	uint64_t 大小;
	uint64_t 下块;
};
namespace 万能文件格式
{
	struct _文件头
	{
		uint64_t 魔数;
		uint64_t 分配块个数;
		uint64_t 第一块索引;
		分配块 分配块区[];
	};

	//指针分配器

	static void 设置上下块(_文件头* 文件头指针, uint64_t 上块索引, uint64_t 本块索引, uint64_t 下块索引)noexcept
	{
		if (上块索引 == 无效值)
			文件头指针->第一块索引 = 本块索引;
		else
			文件头指针->分配块区[上块索引].下块 = 本块索引;
		if (下块索引 != 无效值)
			文件头指针->分配块区[下块索引].上块 = 本块索引;
	}
	static void 搜索空隙(_文件头* 文件头指针, uint64_t 新块索引, uint64_t 字节数)noexcept
	{
		分配块 新块值{ 无效值, sizeof(_文件头) + sizeof(分配块) * 文件头指针->分配块个数, 字节数, 文件头指针->第一块索引 };
		do
		{
			const 分配块& 下块引用 = 文件头指针->分配块区[新块值.下块];
			if (下块引用.偏移 >= 新块值.偏移 + 新块值.大小)
				break;
			新块值.偏移 = 下块引用.偏移 + 下块引用.大小;
			新块值.上块 = 新块值.下块;
			新块值.下块 = 下块引用.下块;
		} while (新块值.下块 != 无效值);
		设置上下块(文件头指针, 新块值.上块, 新块索引, 新块值.下块);
		文件头指针->分配块区[新块索引] = 新块值;
	}
	static void 需挪块先耗尽();
	static void 空隙先耗尽();
	static void 分配块区扩张(指针分配器* 分配器, uint64_t 新块索引, uint64_t 字节数)
	{
		_文件头* const& 文件头指针 = 分配器->_文件头指针;
		const uint64_t 原本分配块个数 = 文件头指针->分配块个数;
		文件头指针->分配块个数 = (新块索引 + 1) * 2;
		// 新扩展的块区都要填充无效值，因此可以预分配
		const uint64_t 分配块区尾偏移 = sizeof(_文件头) + sizeof(分配块) * 文件头指针->分配块个数;
		分配器->分配空间(分配块区尾偏移);
		uint64_t 需挪首块索引 = 文件头指针->第一块索引;
		分配块 新块值{ 无效值, 分配块区尾偏移, 字节数, 无效值 };
		分配块* 分配块区 = 文件头指针->分配块区;
		if (需挪首块索引 == 无效值)
		{
			std::fill_n(分配块区, 文件头指针->分配块个数, 分配块{ 无效值, 无效值, 无效值, 无效值 });
			文件头指针->第一块索引 = 新块索引;
			文件头指针->分配块区[新块索引] = 新块值;
			return;
		}
		分配块* 需挪首块指针 = 分配块区 + 需挪首块索引;
		uint64_t 需挪尾块索引 = 需挪首块指针->下块;
		for (;;)
		{
			if (需挪尾块索引 == 无效值)
			{
				空隙先耗尽();
				return;
			}
			const 分配块& 需挪尾块引用 = 分配块区[需挪尾块索引];
			if (需挪尾块引用.偏移 >= 分配块区尾偏移)
				break;
			需挪尾块索引 = 需挪尾块引用.下块;
		}
		分配块* 寻隙块指针 = 分配块区 + 需挪尾块索引;
		if (新块值.偏移 + 需挪首块指针->大小 > 寻隙块指针->偏移)
		{
			文件头指针->第一块索引 = 需挪尾块索引;
			do
			{
				if (寻隙块指针->下块 == 无效值)
				{
					空隙先耗尽();
					return;
				}
				新块值.偏移 = 寻隙块指针->偏移 + 寻隙块指针->大小;
			} while (新块值.偏移 + 需挪首块指针->大小 > (寻隙块指针 = 分配块区 + 寻隙块指针->下块)->偏移);
		}
		需挪首块指针->上块 = 寻隙块指针->上块;//寻隙块前方有空间，因此寻隙块的上块将变成需挪块的上块
		分配器->分配空间(新块值.偏移 + 需挪首块指针->大小);
		for (;;)
		{
			需挪首块指针 = (分配块区 = 文件头指针->分配块区) + 需挪首块索引;
			分配块* 中间块指针 = 分配块区 + 需挪首块指针->上块;
			寻隙块指针 = 分配块区 + (新块值.下块 = 中间块指针->下块);//中间块此时是需挪块的上块，其原本的下块就是寻隙块
			中间块指针->下块 = 需挪首块索引;//将当前需挪块索引设置为上块的下块
			memcpy(reinterpret_cast<char*>(文件头指针) + 新块值.偏移, reinterpret_cast<const char*>(文件头指针) + 需挪首块指针->偏移, 需挪首块指针->大小);
			需挪首块指针->偏移 = 新块值.偏移;
			新块值.偏移 += 需挪首块指针->大小;
			if (需挪首块指针->下块 == 需挪尾块索引)
			{
				需挪块先耗尽();
				return;
			}
			中间块指针 = 分配块区 + 需挪首块指针->下块;
			if (新块值.偏移 + 中间块指针->大小 > 寻隙块指针->偏移)
			{
				需挪首块指针->下块 = 新块值.下块;//新块值的下块就是寻隙块，寻隙块之前无法容纳更多块，因此当前需挪块的下块就是寻隙块
				寻隙块指针->上块 = 需挪首块索引;
				do
				{
					新块值.偏移 = 寻隙块指针->偏移 + 寻隙块指针->大小;
					if (寻隙块指针->下块 == 无效值)
					{
						空隙先耗尽();
						return;
					}
					寻隙块指针 = 分配块区 + 寻隙块指针->下块;
				} while (新块值.偏移 + 中间块指针->大小 > 寻隙块指针->偏移);
				中间块指针->上块 = 寻隙块指针->上块;
			}
			需挪首块索引 = 需挪首块指针->下块;
			分配器->分配空间(新块值.偏移 + 中间块指针->大小);
		}
		// 为所有新块填充无效值
		std::fill(文件头指针->分配块区 + 原本分配块个数, 文件头指针->分配块区 + 文件头指针->分配块个数, 分配块{ 无效值, 无效值, 无效值, 无效值 });
		设置上下块(文件头指针,需挪首块索引, 新块索引, 需挪尾块索引);
		return 新块值;
	}
	void 指针分配器::初始化(bool 新建)
	{
		分配空间(sizeof(_文件头));
		if (新建)
			*_文件头指针 = { 魔数, 0, 无效值 };
		else if (_文件头指针->魔数 != 魔数)
			throw std::domain_error("试图打开一个无效的文件");
		else
			分配空间(sizeof(_文件头) + _文件头指针->分配块个数 * sizeof(分配块));
	}
	uint64_t 指针分配器::分配(uint64_t 字节数)
	{
		uint64_t 新块索引;
		for (新块索引 = 0; 新块索引 < _文件头指针->分配块个数 && _文件头指针->分配块区[新块索引].偏移 != 无效值; 新块索引++);
		_文件头指针->分配块区[新块索引] = 新块索引 < _文件头指针->分配块个数 ? 搜索空隙(_文件头指针, 字节数, 新块索引) : 分配块区扩张(this, 新块索引, 字节数);
		return 新块索引;
	}
	void 指针分配器::分配(uint64_t 句柄, uint64_t 字节数)
	{
		if (句柄 < _文件头指针->分配块个数)
		{
			分配块 块值 = _文件头指针->分配块区[句柄]; // 无法维持指针有效且所有字段都被用到，不如直接拷贝
			块值.大小 = 字节数;
			if (块值.偏移 == 无效值)
				块值 = 搜索空隙(_文件头指针, 块值.大小, 句柄);
			else if (块值.下块 != 无效值)
			{
				if (_文件头指针->分配块区[块值.下块].偏移 < 块值.偏移 + 块值.大小)
				{
					_文件头指针->分配块区[块值.下块].上块 = 块值.上块;
					if (块值.上块 == 无效值)
						_文件头指针->第一块索引 = 块值.下块;
					else
						_文件头指针->分配块区[块值.上块].下块 = 块值.下块;
					块值 = 搜索空隙(_文件头指针, 块值.大小, 句柄);
				}
			}
			_文件头指针->分配块区[句柄] = 块值;
		}
		else
			_文件头指针->分配块区[句柄] = 分配块区扩张(this, 句柄, 字节数);
	}
	void* 指针分配器::取指针(uint64_t 句柄)
	{
		if (句柄 >= _文件头指针->分配块个数)
			return nullptr;
		const 分配块& 分配块引用 = _文件头指针->分配块区[句柄];
		if (分配块引用.偏移 == 无效值)
			return nullptr;
		分配空间(分配块引用.偏移 + 分配块引用.大小);
		return reinterpret_cast<char*>(_文件头指针) + 分配块引用.偏移;
	}
	uint64_t 指针分配器::块大小(uint64_t 句柄)const noexcept
	{
		return 句柄 < _文件头指针->分配块个数 ? _文件头指针->分配块区[句柄].大小 : 无效值;
	}
	void 指针分配器::释放(uint64_t 句柄)const noexcept
	{
		if (句柄 >= _文件头指针->分配块个数)
			return;
		分配块& 旧块引用 = _文件头指针->分配块区[句柄];
		if (旧块引用.偏移 == 无效值)
			return;
		旧块引用.偏移 = 无效值;
		const uint64_t 上块索引 = 旧块引用.上块;
		const uint64_t 下块索引 = 旧块引用.下块;
		if (上块索引 == 无效值)
			_文件头指针->第一块索引 = 下块索引;
		else
			_文件头指针->分配块区[上块索引].下块 = 下块索引;
		if (下块索引 != 无效值)
			_文件头指针->分配块区[下块索引].上块 = 上块索引;
	}
	文件分配器::文件分配器(HANDLE 文件句柄, 文件选项 选项) :文件句柄(文件句柄), 只读(选项 == 文件选项::只读), _映射句柄(nullptr)
	{
		_文件头指针 = nullptr;
		switch (选项)
		{
		case 文件选项::只读:
			初始化(false);
			break;
		case 文件选项::读写:
		{
			LARGE_INTEGER 文件大小;
			GetFileSizeEx(文件句柄, &文件大小);
			初始化(文件大小.QuadPart < sizeof(_文件头));
		}
			break;
		case 文件选项::覆盖:
			初始化(true);
			break;
		default:
			throw std::invalid_argument("未知的文件选项");
		}
	}
	文件分配器::~文件分配器()
	{
		UnmapViewOfFile(_文件头指针);
		CloseHandle(_映射句柄);
	}
	void 文件分配器::分配空间(uint64_t 字节数)
	{
		LARGE_INTEGER 文件大小;
		GetFileSizeEx(文件句柄, &文件大小);
		if (字节数 > 文件大小.QuadPart)
			文件大小.QuadPart = 字节数 * 2;
		else if (_文件头指针)
			return;
		UnmapViewOfFile(_文件头指针);
		CloseHandle(_映射句柄);
		_映射句柄 = CreateFileMapping(文件句柄, nullptr, 只读 ? PAGE_READONLY : PAGE_READWRITE, 文件大小.HighPart, 文件大小.LowPart, nullptr);
		if (!_映射句柄)
			throw std::system_error(GetLastError(), std::system_category(), "CreateFileMapping失败");
		_文件头指针 = reinterpret_cast<_文件头*>(MapViewOfFile(_映射句柄, 只读 ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0));
		if (!_文件头指针)
		{
			const DWORD 错误码 = GetLastError();
			CloseHandle(_映射句柄);
			_映射句柄 = nullptr;
			throw std::system_error(GetLastError(), std::system_category(), "MapViewOfFile失败");
		}
	}

	//流式分配器

	struct 伪文件头
	{
		uint64_t 分配块个数;
		uint64_t 第一块索引;
	};
	static void 设置上下块(std::ostream& 基础流, uint64_t 上块索引, uint64_t 本块索引, uint64_t 下块索引)
	{
		if (上块索引 == 无效值)
		{
			基础流.seekp(offsetof(_文件头, 第一块索引));
			基础流.write(reinterpret_cast<const char*>(&本块索引), sizeof(本块索引));
		}
		else
		{
			基础流.seekp(offsetof(_文件头, 分配块区) + offsetof(分配块, 下块) + sizeof(分配块) * 上块索引 );
			基础流.write(reinterpret_cast<const char*>(&本块索引), sizeof(本块索引));
		}
		if (下块索引 != 无效值)
		{
			基础流.seekp(offsetof(_文件头, 分配块区) + offsetof(分配块, 上块) + sizeof(分配块) * 下块索引 );
			基础流.write(reinterpret_cast<const char*>(&本块索引), sizeof(本块索引));
		}
	}
	static 分配块 搜索空隙(std::iostream&基础流, uint64_t 字节数, uint64_t 块索引)noexcept
	{
		伪文件头 文件头;
		基础流.seekg(offsetof(_文件头, 分配块个数));
		基础流.read(reinterpret_cast<char*>(&文件头), sizeof(伪文件头));
		分配块 新块值{ 无效值, sizeof(_文件头) + sizeof(分配块) * 文件头.分配块个数, 字节数, 文件头.第一块索引 };
		do
		{
			分配块 下块;
			基础流.seekg(offsetof(_文件头, 分配块区) + sizeof(分配块) * 新块值.下块);
			基础流.read(reinterpret_cast<char*>(&下块), sizeof(下块));
			if (下块.偏移 >= 新块值.偏移 + 新块值.大小)
				break;
			新块值.偏移 = 下块.偏移 + 下块.大小;
			新块值.上块 = 新块值.下块;
			新块值.下块 = 下块.下块;
		} while (新块值.下块 != 无效值);
		设置上下块(基础流, 新块值.上块, 块索引, 新块值.下块);
		return 新块值;
	}
	流式分配器::流式分配器(std::iostream& 基础流, bool 新建) :基础流(基础流)
	{
		constexpr _文件头 新文件头{ 魔数, 0, 无效值 };
		if (新建)
		{
			基础流.seekp(0);
			基础流.write(reinterpret_cast<const char*>(&新文件头), sizeof(新文件头));
		}
		else
		{
			uint64_t 实际魔数;
			基础流.seekg(offsetof(_文件头, 魔数));
			基础流.read(reinterpret_cast<char*>(&实际魔数), sizeof(实际魔数));
			if (实际魔数 != 魔数)
			{
				基础流.seekp(0);
				基础流.write(reinterpret_cast<const char*>(&新文件头), sizeof(新文件头));
			}
		}
	}
	uint64_t 流式分配器::分配(uint64_t 字节数)
	{
		uint64_t 新块索引;
		uint64_t 分配块个数;
		基础流.seekg(offsetof(_文件头, 分配块个数));
		基础流.read(reinterpret_cast<char*>(&分配块个数), sizeof(分配块个数));
		for (新块索引 = 0; 新块索引 < 分配块个数; 新块索引++)
		{
			uint64_t 偏移;
			基础流.seekg(offsetof(_文件头, 分配块区) + offsetof(分配块, 偏移) + sizeof(分配块) * 新块索引);
			基础流.read(reinterpret_cast<char*>(&偏移), sizeof(偏移));
			if (偏移 == 无效值)
				break;
		}
		const 分配块 新块值 = 新块索引 < 分配块个数 ? 搜索空隙(基础流, 字节数, 新块索引) : 分配块区扩张(基础流, 新块索引, 字节数);
		基础流.seekp(offsetof(_文件头, 分配块区) + sizeof(分配块) * 新块索引);
		基础流.write(reinterpret_cast<const char*>(&新块值), sizeof(新块值));
		return 新块索引;
	}
}