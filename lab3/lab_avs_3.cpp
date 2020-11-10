#include <iostream>
#include <omp.h>
#include <chrono>

void drawMatrix(int**matrix, uint16_t a, uint16_t b) {
    for (size_t i = 0; i < a; i++)
    {
        for (size_t j = 0; j < b; j++) 
            std::cout << matrix[i][j] << " ";
        std::cout << '\n';
    }
    std::cout << '\n';
}

void instantiateMatrix(int** matrix, uint16_t a, uint16_t b, bool zero = false) {
    for (uint16_t i = 0; i < a; i++) {
        matrix[i] = new int[b];
        for (uint16_t j = 0; j < b; j++)
            matrix[i][j] = zero ? 0 : rand() % 9;
    }
}

void calculateWithOMP(uint16_t M, uint16_t  N, uint16_t  K, int**a, int** b, int** c, int ** thread_num, bool showThreadNum = false) {
    int i = 0;
    omp_set_num_threads(8);
    #pragma omp parallel shared(M, N, K, a, b, c)
    {
        uint16_t tmp = 0;
        while (i < M * N) {
        #pragma critical section
            tmp = i++;

            // i / N = [i]         // i % N = [j]
            for (int k = 0; k < K; ++k) {
                c[tmp / N][tmp % N] += a[tmp / N][k] * b[k][tmp % N];
            }
            //uint16_t num = omp_get_thread_num() + 1;
            //thread_num[tmp / N][tmp % N] = num;
            //printf("Thread %d processing [%d][%d] element\n", num, tmp / N, tmp % N);
        }
    }
    if (showThreadNum)
    {
        printf("num of thread processed [i][j] element of matrix\n");
        drawMatrix(thread_num, M, N);
    }
    //drawMatrix(c, M, N);
}

void calcaulateWithoutOMP(uint16_t M, uint16_t  N, uint16_t  K, int** a, int** b, int** c) {
    uint16_t i = 0;
    while (i < M * N) {
        for (int k = 0; k < K; ++k) {
            c[i/ N][i % N] += a[i / N][k] * b[k][i % N];
        }
        i++;
    }
    //drawMatrix(c, M, N);
}

void testConfiguration(uint16_t M, uint16_t  N, uint16_t  K, bool showThreadNum = false) {
    uint16_t myid, i = 0;
    int** a = new int* [M], ** b = new int* [K], **c = new int* [M], **thread_num = new int* [M];
    printf("M:%d, N:%d, K:%d\n", M, N, K);
    instantiateMatrix(a, M, K);
    instantiateMatrix(b, K, N);
    instantiateMatrix(c, M, N, true);
    instantiateMatrix(thread_num, M, N, true);
    //printf("matrix a:\n");
    //drawMatrix(a, M, K);
    //printf("matrix b:\n");
    //drawMatrix(b, K, N);

    auto t1 = std::chrono::high_resolution_clock::now();
    calculateWithOMP(M, N, K, a, b, c, thread_num);
    auto t2 = std::chrono::high_resolution_clock::now();
    int total = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    printf("Time taken with OMP:%i microseconds\n", total);

    instantiateMatrix(c, M, N, true);

    t1 = std::chrono::high_resolution_clock::now();
    calcaulateWithoutOMP(M, N, K, a, b, c);
    t2 = std::chrono::high_resolution_clock::now();
    total = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    printf("Time taken without OMP:%i microseconds\n", total);

    /*drawMatrix(c, M, N);*/
}

int main()
{
    testConfiguration(50, 20, 10, true);
    testConfiguration(100, 70, 50, true);
    testConfiguration(150, 250, 170, true);
    
    return 0;
}