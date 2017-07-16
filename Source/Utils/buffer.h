#pragma once
#include <exception>
#include <cstdint>

namespace zenith
{
	namespace util
	{
		class BufferException : public std::exception
		{
		public:
			BufferException() : std::exception("BufferException: unknown cause") {}
			BufferException(const char * p) : std::exception(p) {}
			virtual ~BufferException() {}
		};

		class BufferInvalidException : public BufferException
		{
		public:
			BufferInvalidException() : BufferException("BufferInvalidException: unknown cause") {}
			BufferInvalidException(const char * p) : BufferException(p) {}
			virtual ~BufferInvalidException() {}
		};

		class BufferOutOfMemException : public BufferException
		{
		public:
			BufferOutOfMemException() : BufferException("BufferOutOfMemException: unknown cause") {}
			BufferOutOfMemException(const char * p) : BufferException(p) {}
			virtual ~BufferOutOfMemException() {}
		};

		class dynamic_buffer_holder
		{
			uint64_t bufferSize_ = 0;
			uint8_t * bufferPtr_ = nullptr;
		public:
			inline bool valid() const { return bufferPtr_ && (bufferSize_ > 0); }
			inline void assert_() const { if (!valid())throw BufferInvalidException(); }
			inline void clear()
			{
				if (valid())
					delete bufferPtr_;
				bufferPtr_ = nullptr;
				bufferSize_ = 0;
			}
			~dynamic_buffer_holder() { clear(); }
			dynamic_buffer_holder() : bufferSize_(0), bufferPtr_(nullptr) {}
			dynamic_buffer_holder(uint64_t sz) : bufferSize_(sz), bufferPtr_(new uint8_t[sz]) {}
			dynamic_buffer_holder(const dynamic_buffer_holder &t)
				: bufferSize_(t.bufferSize_), bufferPtr_(new uint8_t[t.bufferSize_]) {}
			dynamic_buffer_holder(dynamic_buffer_holder &&t)
				: bufferSize_(t.bufferSize_), bufferPtr_(t.bufferPtr_)
			{
				t.bufferPtr_ = nullptr;
				t.bufferSize_ = 0;
			}

			dynamic_buffer_holder &operator =(const dynamic_buffer_holder &t)
			{
				clear();
				bufferSize_ = t.bufferSize_;
				bufferPtr_ = new uint8_t[bufferSize_];
				return *this;
			}
			dynamic_buffer_holder &operator =(dynamic_buffer_holder &&t)
			{
				clear();
				bufferPtr_ = t.bufferPtr_;
				bufferSize_ = t.bufferSize_;
				t.bufferPtr_ = nullptr;
				t.bufferSize_ = 0;
				return *this;
			}


			inline const uint8_t * begin() const { return bufferPtr_; }
			inline const uint8_t * end() const { return bufferPtr_ + bufferSize_; }
			inline uint8_t * begin() { return bufferPtr_; }
			inline uint8_t * end() { return bufferPtr_ + bufferSize_; }
			inline void * vbegin() const { return bufferPtr_; }
			inline void * vend() const { return bufferPtr_ + bufferSize_; }
			inline uint64_t size() const { return bufferSize_; }

		};

		class dynamic_dual_buffer
		{
			dynamic_buffer_holder buff_;
			uint8_t * stackLeft_ = nullptr, *stackRight_ = nullptr;
		public:
			inline bool valid() const {
				return buff_.valid()
					&& (stackLeft_ >= buff_.begin()) && (stackRight_ <= buff_.end()) && (stackLeft_ <= stackRight_);
			}
			inline void assert_() const { if (!valid())throw BufferInvalidException(); }
			inline uint64_t left_size() const { return stackLeft_ - buff_.begin(); }
			inline uint64_t right_size() const { return buff_.end() - stackRight_; }
			inline uint64_t mid_size() const { return stackRight_ - stackLeft_; }
			inline void clear()
			{
				buff_.clear();
				stackLeft_ = stackRight_ = nullptr;
			}
			~dynamic_dual_buffer() { clear(); }
			dynamic_dual_buffer() : stackLeft_(nullptr), stackRight_(nullptr) {}
			dynamic_dual_buffer(uint64_t sz) : buff_(sz)
			{
				stackLeft_ = buff_.begin();
				stackRight_ = buff_.end();
			}
			dynamic_dual_buffer(const dynamic_dual_buffer &t) : buff_(t.buff_)
			{
				stackLeft_ = buff_.begin() + t.left_size();
				stackRight_ = buff_.end() - t.right_size();
			}
			dynamic_dual_buffer(dynamic_dual_buffer &&t)
			{
				auto szLeft = t.left_size();
				auto szRight = t.right_size();
				buff_ = std::move(t.buff_);
				stackLeft_ = buff_.begin() + szLeft;
				stackRight_ = buff_.end() - szRight;
				t.stackRight_ = t.stackLeft_ = nullptr;
			}

			dynamic_dual_buffer &operator =(const dynamic_dual_buffer &t)
			{
				clear();
				buff_ = t.buff_;
				stackLeft_ = buff_.begin() + t.left_size();
				stackRight_ = buff_.end() - t.right_size();
				return *this;
			}
			dynamic_dual_buffer &operator =(dynamic_dual_buffer &&t)
			{
				clear();
				auto szLeft = t.left_size();
				auto szRight = t.right_size();
				buff_ = std::move(t.buff_);
				stackLeft_ = buff_.begin() + szLeft;
				stackRight_ = buff_.end() - szRight;
				t.stackRight_ = t.stackLeft_ = nullptr;
				return *this;
			}


			inline const uint8_t * begin() const { return buff_.begin(); }
			inline const uint8_t * end() const { return buff_.end(); }
			inline uint8_t * begin() { return buff_.begin(); }
			inline uint8_t * end() { return buff_.end(); }
			inline void * vbegin() const { return buff_.vbegin(); }
			inline void * vend() const { return buff_.vend(); }

			inline const uint8_t * left() const { return stackLeft_; }
			inline const uint8_t * right() const { return stackRight_; }
			inline uint8_t * left() { return stackLeft_; }
			inline uint8_t * right() { return stackRight_; }
			inline void * vleft() const { return stackLeft_; }
			inline void * vright() const { return stackRight_; }

			inline const uint8_t * left_begin() const { return buff_.begin(); }
			inline const uint8_t * left_end() const { return stackLeft_; }
			inline uint8_t * left_begin() { return buff_.begin(); }
			inline uint8_t * left_end() { return stackLeft_; }
			inline void * left_vbegin() const { return buff_.vbegin(); }
			inline void * left_vend() const { return stackLeft_; }

			inline const uint8_t * right_begin() const { return stackRight_; }
			inline const uint8_t * right_end() const { return buff_.end(); }
			inline uint8_t * right_begin() { return stackRight_; }
			inline uint8_t * right_end() { return buff_.end(); }
			inline void * right_vbegin() const { return stackRight_; }
			inline void * right_vend() const { return buff_.vend(); }

			inline const uint8_t * mid_begin() const { return stackLeft_; }
			inline const uint8_t * mid_end() const { return stackRight_; }
			inline uint8_t * mid_begin() { return stackLeft_; }
			inline uint8_t * mid_end() { return stackRight_; }
			inline void * mid_vbegin() const { return stackLeft_; }
			inline void * mid_vend() const { return stackRight_; }

			inline uint64_t size() const { return buff_.size(); }

			inline bool left_try_grow(uint64_t sz)
			{
				if (sz > mid_size())
					return false;
				stackLeft_ += sz;
				return true;
			}
			inline bool right_try_grow(uint64_t sz)
			{
				if (sz > mid_size())
					return false;
				stackRight_ -= sz;
				return true;
			}
			inline void left_grow(uint64_t sz)
			{
				bool res = left_try_grow(sz);
				if (!res)
					throw BufferOutOfMemException("dynamic_dual_buffer: out of memory while trying to grow-left");
			}
			inline void right_grow(uint64_t sz)
			{
				bool res = right_try_grow(sz);
				if (!res)
					throw BufferOutOfMemException("dynamic_dual_buffer: out of memory while trying to grow-right");
			}

			inline void left_shrink(uint64_t sz)
			{
				if (sz > left_size())
					throw BufferInvalidException("dynamic_dual_buffer: tried to shrink left beyond size");
				stackLeft_ -= sz;
			}
			inline void right_shrink(uint64_t sz)
			{
				if (sz > right_size())
					throw BufferInvalidException("dynamic_dual_buffer: tried to shrink right beyond size");
				stackRight_ += sz;
			}
		};

	}
}