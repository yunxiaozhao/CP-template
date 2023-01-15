#ifndef __OY_BIGINT__
#define __OY_BIGINT__

#include <algorithm>
#include <cstdint>
#include "Montgomery.h"

namespace OY {
#pragma pack(4)
    template <uint32_t _B = 10, uint32_t _W = 6, uint32_t _MAXN = 1 << 20, uint64_t _P = 9223372036737335297, uint64_t _R = 3>
    struct BigInt {
        using bint = BigInt<_B, _W, _MAXN, _P, _R>;
        static constexpr struct _Bases {
            uint64_t val[_W * 2 + 1];
            constexpr _Bases() : val{} {
                for (uint32_t i = 0; i <= _W * 2; i++) val[i] = i ? val[i - 1] * _B : 1;
            }
        } bases{};
        static constexpr uint32_t _N = bases.val[_W];
        static inline bint s_divThresh = bint(__int128_t(LLONG_MAX) / _N - 1);
        static inline Montgomery64 s_mg = Montgomery64(_P);
        static inline uint64_t s_roots[std::__bit_ceil(_MAXN / _W) << 1], s_dftBuffer[std::__bit_ceil(_MAXN / _W) << 2], s_rootSize = 1;
        static inline std::mt19937 s_rander;
        int *m_data;
        uint32_t m_length;
        bool m_negative;
        BigInt() : m_length(0), m_negative(false) {}
        template <typename _Tp, std::enable_if_t<std::is_integral_v<_Tp> | std::is_same_v<_Tp, __int128_t>, bool> = true>
        explicit BigInt(_Tp __number) : m_length(0), m_negative(false) {
            if (!__number) return;
            bool neg = false;
            if (__number < _Tp(0)) {
                neg = true;
                __number = -__number;
            }
            static int s_buffer[256], *s_cursor;
            for (s_cursor = s_buffer; __number; __number /= _N) *s_cursor++ = __number % _N;
            std::copy(s_buffer, s_cursor, m_data = malloc(m_length = s_cursor - s_buffer));
            if (neg) setSign(true);
        }
        explicit BigInt(const char *__number) : BigInt(fromString(__number)) {}
        explicit BigInt(const std::string &__number) : BigInt(fromString(__number.data(), __number.size())) {}
        BigInt(bint &&__other) : m_data(__other.m_data), m_length(__other.m_length), m_negative(__other.m_negative) { __other.m_length = 0; }
        BigInt(const bint &__other) : m_length(__other.m_length), m_negative(__other.m_negative) {
            if (m_length) std::copy_n(__other.m_data, m_length, m_data = malloc(m_length));
        }
        ~BigInt() {
            if (m_length) free(m_data);
        }
        static char fromInt(uint32_t __c) {
            if (__c < 10)
                return '0' + __c;
            else if (__c < 36)
                return 'a' + (__c - 10);
            else
                return 'A' + (__c - 36);
        }
        static uint32_t fromChar(char __c) {
            if (__c <= '9')
                return __c - '0';
            else if (__c <= 'Z')
                return __c - 'A' + 36;
            else
                return __c - 'a' + 10;
        }
        static bint fromString(const char *__number) { return fromString(__number, std::strlen(__number)); }
        static bint fromString(const char *__number, uint32_t __length) {
            uint32_t cursor = std::find_if((__number[0] == '+' || __number[0] == '-') ? __number + 1 : __number, __number + __length, [](char c) { return c != '0'; }) - __number;
            if (cursor == __length) return bint();
            auto [quot, rem] = std::div(int(__length - cursor), int(_W));
            bint res(empty(quot + (rem > 0), __number[0] == '-'));
            uint32_t i = res.m_length - 1;
            if (rem) {
                uint32_t digit = 0;
                for (uint32_t j = rem; j--;) digit = digit * _B + fromChar(__number[cursor++]);
                res.m_data[i--] = digit;
            }
            while (cursor < __length) {
                uint32_t digit = 0;
                for (uint32_t j = _W; j--;) digit = digit * _B + fromChar(__number[cursor++]);
                res.m_data[i--] = digit;
            }
            return res;
        }
        static bint small(int __singleDigit) {
            bint res = empty(1, __singleDigit < 0);
            res.m_data[0] = std::abs(__singleDigit);
            return res;
        }
        static bint rand(uint32_t __length) {
            if (!__length) return bint();
            auto [quot, rem] = std::div(int(__length), int(_W));
            if (!rem) quot--, rem += _W;
            bint res(empty(quot + 1, false));
            for (uint32_t i = 0; i + 1 < res.m_length; i++) res.m_data[i] = s_rander() % _N;
            res.m_data[res.m_length - 1] = s_rander() % (bases.val[rem] - bases.val[rem - 1]) + bases.val[rem - 1];
            return res;
        }
        static bint empty(uint32_t __length, bool __negative) {
            bint res;
            if (__length) res.m_data = malloc(__length);
            res.m_length = __length;
            res.setSign(__negative);
            return res;
        }
        static int absCompare(const bint &__a, const bint &__b) {
            if (__a.m_length != __b.m_length) return __a.m_length > __b.m_length ? 1 : -1;
            for (uint32_t i = __a.m_length - 1; ~i; i--)
                if (__a.m_data[i] != __b.m_data[i]) return __a.m_data[i] > __b.m_data[i] ? 1 : -1;
            return 0;
        }
        static bool absClose(const bint &__a, const bint &__b) {
            if (__a.m_length == __b.m_length) {
                uint32_t i = __a.m_length - 1;
                while (~i && __a.m_data[i] == __b.m_data[i]) i--;
                if (!~i) return true;
                if (__a.m_data[i] == __b.m_data[i] + 1)
                    while (~--i && !__a.m_data[i] && __b.m_data[i] == _N - 1) {}
                else if (__b.m_data[i] == __a.m_data[i] + 1)
                    while (~--i && __a.m_data[i] == _N - 1 && !__b.m_data[i]) {}
                else
                    return false;
                return !~i;
            } else if (__a.m_length == __b.m_length + 1) {
                if (__a.m_data[__a.m_length - 1] != 1) return false;
                for (uint32_t i = 0; i < __b.m_length; i++)
                    if (__a.m_data[i] || __b.m_data[i] != _N - 1) return false;
            } else if (__b.m_length == __a.m_length + 1) {
                if (__b.m_data[__b.m_length - 1] != 1) return false;
                for (uint32_t i = 0; i < __a.m_length; i++)
                    if (__a.m_data[i] != _N - 1 || !__a.m_data[i]) return false;
            } else
                return false;
            return true;
        }
        static int *malloc(uint32_t __length) { return new int[__length]; }
        static void free(int *__data) { delete[] __data; }
        static void prepareRoots(uint32_t __length) {
            if (s_rootSize == 1) s_roots[s_rootSize++] = s_mg.raw_init(1);
            while (s_rootSize < __length) {
                const uint64_t wn = s_mg.pow(s_mg.raw_init(_R), (_P - 1) / (s_rootSize * 2));
                for (uint32_t i = s_rootSize; i < s_rootSize * 2; i += 2) {
                    s_roots[i] = s_roots[i / 2];
                    s_roots[i + 1] = s_mg.multiply(s_roots[i / 2], wn);
                }
                s_rootSize *= 2;
            }
        }
        static void dft(uint64_t *__buffer, uint32_t __length, const bint &__a) {
            prepareRoots(__length);
            for (uint32_t i = 0; i < __a.m_length; i++) __buffer[i] = s_mg.raw_init(__a.m_data[i]);
            for (uint32_t i = __a.m_length; i < __length; i++) __buffer[i] = 0;
            for (uint32_t l = __length / 2; l; l /= 2)
                for (uint32_t i = 0; i < __length; i += l * 2)
                    for (uint32_t j = 0; j < l; j++) {
                        auto x = __buffer[i + j], y = __buffer[i + j + l];
                        __buffer[i + j] = s_mg.plus(x, y);
                        __buffer[i + j + l] = s_mg.multiply(s_roots[j + l], s_mg.minus(x, y));
                    }
        }
        static void idft(uint64_t *__buffer, uint32_t __length) {
            for (uint32_t l = 1; l < __length; l *= 2)
                for (uint32_t i = 0; i < __length; i += l * 2)
                    for (uint32_t j = 0; j < l; j++) {
                        auto x = __buffer[i + j], y = s_mg.multiply(s_roots[j + l], __buffer[i + j + l]);
                        __buffer[i + j] = s_mg.plus(x, y);
                        __buffer[i + j + l] = s_mg.minus(x, y);
                    }
            const uint64_t inv = s_mg.pow(s_mg.raw_init(__length), _P - 2);
            for (uint32_t i = 0; i < __length; i++) __buffer[i] = s_mg.multiply(__buffer[i], inv);
            std::reverse(__buffer + 1, __buffer + __length);
        }
        template <int _Val, bool _Sign>
        static bint &inc_dec(bint &__a) {
            if (!__a.m_length) return __a = small(_Val);
            if (__a.m_negative == _Sign) {
                uint32_t i = 0;
                while (i < __a.m_length && ++(__a.m_data[i]) == _N) __a.m_data[i++] = 0;
                if (i < __a.m_length) return __a;
                __a = empty(__a.m_length + 1, _Sign);
                std::fill_n(__a.m_data, __a.m_length - 1, 0);
                __a.m_data[__a.m_length - 1] = 1;
                return __a;
            } else {
                uint32_t i = 0;
                while (i < __a.m_length && !__a.m_data[i]--) __a.m_data[i++] = _N - 1;
                return __a.shrink();
            }
        }
        template <typename _Compare>
        static bint &plus_minus_by(bint &__a, const bint &__b) {
            if (!__b.m_length) return __a;
            if (_Compare()(__a.m_negative, __b.m_negative)) {
                if (__b.m_length <= __a.m_length && __a.m_data[__b.m_length - 1] + __b.m_data[__b.m_length - 1] < _N) {
                    for (uint32_t i = 0, carry = 0; i < __b.m_length; i++)
                        if (__a.m_data[i] += __b.m_data[i] + carry; (carry = __a.m_data[i] >= _N)) __a.m_data[i] -= _N;
                    return __a;
                }
                bint res(empty(std::max(__a.m_length, __b.m_length) + 1, __a.m_negative));
                for (uint32_t i = 0, carry = 0; i < res.m_length; i++)
                    if (res.m_data[i] = (i < __a.m_length ? __a.m_data[i] : 0) + (i < __b.m_length ? __b.m_data[i] : 0) + carry; (carry = res.m_data[i] >= _N)) res.m_data[i] -= _N;
                return (__a = res).shrink();
            } else {
                if (int comp = absCompare(__a, __b); comp > 0) {
                    for (uint32_t i = 0, borrow = 0; i < __a.m_length; i++)
                        if (__a.m_data[i] -= (i < __b.m_length ? __b.m_data[i] : 0) + borrow; (borrow = __a.m_data[i] < 0)) __a.m_data[i] += _N;
                    return __a.shrink();
                } else if (comp < 0) {
                    bint res(empty(__b.m_length, std::is_same_v<_Compare, std::not_equal_to<bool>> ? !__b.m_negative : __b.m_negative));
                    for (uint32_t i = 0, borrow = 0; i < res.m_length; i++)
                        if (res.m_data[i] = __b.m_data[i] - (i < __a.m_length ? __a.m_data[i] : 0) - borrow; (borrow = res.m_data[i] < 0)) res.m_data[i] += _N;
                    return (__a = res).shrink();
                } else
                    return __a = bint();
            }
        }
        template <typename _Compare, typename _Sign>
        static bint plus_minus(const bint &__a, const bint &__b, _Sign __sign) {
            if (!__a.m_length) return __sign(__b);
            if (!__b.m_length) return __a;
            if (_Compare()(__a.m_negative, __b.m_negative)) {
                bint res(empty(std::max(__a.m_length, __b.m_length) + 1, __a.m_negative));
                for (uint32_t i = 0, carry = 0; i < res.m_length; i++)
                    if (res.m_data[i] = (i < __a.m_length ? __a.m_data[i] : 0) + (i < __b.m_length ? __b.m_data[i] : 0) + carry; (carry = res.m_data[i] >= _N)) res.m_data[i] -= _N;
                res.shrink();
                return res;
            } else if (int comp = absCompare(__a, __b); comp > 0) {
                bint res(empty(__a.m_length, __a.m_negative));
                for (uint32_t i = 0, borrow = 0; i < res.m_length; i++)
                    if (res.m_data[i] = __a.m_data[i] - (i < __b.m_length ? __b.m_data[i] : 0) - borrow; (borrow = res.m_data[i] < 0)) res.m_data[i] += _N;
                res.shrink();
                return res;
            } else if (comp < 0) {
                bint res(empty(__b.m_length, std::is_same_v<_Sign, std::negate<bint>> ? !__b.m_negative : __b.m_negative));
                for (uint32_t i = 0, borrow = 0; i < res.m_length; i++)
                    if (res.m_data[i] = __b.m_data[i] - (i < __a.m_length ? __a.m_data[i] : 0) - borrow; (borrow = res.m_data[i] < 0)) res.m_data[i] += _N;
                res.shrink();
                return res;
            } else
                return bint();
        }
        template <bool _Mod, typename _Result = std::conditional_t<_Mod, std::pair<bint, bint>, bint>>
        static _Result div_mod(const bint &__a, const bint &__b) {
            if (int comp = absCompare(__a, __b); comp <= 0) {
                if constexpr (_Mod)
                    return comp < 0 ? std::make_pair(bint(), __a) : std::make_pair(small(__a.m_negative == __b.m_negative ? 1 : -1), bint());
                else
                    return comp < 0 ? bint() : small(__a.m_negative == __b.m_negative ? 1 : -1);
            }
            uint32_t shift = __a.m_length > __b.m_length * 2 ? __a.m_length - __b.m_length * 2 : 0, n = __a.m_length + shift, m = __b.m_length + shift;
            bint a(__a << shift * _W), b(__b << shift * _W), c(0);
            bint bi(b.setSign(false).inv()), prod(b * bi), res(0);
            if (prod.m_length >= m * 2) prod -= b, --bi;
            for (a.setSign(false); b <= a && (c = a * bi >> (m * 2 * _W)); a -= b * c, res += c) {}
            while (b <= a) a -= b, ++res;
            res.setSign(__a.m_negative != __b.m_negative), a.setSign(__a.m_negative);
            if constexpr (_Mod)
                return {res, shift ? a >> shift * _W : a};
            else
                return res;
        }
        static std::pair<bint, long long> div_mod(const bint &__a, long long __b) {
            if (!__a.m_length) return {bint(), 0};
            bint res(empty(__a.m_length, __b < 0 ? !__a.m_negative : __a.m_negative));
            if (__b < 0) __b = -__b;
            long long carry = 0;
            for (uint32_t i = __a.m_length - 1; ~i; i--) {
                auto [q, r] = std::div(__a.m_data[i] + carry * _N, __b);
                res.m_data[i] = q, carry = r;
            }
            res.shrink();
            return {res, __a.m_negative ? -carry : carry};
        }
        static bint self_multiply(const bint &__a) {
            if (!__a.m_length) return bint();
            uint32_t length = std::__bit_ceil(__a.m_length * 2 - 1);
            dft(s_dftBuffer, length, __a);
            std::transform(s_dftBuffer, s_dftBuffer + length, s_dftBuffer, [](uint64_t x) { return s_mg.multiply(x, x); });
            idft(s_dftBuffer, length);
            bint res(empty(__a.m_length * 2, false));
            long long carry = 0;
            for (uint32_t i = 0; i + 1 < res.m_length; i++) {
                auto [quot, rem] = std::div((long long)(s_mg.reduce(s_dftBuffer[i]) + carry), (long long)_N);
                carry = quot, res.m_data[i] = rem;
            }
            res.m_data[res.m_length - 1] = carry;
            res.shrink();
            return res;
        }
        static bint &inplace_multiply(bint &__a, long long __b) {
            uint64_t carry = 0, i = 0;
            if (__b < 0) {
                __a.opposite();
                __b = -__b;
            }
            while (i < __a.m_length) {
                auto [quot, rem] = std::div((long long)(__a.m_data[i] * __b + carry), (long long)_N);
                __a.m_data[i++] = rem;
                carry = quot;
            }
            return __a;
        }
        static bint multiply(const bint &__a, long long __b) {
            uint64_t carry = 0, i = 0;
            while (i < __a.m_length) {
                auto [quot, rem] = std::div((long long)(__a.m_data[i] * std::abs(__b) + carry), (long long)_N);
                s_dftBuffer[i++] = rem;
                carry = quot;
            }
            while (carry) {
                auto [quot, rem] = std::div((long long)carry, (long long)_N);
                s_dftBuffer[i++] = rem;
                carry = quot;
            }
            bint res(empty(i, __b > 0 ? __a.m_negative : !__a.m_negative));
            while (~--i) res.m_data[i] = s_dftBuffer[i];
            return res;
        }
        bint &shrink() {
            if (!m_length) return *this;
            while (m_length && !m_data[m_length - 1]) m_length--;
            if (m_length) return *this;
            free(m_data);
            return setSign(false);
        }
        bint &opposite() { return setSign(!m_negative); }
        bint &setSign(bool __negative) {
            m_negative = m_length ? __negative : false;
            return *this;
        }
        bint &operator=(bint &&__other) {
            std::swap(m_length, __other.m_length);
            std::swap(m_data, __other.m_data);
            std::swap(m_negative, __other.m_negative);
            return *this;
        }
        bint &operator=(const bint &__other) {
            if (m_length) free(m_data);
            if ((m_length = __other.m_length)) std::copy_n(__other.m_data, m_length, m_data = malloc(m_length));
            return setSign(__other.m_negative);
        }
        bint &operator++() { return inc_dec<1, false>(*this); }
        bint &operator--() { return inc_dec<-1, true>(*this); }
        uint32_t length() const { return m_length ? (m_length - 1) * _W + (std::upper_bound(bases.val, bases.val + _W, m_data[m_length - 1]) - bases.val) : 0; }
        uint32_t ctz() const {
            if (!m_length) return 0;
            uint32_t i = 0, res = 0;
            while (i < m_length && !m_data[i]) {}
            for (uint32_t digit = m_data[i]; digit && digit % _B == 0; digit /= _B, res++) {}
            return i * _W + res;
        }
        bint inv() const {
            if (m_length == 1)
                return bint(bases.val[_W * 2] / m_data[0]);
            else if (m_length == 2)
                return bint(__int128_t(bases.val[_W * 2]) * bases.val[_W * 2] / (__int128_t(m_data[1]) * _N + m_data[0]));
            else {
                bint res((*this >> (m_length - 1) / 2 * _W).inv());
                return ((res + res) << (m_length - 1) / 2 * _W) - ((*this * (res * res)) >> (m_length / 2 + 1) * 2 * _W);
            }
        }
        bint pow(uint64_t __n) const {
            bint res(1), b(*this);
            while (__n) {
                if (__n & 1) res *= b;
                if (__n >>= 1) b = self_multiply(b);
            }
            return res;
        }
        bint sqrt(uint32_t __n) const {
            if (__n == 1) return *this;
            uint32_t _length = length();
            if (_length <= _W * 2) return bint(int64_t(std::pow((long double)(int64_t(*this)), (long double)1 / __n)));
            uint32_t a = (_length - _W * 2) / __n, b = _length - a * __n;
            long double log = (std::log10((long double)(uint64_t(*this >> (_length - _W * 2)))) / std::log10(_B) + (b - _W * 2)) / __n;
            if (a + log <= _W) {
                bint res(small(std::pow((long double)_B, a + log + (0.0000000001L))));
                return res.pow(__n) <= *this ? res : --res;
            }
            if (uint32_t c = std::min<uint32_t>(a, _W * 2)) log += c, a -= c;
            bint res(bint((__int128_t(std::pow((long double)_B, log)))) << a);
            while (true) {
                bint nxt(res * (__n - 1) / __n + (*this) / (res.pow(__n - 1) * __n));
                if (absClose(res, nxt)) {
                    if (nxt.m_data[0] < res.m_data[0]) res = nxt;
                    break;
                }
                res = nxt;
            }
            while (res.pow(__n) <= *this) ++res;
            return --res;
        }
        bint operator++(int) {
            bint old(*this);
            ++*this;
            return old;
        }
        bint operator--(int) {
            bint old(*this);
            --*this;
            return old;
        }
        bint &operator+=(const bint &__other) { return plus_minus_by<std::equal_to<bool>>(*this, __other); }
        bint &operator-=(const bint &__other) { return plus_minus_by<std::not_equal_to<bool>>(*this, __other); }
        bint &operator*=(const bint &__other) { return absCompare(__other, s_divThresh) <= 0 ? *this *= (long long)__other : (*this = *this * __other); }
        bint &operator*=(long long __other) {
            if (!m_length) return *this;
            if (!__other) return *this = bint();
            return (m_data[m_length - 1] + 1) * std::abs(__other) < _N ? inplace_multiply(*this, __other) : *this = multiply(*this, __other);
        }
        bint &operator/=(const bint &__other) { return absCompare(__other, s_divThresh) <= 0 ? *this /= (long long)__other : *this = div_mod<false>(*this, __other); }
        bint &operator/=(long long __other) {
            if (!m_length) return *this;
            if (__other < 0) {
                __other = -__other;
                opposite();
            }
            long long carry = 0;
            for (uint32_t i = m_length - 1; ~i; i--) {
                auto [q, r] = std::div(m_data[i] + carry * _N, __other);
                m_data[i] = q, carry = r;
            }
            return shrink();
        }
        bint &operator%=(const bint &__other) { return absCompare(__other, s_divThresh) <= 0 ? *this = bint(div_mod(*this, (long long)(__other)).second) : div_mod<true>(*this, __other).second; }
        bint &operator%=(long long __other) { return *this = bint(divmod(*this, __other).second); }
        bint &operator<<=(uint32_t __shift) { return *this = *this << __shift; }
        bint &operator>>=(uint32_t __shift) {
            auto [quot, rem] = std::div((int)__shift, (int)_W);
            if (quot >= m_length || (quot == m_length - 1 && m_data[m_length - 1] < bases.val[rem])) return *this = bint();
            std::copy_n(m_data + quot, m_length -= quot, m_data);
            if (!rem) return *this;
            uint64_t carry = 0;
            for (uint32_t i = m_length - 1; ~i; i--) {
                auto [q, r] = std::div(m_data[i], int(bases.val[rem]));
                m_data[i] = carry * bases.val[_W - rem] + q;
                carry = r;
            }
            return shrink();
        }
        bint operator+() const { return *this; }
        bint operator-() const {
            bint res(*this);
            res.opposite();
            return res;
        }
        bool operator==(const bint &__other) const { return m_negative == __other.m_negative && !absCompare(*this, __other); }
        bool operator!=(const bint &__other) const { return m_negative != __other.m_negative || absCompare(*this, __other); }
        bool operator<(const bint &__other) const { return m_negative == __other.m_negative ? (m_negative ? absCompare(*this, __other) > 0 : absCompare(*this, __other) < 0) : m_negative; }
        bool operator>(const bint &__other) const { return m_negative == __other.m_negative ? (m_negative ? absCompare(*this, __other) < 0 : absCompare(*this, __other) > 0) : !m_negative; }
        bool operator<=(const bint &__other) const { return m_negative == __other.m_negative ? (m_negative ? absCompare(*this, __other) >= 0 : absCompare(*this, __other) <= 0) : m_negative; }
        bool operator>=(const bint &__other) const { return m_negative == __other.m_negative ? (m_negative ? absCompare(*this, __other) <= 0 : absCompare(*this, __other) >= 0) : !m_negative; }
        explicit operator bool() const { return m_length; }
        template <typename _Tp>
        explicit operator _Tp() const {
            _Tp res = 0;
            for (uint32_t i = m_length - 1; ~i; i--) res = res * _N + m_data[i];
            return m_negative ? -res : res;
        }
        operator std::string() const {
            if (!m_length) return "0";
            std::string res(m_length * _W, '0');
            for (uint32_t i = 0; i < m_length; i++)
                for (uint32_t j = i * _W, digit = m_data[i]; digit; j++, digit /= _B) res[j] = fromInt(digit % _B);
            while (res.size() && res.back() == '0') res.pop_back();
            if (m_negative) res.push_back('-');
            std::reverse(res.begin(), res.end());
            return res;
        }
        friend bint operator+(const bint &__a, const bint &__b) {
            return plus_minus<std::equal_to<bool>>(__a, __b, [](const bint &x) { return x; });
        }
        friend bint operator-(const bint &__a, const bint &__b) { return plus_minus<std::not_equal_to<bool>>(__a, __b, std::negate<bint>()); }
        friend bint operator*(const bint &__a, const bint &__b) {
            if (&__a == &__b) return self_multiply(__a);
            if (!__a.m_length || !__b.m_length) return bint();
            if (absCompare(__a, s_divThresh) <= 0) return __b * (long long)__a;
            if (absCompare(__b, s_divThresh) <= 0) return __a * (long long)__b;
            uint32_t length = std::__bit_ceil(__a.m_length + __b.m_length - 1);
            dft(s_dftBuffer, length, __a);
            dft(s_dftBuffer + length, length, __b);
            for (uint32_t i = 0; i < length; i++) s_dftBuffer[i] = s_mg.multiply(s_dftBuffer[i], s_dftBuffer[i + length]);
            idft(s_dftBuffer, length);
            bint res(empty(__a.m_length + __b.m_length, __a.m_negative != __b.m_negative));
            long long carry = 0;
            for (uint32_t i = 0; i + 1 < res.m_length; i++) {
                auto [quot, rem] = std::div((long long)(s_mg.reduce(s_dftBuffer[i]) + carry), (long long)_N);
                carry = quot, res.m_data[i] = rem;
            }
            res.m_data[res.m_length - 1] = carry;
            res.shrink();
            return res;
        }
        friend bint operator*(const bint &__a, long long __b) {
            if (!__a.m_length || !__b) return bint();
            if ((__a.m_data[__a.m_length - 1] + 1) * std::abs(__b) < _N) {
                bint res(__a);
                inplace_multiply(res, __b);
                return res;
            } else
                return multiply(__a, __b);
        }
        friend bint operator/(const bint &__a, const bint &__b) { return absCompare(__b, s_divThresh) <= 0 ? div_mod(__a, (long long)(__b)).first : div_mod<false>(__a, __b); }
        friend bint operator/(const bint &__a, long long __b) { return div_mod(__a, __b).first; }
        friend bint operator%(const bint &__a, const bint &__b) { return absCompare(__b, s_divThresh) <= 0 ? bint(div_mod(__a, (long long)(__b)).second) : div_mod<true>(__a, __b).second; }
        friend long long operator%(const bint &__a, long long __b) { return div_mod(__a, __b).second; }
        friend bint operator<<(const bint &__a, uint32_t __shift) {
            auto [quot, rem] = std::div((int)__shift, (int)_W);
            bint res(empty(__a.m_length + quot + (rem > 0), __a.m_negative));
            std::copy_n(__a.m_data, __a.m_length, std::fill_n(res.m_data, quot, 0));
            if (!rem) return res;
            uint64_t carry = 0;
            for (uint32_t i = quot; i + 1 < res.m_length; i++) {
                auto [q, r] = std::div((long long)(res.m_data[i]) * bases.val[rem] + carry, (long long)_N);
                carry = q, res.m_data[i] = r;
            }
            res.m_data[res.m_length - 1] = carry;
            res.shrink();
            return res;
        }
        friend bint operator>>(const bint &__a, uint32_t __shift) {
            auto [quot, rem] = std::div((int)__shift, (int)_W);
            if (quot >= __a.m_length || (quot == __a.m_length - 1 && __a.m_data[__a.m_length - 1] < bases.val[rem])) return bint();
            bint res(empty(__a.m_length - quot, __a.m_negative));
            std::copy_n(__a.m_data + quot, __a.m_length - quot, res.m_data);
            if (!rem) return res;
            uint64_t carry = 0;
            for (uint32_t i = res.m_length - 1; ~i; i--) {
                auto [q, r] = std::div(res.m_data[i], int(bases.val[rem]));
                res.m_data[i] = carry * bases.val[_W - rem] + q, carry = r;
            }
            res.shrink();
            return res;
        }
        template <typename _Istream>
        friend _Istream &operator>>(_Istream &is, bint &self) {
            std::string number;
            is >> number;
            self = fromString(number.data(), number.size());
            return is;
        }
        template <typename _Ostream>
        friend _Ostream &operator<<(_Ostream &os, const bint &self) { return os << std::string(self); }
    };
#pragma pack()
}

#endif