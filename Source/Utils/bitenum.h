#pragma once
#include "enum2str.h"
#include "ioconv\io_handlers.h"

namespace zenith
{
	namespace util
	{
		template<class E, class Base = uint32_t>
		class bitenum
		{
			union {
				E enum_;
				Base bitmask_;
			};
		public:
			inline bitenum() : bitmask_(0) {}
			explicit inline bitenum(E v) : bitmask_(v) {}
			explicit inline bitenum(Base v) : bitmask_(v) {}
			template<class B2>
			inline bitenum(const bitenum<E, B2> &b) : bitmask_(b.bitmask_) {}
			template<class B2>
			inline bitenum(bitenum<E, B2> &&b) : bitmask_(b.bitmask_) {}


			template<class B2>
			inline bitenum<E, Base> &operator =(const bitenum<E, B2> &b) { bitmask_ = b.bitmask_; return *this; }
			template<class B2>
			inline bitenum<E, Base> &operator =(bitenum<E, B2> &&b) { bitmask_ = b.bitmask_; return *this; }
			inline bitenum<E, Base> &operator =(E v) { bitmask_ = static_cast<Base>(v); return *this; }
			inline bitenum<E, Base> &operator =(Base v) { bitmask_ = v; return *this; }

			explicit inline operator E() const { return bitmask_; }
			explicit inline operator Base() const { return bitmask_; }

			inline E enum_value() const { return bitmask_; }
			inline Base bit_value() const { return bitmask_; }

			inline bool operator ==(const bitenum<E, Base> &b) const{return bitmask_ == b.bitmask_;}
			inline bool operator !=(const bitenum<E, Base> &b) const { return bitmask_ != b.bitmask_; }

			inline bitenum<E, Base> operator |(const bitenum<E, Base> &b) const { return bitenum<E, Base>(bitmask_ | b.bitmask_); }
			inline bitenum<E, Base> operator &(const bitenum<E, Base> &b) const { return bitenum<E, Base>(bitmask_ & b.bitmask_); }
			inline bitenum<E, Base> operator ^(const bitenum<E, Base> &b) const { return bitenum<E, Base>(bitmask_ ^ b.bitmask_); }
			inline bitenum<E, Base> operator |(E b) const { return bitenum<E, Base>(bitmask_ | Base(b)); }
			inline bitenum<E, Base> operator &(E b) const { return bitenum<E, Base>(bitmask_ & Base(b)); }
			inline bitenum<E, Base> operator ^(E b) const { return bitenum<E, Base>(bitmask_ ^ Base(b)); }

			inline bitenum<E, Base> &operator |=(const bitenum<E, Base> &b) { bitmask_ |= b.bitmask_; return *this; }
			inline bitenum<E, Base> &operator &=(const bitenum<E, Base> &b) { bitmask_ &= b.bitmask_; return *this; }
			inline bitenum<E, Base> &operator ^=(const bitenum<E, Base> &b) { bitmask_ ^= b.bitmask_; return *this; }
			inline bitenum<E, Base> &operator |=(E b) { bitmask_ |= Base(b); return *this; }
			inline bitenum<E, Base> &operator &=(E b) { bitmask_ &= Base(b); return *this; }
			inline bitenum<E, Base> &operator ^=(E b) { bitmask_ ^= Base(b); return *this; }

			inline bool includes(const bitenum<E, Base> &b) const { return ((b.bitmask_ & bitmask_) == b.bitmask_); }
			inline bool excludes(const bitenum<E, Base> &b) const { return ((b.bitmask_ & bitmask_) == 0); }

			inline bool includes(E b) const { return ((Base(b) & bitmask_) == Base(b)); }
			inline bool excludes(E b) const { return ((Base(b) & bitmask_) == 0); }
			
			inline bitenum<E, Base> &add(E b) { bitmask_ |= Base(b); return *this; }
			inline bitenum<E, Base> &remove(E b) { bitmask_ ^= (bitmask_ & Base(b)); return *this; }
		};

		template<class E, class Base, class It>
		class zenith::util::ioconv::io_handler_impl<bitenum<E, Base>, It>
		{
		public:
			typedef bitenum<E, Base> value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::VALUE;
			inline static void input(bitenum<E, Base> &val, const It &it)
			{
				E eval;
				str2bitenum(std::string(ensure_type(it, NodeType::VALUE).value()), eval);
				val = eval;
			}
			inline static void output(const bitenum<E, Base> &val, It &it)
			{
				std::string tmp;
				bitenum2str(val.enum_value(), tmp);
				it.set_value(tmp.c_str());
			}
		};
	}
}