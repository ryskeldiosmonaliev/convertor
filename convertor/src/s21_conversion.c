#include "s21_decimal.h"
#include "supp_func.h"
#include <stdlib.h>



int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  // Инициализируем переменную ошибки. Если всё пройдет успешно, err останется равным 0.
  int err = 0;

  // Проверка на NULL указатель. Если dst указывает на NULL, возвращаем ошибку (1).
  if (dst == NULL) return 1;

  // Обнуляем все биты структуры s21_decimal, чтобы избежать мусора.
  memset(dst, 0, 16);

  // Используем объединение для упрощенного доступа к битам числа типа float.
  union {
    float f;
    int i;
  } u;
     
  // Присваиваем абсолютное значение src переменной union, чтобы далее манипулировать с мантиссой и экспонентой.
  u.f = fabs(src);

  // Если исходное число было отрицательным, устанавливаем знак в старшем бите bits[3] структуры s21_decimal.
  if (src < 0) set_bit(&(dst->bits[3]), 31, 1);

  // Извлекаем экспоненту из числа float. Для этого смещаем мантиссу вправо на 23 бита и вычитаем смещение экспоненты (127).
  int exp = (u.i >> 23) - 127;

  // Извлекаем мантиссу из числа float. Для этого смещаем мантиссу влево на 9 бит и затем обратно вправо на 9 бит для удаления лишних битов.
  int mant = u.i << 9;
  mant >>= 9;

  // Инициализируем большую структуру s21_big_decimal для хранения промежуточного значения при конверсии.
  s21_big_decimal val = {0};

  // Задаем начальные значения для битов val (эквивалентные мантиссе).
  val.bits[0] = 0b00010000000000000000000000000000;
  val.bits[1] = 0b00111110001001010000001001100001;
  val.bits[2] = 0b00100000010011111100111001011110;

  // Если экспонента положительная, увеличиваем значение мантиссы путем сдвига влево на соответствующее количество бит.
  if (exp >= 0) {
    while (exp) {
      shuffle_big_dec_left(&val);
      --exp;
    }
  } else {
    // Если экспонента отрицательная, уменьшаем значение мантиссы путем сдвига вправо. Если мантисса обнулится до того, как экспонента станет нулевой, устанавливаем ошибку.
    while (exp != 0) {
      shuffle_big_dec_right(&val);
      ++exp;
      if (is_big_dec_zero(val) && exp < 0) err = 1;
    }
  }

  // Если ошибок не возникло, продолжаем выполнение.
  if (!err) {
    // Создаем копию val для дальнейших вычислений.
    s21_big_decimal temp = val;

    // Инициализируем индикатор для манипуляции с мантиссой.
    int indicator = 1 << 22;

    // Постепенно добавляем биты мантиссы к промежуточному значению.
    while (indicator) {
      shuffle_big_dec_right(&temp);
      if (indicator & mant) sum_equal_exp(val, temp, &val, 191);
      indicator >>= 1;
    }

    // Функция make_7_digits() обрезает мантиссу до 7 значимых цифр.
    make_7_digits(&val);

    // Переносим итоговые биты в структуру s21_decimal.
    for (int i = 0; i < 3; ++i) {
      dst->bits[i] = val.bits[i];
    }
    dst->bits[3] = val.bits[6];

    // Если исходное число было отрицательным, инвертируем знак результата.
    if (src < 0) s21_negate(*dst, dst);
  }

  // Возвращаем код ошибки. Если всё прошло успешно, будет возвращен 0.
  return err;
}


int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  // Инициализируем переменную result, которая будет хранить код возврата. По умолчанию она равна 0.
  int result = 0;

  // Обнуляем все биты структуры s21_decimal, чтобы избежать мусора. Размер памяти для dst равен 16 байтам.
  memset(dst, 0, 16);

  // Проверяем, является ли исходное целое число отрицательным.
  if (src < 0) {
    // Если число отрицательное, сохраняем его модуль в первый элемент массива bits структуры dst.
    dst->bits[0] = 0 - src;

    // Применяем функцию s21_negate, чтобы установить отрицательный знак для результата.
    s21_negate(*dst, dst);
  } else {
    // Если число неотрицательное, просто сохраняем его в первый элемент массива bits структуры dst.
    dst->bits[0] = src;
  }

  // Возвращаем код результата. В данном случае всегда возвращается 0, так как ошибок в процессе работы функции нет.
  return result;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  // Инициализируем переменную ошибки. Если всё пройдет успешно, err останется равным 0.
  int err = 0;

  // Получаем экспоненту из старшего бита структуры src.
  int exp = get_exp(src.bits[3]);

  // Определяем знак числа. Если знак отрицательный, то переменная sign будет равна -1, иначе 1.
  int sign = get_sign(src.bits[3]) ? -1 : 1;

  // Если экспонента равна нулю, то проверяем, можно ли преобразовать s21_decimal напрямую в int.
  if (exp == 0) {
    // Проверяем, что значения в bits[1] и bits[2] равны нулю, а bits[0] не превышает максимального значения для int.
    if (src.bits[1] == 0 && src.bits[2] == 0 &&
        (src.bits[0] <= MAX_INT || sign == -1)) {
      // Если все условия выполнены, то присваиваем значение dst с учетом знака.
      *dst = src.bits[0] * sign;
    } else
      // Если одно из условий не выполнено, устанавливаем код ошибки.
      err = 1;
  } else {
    // Если экспонента не равна нулю, преобразуем s21_decimal в s21_big_decimal для дальнейших вычислений.
    s21_big_decimal temp = convert_dec_to_big(src);

    // Делим значение на 10 до тех пор, пока экспонента не станет равной нулю.
    while (exp > 0) {
      div10(&temp);
      --exp;
    }

    // Если после деления значение temp равно нулю, то присваиваем dst ноль.
    if (is_big_dec_zero(temp)) {
      *dst = 0;
    } else {
      // Устанавливаем новую экспоненту в bits[6] для дальнейшего преобразования.
      set_exp(exp, &(temp.bits[6]));

      // Преобразуем обратно в s21_decimal.
      convert_big_to_dec(temp, &src, exp, 0);

      // Рекурсивно вызываем функцию для повторного преобразования уже уменьшенного значения.
      err = s21_from_decimal_to_int(src, dst);
    }
  }

  // Возвращаем код ошибки. Если всё прошло успешно, будет возвращен 0.
  return err;
}
int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  // Инициализируем переменную ошибки. Если все пройдет успешно, err останется равным 0.
  int err = 0;

  // Проверяем, что указатель на результат не равен NULL.
  if (dst == NULL) {
    // Если указатель dst равен NULL, устанавливаем код ошибки.
    err = 1;
  } else {
    // Инициализируем результат как 0.0, если указатель корректен.
    *dst = 0.0;

    // Временные переменные для расчета результата.
    double temp = 1.0, result = 0.0;

    // Получаем знак числа (1 для отрицательного числа, 0 для положительного).
    int sign = get_sign(src.bits[3]);

    // Получаем экспоненту из старшего бита структуры src.
    int exp = get_exp(src.bits[3]);

    // Преобразуем s21_decimal в s21_big_decimal- для удобства дальнейших вычислений.
    s21_big_decimal b_src = convert_dec_to_big(src);

    // Обнуляем старший бит src, чтобы убрать знак и экспоненту.
    src.bits[3] = 0;

    // Переводим значащие биты в значение float, выполняя побитовые операции.
    while (!is_big_dec_zero(b_src)) {
      // Если младший бит равен 1, прибавляем временную переменную temp к результату.
      (b_src.bits[0] & 1) ? result += temp : 1;

      // Увеличиваем temp в два раза.
      temp *= 2;

      // Сдвигаем все биты вправо.
      shuffle_big_dec_right(&b_src);
    }

    // Корректируем результат в зависимости от экспоненты.
    while (exp) {
      result /= 10;
      --exp;
    }

    // Проверяем, превышает ли результат максимальное значение для float.
    if (result > MAX_FLT)
      err = 1;  // Устанавливаем код ошибки, если результат слишком велик.
    else {
      // Присваиваем полученное значение указателю на float.
      *dst = result;

      // Если исходное число отрицательное, делаем результат отрицательным.
      if (sign) *dst *= -1;
    }
  }

  // Возвращаем код ошибки. Если все прошло успешно, err будет равен 0.
  return err;
}
void make_7_digits(s21_big_decimal *val) {
  // Создаем временную переменную для хранения результата деления и других промежуточных значений.
  s21_big_decimal temp = {0};

  // Начальная экспонента равна 28, так как это максимальное количество знаков после запятой в s21_decimal.
  int exp = 28;

  // Пока первое число имеет больше 7 знаков, или остальные биты не равны нулю, продолжаем цикл.
  // Это необходимо для того, чтобы уменьшить значение числа до 7 значащих цифр.
  while (val->bits[0] >= 10000000 || val->bits[1] != 0 || val->bits[2] != 0 ||
         val->bits[3] != 0 || val->bits[4] != 0 || val->bits[5] != 0 ||
         exp > 7) {
    // Делим число на 10 и уменьшаем экспоненту.
    temp = div10(val);
    --exp;
  }

  // Если остаток от деления (последняя цифра) больше или равен 5, прибавляем единицу к последней значащей цифре.
  if (temp.bits[0] >= 5) {
    temp.bits[0] = 1;
    sum_equal_exp(*val, temp, val, 191); // Суммируем с единицей на нужной позиции.
  }

  // Если экспонента ушла в отрицательную зону, возвращаем её в допустимый диапазон умножением числа на 10.
  while (exp < 0) {
    _x10(val); // Умножаем на 10.
    ++exp;
  }

  // Еще раз делим на 10 и корректируем экспоненту.
  temp = div10(val);
  --exp;

  // Пока число не сократилось до 7 значащих цифр и экспонента положительна, продолжаем деление.
  while (is_big_dec_zero(temp) && exp > 0) {
    temp = div10(val);
    --exp;
  }

  // Умножаем на 10, возвращая одно деление назад.
  _x10(val);

  // Суммируем результат, добавляя остаток.
  sum_equal_exp(*val, temp, val, 191);

  // Увеличиваем экспоненту на единицу, так как прои                       зведено одно умножение на 10.
  ++exp;

  // Устанавливаем конечную экспоненту в число, корректируя его представление.
  set_exp(exp, &(val->bits[6]));
}
int s21_from_decimal_to_in(s21_decimal src, int *dst) {
  int err2= 0;
  int err = 0;
  int exp = get_exp(src.bits[3]);
  int sign = get_sign(src.bits[3]) ? -1 : 1;
  if (exp == 0) {
    if (src.bits[1] == 0 && src.bits[2] == 0 &&
        (src.bits[0] <= MAX_INT || sign == -1)) {
      *dst = src.bits[0] * sign;
    } else
      err = 1;
  } else {
    s21_big_decimal temp = convert_dec_to_big(src);
    while (exp > 0) {
      //   print_bits_big_decimal(&temp);
      div10(&temp);
      //  print_bits_big_decimal(&temp);
      --exp;
    }
    if (is_big_dec_zero(temp)) {
      *dst = 0;
    } else {
      set_exp(exp, &(temp.bits[6]));
      convert_big_to_dec(temp, &src, exp, 0);
      err = s21_from_decimal_to_int(src, dst);
    }
  }
  return err&&err2;
}
