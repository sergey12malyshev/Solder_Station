#include <Arduino.h>

#include <PID_v1.h>

// Этот массив содержит сегменты, которые необходимо зажечь для отображения на индикаторе цифр 0-9 
uint8_t const digits[] = {
  B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01101111
};

int16_t digit_common_pins[] = {A3, A4, A5}; // Общие выводы для тройного 7-сегментного светодиодного индикатора
int16_t max_digits = 3;
int16_t current_digit = max_digits - 1;

uint32_t updaterate = 500; // Изменяет, как часто обновляется индикатор. Не ниже 500
uint32_t lastupdate;

int16_t temperature = 0;

// Определяет переменные, к которым мы подключаемся
double Setpoint, Input, Output;


// Определяет агрессивные и консервативные параметры настройки
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;

// Задать ссылки и начальные параметры настройки
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

void show(int16_t value); 

void setup()
{
  DDRD = B11111111;  // установить выводы Arduino с 0 по 7 как выходы
  for (int16_t y = 0; y < max_digits; y++)
  {
    pinMode(digit_common_pins[y], OUTPUT);
  }
  // Мы не хотим разогревать паяльник на 100%, т.к. это может сжечь его, поэтому устанавливаем максимум на 85% (220/255)
  myPID.SetOutputLimits(0, 220);
  myPID.SetMode(AUTOMATIC);
  lastupdate = millis();
  Setpoint = 0;
}


void loop() 
{
  // Прочитать температуру
  Input = analogRead(0);
  // Преобразовать 10-битное число в градусы Цельсия
  Input = map(Input, 0, 303, 25, 350);
  // Отобразить температуру
  if (millis() - lastupdate > updaterate) 
  {
    lastupdate = millis();
    temperature = Input;
  }
  // Прочитать установленное значение и преобразовать его в градусы Цельсия (минимум 150, максимум 350)
  double newSetpoint = analogRead(1);
  newSetpoint = map(newSetpoint, 0, 1023, 150, 350);
  // Отобразить установленное значение
  if (abs(newSetpoint - Setpoint) > 3) 
  {
    Setpoint = newSetpoint;
    temperature = newSetpoint;
    lastupdate = millis();
  }

  double gap = abs(Setpoint - Input); // Расстояние от установленного значения

  if (gap < 10.0)
  { // мы близко к установленному значению, используем консервативные параметры настройки
    myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
    // мы далеко от установленного значения, используем агрессивные параметры настройки
    myPID.SetTunings(aggKp, aggKi, aggKd);
  }

  myPID.Compute();
  // Управлять выходом
  analogWrite(11, Output);
  // Отобразить температуру
  show(temperature);
}

void show(int16_t value) 
{
  int16_t digits_array[] = {};
  bool empty_most_significant = true;
  for (int16_t z = max_digits - 1; z >= 0; z--) // Цикл по всем цифрам
  {
    digits_array[z] = value / pow(10, z); // Теперь берем каждую цифру из числа
    
    if (digits_array[z] != 0 ) 
        empty_most_significant = false; // Не отображать впереди стоящие нули
    
    value = value - digits_array[z] * pow(10, z);
    if (z == current_digit)
    {
      if (!empty_most_significant || z == 0) // Проверить, что это у нас не ведущий ноль, и отобразить текущую цифру
      { 
        PORTD = ~digits[digits_array[z]]; // Удалить ~ для общего катода
      }
      else
      {
        PORTD = B11111111;
      }
      digitalWrite(digit_common_pins[z], HIGH);// Изменить на LOW для общего катода
    } 
    else 
    {
      digitalWrite(digit_common_pins[z], LOW); // Изменить на HIGH для общего катода
    }

  }
  current_digit--;
  if (current_digit < 0)
  {
    current_digit = max_digits; // Начать сначала
  }
}