#include "Factory.hpp"

namespace Illulu::D3D12
{
	Factory::Factory()
	{
		u32 factoryFlags{0};
	
	#if defined(DX_DEBUG)
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	#endif defined()
		
		WIN_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
	}
}

