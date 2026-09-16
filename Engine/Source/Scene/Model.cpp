#include "Scene/Model.hpp"

// TODO: delete this when abstracting ModelLoader
#define CGLTF_IMPLEMENTATION
#pragma warning(disable : 4996)
#include "cgltf.h"

namespace Illulu
{
    Vector<PBRMaterial> g_materialStorage{};

    Vector<Mesh> LoadModel(NarrowString path)
    {
        cgltf_data* data{};
        cgltf_options opts{};

        cgltf_result res = cgltf_parse_file(&opts, path.data(), &data);
        if (res != cgltf_result_success)
        {
            FATAL(L"LoadModel: parsing failed");
        }

        res = cgltf_load_buffers(&opts, data, path.data());
        if (res != cgltf_result_success)
        {
            FATAL(L"LoadModel: loading buffers failed");
        }

        res = cgltf_validate(data);
        if (res != cgltf_result_success)
        {
            FATAL(L"LoadModel: validation failed");
        }

        for (cgltf_size i{ 0 }; i < data->buffer_views_count; ++i)
        {
            if (data->buffer_views[i].has_meshopt_compression)
            {
                FATAL(L"LoadModel: meshopt not implemented yet");
            }
        }

        Vector<Mesh> meshes;
        meshes.reserve(data->meshes_count);

        Vector<PBRMaterial> importedMaterials;

        // here we can hash the pointer
        HashMap<const cgltf_material*, MaterialHandle> materialHandles;

        for (cgltf_size meshIdx{ 0 }; meshIdx < data->meshes_count; ++meshIdx)
        {
            const cgltf_mesh& meshSrc{ data->meshes[meshIdx] };
            Mesh mesh;
            mesh.primitives.reserve(meshSrc.primitives_count);

            for (cgltf_size primitiveIdx{ 0 }; primitiveIdx < meshSrc.primitives_count; ++primitiveIdx)
            {
                const cgltf_primitive& primitiveSrc{ meshSrc.primitives[primitiveIdx] };
                if (primitiveSrc.type != cgltf_primitive_type_triangles)
                {
                    FATAL(L"LoadModel: only triangle primitives are currently supported");
                }

                const cgltf_accessor* posAccessor{ cgltf_find_accessor(&primitiveSrc, cgltf_attribute_type_position, 0) };
                if (!posAccessor)
                {
                    FATAL(L"LoadModel: primitive has no POSITION attribute");
                }

                const cgltf_accessor* normAccessor{ cgltf_find_accessor(&primitiveSrc, cgltf_attribute_type_normal, 0) };
                if (!normAccessor)
                {
                    FATAL(L"LoadModel: primitive has no NORMAL attribute");
                }

                const cgltf_accessor* texCoordAccessor{ cgltf_find_accessor(&primitiveSrc, cgltf_attribute_type_texcoord, 0) };
                if (!texCoordAccessor)
                {
                    FATAL(L"LoadModel: primitive has no TEXCOORD_0 attribute");
                }

                if (posAccessor->count != normAccessor->count ||
                    posAccessor->count != texCoordAccessor->count)
                {
                    FATAL(L"LoadModel: mismatched vertex attribute counts");
                }

                // Decode glTF accessors

                Vector<f32> positions(posAccessor->count * 3);
                Vector<f32> normals(normAccessor->count * 3);
                Vector<f32> texCoords(texCoordAccessor->count * 2);

                if (cgltf_accessor_unpack_floats(posAccessor, positions.data(), positions.size()) != positions.size())
                {
                    FATAL(L"LoadModel: failed to unpack POSITION");
                }

                if (cgltf_accessor_unpack_floats(normAccessor, normals.data(), normals.size()) != normals.size())
                {
                    FATAL(L"LoadModel: failed to unpack NORMAL");
                }

                if (cgltf_accessor_unpack_floats(texCoordAccessor, texCoords.data(), texCoords.size()) != texCoords.size())
                {
                    FATAL(L"LoadModel: failed to unpack TEXCOORD_0");
                }

                // Convert to custom Illulu Vertex

                Vector<Vertex> vertices(posAccessor->count);

                for (cgltf_size vertexIdx{ 0 };
                     vertexIdx < posAccessor->count;
                     ++vertexIdx)
                {
                    Vertex& v{ vertices[vertexIdx] };

                    v.position =
                    {
                        positions[vertexIdx * 3 + 0],
                        positions[vertexIdx * 3 + 1],
                        positions[vertexIdx * 3 + 2]
                    };

                    v.normal =
                    {
                        normals[vertexIdx * 3 + 0],
                        normals[vertexIdx * 3 + 1],
                        normals[vertexIdx * 3 + 2]
                    };

                    v.tex0 =
                    {
                        texCoords[vertexIdx * 2 + 0],
                        texCoords[vertexIdx * 2 + 1]
                    };
                }

                // Indices

                Vector<u32> indices;

                if (primitiveSrc.indices)
                {
                    const cgltf_accessor* indexAccessor{ primitiveSrc.indices };

                    ILL_ASSERT(indexAccessor->count < U32_MAX);
                    
                    indices.resize(indexAccessor->count);

                    for (cgltf_size indexIdx{ 0 }; indexIdx < indexAccessor->count; ++indexIdx)
                    {
                        indices[indexIdx] = 
                            static_cast<u32>(cgltf_accessor_read_index(indexAccessor, indexIdx));
                    }
                }
                else
                {
                    indices.resize(posAccessor->count);

                    for (cgltf_size i{ 0 }; i < posAccessor->count; ++i)
                    {
                        indices[i] = static_cast<u32>(i);
                    }
                }

                mesh.primitives.emplace_back(
                    Primitive
                    {
                        .vertices{ std::move(vertices) },
                        .indices{ std::move(indices) },
                        .materialHandle{}
                    }
                );
            }
        }

        g_materialStorage.append_range(importedMaterials);

        return Vector<Mesh>{};
    }
}