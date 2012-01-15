dnl Ripped from http://gcc.gnu.org/onlinedocs/libstdc++/manual/backwards.html

# AC_COMPILE_STDCXX_OX
AC_DEFUN([AC_COMPILE_STDCXX_0X], [
  AC_CACHE_CHECK(if $CXX supports C++0x features without additional flags,
  ac_cv_cxx_compile_cxx0x_native,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    check_type&& cr = c;],,
  ac_cv_cxx_compile_cxx0x_native=yes, ac_cv_cxx_compile_cxx0x_native=no)
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if $CXX supports C++0x features with -std=c++0x,
  ac_cv_cxx_compile_cxx0x_cxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    void test(check_type &&) {}], [test(check_type())],
  ac_cv_cxx_compile_cxx0x_cxx=yes, ac_cv_cxx_compile_cxx0x_cxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if $CXX supports C++0x features with -std=gnu++0x,
  ac_cv_cxx_compile_cxx0x_gxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    void test(check_type&&){}], [test(check_type());],
  ac_cv_cxx_compile_cxx0x_gxx=yes, ac_cv_cxx_compile_cxx0x_gxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])

  if test "$ac_cv_cxx_compile_cxx0x_native" = yes ||
     test "$ac_cv_cxx_compile_cxx0x_cxx" = yes ||
     test "$ac_cv_cxx_compile_cxx0x_gxx" = yes; then
    AC_DEFINE(HAVE_STDCXX_0X,,[Define if $CXX supports C++0x features. ])
  fi
])

# AC_HEADER_STDCXX_0X
AC_DEFUN([AC_HEADER_STDCXX_0X], [
  AC_CACHE_CHECK(for ISO C++ 0x include files,
  ac_cv_cxx_stdcxx_0x,
  [AC_REQUIRE([AC_COMPILE_STDCXX_0X])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"

  AC_TRY_COMPILE([
    #include <cassert>
    #include <ccomplex>
    #include <cctype>
    #include <cerrno>
    #include <cfenv>
    #include <cfloat>
    #include <cinttypes>
    #include <ciso646>
    #include <climits>
    #include <clocale>
    #include <cmath>
    #include <csetjmp>
    #include <csignal>
    #include <cstdarg>
    #include <cstdbool>
    #include <cstddef>
    #include <cstdint>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <ctgmath>
    #include <ctime>
    #include <cwchar>
    #include <cwctype>

    #include <algorithm>
    #include <array>
    #include <bitset>
    #include <complex>
    #include <deque>
    #include <exception>
    #include <fstream>
    #include <functional>
    #include <iomanip>
    #include <ios>
    #include <iosfwd>
    #include <iostream>
    #include <istream>
    #include <iterator>
    #include <limits>
    #include <list>
    #include <locale>
    #include <map>
    #include <memory>
    #include <new>
    #include <numeric>
    #include <ostream>
    #include <queue>
    #include <random>
    #include <regex>
    #include <set>
    #include <sstream>
    #include <stack>
    #include <stdexcept>
    #include <streambuf>
    #include <string>
    #include <tuple>
    #include <typeinfo>
    #include <type_traits>
    #include <unordered_map>
    #include <unordered_set>
    #include <utility>
    #include <valarray>
    #include <vector>
  ],,
  ac_cv_cxx_stdcxx_0x=yes, ac_cv_cxx_stdcxx_0x=no)
  AC_LANG_RESTORE
  CXXFLAGS="$ac_save_CXXFLAGS"
  ])
  if test "$ac_cv_cxx_stdcxx_0x" = yes; then
    AC_DEFINE(STDCXX_0X_HEADERS,,[Define if ISO C++ 0x header files are present. ])
  fi
])

