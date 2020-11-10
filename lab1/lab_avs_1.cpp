#include <iostream>
#include <mmintrin.h>
#include <string>
#include <sstream>
#include <vector>

// Ввод восьми восьмибитных числа
__m64 input_number()
{
    std::string number = "";
    std::getline(std::cin, number, '\n');
    std::stringstream ss(number);
    long* v = new long [8];
    for (size_t i = 0; i < 8; i++)
    {
        std::string s;
        ss >> s;
        v[i] = (long)std::atol(s.c_str());
    }
    return _mm_set_pi8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
}

// Ввод восьми шестнадцатибитных числа
std::vector<__m64> input_number_16()
{
    std::string number = "";
    std::getline(std::cin, number, '\n');
    std::stringstream ss(number);
    long* v = new long[8];
    for (size_t i = 0; i < 8; i++)
    {
        std::string s;
        ss >> s;
        v[i] = (long)std::atol(s.c_str());
    }
    std::vector<__m64> result;
    result.push_back(_mm_set_pi16(v[0], v[1], v[2], v[3]));
    result.push_back(_mm_set_pi16(v[4], v[5], v[6], v[7]));
    return result;
}

// 8 бит (беззнаковые) * 8 бит (беззнаковые) = 8 бит (беззнаковые) массив // В ЛАБЕ НЕ ИСПОЛЬЗУЕТСЯ
__m64 _mm_mul_pu8(__m64 a, __m64 b)
{
    // Берём нечётные восьмёрки битов, умножаем. в чётные ставим нули
    __m64 dst_even = _mm_mullo_pi16(a, b);
    // Берём чётные восьмёрки битов, умножаем
    a = _mm_srli_pi16(a, 8); b = _mm_srli_pi16(b, 8);
    __m64 dst_odd = _mm_mullo_pi16(a, b);
    // Соединяем через or, только в нечётных сдвигаем туда-обратно, чтобы избавиться от лишних цифр справа
    // которые получились после умножения
    return _m_por(_mm_slli_pi16(dst_odd, 8), _mm_srli_pi16(_mm_slli_pi16(dst_even, 8), 8));
}

// 8 бит * 8 бит = 8 бит массив // В ЛАБЕ НЕ ИСПОЛЬЗУЕТСЯ
__m64 _mm_mul_pi8(__m64 a, __m64 b)
{
    __m64 zero = _mm_setzero_si64();
    // Строим маску, сравнивая 0 с каждым элементом, если 0 больше, то число хранится в доп коде
    __m64 mask_a = _mm_cmpgt_pi8(zero, a);
    // Чередуем часть массива с элементами маски, там где число отрицательное, будет -1, иначе 0
    __m64 a_lo = _mm_unpacklo_pi8(a, mask_a);
    __m64 a_hi = _mm_unpackhi_pi8(a, mask_a);

    __m64 mask_b = _mm_cmpgt_pi8(zero, b);
    __m64 b_lo = _mm_unpacklo_pi8(b, mask_b);
    __m64 b_hi = _mm_unpackhi_pi8(b, mask_b);

    // Умножаем соответствующие числа
    __m64 a_b_lo = _mm_mullo_pi16(a_lo, b_lo);
    __m64 a_b_hi = _mm_mullo_pi16(a_hi, b_hi);

    return _mm_packs_pi16(a_b_lo, a_b_hi);
}

// 8 бит * 8 бит = 2 массива 16 бит
std::vector<__m64> _mm_mul_pi8_e(__m64 a, __m64 b)
{
    __m64 zero = _mm_setzero_si64();
    // Строим маску, сравнивая 0 с каждым элементом, если 0 больше, то число хранится в доп коде
    __m64 mask_a = _mm_cmpgt_pi8(zero, a);
    // Чередуем часть массива с элементами маски, там где число отрицательное, будет -1, иначе 0
    __m64 a_lo = _mm_unpacklo_pi8(a, mask_a);
    __m64 a_hi = _mm_unpackhi_pi8(a, mask_a);

    __m64 mask_b = _mm_cmpgt_pi8(zero, b);
    __m64 b_lo = _mm_unpacklo_pi8(b, mask_b);
    __m64 b_hi = _mm_unpackhi_pi8(b, mask_b);

    // Умножаем соответствующие числа
    __m64 a_b_lo = _mm_mullo_pi16(a_lo, b_lo);
    __m64 a_b_hi = _mm_mullo_pi16(a_hi, b_hi);

    std::vector<__m64> result;
    result.push_back(a_b_lo);
    result.push_back(a_b_hi);
    return result;
}

// 8 бит * 16 бит = 2 массива 16 бит
std::vector<__m64> _mm_mul_pi16_e(__m64 a, std::vector<__m64> b)
{
    std::vector<__m64> result;

    __m64 zero = _mm_setzero_si64();
    __m64 mask_a = _mm_cmpgt_pi8(zero, a);
    __m64 a_lo = _mm_unpacklo_pi8(a, mask_a);
    __m64 a_hi = _mm_unpackhi_pi8(a, mask_a);

    __m64 a_b_lo = _mm_mullo_pi16(a_lo, b[1]);
    __m64 a_b_hi = _mm_mullo_pi16(a_hi, b[0]);

    result.push_back(a_b_lo);
    result.push_back(a_b_hi);

    return result;
}

int main()
{
    // 14. F[i] = (A[i]*C[i])+(B[i]*D[i])
    __m64 A, B, C; // массив, в котором 8 8-битных элементов
    std::vector<__m64> D; // 2 массива, в каждом по 4 16-битных элемента

    std::cout << "Now, enter A (8 elements 8 bit), B (8 e 8 b), C (8 e 8 b), D (8 e 16 b)\n";
    std::cout << "F[i] = (A[i]*C[i])+(B[i]*D[i]) will be calculated\n";

    std::cout << "A: "; A = input_number(); std::cout << "B: "; B = input_number();
    std::cout << "C: "; C = input_number(); std::cout << "D: "; D = input_number_16();

    std::vector<__m64> 
        B_D = _mm_mul_pi16_e(B, D),
        A_C = _mm_mul_pi8_e(A, C), F;

    F.push_back(_mm_add_pi16(B_D[0], A_C[0]));
    F.push_back(_mm_add_pi16(B_D[1], A_C[1]));

    std::cout << "F: ";
    for (char j = 1; j >= 0; j--)
        for (char i = 3; i >= 0; i--)
            std::cout << int(F[j].m64_i16[i]) << ' ';

    std::cout << "\nHere's answer after calculating it manually:\nF: ";
    int k = 0, p = 3;
    for (char i = 7; i >= 0; i--)
    {
        std::cout << __int16((__int8)A.m64_i8[i] * (__int8)C.m64_i8[i]) +
                            ((__int8)B.m64_i8[i] * (__int16)D[k].m64_i16[p]) << ' ';
        p --;
        if (i == 4)
        {
            k += 1; p = 3;
        }
    }

    _m_empty();
    return 0;
}
// Примеры

// 2 3 4 5 1 2 3 4