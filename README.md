# ATtiny85 LD2410C mmWave Radar Sensor

Optimalizovaný firmware pro ATtiny85 Digispark s HLK-LD2410C mmWave radarem.

## Verze 1.0 - Optimized with Fade Effect

### Hardware
- **MCU**: ATtiny85 (Digispark)
- **Sensor**: HLK-LD2410C mmWave Radar
- **LED**: P1 (PB1) - PWM capable
- **Sensor Input**: P2 (PB2)

### Zapojení
```
LD2410C OUT → Digispark P2 (PB2)
LD2410C VCC → Digispark 5V
LD2410C GND → Digispark GND
LED         → Digispark P1 (PB1)
```

## Funkce

### LED Indikace
- **Při detekci**: Rychlé blikání (100ms interval)
- **Bez detekce**: Plynulý fade efekt
  - 3 sekundy fade-in (0 → 255)
  - 1 sekunda fade-out (255 → 0)
  - Celkový cyklus: 4 sekundy

### USB Keyboard Output
- **Detekce pohybu**: Odešle znak `'p'`
  - 3x opakování s 1s intervalem
  - Heartbeat každých 5s během detekce
- **Konec detekce**: Odešle znak `'o'`
  - 3x opakování s 1s intervalem

### Debouncing
- **500ms** stabilizační doba pro změnu stavu
- Eliminuje falešné poplachy

## Technické specifikace

### Paměťové nároky
- **Flash**: 3414 bytes (51% z 6650 bytes)
- **RAM**: 96 bytes (18% z 512 bytes)
- **Volná RAM**: 416 bytes pro lokální proměnné

### Optimalizace pro ATtiny85
- ✅ `uint8_t` pro stavy místo enum (úspora RAM)
- ✅ `uint16_t` pro časové intervaly místo `unsigned long`
- ✅ `#define` konstanty v flash místo RAM
- ✅ `static` proměnné pro lepší optimalizaci
- ✅ Efektivní bitové operace (`^=` pro toggle)
- ✅ `DigiKeyboard.write()` místo `print()` (menší overhead)
- ✅ PWM fade efekt pomocí `analogWrite()`

### Timing konstanty
```cpp
DEBOUNCE_TIME      = 500ms   // Stabilizace stavu
SEND_INTERVAL      = 1000ms  // Interval mezi odesláními
HEARTBEAT_INTERVAL = 5000ms  // Heartbeat při detekci
BLINK_FAST         = 100ms   // Rychlé blikání
FADE_IN_TIME       = 3000ms  // Fade-in doba
FADE_OUT_TIME      = 1000ms  // Fade-out doba
```

## Kompilace a nahrání

### Požadavky
- Arduino CLI
- Digistump AVR boards package

### Příkazy
```bash
# Kompilace
arduino-cli compile --fqbn digistump:avr:digispark-tiny attiny_ld2410c_fixed.ino

# Nahrání (připojte Digispark když se zobrazí výzva)
arduino-cli upload --fqbn digistump:avr:digispark-tiny attiny_ld2410c_fixed.ino
```

## State Machine

```
STATE_LOW ──┐
            │ sensor HIGH
            ├──> STATE_DEBOUNCING_HIGH ──┐
            │                             │ 500ms stable
            │                             ├──> STATE_HIGH
            │                             │
            │    sensor LOW               │
            └────────────────────────────┘
                                          │
                                          │ sensor LOW
                                          ├──> STATE_DEBOUNCING_LOW ──┐
                                          │                            │
                                          │                            │ 500ms stable
                                          └────────────────────────────┘
```

## Changelog

### v1.0 (2025-01-27)
- ✨ Implementován PWM fade efekt (3s in, 1s out)
- ⚡ Optimalizace pro ATtiny85 (51% flash, 18% RAM)
- 🔧 Debouncing 500ms
- 📤 USB keyboard output (p/o znaky)
- ❤️ Heartbeat každých 5s při detekci
- 🔄 3x opakování zpráv s 1s intervalem

## Licence
MIT License

## Autor
Optimalizováno pro maximální efektivitu na ATtiny85
