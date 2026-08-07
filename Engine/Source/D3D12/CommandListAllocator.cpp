#include "CommandListAllocator.hpp"

namespace Illulu::D3D12
{
	void CommandListAllocator::Create(ID3D12DeviceIll* const device, D3D12_COMMAND_LIST_TYPE type)
	{
		WIN_CHECK(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_d3d12commandAllocator)));
	}
}
