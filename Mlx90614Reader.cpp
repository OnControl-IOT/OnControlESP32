#include "Mlx90614Reader.h"

Mlx90614Reader::Mlx90614Reader()
  : mlx() {}

void Mlx90614Reader::begin() {
  // No se hace Wire.begin() porque ya se inicio en el Max30102Reader
  // Solo nos aseguramos (por si acaso) de mantener 100 kHz:
  Wire.setClock(100000);

  if (!mlx.begin()) {  // verificar si las coneccioens estan bien
    Serial.println(F("Error al iniciar el MLX90614. Revisa las conexiones!"));
    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("MLX90614 inicializado correctamente "));
}

void Mlx90614Reader::printOnce() {
  double tempAmbiente = mlx.readAmbientTempC();
  double tempObjeto   = mlx.readObjectTempC();

  Serial.print("Temperatura ambiente: ");
  Serial.print(tempAmbiente);
  Serial.print(" °C   |   Objeto: ");
  Serial.print(tempObjeto);
  Serial.println(" °C");
}
