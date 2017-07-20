#pragma once

#include "vulkan_config.h"
#include "vUtil_Enums.h"

namespace zenith
{
	namespace vulkan
	{
		class VulkanException : public zenith::util::LoggedException
		{
			VkResult errResult_;
		public:
			VulkanException() : zenith::util::LoggedException() {}
			VulkanException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type), errResult_(VK_SUCCESS)
			{
			}
			VulkanException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type), errResult_(VK_SUCCESS)
			{
			}
			VulkanException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR)
				: zenith::util::LoggedException(std::string(p) + " VkResult(" + zenith::util::str_cast<std::string>(static_cast<int>(res)) + ")", type), errResult_(res)
			{
			}
			VulkanException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR)
				: zenith::util::LoggedException(str + " VkResult(" + zenith::util::str_cast<std::string>(static_cast<int>(res)) + ")", type), errResult_(res)
			{
			}
			VkResult getResult() const
			{
				return errResult_;
			}
			virtual ~VulkanException() {}
		};


		inline VkSamplerAddressMode toVkSamplerAddressMode(zenith::vulkan::vSamplerAddress adr)
		{
			return static_cast<VkSamplerAddressMode>(adr);
		}
		inline VkSamplerMipmapMode toVkSamplerMipmapMode(zenith::vulkan::vSamplerFilter flt)
		{
			return static_cast<VkSamplerMipmapMode>(flt);
		}
		inline VkFilter toVkFilter(zenith::vulkan::vSamplerFilter flt)
		{
			return static_cast<VkFilter>(flt);
		}
		
		class vObjectSharingInfo
		{
			static const size_t NumBits = 32;
			uint32_t bitmask_;
			inline bool getBit_(unsigned int idx) const { return (bitmask_ & (1 << idx)) != 0; }
			inline uint32_t numBits_() const
			{
				uint32_t tmp = bitmask_;
				uint32_t res = 0;
				while (tmp)
				{
					res += tmp & 1;
					tmp = tmp >> 1;
				}
				return res;
			}
		public:
			inline vObjectSharingInfo() : bitmask_(0) {}

			template<class It>
			inline vObjectSharingInfo(It begin, It end) : bitmask_(0)
			{
				for (size_t i = 0; begin != end; begin++, i++)
					if (*begin >= NumBits)
						throw VulkanException("vObjectSharingInfo::vObjectSharingInfo: queue family index is larger than number of bits in bitmask.");
					else
						bitmask_ |= (1 << *begin);
				if (numBits_() == 1)
					throw VulkanException("vObjectSharingInfo::vObjectSharingInfo: there cannot be only one family: either none, or two and more.");
			}
			inline vObjectSharingInfo(std::initializer_list<uint32_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<uint16_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<uint8_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<int32_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<int16_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<int8_t> l) : vObjectSharingInfo(l.begin(), l.end()) {}
			inline vObjectSharingInfo(std::initializer_list<bool> l) : bitmask_(0)
			{
				size_t i = 0;
				for (auto it = l.begin(); it != l.end(); it++, i++)
					if (i >= NumBits)
						throw VulkanException("vObjectSharingInfo::vObjectSharingInfo: queue family index is larger than number of bits in bitmask.");
					else if (*it)
						bitmask_ |= (1 << i);
				if (numBits_() == 1)
					throw VulkanException("vObjectSharingInfo::vObjectSharingInfo: there cannot be only one family: either none, or two and more.");
			}

			inline vObjectSharingInfo(const vObjectSharingInfo &oth) : bitmask_(oth.bitmask_) {}
			inline vObjectSharingInfo &operator =(const vObjectSharingInfo &oth) { bitmask_ = oth.bitmask_; return *this; }


			inline VkSharingMode vkSharingMode() const { return (bitmask_ == 0 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT); }
			inline bool isQueueFamilySupported(unsigned int idx) const { return getBit_(idx); }
			inline bool isExclusive() const { return bitmask_ == 0; }
			inline bool isConcurrent() const { return bitmask_ != 0; }
			inline std::vector<uint32_t> supportedQueueFamilies() const
			{
				std::vector<uint32_t> res(numBits_(), 0);
				uint32_t tmp = bitmask_;
				uint32_t cnt = 0, i = 0;
				while (tmp)
				{
					if (tmp & 1)
						res[cnt++] = i;
					tmp = tmp >> 1;
					i++;
				}
				return res;
			}
		};

		template<class T>
		class vAutoVar
		{
			vAutoVar(const vAutoVar<T> &) = delete;
			vAutoVar<T> &operator =(const vAutoVar<T> &) = delete;
		public:
			typedef  void(__stdcall*PFN_DELETE)(VkDevice dev, T handle, const VkAllocationCallbacks * pA);

			vAutoVar(PFN_DELETE deleter, VkDevice dev, const VkAllocationCallbacks * pA = nullptr) : deleter_(deleter), dev_(dev), pA_(pA), obj_(VK_NULL_HANDLE) {}
			vAutoVar(vAutoVar<T> &&v) : deleter_(v.deleter_), dev_(v.dev_), pA_(v.pA_), obj_(v.obj_)
			{
				v.obj_ = VK_NULL_HANDLE;
			}
			vAutoVar<T> &operator =(vAutoVar<T> &&v)
			{
				free_();
				dev_ = v.dev_;
				pA_ = v.pA_;
				deleter_ = v.deleter_;
				obj_ = v.obj_;
				v.obj_ = VK_NULL_HANDLE;
			}
			~vAutoVar() { free_(); }
			void replaceDeleter(PFN_DELETE deleter) { free_(); deleter_ = deleter; }
			void replaceDevice(VkDevice dev) { free_(); dev_ = dev; }
			void replaceAlloc(const VkAllocationCallbacks * pA) { free_(); pA_ = pA; }
			T &replace()
			{
				free_();
				return obj_;
			}
			const T *operator &() const
			{
				return &obj_;
			}
			operator T() const
			{
				return obj_;
			}
			template<class V>
			bool operator ==(V rhs)
			{
				return obj_ == T(rhs);
			}
			template<class V>
			bool operator !=(V rhs)
			{
				return obj_ != T(rhs);
			}
		private:
			PFN_DELETE deleter_;
			VkDevice dev_;
			const VkAllocationCallbacks * pA_;
			T obj_{ VK_NULL_HANDLE };

			void free_()
			{
				if (obj_ != VK_NULL_HANDLE)
					deleter_(dev_, obj_, pA_);
				obj_ = VK_NULL_HANDLE;
			}
		};

		template<class T>
		class vAutoArray
		{
			vAutoArray(const vAutoArray<T> &) = delete;
			vAutoArray<T> &operator =(const vAutoArray<T> &) = delete;
		public:
			typedef  void(__stdcall*PFN_DELETE)(VkDevice dev, T handle, const VkAllocationCallbacks * pA);

			inline vAutoArray(uint32_t num, PFN_DELETE deleter, VkDevice dev, const VkAllocationCallbacks * pA = nullptr) : deleter_(deleter), dev_(dev), pA_(pA), size_(num)
			{
				obj_ = new T[size_];
				for (uint32_t i = 0; i < size_; i++)
					obj_[i] = VK_NULL_HANDLE;
			}
			inline vAutoArray(vAutoArray<T> &&v) : deleter_(v.deleter_), dev_(v.dev_), pA_(v.pA_), obj_(v.obj_), size_(v.size_)
			{
				v.obj_ = nullptr;
				v.size_ = 0;
			}
			inline vAutoArray<T> &operator =(vAutoArray<T> &&v)
			{
				free_();
				dev_ = v.dev_;
				pA_ = v.pA_;
				deleter_ = v.deleter_;
				obj_ = v.obj_;
				size_ = v.size_;
				v.obj_ = nullptr;
				v.size_ = 0;
			}
			inline ~vAutoArray() { free_(); }
			inline uint32_t size() const { return size_; }
			inline T &operator[](uint32_t i) { return obj_[i]; }
			inline const T &operator[](uint32_t i) const { return obj_[i]; }

			inline T &at(uint32_t i) { return if (i < size_)obj_[i];else throw VulkanException("vAutoArray::at(): out of bounds."); }
			inline const T &at(uint32_t i) const { return if (i < size_)obj_[i];else throw VulkanException("vAutoArray::at(): out of bounds."); }
		private:
			PFN_DELETE deleter_;
			VkDevice dev_;
			const VkAllocationCallbacks * pA_;
			uint32_t size_ = 0;
			T * obj_;

			inline void free_()
			{
				if (obj_ && size_ > 0)
				{
					for(uint32_t i = 0; i < size_; i++)
						if(obj_[i] != VK_NULL_HANDLE)
							deleter_(dev_, obj_[i], pA_);
					delete[] obj_;
				}
				obj_ = nullptr;
				size_ = 0;
			}
		};
	}
}