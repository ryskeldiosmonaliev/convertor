#include <stdio.h>

#include "s21_decimal.h"
#include "supp_func.h"


s21_big_decimal s21_div_supp(s21_big_decimal value_1, s21_big_decimal value_2,
                             s21_big_decimal *result) {
  // @param value_1: Делимое в операции деления.
  // @param value_2: Делитель в операции деления.
  // @param result: Указатель на переменную, в которую будет записан результат деления.

  // Обнуляем переменную result, чтобы подготовить её для записи результата.
  memset(result, 0, 28);

  // Устанавливаем экспоненту (масштаб) обоих значений в 0.
  set_exp(0, &(value_1.bits[6]));
  set_exp(0, &(value_2.bits[6]));

  // Получаем позиции последних значащих битов в value_1 и value_2.
  int lb1 = get_last_bit(value_1), lb2 = get_last_bit(value_2);

  // Вычисляем разницу в позициях последних битов.
  int diff = lb1 - lb2;

  // Сдвигаем делитель (value_2) влево, чтобы выровнять его с делимым (value_1).
  for (int i = 0; i < diff; ++i) shuffle_big_dec_left(&value_2);

  // Основной цикл деления.
  while (diff >= 0) {
    // Если делитель меньше или равен делимому, выполняем вычитание.
    if (s21_is_less_or_equal_big(value_2, value_1)) {
      // Вычитаем делитель из делимого.
      minus_equal_exp(value_1, value_2, &value_1);

      // Устанавливаем соответствующий бит в результате.
      int bit = diff % 32;
      bit == 0 ? bit = diff : 1;
      set_bit(&(result->bits[diff / 32]), bit, 1);

      // Сдвигаем делитель вправо.
      shuffle_big_dec_right(&value_2);
    } else {
      // Если делитель больше делимого, устанавливаем бит в 0.
      int bit = diff % 32;
      diff < 32 ? bit = diff : 1;
      set_bit(&(result->bits[diff / 32]), bit, 0);

      // Сдвигаем делитель вправо.
      shuffle_big_dec_right(&value_2);
    }
    // Уменьшаем разницу битов на 1.
    --diff;
  }
  
  // Возвращаем остаток от деления (остаток в value_1).
  return value_1;
}


s21_big_decimal div10(s21_big_decimal *value_1) {
  // @param value_1: Указатель на значение, которое нужно разделить на 10.
  // @return: Результат деления value_1 на 10.

  // Инициализируем переменную value_2 для хранения числа 10.
  s21_big_decimal value_2;
  memset(&value_2, 0, 28); // Обнуляем все биты в переменной value_2.
  value_2.bits[0] = 10; // Устанавливаем значение 10 в переменной value_2.

  // Инициализируем переменную result для хранения результата деления.
  s21_big_decimal result;
  memset(&result, 0, 28); // Обнуляем все биты в переменной result.

  // Выполняем деление value_1 на 10 с использованием вспомогательной функции s21_div_supp.
  // Результат деления сохраняется в result, а остаток записывается обратно в value_1.
  result = s21_div_supp(*value_1, value_2, value_1);

  // Возвращаем результат деления.
  return result;
}
