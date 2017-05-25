#pragma once
#include "Numeric.h"
#include <random>
#include <vector>

namespace zenith
{
	namespace util
	{
		namespace math
		{
			template<class T>
			class categorical_distribution
			{
			public:
				typedef int _Ty;
				typedef categorical_distribution<T> _Myt;
				typedef std::vector<double> _Myvec;
				typedef std::vector<T> _MapCont;
				typedef T result_type;

				struct param_type
				{	// parameter package
					typedef _Myt distribution_type;
					struct _Noinit
					{	// placeholder to suppress initialization
					};

					param_type(_Noinit)
					{	// do-nothing constructor for derived classes
					}

					param_type()
					{	// default constructor
						_Init();
					}

					template<class _InIt, class _InValIt>
					param_type(_InIt _First, _InIt _Last, _InValIt _FirstV, _InValIt _LastV)
						: _Pvec(_First, _Last), _Pmap(_FirstV, _LastV)
					{	// construct from [_First, _Last) and [_FirstV, _LastV)
						_Init();
					}

					template<class _InIt>
					param_type(_InIt _First, _InIt _Last)
					{	// construct from [_First, _Last)
						for (auto It = _First; It != _Last; It++)
						{
							_Pmap.push_back(It->first);
							_Pvec.push_back(It->second);
						}
						_Init();
					}
					template<class _InIt, class U, T U::*v, double U::*p>
					param_type(_InIt _First, _InIt _Last)
					{	// construct from [_First, _Last)
						for (auto It = _First; It != _Last; It++)
						{
							_Pmap.push_back(It->*v);
							_Pvec.push_back(It->*p);
						}
						_Init();
					}

					template<class _InIt, class _F1, class _F2>
					param_type(_InIt _First, _InIt _Last, _F1 &&_f1, _F2 &&_f2)
					{	// construct from [_First, _Last)
						for (auto It = _First; It != _Last; It++)
						{
							_Pmap.push_back(_f1(*It));
							_Pvec.push_back(_f2(*It));
						}
						_Init();
					}
					

					param_type(std::initializer_list<std::pair<T, double>> _Ilist)
					{	// construct from initializer list
						_Pvec.reserve(_Ilist.size());
						_Pmap.reserve(_Ilist.size());
						for (const auto &x : _Ilist)
						{
							_Pmap.emplace_back(std::move(x.first));
							_Pvec.emplace_back(std::move(x.second));
						}
						_Init();
					}
					

					bool operator==(const param_type& _Right) const
					{	// test for equality
						return (_Pvec == _Right._Pvec) && (_Pmap == _Right._Pmap);
					}

					bool operator!=(const param_type& _Right) const
					{	// test for inequality
						return (!(*this == _Right));
					}

					_Myvec probabilities() const
					{	// return probabilities
						return (_Pvec);
					}

					void _Init(bool _Renorm = true)
					{	// initialize
						size_t _Size = _Pvec.size();
						if (_Pmap.size() != _Size)
							throw MathException("categorical_distribution::param_type: prob-size is not equal to map-size!");
						size_t _Idx;

						if (!_Renorm)
							;
						else if (_Pvec.empty())
						{
							_Pvec.push_back(1.0);	// make empty vector degenerate
							_Pmap.push_back(T());
						}
						else
						{	// normalize probabilities
							double _Sum = 0;

							for (_Idx = 0; _Idx < _Size; ++_Idx)
							{	// sum all probabilities
								//_RNG_ASSERT(0.0 <= _Pvec[_Idx],
								//	"invalid probability for discrete_distribution");
								_Sum += _Pvec[_Idx];
							}

							//_RNG_ASSERT(0.0 < _Sum,
							//	"invalid probability vector for discrete_distribution");
							if (_Sum != 1.0)
								for (_Idx = 0; _Idx < _Size; ++_Idx)
									_Pvec[_Idx] /= _Sum;
						}

						_Pcdf.assign(1, _Pvec[0]);
						for (_Idx = 1; _Idx < _Size; ++_Idx)
							_Pcdf.push_back(_Pvec[_Idx] + _Pcdf[_Idx - 1]);
					}

					_MapCont _Pmap;
					_Myvec _Pvec;
					_Myvec _Pcdf;
				};

				categorical_distribution()
				{	// default constructor
				}

				template<class _InIt, class _InValIt>
				categorical_distribution(_InIt _First, _InIt _Last, _InValIt _FirstV, _InValIt _LastV)
					: _Par(_First, _Last, _FirstV, _LastV)
				{	// construct from [_First, _Last)
				}

				template<class _InIt>
				categorical_distribution(_InIt _First, _InIt _Last)
					: _Par(_First, _Last)
				{	// construct from [_First, _Last)
				}

				template<class _InIt, class U, T U::*v, double U::*p>
				categorical_distribution(_InIt _First, _InIt _Last)
					: _Par<_InIt, U, v, p>(_First, _Last)
				{
				}

				template<class _InIt, class _F1, class _F2>
				categorical_distribution(_InIt _First, _InIt _Last, _F1 &&_f1, _F2 &&_f2)
					: _Par(_First, _Last, std::move(_f1), std::move(_f2))
				{	// construct from [_First, _Last)
				}

				
				categorical_distribution(std::initializer_list<std::pair<T, double>> _Ilist)
					: _Par(_Ilist)
				{	// construct from initializer list
				}
				

				explicit categorical_distribution(const param_type& _Par0)
					: _Par(_Par0)
				{	// construct from parameter package
				}

				_Myvec probabilities() const
				{	// return scaled probabilities vector
					return (_Par.probabilities());
				}

				param_type param() const
				{	// return parameter package
					return (_Par);
				}

				void param(const param_type& _Par0)
				{	// set parameter package
					_Par = _Par0;
				}

				void reset()
				{	// clear internal state
				}

				template<class _Engine>
				result_type operator()(_Engine& _Eng) const
				{	// return next value
					return _Par._Pmap.at((_Eval(_Eng, _Par)));
				}

				template<class _Engine>
				result_type operator()(_Engine& _Eng, const param_type& _Par0) const
				{	// return next value, given parameter package
					return _Par0._Pmap.at((_Eval(_Eng, _Par0)));
				}


			private:
				template<class _Engine>
				_Ty _Eval(_Engine& _Eng, const param_type& _Par0) const
				{	// return next value
					double _Px = std::generate_canonical<double, static_cast<size_t>(-1)>(_Eng);
					_Ty _Count = (_Ty)_Par0._Pcdf.size();
					_Ty _First = 0;

					while (0 < _Count)
					{	// divide and conquer, find half that contains answer
						_Ty _Count2 = _Count / 2;
						_Ty _Mid = _First + _Count2;

						if (_Px <= _Par0._Pcdf[_Mid])
							_Count = _Count2;	// answer in first half, stay there
						else
						{	// answer in second half, move up
							_First = ++_Mid;
							_Count -= _Count2 + 1;
						}
					}
					return (_First);
				}

			public:
				param_type _Par;
			};

			template<class _Ty>
			bool operator==(const categorical_distribution<_Ty>& _Left,
				const categorical_distribution<_Ty>& _Right)
			{	// test for equality
				return (_Left.param() == _Right.param());
			}

			template<class _Ty>
			bool operator!=(const categorical_distribution<_Ty>& _Left,
				const categorical_distribution<_Ty>& _Right)
			{	// test for inequality
				return (!(_Left == _Right));
			}
		}
	}
}