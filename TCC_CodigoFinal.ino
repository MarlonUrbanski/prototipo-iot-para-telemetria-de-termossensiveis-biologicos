/******************** Bibliotecas utilizadas *********************/

#include <SPI.h>                // Comunicação com dispositivos SPI
#include <Wire.h>               // Comunicação I2C
#include <OneWire.h>            // Protocolo OneWire
#include <DallasTemperature.h>  // Sensor de temperatura DS18B20
#include <LiquidCrystal_I2C.h>  // Display LCD com adaptador I2C PCF8574T
#include <I2C_RTC.h>            // Módulo Real Clock Time (RTC) PCF8563
#include <SD.h>                 // Módulo de Cartão SD
#include <TinyGPS++.h>          // Módulo GPS GY-NEO6MV2
#include <TimeLib.h>            // Biblioteca de manipulação de tempo

/******************** Definição de pinos ********************/

#define PINO_SENSOR_TEMPERATURA 5     // Pino de dados do sensor de temperatura DS18B20
#define PINO_BOTAO_CONFIRMAR 6        // Pino do botão confirmar
#define PINO_BOTAO_DIMINUIR 7         // Pino do botão diminuir
#define PINO_BOTAO_AUMENTAR 8         // Pino do botão aumentar
#define PINO_BOTAO_MENU 9            // Pino do botão menu
#define PINO_BUZZER 10                // Pino para acionamento do buzzer para aviso sonoro
#define PINO_MODULO_SD 53 
#define PINO_SENSOR_BATERIA A0        // Pino de dados do sensor de tensão da bateria
#define PINO_SENSOR_PAINEL_SOLAR A1   // Pino de dados do sensor de tensão do painel solar

/******************** Inicialização de objetos ********************/

OneWire oneWire(PINO_SENSOR_TEMPERATURA);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);   /* Endereço, número de colunas e linhas do display */
static PCF8563 RTC;
File Datalog;
TinyGPSPlus gps;

/******************** Variáveis globais ********************/

float LEITURA_SENSOR_BATERIA = 0;       /* Variável que armazena leitura do sensor de bateria */
float LEITURA_SENSOR_PAINEL_SOLAR = 0;  /* Variável que armazena leitura do sensor do painel solar */
float LEITURA_SENSOR_TEMPERATURA = 0;   /* Variável que armazena leitura do sensor de temperatura */
int CONDICAO_BUZZER = 0;                /* Variável de condição do buzzer */
int VALOR_TEMPERATURA_MINIMA = 0;       /* Variável que armazena valor de temperatura mínima configurada*/
int VALOR_TEMPERATURA_MAXIMA = 0;       /* Variável que armazena valor de temperatura máxima configurada*/
int CONDICAO_TELA_MENU = 0;             /* Variável de controle das telas de menu */
int CONDICAO_ARQUIVO = 0;               /* Variável de controle da criação do arquivo */
bool CONDICAO_TELA_MENU_2 = false;      /* Segunda variável de controle das telas de menu */
bool ESTADO_BOTAO_MENU = false;         /* Variável de condição do botão menu */
bool VALOR_MIN_MAX_TEMPERATURA = false; /* Variável de controle para iniciar configuração de máximo e mínimo*/
bool AJUSTE_TIME_ZONE = false;          /* Variável para controle do fuso horário */
bool STATUS_GPS = false;                /* Variável para monitoramento do funcionamento do serviço de GPS */
bool ALERTA_1_TEMPERATURA = false;      /* Variável de condição de alerta inicial da temperatura */
bool ALERTA_2_TEMPERATURA = false;      /* Variável de condição de alerta critico da temperatura */
bool ALERTA_1_BATERIA = false;          /* Variável de condição de alerta inicial da bateria */
bool ALERTA_2_BATERIA = false;          /* Variável de condição de alerta critico da bateria */
bool SINCRONIZA_RTC_GPS = false;        /* Variável de condição para sincronismo da data e hora do móduto RTC com do dados de GPS */
unsigned long TEMPO_ATUAL = 0;          /* Variável herdará o tempo atual de funcionamento do Arduino */
unsigned long ULTIMA_EXECUCAO = 0;      /* Variável para armazenar o tempo da última execução */
const long INTERVALO_TEMPO = 1000;      /* Intervalo de tempo para sincronismo de dados */
const int UTC_UTC_OFFSET = -3;          /* Ajuste de fuso horário para Brasília */          
char NOME_ARQUIVO_LOG[15];              /* Declara variável char com 15 posições */

/******************** Icones personalizados ********************/

byte ICONE_TEMPERATURA[8] = {B00100,B01010,B01010,B01110,B01110,B11111,B11111,B01110};
byte ICONE_AVISO_ERRO[8] = {B11111,B11011,B11011,B11011,B11011,B11111,B11011,B11111};
byte ICONE_PAINEL_SOLAR[8] = {B00100,B10101,B01110,B11011,B01110,B10101,B00100,B00000};
byte ICONE_BATERIA_01_20[8] = {B01110,B11011,B10001,B10001,B10001,B10001,B11011,B11111};
byte ICONE_BATERIA_21_40[8] = {B01110,B11011,B10001,B10001,B10001,B11111,B11111,B11111};
byte ICONE_BATERIA_41_60[8] = {B01110,B11011,B10001,B10001,B11111,B11111,B11111,B11111};
byte ICONE_BATERIA_61_80[8] = {B01110,B11011,B10001,B11111,B11111,B11111,B11111,B11111};
byte ICONE_BATERIA_81_100[8] = {B01110,B11111,B11111,B11111,B11111,B11111,B11111,B11111};

/******************** FUNÇÕES ********************/

/********** Funções que imprimem no LCD **********/

void Erro_Modulo_SD() {
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print(" AVISO: Cartao SD ");
  lcd.write(byte(1));
  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  lcd.print(" Insira um cartao ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 2);
  lcd.write(byte(1));      
  lcd.print(" SD e reinicie    ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 3);
  lcd.write(byte(1));      
  lcd.print(" o dispositivo.   ");
  lcd.write(byte(1));
}

void Erro_Sensor_Temperatura() {
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print("AVISO: Temperatura");
  lcd.write(byte(1));
  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  lcd.print(" Conecte o sensor ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 2);
  lcd.write(byte(1));      
  lcd.print(" e reinicie o     ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 3);
  lcd.write(byte(1));      
  lcd.print(" dispositivo.     ");
  lcd.write(byte(1));
}

void Erro_Modulo_GPS() {
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print("AVISO: ERRO COM GPS");
  lcd.write(byte(1));
  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  lcd.print(" Modulo com falha ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 2);
  lcd.write(byte(1));      
  lcd.print("Teste reiniciar o ");
  lcd.write(byte(1));      
  lcd.setCursor(0, 3);
  lcd.write(byte(1));      
  lcd.print("   dispositivo    ");
  lcd.write(byte(1));
}

void IconesLCD() {
  lcd.createChar(0, ICONE_TEMPERATURA);
  lcd.createChar(1, ICONE_AVISO_ERRO);
  lcd.createChar(2, ICONE_PAINEL_SOLAR);
  lcd.createChar(3, ICONE_BATERIA_01_20);
  lcd.createChar(4, ICONE_BATERIA_21_40);
  lcd.createChar(5, ICONE_BATERIA_41_60);
  lcd.createChar(6, ICONE_BATERIA_61_80);
  lcd.createChar(7, ICONE_BATERIA_81_100);
}

void Imprime_LCD_Horario() {
  char HORA[6];                    /* Buffer para armazenar a string formatada */
  sprintf(HORA, "%02d:%02d", RTC.getHours(), RTC.getMinutes());   /* Formatar a data */
  lcd.print(HORA);
}

void Imprime_LCD_Data() {
  char DATA[9];                     /* Buffer para armazenar a string formatada */
  int ano = RTC.getYear() % 100;    /* Armazena os dois últimos dígitos do ano  */
  sprintf(DATA, "%02d/%02d/%02d", RTC.getDay(), RTC.getMonth(), ano);  /* Formatar a data */
  lcd.print(DATA);
}

void Imprime_LCD_Sensor_Bateria() {
  if (LEITURA_SENSOR_BATERIA > 4.2) {
    lcd.print("100%");
    lcd.write(byte(7)); /* Ícone de bateria cheia */
  } else if (LEITURA_SENSOR_BATERIA > 4.1) {
    lcd.print(" 90%");
    lcd.write(byte(7));
  } else if (LEITURA_SENSOR_BATERIA > 4.0) {
    lcd.print(" 80%");
    lcd.write(byte(6));  /* Ícone de bateria quase cheia */
  } else if (LEITURA_SENSOR_BATERIA > 3.8) {
    lcd.print(" 70%");
    lcd.write(byte(6));
  } else if (LEITURA_SENSOR_BATERIA > 3.7) {
    lcd.print(" 60%");
    lcd.write(byte(5)); /* Ícone de bateria média */
  } else if (LEITURA_SENSOR_BATERIA > 3.6) {
    lcd.print(" 50%");
    lcd.write(byte(5));
  } else if (LEITURA_SENSOR_BATERIA > 3.5) {
    lcd.print(" 40%");
    lcd.write(byte(4));  /* Ícone de bateria quase vazia */
  } else if (LEITURA_SENSOR_BATERIA > 3.4) {
    lcd.print(" 30%");
    lcd.write(byte(4));
  } else if (LEITURA_SENSOR_BATERIA > 3.3) {
    lcd.print(" 20%");
    lcd.write(byte(3)); /* Ícone de bateria quase esgotada */
  } else if (LEITURA_SENSOR_BATERIA > 3.2) {
    lcd.print(" 10%");
    lcd.write(byte(3));
  } else if (LEITURA_SENSOR_BATERIA > 3.1) {
    lcd.print("  5%");
    lcd.write(byte(3));
  } else if (LEITURA_SENSOR_BATERIA > 3.0) {
    lcd.print("  1%_");
  } else {
    lcd.print("  0% "); /* Bateria não identificada */
  }
}

void Imprime_LCD_Sensor_Temperatura() {
  lcd.write(byte(0));
  lcd.print(LEITURA_SENSOR_TEMPERATURA,1);
  lcd.print((char)223);
  lcd.print("C ");
}

void Imprime_LCD_Sensor_Painel_Solar() {
  lcd.write(byte(2));
  lcd.print((LEITURA_SENSOR_PAINEL_SOLAR > 5) ? 99 : LEITURA_SENSOR_PAINEL_SOLAR * 20, 0);
  lcd.print("%");
}

void Imprime_LCD_Status_GPS() {
  lcd.print("GPS:");
  if (STATUS_GPS == true){
    lcd.print("OK");  
  } else {
    lcd.print(" ?");  
  }
}

void Imprime_LCD_Valor_Temperatura_Minima() {
  lcd.write(byte(0));
  lcd.print("Min:");
  lcd.print(VALOR_TEMPERATURA_MINIMA);
  lcd.print((char)223);
  lcd.print("C ");
}

void Imprime_LCD_Valor_Temperatura_Maxima() {
  lcd.write(byte(0));
  lcd.print("Max:");
  lcd.print(VALOR_TEMPERATURA_MAXIMA);
  lcd.print((char)223);
  lcd.print("C");
}

void Configura_Valores_Temperatura() {
    lcd.clear();
    noTone(PINO_BUZZER);
    int CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP = 0;
      while (CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP == 0) { 
        lcd.setCursor(0, 0);
        lcd.print("# PARA CONFIGURAR  #");
        lcd.setCursor(0, 1);
        lcd.print("# A TEMPERATURA    #");
        lcd.setCursor(0, 2);
        lcd.print("# MIN E MAX CLIQUE #");
        lcd.setCursor(0, 3);
        lcd.print("# EM CONFIRMAR (>) #");
        if (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW) {
          CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP = 1;
          while (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW) { }
        }
      }

      lcd.clear();
      while (CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP == 1) { 
        lcd.setCursor(0, 0);
        lcd.print(" TEMPERATURA MINIMA ");
        lcd.setCursor(0, 1);
        lcd.print("CLIQUE EM (-) OU (+)");
        lcd.setCursor(6, 2);
        Imprime_LCD_Valor_Temperatura_Minima();
        lcd.print("  ");
        lcd.setCursor(3, 3);
        lcd.print("CONFIRMAR (>)");
        if (digitalRead(PINO_BOTAO_AUMENTAR) == LOW) {
          while (digitalRead(PINO_BOTAO_AUMENTAR) == LOW);
          if (VALOR_TEMPERATURA_MINIMA < 83) {
            VALOR_TEMPERATURA_MINIMA++;
          }
        }

        if (digitalRead(PINO_BOTAO_DIMINUIR) == LOW) {
          while (digitalRead(PINO_BOTAO_DIMINUIR) == LOW);
          if (VALOR_TEMPERATURA_MINIMA > -55) {
            VALOR_TEMPERATURA_MINIMA--;
          }
        } 

    if (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW) {
      while (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW);
      CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP = 2;
    }
  }

  VALOR_TEMPERATURA_MAXIMA = VALOR_TEMPERATURA_MINIMA + 1;
  lcd.clear();
  while (CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP == 2) { 
    lcd.setCursor(0, 0);
    lcd.print(" TEMPERATURA MAXIMA ");
    lcd.setCursor(0, 1);
    lcd.print("CLIQUE EM (-) OU (+)");
    lcd.setCursor(6, 2);
    Imprime_LCD_Valor_Temperatura_Maxima();
    lcd.print("  ");
    lcd.setCursor(3, 3);
    lcd.print("CONFIRMAR (>)");
    if (digitalRead(PINO_BOTAO_AUMENTAR) == LOW) {
      while (digitalRead(PINO_BOTAO_AUMENTAR) == LOW);
      if (VALOR_TEMPERATURA_MAXIMA < 83) {
        VALOR_TEMPERATURA_MAXIMA++;
      } 
    } 
    /* Temperatura máxima só poderá ser maior que a temperatura mínima */
    if (digitalRead(PINO_BOTAO_DIMINUIR) == LOW) {
      while (digitalRead(PINO_BOTAO_DIMINUIR) == LOW) { }
      if (VALOR_TEMPERATURA_MAXIMA > VALOR_TEMPERATURA_MINIMA +1) {
        VALOR_TEMPERATURA_MAXIMA--;
      }
    }

    if (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW) {
      while (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW);
      CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP = 3;
    }
  }

    lcd.clear();
    while (CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP == 3) { 
      lcd.setCursor(0, 0);
      lcd.print("VALORES CONFIGURADOS");
      lcd.setCursor(0, 1);
      Imprime_LCD_Valor_Temperatura_Minima();
      Imprime_LCD_Valor_Temperatura_Maxima();
      lcd.setCursor(0, 2);
      lcd.print("     CONFIRMA?");
      lcd.setCursor(0, 3);
      lcd.print("  NAO (-)  SIM (+)");
      while (CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP == 3) {
        if (digitalRead(PINO_BOTAO_AUMENTAR) == LOW) {
          CONDICAO_TELA_CONFIGURADOR_VALORES_TEMP = 4;
          VALOR_MIN_MAX_TEMPERATURA = true;
          CONDICAO_TELA_MENU = 0;
          while (digitalRead(PINO_BOTAO_AUMENTAR) == LOW);
        } 

        if (digitalRead(PINO_BOTAO_DIMINUIR) == LOW) {
          while (digitalRead(PINO_BOTAO_DIMINUIR) == LOW);
          Configura_Valores_Temperatura();
        }  
      }  
    }
  lcd.clear();
}

void Valores_Min_Max_Temperatura() {
  if (VALOR_MIN_MAX_TEMPERATURA) {
    Imprime_LCD_Valor_Temperatura_Minima();
    lcd.setCursor(10, 2);
    Imprime_LCD_Valor_Temperatura_Maxima();
  } else {
    Configura_Valores_Temperatura();
  }
}

void Menu() {
  if (ESTADO_BOTAO_MENU == true) {
    CONDICAO_TELA_MENU++;
    if (CONDICAO_TELA_MENU > 1 ) CONDICAO_TELA_MENU = 0;
    ESTADO_BOTAO_MENU = false;
  }

 switch (CONDICAO_TELA_MENU) {
  case 0:
    if (ALERTA_1_TEMPERATURA == true || ALERTA_2_TEMPERATURA == true && ALERTA_1_BATERIA == false && ALERTA_2_BATERIA == false) {
        if (CONDICAO_TELA_MENU_2 == false) {
          lcd.setCursor(0, 3);
          lcd.write(byte(0));
          lcd.print("ALERTA TEMPERATURA");
          lcd.write(byte(0));
          CONDICAO_TELA_MENU_2 = true;
        } else {
          lcd.print("      MENU ::       ");
          CONDICAO_TELA_MENU_2 = false;
        }
    } else if (ALERTA_1_BATERIA == true || ALERTA_2_BATERIA == true && ALERTA_1_TEMPERATURA == false && ALERTA_2_TEMPERATURA == false) {
      if (CONDICAO_TELA_MENU_2 == false) {
        lcd.setCursor(0, 3);
        lcd.write(byte(3));
        lcd.write(byte(3));
        lcd.print(" ALERTA BATERIA ");
        lcd.write(byte(3));
        lcd.write(byte(3));
        CONDICAO_TELA_MENU_2 = true;
        lcd.display();
        } else {
          lcd.print("      MENU ::       ");
          CONDICAO_TELA_MENU_2 = false;
          if (ALERTA_2_BATERIA == true) {
            lcd.noDisplay();
          }
        } 
    } else {
      lcd.display();
      lcd.print("      MENU ::       ");
    }
    break;

  case 1:
    lcd.print("  TEMP MIN MAX (>)  ");
    if (digitalRead(PINO_BOTAO_MENU) == LOW) {
      VALOR_MIN_MAX_TEMPERATURA = false;
      CONDICAO_TELA_MENU = 0;
      while (digitalRead(PINO_BOTAO_MENU) == LOW);
    }
    break;
 }
}

void Imprime_LCD() {
  lcd.setCursor(0, 0);
  Imprime_LCD_Horario();
  lcd.setCursor(6, 0);
  Imprime_LCD_Data();
  lcd.setCursor(15, 0);
  Imprime_LCD_Sensor_Bateria();
  lcd.setCursor(0, 1);
  Imprime_LCD_Sensor_Temperatura();
  lcd.setCursor(8, 1);
  Imprime_LCD_Sensor_Painel_Solar();
  lcd.setCursor(13, 1);
  Imprime_LCD_Status_GPS();
  lcd.setCursor(0, 2);
  Valores_Min_Max_Temperatura();
  lcd.setCursor(0, 3);
  Menu();
}

/********** Funções que escrevem na porta serial **********/

void Escreve_Serial_Horario() {
  char HORA[9];                     /* Buffer para armazenar a string formatada */
  sprintf(HORA, "%02d:%02d:%02d", RTC.getHours(), RTC.getMinutes(), RTC.getSeconds());   /* Formatar a data */
  Serial.print(HORA);
    Serial.print(" ");
}

void Escreve_Serial_Data() {
  char DATA[9];                     /* Buffer para armazenar a string formatada */
  sprintf(DATA, "%02d/%02d/%02d", RTC.getDay(), RTC.getMonth(), RTC.getYear());  /* Formatar a data */
  Serial.print(DATA);
  Serial.print(" ");
}

void Escreve_Serial_Sensor_Temperatura() {
  Serial.print(" Temp: "); 
  Serial.print(LEITURA_SENSOR_TEMPERATURA, 1);
  Serial.print("°C ");
}

void Escreve_Serial_Temperatura_Minima_Maxima() {
  Serial.print(" Min: ");
  Serial.print(VALOR_TEMPERATURA_MINIMA);
  Serial.print("°C ");
  Serial.print(" Max: ");
  Serial.print(VALOR_TEMPERATURA_MAXIMA);
  Serial.print("°C ");
}

void Escreve_Serial_Sensor_Bateria() {
  Serial.print(" Bateria: ");
  Serial.print(LEITURA_SENSOR_BATERIA, 2);
  Serial.print("v ");
}

void Escreve_Serial_Sensor_Painel_Solar() {
  Serial.print(" Painel Solar: ");
  Serial.print(LEITURA_SENSOR_PAINEL_SOLAR, 2);
  Serial.print("v ");
}

void Escreve_Serial_GPS() {
  Serial.print(" Satelites: ");
  Serial.print(gps.satellites.value()); /* Numero de satélites */
  Serial.print(F(" Localização: "));
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6); /* Latitude */
    Serial.print(",");
    Serial.print(gps.location.lng(), 6); /* Longitude */
  } else {
    Serial.print("INVALIDO");
  }
  /* Mostra Data/Hora ajustada */
  if (gps.time.isValid() && gps.date.isValid()) {
    Serial.print(" Data/Hora GPS: ");
    if (day() < 10) Serial.print("0");
    Serial.print(day());
    Serial.print("/");
    if (month() < 10) Serial.print("0");
    Serial.print(month());
    Serial.print("/");
    Serial.print(year());
    Serial.print(" ");
    if (hour() < 10) Serial.print("0");
    Serial.print(hour());
    Serial.print(":");
    if (minute() < 10) Serial.print("0");
    Serial.print(minute());
    Serial.print(":");
    if (second() < 10) Serial.print("0");
    Serial.print(second());
  } else {
    Serial.print(" Data/Hora: INVALIDO");
  }
  Serial.print(" Velocidade Km/h: ");
  Serial.print(gps.speed.kmph()); /* Velocidade em Km/h */
  Serial.print(" Altitude Metros: ");
  Serial.print(gps.altitude.meters()); /* Altitude em metros */  
}

void Escreve_Serial_Alertas() {
  if (ALERTA_1_TEMPERATURA == true) {
    Serial.print(" Alerta 1: Temperatura");
  }

  if (ALERTA_2_TEMPERATURA == true) {
    Serial.print(" Alerta 2: Temperatura Critica");
  }

  if (ALERTA_1_BATERIA == true) {
    Serial.print(" Alerta 1: Bateria");
  }   

  if (ALERTA_2_BATERIA == true) {
    Serial.print(" Alerta 2: Bateria Critica");
  }
}

void Escreve_Serial() {
  Serial.println("");
  Escreve_Serial_Horario();
  Escreve_Serial_Data();
  Escreve_Serial_Sensor_Temperatura();
  Escreve_Serial_Temperatura_Minima_Maxima();
  Escreve_Serial_Sensor_Bateria();
  Escreve_Serial_Sensor_Painel_Solar();
  Escreve_Serial_GPS();
  Escreve_Serial_Alertas();
}

/********** Funções que amazenam dados no cartão SD **********/

void Cria_Primeira_Linha() {
  Datalog = SD.open(NOME_ARQUIVO_LOG, FILE_WRITE);  /* Abre o arquivo para escrita */
  if (Datalog) {
    char CaracterSeparadorDatalog = ';';    
    Datalog.println("");
    Datalog.print("Horario");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Data");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Temperatura (°C)");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Temperatura Mínima (°C)");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Temperatura Máxima (°C)");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Bateria");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Painel Solar");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Satelites");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Latitude");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Longitude");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Data GPS");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Hora GPS");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Velocidade (Km/h)");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Altitude (m)");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Alerta Temperatura");
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print("Alerta Bateria");
    Datalog.close();
  } else {
    Serial.println("Erro para registro dos logs no arquivo ");
    Serial.print(Datalog);
    Erro_Modulo_SD(); 
  }
}

void Armazena_Dados() {
  Datalog = SD.open(NOME_ARQUIVO_LOG, FILE_WRITE);  /* Abre o arquivo para escrita */
  if (Datalog) {
    char CaracterSeparadorDatalog = ';';    
    Datalog.println("");
    if(RTC.getHours() < 10) Datalog.print("0");
    Datalog.print(RTC.getHours());
    Datalog.print(":");
    if(RTC.getMinutes() < 10) Datalog.print("0");
    Datalog.print(RTC.getMinutes());
    Datalog.print(":");
    if(RTC.getSeconds() < 10) Datalog.print("0");
    Datalog.print(RTC.getSeconds());
    Datalog.print(CaracterSeparadorDatalog);
    if(RTC.getDay() < 10) Datalog.print("0");
    Datalog.print(RTC.getDay());
    Datalog.print("-");
    if(RTC.getMonth() < 10) Datalog.print("0");
    Datalog.print(RTC.getMonth());
    Datalog.print("-");
    Datalog.print(RTC.getYear());
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(LEITURA_SENSOR_TEMPERATURA,2);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(VALOR_TEMPERATURA_MINIMA);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(VALOR_TEMPERATURA_MAXIMA);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(LEITURA_SENSOR_BATERIA);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(LEITURA_SENSOR_PAINEL_SOLAR);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(gps.satellites.value());
    Datalog.print(CaracterSeparadorDatalog);
    if (gps.location.isValid()) {
    Datalog.print(gps.location.lat(), 6);
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(gps.location.lng(), 6); 
    } else {
      Datalog.print("INVALIDO");
      Datalog.print(CaracterSeparadorDatalog);
      Datalog.print("INVALIDO"); 
    }    
    Datalog.print(CaracterSeparadorDatalog);
    if (gps.time.isValid() && gps.date.isValid()) {
      if (day() < 10) Datalog.print("0");
      Datalog.print(day());
      Datalog.print("/");
      if (month() < 10) Datalog.print("0");
      Datalog.print(month());
      Datalog.print("/");
      Datalog.print(year());
      Datalog.print(CaracterSeparadorDatalog);
      if (hour() < 10) Datalog.print("0");
      Datalog.print(hour());
      Datalog.print(":");
      if (minute() < 10) Datalog.print("0");
      Datalog.print(minute());
      Datalog.print(":");
      if (second() < 10) Datalog.print("0");
      Datalog.print(second());
    } else {
      Datalog.print("INVALIDO");
      Datalog.print(CaracterSeparadorDatalog);
      Datalog.print("INVALIDO");
    }
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(gps.speed.kmph());
    Datalog.print(CaracterSeparadorDatalog);
    Datalog.print(gps.altitude.meters());
    Datalog.print(CaracterSeparadorDatalog);
    if (ALERTA_1_TEMPERATURA == true) Datalog.print("Alerta 1: Temperatura");
    if (ALERTA_2_TEMPERATURA == true) Datalog.print("Alerta 2: Temperatura Critica");
    Datalog.print(CaracterSeparadorDatalog);
    if (ALERTA_1_BATERIA == true) Datalog.print("Alerta 1: Bateria");
    if (ALERTA_2_BATERIA == true) Datalog.print("Alerta 2: Bateria Critica");
    Datalog.close();
  } else {
    Serial.println("Erro para registro dos logs no arquivo ");
    Serial.print(Datalog);
    Erro_Modulo_SD(); 
  }
}

void Deleta_Arquivo_Antigo() {
  char ARQUIVO_ANTIGO[15];
  int ANO_ANTERIOR = RTC.getYear() - 1;
  String NOME_ARQUIVO_ANTIGO = String(ANO_ANTERIOR);
  if(RTC.getMonth() < 10) NOME_ARQUIVO_ANTIGO += "0"; 
  NOME_ARQUIVO_ANTIGO += RTC.getMonth();
  if(RTC.getDay() < 10) NOME_ARQUIVO_ANTIGO += "0";
  NOME_ARQUIVO_ANTIGO += RTC.getDay() + ".txt";
  NOME_ARQUIVO_ANTIGO.toCharArray(ARQUIVO_ANTIGO, NOME_ARQUIVO_ANTIGO.length() + 1);
  if(SD.exists(ARQUIVO_ANTIGO)) {
    SD.remove(ARQUIVO_ANTIGO);
    Serial.println("Arquivo deletado com sucesso: " + String(ARQUIVO_ANTIGO));
  }
}

void Cria_Nome_Arquivo_Log() {
  if (CONDICAO_ARQUIVO != RTC.getDay()) {
    CONDICAO_ARQUIVO = RTC.getDay();
    String CRIANDO_NOME_ARQUIVO_LOG = String(RTC.getYear());
    if(RTC.getMonth() < 10) CRIANDO_NOME_ARQUIVO_LOG += "0"; 
    CRIANDO_NOME_ARQUIVO_LOG += String(RTC.getMonth());
    if(RTC.getDay() < 10) CRIANDO_NOME_ARQUIVO_LOG += "0";
    CRIANDO_NOME_ARQUIVO_LOG += String(RTC.getDay()) + ".txt";
    CRIANDO_NOME_ARQUIVO_LOG.toCharArray(NOME_ARQUIVO_LOG, CRIANDO_NOME_ARQUIVO_LOG.length() + 1);
  }
}

void Armazena_Cartao_SD() {
  Cria_Nome_Arquivo_Log(); 
  if (!SD.exists(NOME_ARQUIVO_LOG)) {
    Deleta_Arquivo_Antigo();
    Cria_Primeira_Linha();
  }
  Armazena_Dados();
}

/********** Funções dos módulos e sensores **********/

/* Leitura, processamento e armazenamento dos valores dos sensores de tensão */
void Sensores_Tensao() {
  LEITURA_SENSOR_BATERIA  = analogRead(PINO_SENSOR_BATERIA)*(26/1023.00);
  LEITURA_SENSOR_PAINEL_SOLAR  = analogRead(PINO_SENSOR_PAINEL_SOLAR)*(25/1023.00);
}

void Sensor_Temperatura() {
  sensors.requestTemperatures();
  LEITURA_SENSOR_TEMPERATURA = sensors.getTempCByIndex(0);
  if (LEITURA_SENSOR_TEMPERATURA == DEVICE_DISCONNECTED_C) {
    Serial.println("\n Erro: Sensor de temperatura não conectado. \n");
    while (LEITURA_SENSOR_TEMPERATURA == DEVICE_DISCONNECTED_C) {
      Erro_Sensor_Temperatura();
      LEITURA_SENSOR_TEMPERATURA = sensors.getTempCByIndex(0);
    }
    lcd.clear();
  }
}

void Inicializa_LCD() {
  lcd.init(); /* Inicia O LCD e o painel luminoso */
  lcd.backlight();
  lcd.clear();
  IconesLCD();
}

/* 1999 é o valor padrão usado pela biblioteca até que o GPS obtenha valores reais */
void Status_GPS() {
  if (STATUS_GPS == false) { 
    if (year() != 1999 && year() > 1999 && gps.location.isValid() == true) {
      STATUS_GPS = true;
    }
  }
}

void Ajusta_Fuso_Horario_GPS() {
  int Ano = gps.date.year();
  byte Mes = gps.date.month();
  byte Dia = gps.date.day();
  byte Hora = gps.time.hour();
  byte Minuto = gps.time.minute();
  byte Segundo = gps.time.second();
  setTime(Hora, Minuto, Segundo, Dia, Mes, Ano);
  adjustTime(UTC_UTC_OFFSET * SECS_PER_HOUR);   
}

/* Conexao com modulo GPS */
void GPS() {
  while (Serial1.available() > 0) {
  gps.encode(Serial1.read());
  }

  /* Analisa funcionamento do módulo GPS */
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Erro: GPS não detectado.");
    Erro_Modulo_GPS();
    while (true);
  }

  /* Verifica se já ajustou o tempo para o fuso horário de Brasília */
  if (!AJUSTE_TIME_ZONE && gps.time.isValid() && gps.date.isValid()) {
    Ajusta_Fuso_Horario_GPS();
    AJUSTE_TIME_ZONE = true;
  }
  Status_GPS();
}

void Condicoes () {
  if (digitalRead(PINO_BOTAO_MENU) == LOW && VALOR_MIN_MAX_TEMPERATURA == true) {
    ESTADO_BOTAO_MENU = true;
  }
  
  if (digitalRead(PINO_BOTAO_CONFIRMAR) == LOW && CONDICAO_TELA_MENU == 1) {
    VALOR_MIN_MAX_TEMPERATURA = false;
  }
  
  if (LEITURA_SENSOR_TEMPERATURA <= VALOR_TEMPERATURA_MINIMA + 1 && LEITURA_SENSOR_TEMPERATURA > VALOR_TEMPERATURA_MINIMA || LEITURA_SENSOR_TEMPERATURA >= VALOR_TEMPERATURA_MAXIMA - 1 && LEITURA_SENSOR_TEMPERATURA < VALOR_TEMPERATURA_MAXIMA) {
    ALERTA_1_TEMPERATURA = true;
    ALERTA_2_TEMPERATURA = false;
  } else if (LEITURA_SENSOR_TEMPERATURA <= VALOR_TEMPERATURA_MINIMA || LEITURA_SENSOR_TEMPERATURA >= VALOR_TEMPERATURA_MAXIMA) {
    ALERTA_1_TEMPERATURA = false;
    ALERTA_2_TEMPERATURA = true;
  } else {
    ALERTA_1_TEMPERATURA = false;
    ALERTA_2_TEMPERATURA = false;
  }

  if (LEITURA_SENSOR_BATERIA <= 3.2 && LEITURA_SENSOR_BATERIA > 3.1) {
    ALERTA_1_BATERIA = true;
    ALERTA_2_BATERIA = false;
  } else if (LEITURA_SENSOR_BATERIA <= 3.1) {
    ALERTA_1_BATERIA = false;
    ALERTA_2_BATERIA = true;
  } else {
    ALERTA_1_BATERIA = false;
    ALERTA_2_BATERIA = false;
  }

	if (SINCRONIZA_RTC_GPS == false) {
    if (STATUS_GPS == true) {
      SINCRONIZA_RTC_GPS = true;
      if (gps.time.isValid() && gps.date.isValid()) {
        int ano = year() % 100;
        RTC.setDate(day(), month(), ano);   
	      RTC.setTime(hour(), minute(), second());   
      }
    }
  }
}

void Alertas() {
  if (TEMPO_ATUAL - ULTIMA_EXECUCAO >= INTERVALO_TEMPO ) {
    if (ALERTA_1_TEMPERATURA == true || ALERTA_1_BATERIA == true && ALERTA_2_TEMPERATURA == false && ALERTA_2_BATERIA == false) {
      switch (CONDICAO_BUZZER) {
        case 1:
          tone(PINO_BUZZER, 261);
          CONDICAO_BUZZER++;
        break;
        case 2:
          tone(PINO_BUZZER, 293);
          CONDICAO_BUZZER++;
        break;
        case 3:
          tone(PINO_BUZZER, 329);
          CONDICAO_BUZZER++;
        break;
        case 4:
          tone(PINO_BUZZER, 349);
          CONDICAO_BUZZER++;
        break;
        case 5:
          tone(PINO_BUZZER, 392);
          CONDICAO_BUZZER++;
        break;                       
        default:
          noTone(PINO_BUZZER);
          CONDICAO_BUZZER++;
          if (CONDICAO_BUZZER == 8) CONDICAO_BUZZER = 0;
        break;   
      }
    } else if (ALERTA_2_TEMPERATURA == true || ALERTA_2_BATERIA == true ) {
      if (CONDICAO_BUZZER != 0) {
        tone(PINO_BUZZER, 1000);
        CONDICAO_BUZZER = 0;   
      } else {
        tone(PINO_BUZZER, 1800);
        CONDICAO_BUZZER = 1;
      }
    } else {
      noTone(PINO_BUZZER);
    }
  }
}

void Inicializa_Portas() {
  pinMode(PINO_BOTAO_CONFIRMAR, INPUT_PULLUP);
  pinMode(PINO_BOTAO_AUMENTAR, INPUT_PULLUP);
  pinMode(PINO_BOTAO_DIMINUIR, INPUT_PULLUP);
  pinMode(PINO_BOTAO_MENU, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  /* Uso da Serial 1 (pinos 18 e 19) */
  Inicializa_LCD();
  RTC.begin();  /* Inicia Módulo Real Clock Time (RTC) PCF8563 */
  if (!SD.begin(PINO_MODULO_SD)) Erro_Modulo_SD();  /* Inicia e verifica o Módulo de Cartão SD */
  /* Inicia o sensor de temperatura */
  while (digitalRead(PINO_SENSOR_TEMPERATURA) == LOW) {
    Serial.println("Falha na inicialização do sensor de temperatura.");
    while (digitalRead(PINO_SENSOR_TEMPERATURA) == LOW) Erro_Sensor_Temperatura();
    lcd.clear();
  }	
  sensors.begin();  /* Inicia sensor de temperatura DS18B20 */
  Inicializa_Portas();
  Configura_Valores_Temperatura();
}

void loop() {
  TEMPO_ATUAL = millis();    /* Obtém e armazena o tempo atual */
  Sensores_Tensao();
  Sensor_Temperatura();
  GPS();
  Condicoes();
  Alertas();
 
  if (TEMPO_ATUAL - ULTIMA_EXECUCAO >= INTERVALO_TEMPO) {  
    ULTIMA_EXECUCAO = TEMPO_ATUAL;
    Imprime_LCD();
    Escreve_Serial();
    Armazena_Cartao_SD();
  }
}
