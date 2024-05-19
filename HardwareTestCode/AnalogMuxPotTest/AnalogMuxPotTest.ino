#define MUX_POT  32
#define MUX_S0   33
#define MUX_S1   27
#define MUX_S2   14
#define MUX_S3   12

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode (MUX_S0, OUTPUT);
  pinMode (MUX_S1, OUTPUT);
  pinMode (MUX_S2, OUTPUT);
  pinMode (MUX_S3, OUTPUT);
}

int muxAnalogRead (int pot) {
  if (pot & 1) { digitalWrite(MUX_S0, HIGH);
      } else { digitalWrite(MUX_S0, LOW);
    }
  if (pot & 2) { digitalWrite(MUX_S1, HIGH);
      } else { digitalWrite(MUX_S1, LOW);
    }
  if (pot & 4) { digitalWrite(MUX_S2, HIGH);
      } else { digitalWrite(MUX_S2, LOW);
    }
  if (pot & 8) { digitalWrite(MUX_S3, HIGH);
      } else { digitalWrite(MUX_S3, LOW);
    }
  return analogRead(MUX_POT);
}

void loop() {
  Serial.print("V1: ");
  Serial.print(muxAnalogRead(5));
  Serial.print("\tA1: ");
  Serial.print(muxAnalogRead(6));
  Serial.print("\tV2: ");
  Serial.print(muxAnalogRead(7));
  Serial.print("\tLR: ");
  Serial.print(muxAnalogRead(1));
  Serial.print("\tLD: ");
  Serial.print(muxAnalogRead(2));
  Serial.print("\t1A: ");
  Serial.print(muxAnalogRead(3));
  Serial.print("\t1D: ");
  Serial.print(muxAnalogRead(4));
  Serial.print("\t1S: ");
  Serial.print(muxAnalogRead(9));
  Serial.print("\t1R: ");
  Serial.print(muxAnalogRead(8));
  Serial.print("\t2A: ");
  Serial.print(muxAnalogRead(15));
  Serial.print("\t2D: ");
  Serial.print(muxAnalogRead(14));
  Serial.print("\t2S: ");
  Serial.print(muxAnalogRead(13));
  Serial.print("\t2R: ");
  Serial.print(muxAnalogRead(12));
  Serial.print("\tS1: ");
  Serial.print(muxAnalogRead(0));
  Serial.print("\tS2: ");
  Serial.print(muxAnalogRead(10));
  Serial.print("\tS3: ");
  Serial.print(muxAnalogRead(11));

  Serial.print("\n");
  delay(100);
}
