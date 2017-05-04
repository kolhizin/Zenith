#pragma once
#include "alloc_config.h"
#include "zdeque.h"
#include <stack>


namespace zenith
{
	template<class T, class Cont = zdeque<T>>
	using zstack = std::stack<T, Cont>;

	template<class T, class Cont = zdeque_loc<T>>
	using zstack_loc = std::stack<T, Cont>;
}