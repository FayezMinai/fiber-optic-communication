// ─── Receiver: HFBR‑2414Z + Manchester Decoding ───
const int RX_PIN    = 2;           // Data pin from HFBR‑2414Z receiver
const unsigned long BIT_RATE = 500; // must match transmitter
unsigned long BIT_PERIOD;
uint8_t lastState;
uint8_t buffer[64];

void setup() {
  pinMode(RX_PIN, INPUT);
  Serial.begin(115200);
  BIT_PERIOD = 1000000UL / BIT_RATE;
  lastState  = digitalRead(RX_PIN);
}

void loop() {
  int len = receiveFrame();
  if (len > 0) {
    Serial.print("Received: ");
    for (int i = 0; i < len; i++) {
      Serial.write(buffer[i]);
    }
    Serial.println();
  }
}

// Waits for the next edge, then samples at halfway through the bit‑period
// to recover the Manchester‑encoded bit
bool receiveManchesterBit() {
  // wait for any transition
  while (digitalRead(RX_PIN) == lastState);
  unsigned long t0 = micros();
  lastState = digitalRead(RX_PIN);
  // wait until mid‑bit
  while (micros() - t0 < BIT_PERIOD / 2);
  bool mid = digitalRead(RX_PIN);
  // wait out the rest of the bit
  while (micros() - t0 < BIT_PERIOD);
  return mid;
}

// Reads 8 Manchester bits and assembles a byte
uint8_t receiveManchesterByte() {
  uint8_t v = 0;
  for (int i = 0; i < 8; i++) {
    v = (v << 1) | (receiveManchesterBit() ? 1 : 0);
  }
  return v;
}

// Frame parsing identical to transmitter framing
int receiveFrame() {
  const uint8_t PRE = 0xAA;
  // synchronize on two preamble bytes
  uint8_t p;
  do { p = receiveManchesterByte(); } while (p != PRE);
  if (receiveManchesterByte() != PRE) return -1;
  // length
  uint8_t len = receiveManchesterByte();
  if (len > sizeof(buffer)) return -1;
  // payload
  for (int i = 0; i < len; i++) {
    buffer[i] = receiveManchesterByte();
  }
  return len;
}
