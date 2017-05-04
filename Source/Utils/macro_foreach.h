#pragma once

#define ZENITH_EXPAND(x) x
#define ZENITH_AUGMENTER(...) __VA_ARGS__


#define ZENITH_NARGS_1(...) ZENITH_EXPAND(ZENITH_ARG_N(__VA_ARGS__,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))

#define ZENITH_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24, _25, _26, _27, _28, _29, N,...) N

#define ZENITH_NARGS(...) ZENITH_NARGS_1(ZENITH_AUGMENTER(__VA_ARGS__))


#define ZENITH_fe_0(_call, ...)
#define ZENITH_fe_1(_call, x) _call(x)
#define ZENITH_fe_2(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_1(_call, __VA_ARGS__))
#define ZENITH_fe_3(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_2(_call, __VA_ARGS__))
#define ZENITH_fe_4(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_3(_call, __VA_ARGS__))
#define ZENITH_fe_5(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_4(_call, __VA_ARGS__))
#define ZENITH_fe_6(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_5(_call, __VA_ARGS__))
#define ZENITH_fe_7(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_6(_call, __VA_ARGS__))
#define ZENITH_fe_8(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_7(_call, __VA_ARGS__))
#define ZENITH_fe_9(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_8(_call, __VA_ARGS__))
#define ZENITH_fe_10(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_9(_call, __VA_ARGS__))
#define ZENITH_fe_11(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_10(_call, __VA_ARGS__))
#define ZENITH_fe_12(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_11(_call, __VA_ARGS__))
#define ZENITH_fe_13(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_12(_call, __VA_ARGS__))
#define ZENITH_fe_14(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_13(_call, __VA_ARGS__))
#define ZENITH_fe_15(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_14(_call, __VA_ARGS__))
#define ZENITH_fe_16(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_15(_call, __VA_ARGS__))
#define ZENITH_fe_17(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_16(_call, __VA_ARGS__))
#define ZENITH_fe_18(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_17(_call, __VA_ARGS__))
#define ZENITH_fe_19(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_18(_call, __VA_ARGS__))
#define ZENITH_fe_20(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_19(_call, __VA_ARGS__))
#define ZENITH_fe_21(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_20(_call, __VA_ARGS__))
#define ZENITH_fe_22(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_21(_call, __VA_ARGS__))
#define ZENITH_fe_23(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_22(_call, __VA_ARGS__))
#define ZENITH_fe_24(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_23(_call, __VA_ARGS__))
#define ZENITH_fe_25(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_24(_call, __VA_ARGS__))
#define ZENITH_fe_26(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_25(_call, __VA_ARGS__))
#define ZENITH_fe_27(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_26(_call, __VA_ARGS__))
#define ZENITH_fe_28(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_27(_call, __VA_ARGS__))
#define ZENITH_fe_29(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_28(_call, __VA_ARGS__))
#define ZENITH_fe_30(_call, x, ...) _call(x) ZENITH_EXPAND(ZENITH_fe_29(_call, __VA_ARGS__))


#define ZENITH_FOR_EACH(_call,  ...) ZENITH_EXPAND(ZENITH_ARG_N(__VA_ARGS__,\
	ZENITH_fe_29, ZENITH_fe_28, ZENITH_fe_27, ZENITH_fe_26, ZENITH_fe_25, ZENITH_fe_24, ZENITH_fe_23, ZENITH_fe_22, ZENITH_fe_21, ZENITH_fe_20,\
	ZENITH_fe_19, ZENITH_fe_18, ZENITH_fe_17, ZENITH_fe_16, ZENITH_fe_15, ZENITH_fe_14, ZENITH_fe_13, ZENITH_fe_12, ZENITH_fe_11, ZENITH_fe_10,\
	ZENITH_fe_9, ZENITH_fe_8, ZENITH_fe_7, ZENITH_fe_6, ZENITH_fe_5, ZENITH_fe_4, ZENITH_fe_3, ZENITH_fe_2, ZENITH_fe_1, ZENITH_fe_0) (_call, __VA_ARGS__))