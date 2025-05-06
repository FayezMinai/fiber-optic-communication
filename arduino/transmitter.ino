// ─── Transmitter: HFBR‑1414Z + Manchester Encoding ───
const int TX_PIN    = 11;         // Data pin to HFBR‑1414Z transmitter
const unsigned long BIT_RATE = 500; // bits per second (adjust as needed)
unsigned long BIT_PERIOD;           // microseconds per bit

void setup() {
  pinMode(TX_PIN, OUTPUT);
  Serial.begin(115200);
  BIT_PERIOD = 1000000UL / BIT_RATE;
}

void loop() {
  // Read a line from Serial, then send it
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    sendFrame((uint8_t*)message.c_str(), message.length());
  }
}

// Send one Manchester‑encoded bit:
//  • logical ‘1’ → LOW for half‑period, then HIGH
//  • logical ‘0’ → HIGH for half‑period, then LOW
void sendManchesterBit(bool bit) {
  unsigned long half = BIT_PERIOD / 2;
  if (bit) {
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(half);
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(half);
  } else {
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(half);
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(half);
  }
}

// Frame format:
//   [0xAA] ×2   ← preamble (10101010…)
//   [LEN]       ← number of payload bytes
//   [DATA0]…    ← payload bytes
//   Idle LOW for one bit‑period
void sendFrame(uint8_t *data, uint8_t len) {
  const uint8_t PRE = 0xAA;
  // two‑byte preamble
  for (int i = 0; i < 8; i++) sendManchesterBit((PRE >> (7 - i)) & 1);
  for (int i = 0; i < 8; i++) sendManchesterBit((PRE >> (7 - i)) & 1);
  // length byte
  for (int i = 0; i < 8; i++) sendManchesterBit((len >> (7 - i)) & 1);
  // payload
  for (int i = 0; i < len; i++) {
    for (int b = 7; b >= 0; b--) {
      sendManchesterBit((data[i] >> b) & 1);
    }
  }
  // end of frame: line idle LOW
  digitalWrite(TX_PIN, LOW);
  delayMicroseconds(BIT_PERIOD);
}
