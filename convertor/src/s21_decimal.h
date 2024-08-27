#ifndef s21_decimal_h
#define s21_decimal_h
#define MAX(i, j) (i > j ? i : j)
#define MAX_INT 2147483647
#define MAX_UINT 4294967295
#define MAX_FLT 3.402823e+38

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint32_t bits[4];
} s21_decimal;
typedef struct {
  uint32_t bits[7];
} s21_big_decimal;

int s21_from_float_to_decimal(float src, s21_decimal *dst);
void make_7_digits(s21_big_decimal *val);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_negate(s21_decimal value, s21_decimal *result);
int s21_negate_big(s21_big_decimal value, s21_big_decimal *result);
s21_big_decimal s21_div_supp(s21_big_decimal value_1, s21_big_decimal value_2,
                             s21_big_decimal *result);
#endif /* s21_decimal_h */
