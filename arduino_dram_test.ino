#include <MsTimer2.h>


//pin define
#define ADRESS0 5
#define ADRESS1 6
#define ADRESS2 7
#define ADRESS3 8
#define ADRESS4 9
#define ADRESS5 10
#define ADRESS6 11
#define ADRESS7 12
#define ADRESS8 13

#define DIN 15  //A1
#define RW 16   //A2
#define RAS 17  //A3
#define DOUT 18 //A4
#define CAS 19  //A5


//Dram Refresh function
void __refresh() {
 interrupts();
  unsigned long count = 512;
  //Serial.println("refresh-start");
  while (count) {
    //    Serial.println("Refresh");
    //digitalWrite(CAS, LOW);
    PORTC &= ~_BV(5);
    //digitalWrite(RAS, LOW);
    PORTC &= ~_BV(3);
    //digitalWrite(CAS, HIGH);
    PORTC |= _BV(5);
    //digitalWrite(RAS, HIGH);
    PORTC |= _BV(3);
    count -- ;
  }
  //Serial.println("refresh-end");
}

void setup() {
  // put your setup code here, to run once:

  pinMode(ADRESS0, OUTPUT);
  pinMode(ADRESS1, OUTPUT);
  pinMode(ADRESS2, OUTPUT);
  pinMode(ADRESS3, OUTPUT);
  pinMode(ADRESS4, OUTPUT);
  pinMode(ADRESS5, OUTPUT);
  pinMode(ADRESS6, OUTPUT);
  pinMode(ADRESS7, OUTPUT);
  pinMode(ADRESS8, OUTPUT);

  pinMode(DOUT, INPUT);
  pinMode(DIN, OUTPUT);

  pinMode(RAS, OUTPUT);
  pinMode(CAS, OUTPUT);
  pinMode(RW, OUTPUT);

  //memory initialize
  //digitalWrite(RAS, HIGH);
  PORTC |= _BV(3);
  //digitalWrite(CAS, HIGH);
  PORTC |= _BV(5);
  //digitalWrite(RW, HIGH);
  PORTC |= _BV(2);

  //Refresh timer set
  MsTimer2::set(1, __refresh); //500ms
  MsTimer2::start();

  Serial.begin(57600);
  while (!Serial) {
    ; //wait
  }
  Serial.println("START");
  
  //memory write test
  char txt[] = "Memory Test!!\n";
  int tmp;
  Serial.println("------");
  for (unsigned int i = 0; i < strlen(txt); ++i) {
    tmp = txt[i];
    //Serial.println(tmp);
    dram_write_byte(0,0 + (8*i),tmp);
  }
  


}

void set_address(unsigned int address) {
  /*
  unsigned int tmp[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned int i = 0;
  unsigned int pin = 5;
  if (address > 0) {
    while (address > 0) {
      tmp[i] = address % 2;
      address = address /2;
      i += 1;
    }
  } else {
    tmp[i] = 0;
  }
  for (i = 0; i < 9; ++i) {
    if (tmp[i]) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    pin ++;
  }
  */
  /*
  static uint8_t tmp[] = {0,0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x10};
  for (unsigned int i=0;i<9;++i){
    if ((address & tmp[i]) != 0){
      digitalWrite(i + 5,HIGH);
    }else{
      digitalWrite(i + 5,LOW);
    }
  }
  */
  PORTB = address >>3; //8-13 上位6bit
  PORTD = (address & B00000111) <<5;     //5-7 下位3bit

}

unsigned int dram_read(unsigned int row, unsigned int col) {
  unsigned int data;
  //read
  //Serial.println("Read");
  MsTimer2::stop();

  //digitalWrite(RAS, HIGH);
  PORTC |= _BV(3);
  //digitalWrite(CAS, HIGH);
  PORTC |= _BV(5);
  //set row address
  set_address(row);
  //digitalWrite(RAS, LOW);;
  PORTC &= ~_BV(3);
  //digitalWrite(RW, HIGH);
  PORTC |= _BV(2);
  //set col address
  set_address(col);
  //digitalWrite(CAS, LOW);
  PORTC &= ~_BV(5);
  //delay(10);
  data = digitalRead(DOUT);
  //data = PINC & _BV(4);
  //digitalWrite(CAS, HIGH);
  PORTC |= _BV(5);
  //digitalWrite(RAS, HIGH);
  PORTC |= _BV(3);

  MsTimer2::start();

  return data;
}
void dram_write(unsigned int row, unsigned int col, unsigned int data) {
  //write
  //Serial.println("Write");
  MsTimer2::stop();

  //digitalWrite(RAS, HIGH);
  PORTC |= _BV(3);
  //digitalWrite(CAS, HIGH);
  PORTC |= _BV(5);
  //set row address
  set_address(row);
  //digitalWrite(RAS, LOW);;
  PORTC &= ~_BV(3);
  //digitalWrite(RW, LOW);
  PORTC &= ~_BV(2);
  //set write data
  if (data) {
    //digitalWrite(DIN, HIGH);
    PORTC |= _BV(1);
  } else {
    //digitalWrite(DIN, LOW);
    PORTC &= ~_BV(1);
  }

  //set col address
  set_address(col);
  //digitalWrite(CAS, LOW);
  PORTC &= ~_BV(5);

  //digitalWrite(RW, HIGH);
  PORTC |= _BV(2);
  //digitalWrite(CAS, HIGH);
  PORTC |= _BV(5);
  //digitalWrite(RAS, HIGH);
  PORTC |= _BV(3);

  MsTimer2::start();

}
void dram_write_byte(unsigned int row, unsigned int col, unsigned int value) {
  
  unsigned int tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  unsigned int i = 0;
  if (value > 0) {
    while (value > 0) {
      tmp[i] = value % 2;
      value = value / 2;
      i += 1;
    }
  } else {
    tmp[i] = 0;
  }
  for (i = 0; i < 8; ++i) {
    dram_write(row, col + i, tmp[7 - i]);
  }
  
  /*
  static uint8_t tmp[] = {0,0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
  for (unsigned int i=0;i<8;++i){
    dram_write(row,col + i,((value & tmp[i]) != 0));
  }
  */
}

unsigned int dram_read_byte(unsigned int row, unsigned int col) {
  unsigned int i = 0;
  unsigned int tmp = 0;
  
  for (i = 0; i < 8; ++i) {
    //Serial.println(dram_read(row, col + i));
    tmp = tmp + dram_read(row, col + i) * int(round(pow(2, (7 - i))));
  }
  return tmp;
}
void memory_test(){
  unsigned int data;
  
  Serial.println("DATA WRITE");
  for (unsigned int i = 0; i < 512; ++i) {
    for (unsigned int j = 0; j < 512; ++j) {
      dram_write(i, j, 0);
    }
  }
  
  Serial.println("CHK");
  for (unsigned int i = 0; i < 512; ++i) {
    for (unsigned int j = 0; j < 512; ++j) {
      data = dram_read(i, j);
      //Serial.println(data);

      if (data != 0) {
        Serial.print("Row=");
        Serial.println(i);
        Serial.print("COL=");
        Serial.println(j);
        Serial.print("DATA=");
        Serial.println(data);
      }

    }
  }
}
void loop() {
  
  // put your main code here, to run repeatedly:
  /*
  Serial.println("Write");
  unsigned int data;
  //Write Data
  dram_write(6, 138, 0);

  //Read Data
  data = dram_read(6, 138);
  Serial.println("ReadData");
  Serial.println(data);
  */

  char txt[] = "Memory Test!!\n";
  char txt2[15]={};
  int tmp;
  int bin;

  
  Serial.println("------");
  /*
  for(unsigned int i =0;i< strlen(txt); ++i) {
    tmp = dram_read_byte(0,0 + (8*i));
    txt2[i]=tmp;
    //Serial.println(tmp);
  }
  Serial.print(txt2);
  */
  Serial.println("-----");
    memory_test();
  delay(1000);//1s
}
