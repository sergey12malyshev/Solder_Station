/*
* Доработанный проект паяльной станции из хлама
* https://github.com/sergey12malyshev/Solder_Station
* Origin: https://www.allaboutcircuits.com/projects/do-it-yourself-soldering-station-with-an-atmega8/
* Version: 1.1.0
*/
#include <Arduino.h>
#include <PID_v1.h>

#define COMMON_CATHODE false // Установить в true для общего катода

enum Modes {WORK, NO_SOLDER};
Modes mode = WORK;

static int16_t digit_common_pins[] = {A3, A4, A5}; // Общие выводы для тройного 7-сегментного светодиодного индикатора
static int16_t max_digits = 3;
static int16_t current_digit = max_digits - 1;

static uint32_t updaterate = 500; // Изменяет, как часто обновляется индикатор. Не ниже 500
static uint32_t lastupdate;

static int16_t temperature = 0;

/* Определяет агрессивные и консервативные параметры настройки ПИД-регулятора */ 
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;

double Setpoint, Input, Output;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT); // Задать ссылки и начальные параметры настройки

static void show(int16_t value, Modes workMode); 

static void convertThermocoupleData(void)
{
  const uint16_t measured = 303; // Измеренное значение при калибровке (Уже подстроил под свою конструкцию)
  Input = analogRead(0);  // Прочитать температуру
  Input = map(Input, 0, measured, 25, 350); // Преобразовать 10-битное число в градусы Цельсия
}

static double convertReostatData(void)
{  
  double newSetpoint = analogRead(1); // Прочитать установленное значение потенциометром
  newSetpoint = map(newSetpoint, 0, 1023, 150, 350); // и преобразовать его в градусы Цельсия (минимум 150, максимум 350)
  return newSetpoint;
}

static inline void heratbeatLed(void)
{
  PORTB ^=(1UL << PINB5);
}

static inline void disablePWM(void)
{
  analogWrite(11, 0);
}

static void regulator(void)
{
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
  analogWrite(11, Output); // Уcтановить PWM
}

int main(void)
{
  init();
 //initVariant();

  DDRD = B11111111;  // Установить выводы ATMEGA порта D с 0 по 7 как выходы
  DDRB |= (1 << PINB5); // Установим pb5 на выход

  for (int16_t y = 0; y < max_digits; y++)
  {
    pinMode(digit_common_pins[y], OUTPUT);
  }

  myPID.SetOutputLimits(0, 220); // Устанавливаем ограничение мощности максимум на 85% (220/255)
  myPID.SetMode(AUTOMATIC);
  lastupdate = millis();
  Setpoint = 0;

  while(true) 
  {
    convertThermocoupleData();
     
    if (millis() - lastupdate > updaterate)  /* Обновить температуру жала */
    {
      lastupdate = millis();
      temperature = Input; 
      heratbeatLed();
    }

    double newSetpoint = convertReostatData();
    double change  = abs(newSetpoint - Setpoint);

    if (change > 3)   /* Отобразить установленное значение, если было изменение */ 
    {
      Setpoint = newSetpoint;
      temperature = newSetpoint;
      lastupdate = millis();
    }

    /* Автомат состояний */ 
    if (temperature <= 380)
    {
      regulator();
      show(temperature, WORK);
    }
    else
    {
      show(temperature, NO_SOLDER); // Отобразим аварию (перегрев или отсутсвие паяльника)
      disablePWM();
    }

    if (serialEventRun) serialEventRun(); // is loop
  }
}

static void show(int16_t value, Modes workMode) 
{
  uint8_t const digits[] = {
    B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01101111,\
    B01000000 /*- no soldering-iron symbol*/
  };
  const uint8_t noSolderingIronSymbol = 10u;// ---
  int16_t digits_array[] = {0};
  bool empty_most_significant = true;

  for (int16_t z = max_digits - 1; z >= 0; z--) // Цикл по всем цифрам
  {
    if (workMode == WORK)
    {
      digits_array[z] = value / pow(10, z); // Теперь берем каждую цифру из числа
    }
    else if(workMode == NO_SOLDER)
    {
     digits_array[z] = noSolderingIronSymbol; 
    }
    
    if (digits_array[z] != 0 ) 
        empty_most_significant = false; // Не отображать впереди стоящие нули
    
    value = value - digits_array[z] * pow(10, z);
    if (z == current_digit)
    {
      if (!empty_most_significant || z == 0) // Проверить, что это у нас не ведущий ноль, и отобразить текущую цифру
      { 
#if (!COMMON_CATHODE)
        PORTD = ~digits[digits_array[z]];
#endif
      }
      else
      {
        PORTD = B11111111;
      }
#if COMMON_CATHODE
     digitalWrite(digit_common_pins[z], LOW);
#else
     digitalWrite(digit_common_pins[z], HIGH);
#endif
    } 
    else 
    {
#if COMMON_CATHODE
      digitalWrite(digit_common_pins[z], HIGH);
#else
      digitalWrite(digit_common_pins[z], LOW);
#endif
    }

  }
  current_digit--;
  if (current_digit < 0)
  {
    current_digit = max_digits;
  }
}