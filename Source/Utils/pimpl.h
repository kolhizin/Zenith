#pragma once

#include <stdexcept>

namespace zenith
{
	namespace util
	{
		template<class Impl>
		class pimpl
		{
			static const size_t BuffSize = sizeof(Impl);
			uint8_t implPool_[BuffSize];
			Impl * impl_;
			void zero_()
			{
				impl_ = nullptr;
				for (size_t i = 0; i < BuffSize; i++)
					implPool_[i] = 0;				
			}
			void copy_and_zero_(uint8_t * src_)
			{
				for (size_t i = 0; i < BuffSize; i++)
				{
					implPool_[i] = src_[i];
					src_[i] = 0;
				}
			}
		protected:

			inline Impl * raw() noexcept { return impl_; }
			inline const Impl * raw() const noexcept { return impl_; }

		public:
			inline bool check() const noexcept { return impl_ != nullptr; }
			inline bool valid() const noexcept { return impl_ != nullptr; }
			inline Impl * get()
			{
				if (impl_ == nullptr)
					throw std::logic_error("Accessing null-ptr implementation of PImpl idiom!");
				return impl_;
			}
			inline const Impl * get() const
			{
				if (impl_ == nullptr)
					throw std::logic_error("Accessing null-ptr implementation of PImpl idiom!");
				return impl_;
			}
			inline void destroy()
			{
				if (impl_)
					impl_->~Impl();
				zero_();
			}
			template<typename ...Args>
			inline void create(Args&&... args)
			{
				destroy();
				new (implPool_) Impl(std::forward<Args>(args)...);
				impl_ = reinterpret_cast<Impl *>(&implPool_[0]);
			}
			pimpl() : impl_(nullptr)
			{
				zero_();
			}
			template<typename ...Args>
			pimpl(Args&&... args) : impl_(nullptr)
			{
				new (implPool_) Impl(std::forward<Args>(args)...);
				impl_ = reinterpret_cast<Impl *>(&implPool_[0]);
			}
			pimpl(pimpl<Impl> &&p)
			{
				copy_and_zero_(&p.implPool_[0]);
				if (p.impl_)
				{
					impl_ = reinterpret_cast<Impl *>(&implPool_[0]);
					p.impl_ = nullptr;
				}
				else impl_ = nullptr;
			}
			pimpl<Impl> &operator =(pimpl<Impl> &&p)
			{
				destroy();
				copy_and_zero_(&p.implPool_[0]);
				if (p.impl_)
				{
					impl_ = reinterpret_cast<Impl *>(&implPool_[0]);
					p.impl_ = nullptr;
				}
				else impl_ = nullptr;
				return *this;
			}
			~pimpl()
			{
				destroy();
			}
			
		private:
			pimpl(const pimpl &) = delete;			
			pimpl &operator =(const pimpl &) = delete;
		};
	}
}
