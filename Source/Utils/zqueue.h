#pragma once
#include "alloc_config.h"
#include "zdeque.h"
#include <queue>


namespace zenith
{
	template<class T, class Cont = zdeque<T>>
	using zqueue = std::queue<T, Cont>;

	template<class T, class Cont = zdeque_loc<T>>
	using zqueue_loc = std::queue<T, Cont>;
}