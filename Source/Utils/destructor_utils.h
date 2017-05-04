#pragma once

namespace zenith
{
	namespace util
	{
		template<class F>
		class OnScopeExit_0
		{
			F f_;
			bool active;
			
			OnScopeExit_0(const OnScopeExit_0 &) = delete;
			OnScopeExit_0 &operator =(OnScopeExit_0 &&) = delete;
			OnScopeExit_0 &operator =(const OnScopeExit_0 &) = delete;
		public:
			OnScopeExit_0(OnScopeExit_0 &&oth) : f_(std::move(oth.f_)), active(oth.active)	{oth.active = false;}
			OnScopeExit_0(F &&f) : f_(std::move(f)), active(true) {}
			inline bool isActive() const { return active; }
			inline void deactivate() { active = false; }
			inline ~OnScopeExit_0()
			{
				if(active)
					f_();
			}
		};
		template<class F, class A1>
		class OnScopeExit_1
		{
			F f_;
			A1 a1_;
			bool active;
			OnScopeExit_1(const OnScopeExit_1 &) = delete;
			OnScopeExit_1 &operator =(OnScopeExit_1 &&) = delete;
			OnScopeExit_1 &operator =(const OnScopeExit_1 &) = delete;
		public:
			OnScopeExit_1(OnScopeExit_1 &&oth) : f_(std::move(oth.f_)), a1_(std::move(oth.a1_)), active(oth.active) { oth.active = false; }
			OnScopeExit_1(F &&f, A1 &&a1) : f_(std::move(f)), a_(std::move(a1)), active(true) {}
			inline bool isActive() const { return active; }
			inline void deactivate() { active = false; }
			inline ~OnScopeExit_1()
			{
				if (active)
					f_();
			}
		};
		template<class F>
		inline OnScopeExit_0<F> doOnScopeExit(F && f)
		{
			return OnScopeExit_0<F>(std::move(f));
		}
		template<class F, class A1>
		inline OnScopeExit_1<F, A1> doOnScopeExit(F && f, A1 && a1)
		{
			return OnScopeExit_1<F, A1>(std::move(f), std::move(a1));
		}
	}
}