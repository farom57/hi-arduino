#define MAX_LENGTH 64


void setup() {
  Serial.begin(9600);

  Serial1.begin(9600, SERIAL_8O1);
  Serial1.setTimeout(200);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Serial.println("Started");
}

byte hex2byte(char in) {
  if (isDigit(in)) {
    return in - '0';
  } else {
    return in - 'A' + 10;
  }
}

byte hex2byte(char MSB, char LSB) {
  return (hex2byte(MSB) << 4) + hex2byte(LSB);
}

void hex2byte(char* in, byte* out, int len) {
  for (int i = 0; i * 2 < len; i++) {
    out[i] = hex2byte(in[2 * i], in[2 * i + 1]);
  }
}
int hex2int(char* in, int len) {
  int res = 0;

  for (int i = 0; i * 2 + 1 < len; i++) {
    res = (res << 8);
    res += hex2byte(in[2 * i], in[2 * i + 1]);
  }
  return res;
}

int checksum(byte* in, int len) {
  int sum = 0xFFFF;
  for (int i = 0; i < len; i++) {
    sum -= in[i];
  }
  return sum;
}

// receive data from HVAC
// data: null terminated char table to store the result
// return the number of char readed, 0 if nothing after the timeout, -1 in case of checksum error
int receive(char* data) {
  char buf[MAX_LENGTH];   // full message
  char p_buf[MAX_LENGTH]; // parameter only
  int len;                // message length
  int p_len;              // parameter length
  int i = 0;

  char terminator = 0x0D;

  len = Serial1.readBytesUntil(terminator, buf, 64);


  char* buf2;             // message with correct start
  if (buf[0] == 0) { // discard null leading character
    buf2 = buf + 1;
    len = len - 1;
  } else {
    buf2 = buf;
  }

  if (len < 1) {
    Serial.println("Timeout");
    return 0;
  }


  // parameter
  while (isHexadecimalDigit(buf2[i + 5])) {
    p_buf[i] = buf2[i + 5];
    i++;
  }
  p_len = i;

  // checksum
  chk_buf[0] = buf2[p_len + 8 + 0];
  chk_buf[1] = buf2[p_len + 8 + 1];
  chk_buf[2] = buf2[p_len + 8 + 2];
  chk_buf[3] = buf2[p_len + 8 + 3];
  chk_buf[4] = 0;

  // verify checksum
  byte p_bytes[MAX_LENGTH / 2];
  int p;
  char chk_buf[5];
  int chk_read, chk_verif;
  p = hex2int(p_buf, p_len);
  hex2byte(p_buf, p_bytes, p_len);
  chk_read = hex2int(chk_buf, 4);
  chk_verif = checksum(p_bytes, p_len / 2);

  if (chk_read == chk_verif) {
    for (int i = 0; i < len; i++) {
      data[i] = p_char[i];
    }
    data[len] = 0;
    Serial.print("Value received OK, len=");
    Serial.print(len);
    Serial.print(": ");
    Serial.print(data);
    Serial.println();
    return len;
  } else {
    Serial.print("Checksum error, calculated chksum=");
    Serial.print(chk_verif, HEX);
    Serial.print(": ");
    Serial.write(buf2, len);
    Serial.println();
    return -1;
  }



}

void loop() {

  Serial1.print("MT P=0003 C=FFFC");
  Serial1.write(0x0D);

  delay(2000);

}
