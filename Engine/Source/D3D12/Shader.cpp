#include "Shader.hpp"

#include "Vector.hpp"

#include <utility>

namespace Illulu::D3D12
{
    Shader::Shader()
    {
        if (!m_dxcUtils)
        {
            WIN_CHECK(DxcCreateInstance(
                CLSID_DxcUtils, 
                IID_PPV_ARGS(&m_dxcUtils)
            ));
        }
        if (!m_dxcCompiler)
        {
            WIN_CHECK(DxcCreateInstance(
                CLSID_DxcCompiler,
                IID_PPV_ARGS(&m_dxcCompiler)
            ));
        }
    }
    
    void Shader::Init(StringView input, ShaderType type)
    {
        m_inputPath = input;
        m_type = type;

        if (static_cast<u8>(type & ShaderType::VERTEX) == 0 || static_cast<u8>(type & ShaderType::PIXEL) == 0)
        {
            FATAL(L"Shader file on the input has no vertex or pixel shader");
        }
    }

    bool Shader::Compile(bool permissiveMode)
    {
        if (m_inputPath.empty() || static_cast<u8>(m_type) == 0)
        {
            FATAL(L"Compiling uninitialized shader!");
            return false;
        }
        
        _Reset();
        ComPtr<IDxcBlobEncoding> source;

        u32 codePage{DXC_CP_UTF8};
        WIN_CHECK(m_dxcUtils->LoadFile(
            m_inputPath.c_str(), 
            &codePage, 
            source.GetAddressOf()
        ));

        DxcBuffer sourceBuffer
        {
            .Ptr{source->GetBufferPointer()},
            .Size{source->GetBufferSize()},
            .Encoding{codePage}
        };

        for (u8 i{1}; i != 0; i <<= 1)
        {
            if ((static_cast<u8>(m_type) & i) == 0)
                continue; 

            ShaderType curr{static_cast<ShaderType>(i)};

            Vector<const wchar*> args
            {
                L"-WX", 
                L"-Zi",
                L"-Qembed_debug"
            };
            
            Vector<DxcDefine> defines
            {
            };

            ComPtr<IDxcCompilerArgs> dxcArgs;

            WIN_CHECK(m_dxcUtils->BuildArguments(
                m_inputPath.data(),
                GetEntryPoint(curr),
                GetProfile(curr),
                args.data(),
                static_cast<u32>(args.size()),
                defines.data(),
                static_cast<u32>(defines.size()),
                dxcArgs.GetAddressOf()
            ));

            ILL_ASSERT(dxcArgs);

            ComPtr<IDxcResult> dxcRes;

            WIN_CHECK(m_dxcCompiler->Compile(
                &sourceBuffer, 
                dxcArgs->GetArguments(), 
                dxcArgs->GetCount(),
                nullptr,
                IID_PPV_ARGS(dxcRes.GetAddressOf())
            ));

            ComPtr<IDxcBlobUtf8> dxcDiagnostics;

            WIN_CHECK(dxcRes->GetOutput(
                DXC_OUT_ERRORS, 
                IID_PPV_ARGS(&dxcDiagnostics), 
                nullptr
            ));

            if (dxcDiagnostics && dxcDiagnostics->GetStringLength() != 0)
            {
                if (permissiveMode)
                    WARNING(L"Shader compilation error: {}", Utf8ToWide(dxcDiagnostics->GetStringPointer()));
                else
                    FATAL(L"Shader compilation error: {}", Utf8ToWide(dxcDiagnostics->GetStringPointer()));
                return false;
            }

            HRESULT hrStatus;
            if (!WIN_OK(dxcRes->GetStatus(&hrStatus)) || !WIN_OK(hrStatus))
            {
                if (permissiveMode)
                    WARNING(L"Shader compilation failed");
                else
                    FATAL(L"Shader compilation failed");
                return false;
            }
            
            auto& blob{m_blobs[ShaderTypeToIndex(curr)]};
            WIN_OK(dxcRes->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blob), nullptr));
        }
        return true;
    }

    D3D12_SHADER_BYTECODE Shader::GetBlob(ShaderType type) const
    {
        u32 index{ShaderTypeToIndex(type)};
        
        auto& blob{m_blobs[index]};
        
        return blob ? D3D12_SHADER_BYTECODE{blob->GetBufferPointer(), blob->GetBufferSize()}
                    : D3D12_SHADER_BYTECODE{};
    }
    void Shader::_Reset()
    {
        for (auto& blob : m_blobs)
        {
            blob.Reset();
        }
    }
}
