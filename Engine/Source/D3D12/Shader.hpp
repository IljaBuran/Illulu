#pragma once

#include "Common.hpp"

#include "DX12.hpp"

namespace Illulu::D3D12
{
    enum class ShaderType : u8
    {
        VERTEX   = 1u,
        HULL     = 1u << 1,
        DOMAIN_  = 1u << 2,
        GEOMETRY = 1u << 3,
        PIXEL    = 1u << 4,
        COMPUTE  = 1u << 5,

        AMPLIFICATION = 1u << 6,
        MESH          = 1u << 7,
    };

    constexpr ShaderType operator|(ShaderType lhs, ShaderType rhs)
    {
        return static_cast<ShaderType>(static_cast<u8>(lhs) | static_cast<u8>(rhs));
    }

    constexpr ShaderType operator&(ShaderType lhs, ShaderType rhs)
    {
        return static_cast<ShaderType>(static_cast<u8>(lhs) & static_cast<u8>(rhs));
    }

    constexpr u32 ShaderTypeToIndex(ShaderType type) noexcept
    {
        return std::countr_zero(std::to_underlying(type));
    }

    constexpr const wchar* GetEntryPoint(ShaderType type)
    {
        switch (type)
        {
            case ShaderType::VERTEX:        return L"VSMain";
            case ShaderType::HULL:          return L"HSMain";
            case ShaderType::DOMAIN_:       return L"DSMain";
            case ShaderType::GEOMETRY:      return L"GSMain";
            case ShaderType::PIXEL:         return L"PSMain";
            case ShaderType::COMPUTE:       return L"CSMain";
            case ShaderType::AMPLIFICATION: return L"ASMain";
            case ShaderType::MESH:          return L"MSMain";
        }
        std::unreachable();
    }

    constexpr const wchar* GetProfile(ShaderType type)
    {
        switch (type)
        {
            case ShaderType::VERTEX:        return L"vs_6_8";
            case ShaderType::HULL:          return L"hs_6_8";
            case ShaderType::DOMAIN_:       return L"ds_6_8";
            case ShaderType::GEOMETRY:      return L"gs_6_8";
            case ShaderType::PIXEL:         return L"ps_6_8";
            case ShaderType::COMPUTE:       return L"cs_6_8";
            case ShaderType::AMPLIFICATION: return L"as_6_8";
            case ShaderType::MESH:          return L"ms_6_8";
        }
        std::unreachable();
    }

    class Shader
    {
    public:

        Shader();
        
        void Init(StringView input, ShaderType type);

        bool Compile(bool permissiveMode = false);

        D3D12_SHADER_BYTECODE GetBlob(ShaderType type) const;

    private:

        void _Reset();

    private:
        
        inline static ComPtr<IDxcUtils>     m_dxcUtils;
        inline static ComPtr<IDxcCompiler3> m_dxcCompiler;

        ShaderType m_type{0};
        String m_inputPath;

        //FileWatcher

        Array<ComPtr<IDxcBlob>, 8> m_blobs;
    };
}
