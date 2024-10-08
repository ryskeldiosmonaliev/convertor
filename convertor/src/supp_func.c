#include "supp_func.h"

int get_sign(uint32_t val) { return (val >> 31) & 1; }

// set 0 or 1 into pos
//myfund
void set_bit(uint32_t *dst, int pos, int bit) {
  // @param dst: Указатель на 32-битное целое число, в котором нужно установить или сбросить бит.
  // @param pos: Позиция бита, который нужно установить или сбросить (0-31).
  // @param bit: Значение бита, которое нужно установить (1 или 0).

  // Если значение bit равно 1, устанавливаем бит на позиции pos.
  if (bit) {
    *dst |= (1 << pos); // Используем побитовое ИЛИ, чтобы установить бит.
  } else {
    // Если значение bit равно 0, сбрасываем бит на позиции pos.
    *dst &= ~(1 << pos); // Используем побитовое И НЕ, чтобы сбросить бит.
  }
}


uint32_t get_exp(uint32_t val) {
  // @param val: 32-битное целое число, из которого нужно извлечь значение экспоненты.
  // @return: Возвращает значение экспоненты, извлечённое из числа val.

  // Сдвигаем значение val влево на 1 бит, чтобы убрать знак, находящийся в 31-м бите.
  val <<= 1;

  // Сдвигаем значение val вправо на 17 бит, чтобы извлечь значение экспоненты,
  // находящееся в битах с 16-го по 23-й (включительно).
  val >>= 17;

  // Возвращаем полученное значение экспоненты.
  return val;
}

void set_exp(uint32_t val, uint32_t *data) {
  // @param val: Значение экспоненты, которое нужно установить.
  // @param data: Указатель на 32-битное целое число, в которое будет установлена экспонента.

  // Сдвигаем значение val влево на 16 бит, чтобы подготовить его для установки в соответствующие биты.
  val <<= 16;

  // Проверяем, установлен ли знак в исходном значении *data.
  if (get_sign(*data)) {
    *data = val; // Устанавливаем экспоненту в data, перезаписывая старые данные.
    set_bit(data, 31, 1); // Восстанавливаем знак, устанавливая 31-й бит.
  } else {
    *data = val; // Устанавливаем экспоненту в data, перезаписывая старые данные.
  }
}


// multiply mantissa x10 exp doesnt change
// returns 0 if overflow
int _x10(s21_big_decimal *dst) {
  // @param dst: Указатель на большое десятичное число (s21_big_decimal), которое нужно умножить на 10.
  // @return: Возвращает 1, если операция прошла успешно, и код ошибки в случае неудачи.

  int err = 1; // Переменная для хранения кода ошибки. Изначально предполагается, что операция успешна.

  // Сдвигаем число влево на 1 бит, что эквивалентно умножению на 2.
  err = shuffle_big_dec_left(dst);

  // Создаем временную переменную temp, копируя в нее значение dst.
  s21_big_decimal temp = *dst;

  // Снова сдвигаем temp влево на 1 бит (умножение на 2).
  err = shuffle_big_dec_left(&temp);

  // Еще раз сдвигаем temp влево на 1 бит (умножение на 2).
  err = shuffle_big_dec_left(&temp);

  // Складываем dst и temp, результат сохраняется в dst. Это завершает умножение исходного значения на 10.
  err = sum_equal_exp(*dst, temp, dst, 192);

  // Возвращаем код ошибки (1, если все прошло успешно).
  return err;
}

// returns 0 if overflow, shuffles decimal struct by 1 to the left
iint shuffle_big_dec_left(s21_big_decimal *dst) {
  // @param dst: Указатель на большое десятичное число (s21_big_decimal), которое нужно сдвинуть влево на один бит.
  // @return: Возвращает 1, если операция прошла успешно.

  int err = 1; // Переменная для хранения кода ошибки. Изначально предполагается, что операция успешна.
  int flag_plus = 0; // Флаг, указывающий, нужно ли переносить бит в следующий элемент массива bits.

  // Проходим по всем элементам массива bits, кроме последнего (индексы от 0 до 5).
  for (int i = 0; i < 6; ++i) {
    int temp = 0; // Временная переменная для хранения значения бита, который нужно перенести.

    // Если flag_plus установлен, сохраняем в temp значение 1.
    if (flag_plus == 1) {
      temp = 1;
    }

    // Проверяем, установлен ли старший бит (31-й) в текущем элементе массива bits.
    if (dst->bits[i] & (1 << 31)) {
      flag_plus = 1; // Если старший бит установлен, устанавливаем flag_plus для переноса.
    } else {
      flag_plus = 0; // Если старший бит не установлен, сбрасываем flag_plus.
    }

    // Сдвигаем текущий элемент массива bits влево на 1 бит.
    dst->bits[i] <<= 1;

    // Если temp равен 1, устанавливаем младший бит (0-й) в текущем элементе массива bits.
    if (temp) {
      set_bit(&(dst->bits[i]), 0, 1);
    }
  }

  // Возвращаем код ошибки (1, если все прошло успешно).
  return err;
}

void shuffle_big_dec_right(s21_big_decimal *dst) {
  // @param dst: Указатель на большое десятичное число (s21_big_decimal), которое нужно сдвинуть вправо на один бит.

  // Сдвигаем младший элемент массива bits вправо на 1 бит.
  dst->bits[0] >>= 1;

  // Проходим по остальным элементам массива bits (индексы от 1 до 5).
  for (int i = 1; i < 6; ++i) {
    // Если младший бит текущего элемента установлен, устанавливаем старший бит предыдущего элемента.
    if (dst->bits[i] & 1) {
      set_bit(&(dst->bits[i - 1]), 31, 1);
    }

    // Сдвигаем текущий элемент массива bits вправо на 1 бит.
    dst->bits[i] >>= 1;
  }
}

// summ 2 decimal, from 0 to last_bit, which have equal exp returns -1 if exp
// are not equal
int sum_equal_exp(s21_big_decimal summand, s21_big_decimal addend,
                  s21_big_decimal *result, int last_bit) {
  // @param summand: Первое большое десятичное число (s21_big_decimal), которое участвует в сложении.
  // @param addend: Второе большое десятичное число (s21_big_decimal), которое участвует в сложении.
  // @param result: Указатель на большое десятичное число (s21_big_decimal), в котором будет сохранен результат сложения.
  // @param last_bit: Индекс последнего бита для выполнения операции сложения.

  int err = 1; // Переменная для хранения кода ошибки. Изначально предполагается, что операция успешна.
  s21_big_decimal temp; // Временная переменная для хранения промежуточных значений.
  memset(&temp, 0, 28); // Инициализируем temp нулями.
  memset(result, 0, 28); // Инициализируем result нулями.

  // Устанавливаем экспоненту результата в значение экспоненты первого операнда.
  set_exp(get_exp(summand.bits[6]), &(result->bits[6]));

  int flag_plus = 0; // Флаг для хранения необходимости переноса при сложении.

  // Проходим по всем элементам массива bits, учитывая размер в битах (last_bit).
  for (int i = 0; i < last_bit / 32 + 1; ++i) {
    int ind = 0; // Индекс текущего бита в элементе массива bits.
    int flag_local_plus = 0; // Локальный флаг переноса для текущего элемента.
    int bit = 31; // Количество битов для текущего элемента.

    // Определяем количество битов для текущего элемента, учитывая последний бит.
    i == last_bit / 32 ? bit = last_bit % 32 : 1;
    last_bit < 32 ? bit = last_bit : 1;

    // Проходим по всем битам в текущем элементе массива bits.
    for (int j = 0; j < bit + 1; ++j) {
      int s_bite = 0, a_bite = 0; // Значения битов для сложения.
      s_bite = summand.bits[i] & 1; // Получаем младший бит из summand.
      a_bite = addend.bits[i] & 1; // Получаем младший бит из addend.
      int summ = a_bite + s_bite + flag_local_plus; // Суммируем биты и флаг переноса.

      // Учитываем перенос, если он был.
      if (flag_plus) {
        summ += flag_plus;
        flag_plus = 0;
      }

      // Определяем значение бита в результате и устанавливаем флаг переноса.
      if (summ == 1) {
        flag_local_plus = 0;
        set_bit(&(result->bits[i]), ind, 1);
      } else if (summ == 2) {
        flag_local_plus = 1;
        set_bit(&(result->bits[i]), ind, 0);
      } else if (summ == 3) {
        flag_local_plus = 1;
        set_bit(&(result->bits[i]), ind, 1);
      }

      // Устанавливаем флаг переноса для следующего бита, если нужно.
      if (j == bit && flag_local_plus) {
        flag_plus = 1;
      }

      ++ind; // Переходим к следующему биту.
      summand.bits[i] >>= 1; // Сдвигаем summand вправо на 1 бит.
      addend.bits[i] >>= 1; // Сдвигаем addend вправо на 1 бит.
    }
  }

  // Если флаг переноса установлен, возвращаем 0 (успех), иначе возвращаем 1 (ошибка).
  flag_plus == 1 ? err = 0 : 1;
  return err;
}


int minus_equal_exp(s21_big_decimal red, s21_big_decimal ded,
                    s21_big_decimal *result) {
  // @param red: Минусуемое большое десятичное число (s21_big_decimal).
  // @param ded: Вычитаемое большое десятичное число (s21_big_decimal).
  // @param result: Указатель на большое десятичное число (s21_big_decimal), в котором будет сохранен результат вычитания.

  int err = 0; // Переменная для хранения кода ошибки. Изначально предполагается, что операция успешна.
  int last_bit = MAX(get_last_bit(ded), get_last_bit(red)); // Находим индекс последнего бита для вычитания.

  memset(result, 0, 28); // Инициализируем результат нулями.
  set_exp(red.bits[6], &(result->bits[6])); // Устанавливаем экспоненту результата в значение экспоненты минусуемого числа.

  // Если знаки чисел разные, инвертируем знак вычитаемого числа и выполняем сложение.
  if (!get_sign(red.bits[6]) && get_sign(ded.bits[6])) {
    s21_negate_big(ded, &ded);  // Инвертируем знак вычитаемого числа.
    err = sum_equal_exp(red, ded, result, 120); // Выполняем сложение после инвертирования знака.
  } else if (get_sign(red.bits[6]) && !get_sign(ded.bits[6])) { 
    // Если знаки чисел разные, инвертируем знак минусуемого числа, выполняем сложение и инвертируем знак результата.
    s21_negate_big(red, &red);
    err = sum_equal_exp(red, ded, result, 120);
    s21_negate_big(*result, result);
  } else if (get_sign(red.bits[6]) && get_sign(ded.bits[6])) {
    // Если знаки обоих чисел одинаковые, инвертируем оба числа и выполняем вычитание.
    s21_negate_big(ded, &ded);
    s21_negate_big(red, &red);
    err = minus_equal_exp(ded, red, result);
  } else {
    // Если знаки обоих чисел одинаковые и положительные, выполняем вычитание по обычной схеме.

    // Инвертируем биты вычитаемого числа.
    for (int i = 0; i < last_bit / 32 + 1; ++i) {
      if (i == last_bit / 32) {
        int bit = last_bit % 32, temp = ded.bits[i];
        last_bit < 32 ? bit = last_bit : 1;
        for (int j = 0; j < bit + 1; ++j) {
          if (temp & 1)
            set_bit(&ded.bits[i], j, 0);
          else
            set_bit(&ded.bits[i], j, 1);
          temp >>= 1;
        }
      } else {
        ded.bits[i] = ~ded.bits[i];
      }
    }

    // Создаем временное значение для добавления единицы после инвертирования.
    s21_big_decimal temp;
    memset(&temp, 0, 28);
    temp.bits[0] = 1;

    // Добавляем единицу к инвертированному числу.
    sum_equal_exp(ded, temp, &ded, last_bit);

    // Выполняем сложение минусуемого числа с результатом.
    int sign = sum_equal_exp(red, ded, result, last_bit);
    if (sign && !is_big_dec_zero(ded)) {
      s21_big_decimal temp1 = *result;
      memset(result, 0, 28);

      // Инвертируем результат, если требуется.
      for (int i = 0; i < last_bit / 32 + 1; ++i) {
        int bit = 31;
        i == last_bit / 32 ? bit = last_bit % 32 : 1;
        last_bit < 32 ? bit = last_bit : 1;
        for (int j = 0; j < bit + 1; ++j) {
          if (!(temp1.bits[i] & 1)) {
            set_bit(&(result->bits[i]), j, 1);
          }
          temp1.bits[i] >>= 1;
        }
      }

      // Добавляем единицу и инвертируем знак результата.
      sum_equal_exp(*result, temp, result, 191);
      s21_negate_big(*result, result);
    }
  }
  return err; // Возвращаем код ошибки (0 - успех, 1 - ошибка).
}


// проверить валгриндом можно ли сдвигать биты для инта 33 раза
int get_last_bit(s21_big_decimal val) {
  // @param val: Большое десятичное число (s21_big_decimal), для которого нужно определить позицию последнего установленного бита.

  int result = 0; // Переменная для хранения позиции последнего установленного бита. Изначально устанавливаем в 0.

  // Проходим по всем 192 битам (с учетом 6 элементов по 32 бита).
  for (int i = 0; i < 192; ++i) {
    // Проверяем, установлен ли младший бит в текущем элементе.
    if (val.bits[i / 32] & 1) result = i; // Если да, сохраняем позицию текущего бита.
    
    // Сдвигаем биты текущего элемента вправо на 1 позицию.
    val.bits[i / 32] >>= 1;
  }

  // Возвращаем позицию последнего установленного бита.
  return result;
}


int is_big_dec_zero(s21_big_decimal data) {
  // @param data: Большое десятичное число (s21_big_decimal), которое нужно проверить на равенство нулю.

  int res = 1; // Переменная для хранения результата проверки. Изначально устанавливается в 1 (предполагаем, что число нулевое).

  // Проходим по всем 6 элементам массива bits.
  for (int i = 0; i < 6; ++i) {
    // Если текущий элемент не равен 0, устанавливаем res в 0 и прерываем цикл, так как число не является нулевым.
    if (data.bits[i] != 0) {
      res = 0;
      break; // Прерываем цикл, так как обнаружен ненулевой элемент.
    }
  }

  // Возвращаем результат проверки.
  // Если res остается равным 1, значит все элементы равны 0 и число действительно нулевое.
  return res;
}

//


s21_big_decimal convert_dec_to_big(s21_decimal val) {
  // @param val: Значение типа s21_decimal, которое нужно преобразовать в s21_big_decimal.

  s21_big_decimal res; // Переменная для хранения результата преобразования.
  memset(&res, 0, 28); // Инициализируем все биты переменной res нулями.

  // Копируем первые 3 элемента массива bits из s21_decimal в s21_big_decimal.
  for (int i = 0; i < 3; ++i) {
    res.bits[i] = val.bits[i];
  }

  // Копируем 4-й элемент массива bits из s21_decimal в последний элемент массива bits в s21_big_decimal.
  res.bits[6] = val.bits[3];

  // Возвращаем преобразованное значение типа s21_big_decimal.
  return res;
}

