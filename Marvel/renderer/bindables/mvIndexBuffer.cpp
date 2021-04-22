#include "mvIndexBuffer.h"
#include "mvGraphics.h"

namespace Marvel
{

    mvIndexBuffer::mvIndexBuffer(mvGraphics& graphics, const std::vector<unsigned short>& indices, bool dynamic)
    {

        m_count = (UINT)indices.size();
        m_indexBuffer = mvComPtr<ID3D11Buffer>();
        m_data = indices;
        m_dynamic = dynamic;

        // Fill in a buffer description.
        D3D11_BUFFER_DESC bufferDesc = {};

        if (m_dynamic)
        {
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            m_buffersize = m_count * sizeof(unsigned short) + 10000;
        }
        else
        {
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.CPUAccessFlags = 0u;
            m_buffersize = UINT(sizeof(unsigned short) * m_count);
        }

        bufferDesc.ByteWidth = m_buffersize;
        bufferDesc.StructureByteStride = sizeof(unsigned short);
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.MiscFlags = 0u;

        // Define the resource data.
        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = m_data.data();

        // Create the buffer with the device.
        graphics.getDevice()->CreateBuffer(&bufferDesc, &InitData, m_indexBuffer.GetAddressOf());

    }

    void mvIndexBuffer::bind(mvGraphics& graphics)
    {
        update(graphics);
        graphics.getContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
    }

    void mvIndexBuffer::update(mvGraphics& graphics)
    {
        if (!m_dynamic)
            return;
        
        m_count = m_data.size();

        if (m_buffersize < m_data.size()*sizeof(unsigned short))
        {
            m_indexBuffer->Release();
            m_buffersize = m_count*sizeof(unsigned short) + 10000;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.ByteWidth = m_buffersize;
            bufferDesc.StructureByteStride = sizeof(unsigned short);
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.MiscFlags = 0u;
            graphics.getDevice()->CreateBuffer(&bufferDesc, nullptr, m_indexBuffer.GetAddressOf());
        }

        // Upload vertex/index data into a single contiguous GPU buffer
        D3D11_MAPPED_SUBRESOURCE resource;

        if (graphics.getContext()->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource) != S_OK)
            return;

        memcpy((unsigned short*)resource.pData, m_data.data(), m_data.size() * sizeof(unsigned short));

        graphics.getContext()->Unmap(m_indexBuffer.Get(), 0);
    }

    UINT mvIndexBuffer::getCount() const 
    { 
        return m_count; 
    }

    bool mvIndexBuffer::isDynamic() const
    {
        return m_dynamic;
    }

    std::vector<unsigned short>& mvIndexBuffer::getData()
    {
        return m_data;
    }

}