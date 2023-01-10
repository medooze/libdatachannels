#ifndef LIBDATACHANNELS_INTERNAL_READER_H_
#define LIBDATACHANNELS_INTERNAL_READER_H_
#include <stdint.h>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <array>
#include <limits>

#include "Buffer.h"

class BufferReader
{
public:
	BufferReader() = default;
	BufferReader(const Buffer& buffer)  :
		data(buffer.GetData()),
		size(buffer.GetSize())
	{
		this->pos = 0;
	}
	
	BufferReader(const uint8_t* data, const size_t size)  :
		data(data),
		size(size)
	{
		this->pos = 0;
	}
	
	template<std::size_t N> std::array<uint8_t,N> Get(size_t i) const 
	{
		std::array<uint8_t,N> array;
		memcpy(array.data(),data+i,N);
		return array;
	} 
	
	inline BufferReader GetReader(size_t i, size_t num) const 
	{ 
		return BufferReader(data+i,num);
	}
	
	inline std::string GetString(size_t i, size_t num) const 
	{ 
		return std::string((char*)data+i,num);
	}
	
	inline Buffer GetBuffer(size_t i, size_t num) const 
	{
		return Buffer(data+i,num);
	}

	inline uint8_t Get1(size_t i) const 
	{
		return data[i];
	}
	
	inline uint16_t Get2(size_t i) const 
	{ 
		return (uint16_t)(data[i+1]) | ((uint16_t)(data[i]))<<8; 
	}
	
	inline uint32_t Get3(size_t i) const 
	{
		return (uint32_t)(data[i+2]) | ((uint32_t)(data[i+1]))<<8 | ((uint32_t)(data[i]))<<16; 
	}
	
	inline uint32_t Get4(size_t i) const 
	{
		return (uint32_t)(data[i+3]) | ((uint32_t)(data[i+2]))<<8 | ((uint32_t)(data[i+1]))<<16 | ((uint32_t)(data[i]))<<24; 
	}
	inline uint32_t Get4Reversed(size_t i) const 
	{
		return (uint32_t)(data[i]) | ((uint32_t)(data[i+1]))<<8 | ((uint32_t)(data[i+2]))<<16 | ((uint32_t)(data[i+3]))<<24; 
	}
	
	inline uint64_t Get8(size_t i) const 
	{
		return ((uint64_t)Get4(i))<<32 | Get4(i+4);
	}

	template<std::size_t N> 
	std::array<uint8_t,N> Get() 			{ pos+=N; return Get<N>(pos-N);				}
	inline BufferReader GetReader(size_t num) 	{ pos+=num; return GetReader(pos-num, num);		}
	inline std::string  GetString(size_t num) 	{ pos+=num; return GetString(pos-num, num);		}
	inline Buffer	    GetBuffer(size_t num)	{ pos+=num; return GetBuffer(pos-num,num);		}
	inline const uint8_t* GetData(size_t num)	{ const uint8_t* val = data+pos; pos+=num; return val;	}
	inline uint8_t  Get1() 				{ auto val = Get1(pos); pos+=1; return val;		}
	inline uint16_t Get2() 				{ auto val = Get2(pos); pos+=2; return val;		}
	inline uint32_t Get3() 				{ auto val = Get3(pos); pos+=3; return val;		}
	inline uint32_t Get4() 				{ auto val = Get4(pos); pos+=4; return val;		}
	inline uint32_t Get4Reversed()			{ auto val = Get4Reversed(pos); pos+=4; return val;	}
	inline uint64_t Get8() 				{ auto val = Get8(pos); pos+=8; return val;		}
	inline uint8_t  Peek1()				{ return Get1(pos); }
	inline uint16_t Peek2()				{ return Get2(pos); }
	inline uint32_t Peek3()				{ return Get3(pos); }
	inline uint32_t Peek4()				{ return Get4(pos); }
	inline uint64_t Peek8()				{ return Get8(pos); }
	inline const uint8_t* PeekData()		{ return data+pos;  }
	size_t   PadTo(size_t num)
	{
		size_t reminder = pos % num;
		size_t padding = reminder ? num - reminder : 0;
		if (!Assert(padding))
			return 0;
		return Skip(padding);
	}
	
	bool   Assert(size_t num) const 	{ return pos+num<=size;	}
	void   GoTo(size_t mark) 		{ pos = mark;		}
	size_t Skip(size_t num) 		{ size_t mark = pos; pos += num; return mark;	}
	int64_t  GetOffset(size_t mark) const 	{ return pos-mark;	}
	size_t Mark() const 			{ return pos;		}
	size_t GetLength() const 		{ return pos;		}
	size_t GetLeft() const 			{ return size-pos;	}
	size_t GetSize() const 			{ return size;		}

	uint64_t DecodeLev128() 
	{
		uint64_t val = 0;
		uint32_t len = 0;
		

		//While we have data
		while (GetLeft())
		{
			//Get curr value
			uint64_t cur = Get1();

			//We only read the 7 least significant bits of each byte
			val |= (cur & 0x7f) << len;
			len += 7;

			// Most significant bit is 0, we're done
			if ((cur & 0x80) == 0)
				return val;
		}

		// If we got here, we read all bytes, but no one with 0 as MSB
		return std::numeric_limits<uint64_t>::max();
	}

private:
	const uint8_t* data = nullptr;
	size_t size = 0;
	size_t pos = 0;
};

#endif
