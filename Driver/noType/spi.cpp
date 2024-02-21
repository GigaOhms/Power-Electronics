#include<Arduino.h>

//SPI speed : SCK MHz(real) ,Khz (simulation)
//Slow_01: f = 20Khz => T = 0.00002s = 50us
//Slow_02: d = 1Khz => T= 0.001 = 1000us
#define Tfull 50
#define Thalef Tfull/2

//Setup pin SPI
#define SCK_PIN 4
#define MOSI_PIN 5
#define MISO_PIN 6
#define SS_PIN 7
//Thay vì sử dụng hàm pinMode ta sử dụng tác động trực tiếp vào thanh ghi
#define SCK_OUTPUT  DDRD |= (1 << DDD4)  
#define MOSI_OUTPUT DDRD |= (1 << DDD5)
#define MISO_INPUT  DDRD &= ~(1 << DDD6)
#define SS_OUTPUT   DDRD |= (1 << DDD7)

// SS luôn ở mức cao khi hoạt động cho mức thấp
// #define MOSI_HIGH  PORTD |= (1 << PD5)  //digitalWrite(MOSI_PIN,HIGH)
// #define MOSI_LOW   PORT0 &= ~(1 << PD5)  // 
//Sử dụng toán tử ba ngôi (hàm tắt) khi truyền giá trị x mức cao và mức thấp thây hàm digitalWrite
//IF value of x == true , PORTD |= (1 << PD5) and ~
#define write_CLK(x)   PORTD = ((x) ? (PORTD | 1 << PD4) : (PORTD & (~(1 << PD4))))

//true = #0 HIGH , false = 0 LOW
#define write_MOSI(x)  PORTD = ((x) ? (PORTD | 1 << PD5) : (PORTD & (~(1 << PD5))))
#define write_SS(x)    PORTD = ((x) ? (PORTD | 1 << PD7) : (PORTD & (~(1 << PD7))))
#define read_MISO()    ((PIND & (1 << PIND6)) ? HIGH : LOW)  //digitalRead(MISO_PIN)
  
void setup()
{
      Serial.begin(9600);
      SPI_setup();
}

void loop()
{         
      uint8_t  rev;
      SPI_begin();
      rev = SPI_transfor('a'); //0x61 97
      SPI_end();
      Serial.println(rev);
      Serial.println("M: " + String((char)rev));
      delay(1000);
}
//Cấu hình của SPI ban đầu
void SPI_setup()
{
    MOSI_OUTPUT;
    MISO_INPUT;
    write_CLK(LOW);
    SCK_OUTPUT;
    write_SS(HIGH);
    SS_OUTPUT;
    delay(1);

} 
void SPI_begin(void)
{
  write_SS(LOW);  //change when using mode : 1 maste - many slave
}
//Đưa về trạng thái lý tưởng SCK =  LOW và SS = HIGH 
void SPI_end(void)
{
      write_CLK(LOW);
      write_SS(HIGH); 
}
// 1 lần SCK có thể gửi 1 byte = 8bit và nhận được cùng 1 lúc
// phát hiện SCK cạnh lên đi xuống xuất data
// using mode 0 : CPOL = 0, CPHASE = 0. bit Oder = MSB truyền đi trước
//byte_out 0xAA(1010.1010) 
//step(7) lấy giá trị cao nhất: byte_out & 0x80 =1010.01010 & 1000.0000 = 1000.0000
//Khi and với 0x80 ta sẽ giữ lại giá trị cao nhất (MSB) còn lại đưa về 
//nếu truyền 0 thì write_MOSI(HIGH) và ngược lại
//Step 2(6): lấy giá trị byte_out and với giá trị nào đó để giữ được giá trị thứ (6) còn lại đưa về 0
//Ta lấy byte_out = 0x40 & 1010.1010 = 0100.0000 = 0100.0000 ->Write_MOSI(HIGH)


//.....

//1000.0000(0x80) -> 0100.0000 .... -> 0000.0001 -> 0000.00000 (stop)
//byte_out & init  = ra 2 value (#0) or (=0)
uint8_t SPI_transfor(uint8_t byte_out)
{
    uint8_t byte_in = 0;   // 0000.0000
    uint8_t ibit,result;
    //chạy 8 chu kì SCK
      for ( ibit = 0x80; ibit > 0; ibit = ibit >> 1)  //ibit = 1000.0000
      {
        /* code */
      result = byte_out & ibit; // (#0 = true) or (0 = false)
      write_MOSI(result);

      delayMicroseconds(Thalef);
      write_CLK(HIGH);
        //setbit cao nhat hoac thap nhat dau byte dau tien
      if(read_MISO()  == HIGH)
        byte_in = byte_in | ibit; // 0000.0000 or 1000.0000 = 1000.0000   
      
      
      // else
      // byte_in = byte_in & (~ibit);   // 0 can thiet

      delayMicroseconds(Thalef);
      write_CLK(LOW); // end 1 clock cycle

      }

    return byte_in;
}