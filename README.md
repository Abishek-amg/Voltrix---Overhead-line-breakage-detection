# VOLTRIX – Overhead Line Breakage Detection

**Cost-effective IoT system using ESP32, LoRa, and voltage sensors to detect overhead power line breakages, auto-isolate faults, and send alerts – saving lives in rural India.**

## Problem Statement
Developing a cost-effective solution for detecting the breakage of Low Voltage AC Distribution Overhead conductors.

- **Theme:** Social, Environmental  
- **Category:** Hardware  
- **Team Name:** Volt Warriors  
- **College:** J.J. College of Engineering and Technology, Tiruchirappalli

## Overview
VOLTRIX is a distributed monitoring system that:
- Places paired Transmitter & Receiver units every 1–1.5 km along overhead lines.
- Compares voltage values wirelessly via LoRa (long-range, low-power).
- Detects sudden drops → Triggers relay to isolate fault instantly.
- Sends real-time alerts via web dashboard (Firebase) and SMS (GSM).
- Works offline with local buffering – perfect for rural areas.

## Key Features
- **Proactive Safety:** Automatic isolation prevents electrocutions and fires.
- **Offline Resilience:** LoRa communication (no internet needed for detection).
- **Zero Infrastructure Change:** Easy retrofit on existing poles.
- **Rural-Optimized:** 10+ km LoRa range, low power (solar-compatible).
- **Ultra-Affordable:** ~₹4,000–6,000 per pair.

## Hardware
- **Transmitter:** ESP32 + LoRa (SX1278) + Voltage Sensor (ZMPT101B)
- **Receiver:** ESP32 + LoRa + Voltage Sensor + Relay + GSM (SIM800L)

## Code
- `voltrix_transmitter.ino` → Measures downstream voltage and sends via LoRa.
- `voltrix_receiver.ino` → Compares voltages, triggers relay on fault, pushes data to Firebase, sends SMS alerts.

## Setup & Usage
1. Flash `voltrix_transmitter.ino` to downstream ESP32.
2. Flash `voltrix_receiver.ino` to upstream ESP32.
3. Configure WiFi/Firebase credentials and GSM in receiver code.
4. Deploy units on poles and monitor via Firebase dashboard.

## Impact
- Prevents fallen live wire accidents.
- Reduces manual patrolling and outages.
- Aligns with RDSS and Digital India grid modernization goals.

## References
- RDSS Scheme: https://powermin.gov.in/en/content/overview-5
- CEA Electrical Accidents: https://cea.nic.in/cei-electrical-accident-statistics/?lang=en
- LoRa Fault Detection Research: https://www.researchgate.net/publication/361859723

**License:** MIT – Free to use and modify.

**Team Volt Warriors** ⚡
