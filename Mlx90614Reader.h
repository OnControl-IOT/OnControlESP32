#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

class Mlx90614Reader {
public:
  Mlx90614Reader();

  void begin();       // inicializa sensor en el mismo Wire de Max30102Reader
  void printOnce();   // lee e imprime una vez

private:
  Adafruit_MLX90614 mlx;
};