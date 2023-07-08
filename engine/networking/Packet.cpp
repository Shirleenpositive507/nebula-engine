#include "Packet.h"
#include <cstring>
#include <algorithm>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

namespace nebula {

    uint32_t Packet::s_nextPacketId = 1;

    Packet::Packet()
        : m_sequenceNumber(0)
        , m_checksum(0)
        , m_fragmented(false)
        , m_packetId(0)
    {
    }

    void Packet::clear() {
        m_packet.clear();
        m_sequenceNumber = 0;
        m_checksum = 0;
        m_fragmented = false;
        m_packetId = 0;
        m_fragments.clear();
    }

    bool Packet::endOfPacket() const {
        return static_cast<bool>(m_packet);
    }

    const void* Packet::getData() const {
        return m_packet.getData();
    }

    size_t Packet::getDataSize() const {
        return m_packet.getDataSize();
    }

    void Packet::writeString(const std::string& value) {
        m_packet << value;
    }

    std::string Packet::readString() {
        std::string value;
        m_packet >> value;
        return value;
    }

    void Packet::writeVector2(float x, float y) {
        m_packet << x << y;
    }

    void Packet::readVector2(float& x, float& y) {
        m_packet >> x >> y;
    }

    void Packet::writeVector3(float x, float y, float z) {
        m_packet << x << y << z;
    }

    void Packet::readVector3(float& x, float& y, float& z) {
        m_packet >> x >> y >> z;
    }

    void Packet::writeVector4(float x, float y, float z, float w) {
        m_packet << x << y << z << w;
    }

    void Packet::readVector4(float& x, float& y, float& z, float& w) {
        m_packet >> x >> y >> z >> w;
    }

    void Packet::writeInt8(int8_t value) {
        m_packet << value;
    }

    int8_t Packet::readInt8() {
        int8_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt16(int16_t value) {
        m_packet << value;
    }

    int16_t Packet::readInt16() {
        int16_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt32(int32_t value) {
        m_packet << value;
    }

    int32_t Packet::readInt32() {
        int32_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt64(int64_t value) {
        m_packet << value;
    }

    int64_t Packet::readInt64() {
        int64_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeFloat(float value) {
        m_packet << value;
    }

    float Packet::readFloat() {
        float value = 0.0f;
        m_packet >> value;
        return value;
    }

    void Packet::writeBool(bool value) {
        m_packet << value;
    }

    bool Packet::readBool() {
        bool value = false;
        m_packet >> value;
        return value;
    }

    size_t Packet::getRemainingBytes() const {
        return m_packet.getDataSize();
    }

    void Packet::reserve(size_t size) {
        (void)size;
    }

    void Packet::setSequenceNumber(uint16_t seq) {
        m_sequenceNumber = seq;
    }

    uint16_t Packet::getSequenceNumber() const {
        return m_sequenceNumber;
    }

    void Packet::setChecksum(uint32_t crc) {
        m_checksum = crc;
    }

    uint32_t Packet::getChecksum() const {
        return m_checksum;
    }

    bool Packet::verifyChecksum() const {
        if (m_checksum == 0) return true;
        return calculateCRC32() == m_checksum;
    }

    uint32_t Packet::calculateCRC32() const {
        const uint8_t* data = static_cast<const uint8_t*>(m_packet.getData());
        size_t size = m_packet.getDataSize();
        uint32_t crc = 0xFFFFFFFF;
        static const uint32_t table[256] = {
            0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
            0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
            0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
            0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
            0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
            0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
            0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
            0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
            0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
            0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
            0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
            0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
            0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
            0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
            0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
            0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
            0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
            0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
            0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
            0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
            0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
            0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
            0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
            0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
            0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
            0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
            0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
            0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
            0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
            0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
            0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
            0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
            0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
            0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
            0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
            0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
            0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
            0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
            0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
            0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
            0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
            0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
            0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
            0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
            0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
            0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
            0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
            0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
            0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
            0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
            0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
            0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
            0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
            0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
            0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
            0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
            0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
            0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
            0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
            0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
            0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
            0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
            0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
            0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
        };
        for (size_t i = 0; i < size; ++i) {
            crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
        }
        return ~crc;
    }

    void Packet::compress() {
        const uint8_t* data = static_cast<const uint8_t*>(m_packet.getData());
        size_t size = m_packet.getDataSize();
        if (size == 0) return;

        std::vector<uint8_t> compressed;
        compressed.reserve(size + 12);
        compressed.push_back(0x78);
        compressed.push_back(0x01);

        size_t blockSize = 65535;
        uint32_t adler = 1;
        for (size_t offset = 0; offset < size; offset += blockSize) {
            size_t chunkSize = std::min(blockSize, size - offset);
            bool final = (offset + chunkSize >= size);
            uint8_t bfinal = final ? 1 : 0;
            compressed.push_back(static_cast<uint8_t>((chunkSize & 0xFF)));
            compressed.push_back(static_cast<uint8_t>((chunkSize >> 8) & 0xFF));
            compressed.push_back(static_cast<uint8_t>((~chunkSize) & 0xFF));
            compressed.push_back(static_cast<uint8_t>((~chunkSize >> 8) & 0xFF));
            compressed.insert(compressed.end(), data + offset, data + offset + chunkSize);

            for (size_t i = offset; i < offset + chunkSize; ++i) {
                adler = (adler + data[i]) % 65521;
                adler = (adler + (adler << 16)) % 65521;
            }
        }

        compressed.push_back(static_cast<uint8_t>((adler >> 24) & 0xFF));
        compressed.push_back(static_cast<uint8_t>((adler >> 16) & 0xFF));
        compressed.push_back(static_cast<uint8_t>((adler >> 8) & 0xFF));
        compressed.push_back(static_cast<uint8_t>(adler & 0xFF));

        m_packet.clear();
        for (auto byte : compressed) {
            m_packet << byte;
        }
    }

    void Packet::decompress() {
        const uint8_t* data = static_cast<const uint8_t*>(m_packet.getData());
        size_t size = m_packet.getDataSize();
        if (size < 2 || data[0] != 0x78) return;

        size_t offset = 2;
        std::vector<uint8_t> decompressed;
        while (offset < size) {
            if (offset + 6 > size) break;
            size_t chunkSize = static_cast<size_t>(data[offset]) |
                              (static_cast<size_t>(data[offset + 1]) << 8);
            bool final = (data[offset] & 0x01) != 0;
            offset += 4;
            if (offset + chunkSize > size) break;
            decompressed.insert(decompressed.end(), data + offset, data + offset + chunkSize);
            offset += chunkSize;
            if (final) break;
        }

        m_packet.clear();
        for (auto byte : decompressed) {
            m_packet << byte;
        }
    }

    void Packet::encrypt(const uint8_t* key, size_t keyLength) {
        if (!key || keyLength == 0) return;
        uint8_t* data = static_cast<uint8_t*>(const_cast<void*>(m_packet.getData()));
        size_t size = m_packet.getDataSize();
        for (size_t i = 0; i < size; ++i) {
            data[i] ^= key[i % keyLength];
        }
    }

    void Packet::decrypt(const uint8_t* key, size_t keyLength) {
        encrypt(key, keyLength);
    }

    std::vector<Packet> Packet::fragment(size_t maxFragmentSize) {
        std::vector<Packet> fragments;
        const uint8_t* data = static_cast<const uint8_t*>(m_packet.getData());
        size_t size = m_packet.getDataSize();

        if (size <= maxFragmentSize) {
            fragments.push_back(*this);
            return fragments;
        }

        m_packetId = s_nextPacketId++;
        m_fragmented = true;
        uint16_t totalFragments = static_cast<uint16_t>((size + maxFragmentSize - 1) / maxFragmentSize);

        for (uint16_t i = 0; i < totalFragments; ++i) {
            Packet frag;
            size_t fragSize = std::min(maxFragmentSize, size - i * maxFragmentSize);

            frag.m_packetId = m_packetId;
            frag.m_fragmented = true;

            frag.m_packet.clear();
            frag.m_packet << m_packetId;
            frag.m_packet << i;
            frag.m_packet << totalFragments;
            frag.m_packet << static_cast<uint16_t>(fragSize);
            for (size_t j = 0; j < fragSize; ++j) {
                frag.m_packet << data[i * maxFragmentSize + j];
            }

            fragments.push_back(std::move(frag));
        }

        return fragments;
    }

    bool Packet::reassemble(const std::vector<Packet>& fragments) {
        if (fragments.empty()) return false;
        m_packet.clear();
        for (const auto& frag : fragments) {
            const uint8_t* data = static_cast<const uint8_t*>(frag.m_packet.getData());
            size_t size = frag.m_packet.getDataSize();
            m_packet.onReceive(data, size);
        }
        m_fragmented = false;
        return true;
    }

    bool Packet::addFragment(const PacketFragment& fragment) {
        if (fragment.fragmentIndex >= fragment.totalFragments) return false;
        for (auto& f : m_fragments) {
            if (f.fragmentIndex == fragment.fragmentIndex) return true;
        }
        m_fragments.push_back(fragment);
        return true;
    }

    bool Packet::isReassemblyComplete() const {
        if (m_fragments.empty()) return false;
        uint16_t total = m_fragments[0].totalFragments;
        if (m_fragments.size() >= total) {
            std::vector<bool> present(total, false);
            for (const auto& f : m_fragments) {
                if (f.fragmentIndex < total) present[f.fragmentIndex] = true;
            }
            for (bool p : present) {
                if (!p) return false;
            }
            return true;
        }
        return false;
    }

    PacketPool::PacketPool(size_t initialSize) {
        m_pool.resize(initialSize);
        m_freeList.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

    Packet* PacketPool::acquire() {
        if (m_freeList.empty()) {
            size_t oldSize = m_pool.size();
            size_t newSize = oldSize + (oldSize / 2) + 1;
            m_pool.resize(newSize);
            for (size_t i = oldSize; i < newSize; ++i) {
                m_freeList.push_back(&m_pool[i]);
            }
        }

        Packet* p = m_freeList.back();
        m_freeList.pop_back();
        p->clear();
        ++m_activeCount;
        return p;
    }

    void PacketPool::release(Packet* packet) {
        if (!packet) return;
        packet->clear();
        m_freeList.push_back(packet);
        --m_activeCount;
    }

    void PacketPool::clear() {
        m_freeList.clear();
        m_activeCount = 0;
        for (size_t i = 0; i < m_pool.size(); ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

    PacketFragmenter::PacketFragmenter(size_t maxFragmentSize)
        : m_maxFragmentSize(maxFragmentSize)
    {
    }

    std::vector<PacketFragment> PacketFragmenter::fragment(const Packet& packet) {
        std::vector<PacketFragment> fragments;
        const uint8_t* data = static_cast<const uint8_t*>(packet.getData());
        size_t size = packet.getDataSize();
        if (size == 0) return fragments;

        uint32_t packetId = Packet::s_nextPacketId++;
        uint16_t totalFragments = static_cast<uint16_t>((size + m_maxFragmentSize - 1) / m_maxFragmentSize);

        for (uint16_t i = 0; i < totalFragments; ++i) {
            PacketFragment frag;
            frag.packetId = packetId;
            frag.fragmentIndex = i;
            frag.totalFragments = totalFragments;
            frag.fragmentSize = static_cast<uint16_t>(std::min(m_maxFragmentSize, size - i * m_maxFragmentSize));
            frag.data.assign(data + i * m_maxFragmentSize, data + i * m_maxFragmentSize + frag.fragmentSize);
            fragments.push_back(std::move(frag));
        }

        return fragments;
    }

    bool PacketFragmenter::addFragment(const PacketFragment& fragment) {
        ReassemblyBuffer& buf = m_reassemblyBuffers[fragment.packetId];
        if (buf.fragments.empty()) {
            buf.totalFragments = fragment.totalFragments;
        }
        for (const auto& f : buf.fragments) {
            if (f.fragmentIndex == fragment.fragmentIndex) return true;
        }
        buf.fragments.push_back(fragment);
        buf.lastReceiveTime = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
        return true;
    }

    Packet PacketFragmenter::reassemble(uint32_t packetId) {
        Packet result;
        auto it = m_reassemblyBuffers.find(packetId);
        if (it == m_reassemblyBuffers.end()) return result;

        std::sort(it->second.fragments.begin(), it->second.fragments.end(),
            [](const PacketFragment& a, const PacketFragment& b) {
                return a.fragmentIndex < b.fragmentIndex;
            });

        for (const auto& frag : it->second.fragments) {
            result.m_packet.onReceive(frag.data.data(), frag.data.size());
        }

        m_reassemblyBuffers.erase(it);
        return result;
    }

    bool PacketFragmenter::isComplete(uint32_t packetId) const {
        auto it = m_reassemblyBuffers.find(packetId);
        if (it == m_reassemblyBuffers.end()) return false;
        return it->second.fragments.size() >= it->second.totalFragments;
    }

    void PacketFragmenter::clear(uint32_t packetId) {
        m_reassemblyBuffers.erase(packetId);
    }

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
