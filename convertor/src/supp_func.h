#ifndef supp_func_h
#define supp_func_h

#include <stdio.h>
#include <string.h>

#include "s21_decimal.h"
void set_bit(uint32_t* dst, int pos, int bit);
int _x10(s21_big_decimal* dst);
int sum_equal_exp(s21_big_decimal summand, s21_big_decimal addend,
                  s21_big_decimal* result, int last_bit);
int minus_equal_exp(s21_big_decimal red, s21_big_decimal ded,
                    s21_big_decimal* result);
void shuffle_big_dec_right(s21_big_decimal* dst);
int shuffle_big_dec_left(s21_big_decimal* dst);
int get_sign(uint32_t val);
uint32_t get_exp(uint32_t val);
void set_exp(uint32_t val, uint32_t* data);
int get_last_bit(s21_big_decimal val);
int is_big_dec_zero(s21_big_decimal data);
s21_big_decimal convert_dec_to_big(s21_decimal val);
#endif /* supp_func_h */