#pragma once
#include "common.h"
#include "gfx/render_types.h"

namespace gfx {

    enum class AttributeType { Byte, UByte, Short, UShort, Int, UInt, Half, Float };
    enum class AttributeName {
        Position,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        TexCoord4,
        TexCoord5,
        TexCoord6,
        Normal,
        Tangent,
        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6
    };

    struct VertexAttribute
    {
        AttributeName name;
        AttributeType type;
        ushort count;
        uint offset;
        bool normalized;
        ushort divisor;
        ushort binding;
        ushort stride;
        bool enabled;
    };

    using AttributeList = std::vector<VertexAttribute>;

    class VertexDecl
    {
    public:

        VertexDecl() = default;
        ~VertexDecl() = default;

        bool empty() const;
        VertexDecl& begin();
        VertexDecl& end();
        VertexDecl& switch_buffer();
        size_t size() const;
        const AttributeList& attributes() const;
        VertexDecl& add(AttributeName name, AttributeType type, ushort count, bool normalized, ushort stride, bool enabled = true, ushort binding = 0, ushort divisor = 0);
        void setHandle(VertexLayoutHandle handle);
        VertexLayoutHandle handle() const;
        const VertexAttribute& operator[](const size_t i) const;

    private:
        static ushort getTypeSize(AttributeType type);
        ushort offset_{};
        VertexLayoutHandle handle_;
        AttributeList attributes_;
    };

}
