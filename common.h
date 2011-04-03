#ifndef COMMON_H
#define COMMON_H

#define ACCESS(type, n) \
		inline type get##n() const {return n;} \
		inline void  set##n(type i){n = i;}

#endif
