#include "Common.hpp"

#include "DX12.hpp"

namespace Illulu::D3D12
{
	class Factory
	{
	public:

		Factory();

		[[nodiscard]]
		IDXGIFactoryIll* const GetFactoryPtr()
		{
			ILL_ASSERT(m_dxgiFactory);
			return m_dxgiFactory.Get();
		}

		[[nodiscard]]
		IDXGIFactoryIll* const operator->()
		{
			return GetFactoryPtr();
		}

		operator IDXGIFactoryIll*()
		{
			return m_dxgiFactory.Get();
		}
		
		Factory(const Factory&) = delete;
		Factory(Factory&&) = delete;
		void operator=(const Factory&) = delete;
		void operator=(Factory&&) = delete;

	private:

		ComPtr<IDXGIFactoryIll> m_dxgiFactory;
	};
}