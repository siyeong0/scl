#pragma once
#include <vector>
#include "Common.h"
#include "Memory.h"

namespace srl
{
	static size_t ccc = 0;
	template<size_t typeSize, size_t bucketSize = 1024>
	class MemPool
	{
	private:
		struct Bucket
		{
			void* Ptr;
			uint16_t Idx;
			uint16_t* IdxTable;
		};
	public:
		MemPool()
		{
			static_assert(bucketSize < 65536);
			static_assert(typeSize * bucketSize < 1e+9);
			allocNewBucket();
		}

		~MemPool()
		{
			std::cout << "Bucket<" << typeSize << ", " << bucketSize << "> size : " << mBuckets.size() << std::endl;
			for (auto& bucket : mBuckets)
			{
				srl::Free(bucket.Ptr);
				srl::Free(bucket.IdxTable);
			}
			std::cout << ccc << std::endl;
		}

		void* Alloc()
		{
			ccc++;
			for (int i = (int)mBuckets.size() - 1; i >= 0; i--)
			{
				Bucket& currBucket = mBuckets[i];
				if (currBucket.Idx == bucketSize)
				{
					continue;
				}

				char* bytePtr = reinterpret_cast<char*>(currBucket.Ptr);
				void* retPtr = reinterpret_cast<void*>(bytePtr + currBucket.IdxTable[currBucket.Idx] * typeSize);
				currBucket.Idx++;
				return retPtr;
			}
			// All buckets are full, allocate new bucket
			Bucket& bucket = allocNewBucket();
			void* retPtr = bucket.Ptr;
			bucket.Idx++;
			return retPtr;
		}

		void Free(void* ptr)
		{
			ccc--;
			const size_t STRIDE = typeSize * bucketSize;
			const size_t ptrVal = reinterpret_cast<size_t>(ptr);
			for (int i = (int)mBuckets.size() - 1; i >= 0; i--)
			{
				Bucket& currBucket = mBuckets[i];
				const size_t currPtrVal = reinterpret_cast<size_t>(currBucket.Ptr);
				if (ptrVal >= currPtrVal && ptrVal < currPtrVal + STRIDE)
				{
					uint16_t targetIdx = uint16_t((ptrVal - currPtrVal) / typeSize);
					currBucket.Idx--;
					currBucket.IdxTable[currBucket.Idx] = targetIdx;
					return;
				}
			}
			Assert(false);
		}

	private:
		Bucket& allocNewBucket()
		{
			Bucket bucket;
			bucket.Ptr = srl::Alloc(typeSize * bucketSize);
			bucket.Idx = 0;
			bucket.IdxTable = (uint16_t*)srl::Alloc(sizeof(uint16_t) * bucketSize);
			for (uint16_t i = 0; i < bucketSize; i++)
			{
				bucket.IdxTable[i] = i;
			}
			mBuckets.push_back(bucket);

			return mBuckets[mBuckets.size() - 1];
		}
	public:
		std::vector<Bucket> mBuckets;
	};
}