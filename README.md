# ScamShield 🛡️

> **"Don’t just detect the scam. Help the user understand it."**

ScamShield is an offline, lightweight, privacy-first heuristic risk engine written in C to analyze incoming SMS and text messages for phishing, urgency coercion, and fraudulent indicators. Instead of returning an opaque binary verdict, ScamShield calculates a composite risk score (0–100) and displays an itemized breakdown of the warning signals detected.

---

## 📌 Table of Contents
- [Overview](#-overview)
- [Key Features](#-key-features)
- [Architecture & Workflow](#-architecture--workflow)
- [Risk Scoring Model](#-risk-scoring-model)
- [Directory Layout](#-directory-layout)
- [Getting Started](#-getting-started)
  - [Prerequisites](#prerequisites)
  - [Compilation](#compilation)
  - [Running ScamShield](#running-scamshield)
- [Benchmark Examples](#-benchmark-examples)
- [Privacy & Security](#-privacy--security)
- [Roadmap](#-roadmap)
- [Team Logic X](#-team-logic-x)
- [License](#-license)

---

## 🔍 Overview

Social engineering and phishing attacks rely on emotional manipulation—fabricating urgency, severe threats, or artificial windfalls—to trick individuals into taking immediate action before verifying the sender. Non-technical users and senior citizens are disproportionately impacted by deceptive service disconnections or fake compliance warnings.

ScamShield functions as a local, immediate triage layer:
1. **Paste** — Accepts the complete raw message text via the terminal.
2. **Scan** — Inspects lexical tokens against curated pattern categories.
3. **Score** — Calculates an aggregate threat weight up to 100 points.
4. **Explain** — Prints each triggered keyword, domain pattern, or urgency indicator.
5. **Decide** — Categorizes the message into `SAFE`, `CAUTION`, or `HIGH RISK`.

---

## ✨ Key Features

- **100% Offline & Private:** Zero network requests, remote telemetry, or cloud dependencies. Message analysis executes purely within local system memory.
- **Explainable Diagnostics:** Provides an itemized audit log of triggers so users learn to identify social engineering patterns.
- **Context-Weighted Heuristics:** Calibrates risk weights to differentiate between casual terms (such as an isolated "today" or a bill split via UPI) and coordinated extortion patterns.
- **Cross-Platform Multi-Line Buffer:** Utilizes non-blocking platform stream listeners (`select` on POSIX systems, `_kbhit` on Windows) to ingest multi-line clipboard pastes cleanly without requiring special termination delimiters.
- **Self-Contained C Implementation:** Built with ANSI C standard libraries without external package dependencies, ensuring near-instant compilation on any system.

---

## ⚙️ Architecture & Workflow

```text
               +--------------------------------------+
               |      User Pastes Raw Message         |
               +--------------------------------------+
                                  |
                                  v
               +--------------------------------------+
               |   Non-Blocking Stream Buffer Reader  |
               +--------------------------------------+
                                  |
                                  v
               +--------------------------------------+
               |  Case Normalization & Tokenization   |
               +--------------------------------------+
                                  |
            +---------------------+---------------------+
            |                     |                     |
            v                     v                     v
     [Urgency & Coercion]   [Threat Verbs]     [Credential Harvesting]
            |                     |                     |
            +---------------------+---------------------+
            |                     |                     |
            v                     v                     v
     [Reward / Lottery]     [Action Hooks]      [Shortened/Raw URLs]
            |                     |                     |
            +---------------------+---------------------+
                                  |
                                  v
               +--------------------------------------+
               |    Additive Weight Calculator        |
               |      (Capped at 100 Points)          |
               +--------------------------------------+
                                  |
                                  v
               +--------------------------------------+
               |  Explainable Audit Log & Risk Tier   |
               +--------------------------------------+
```

---

## 📊 Risk Scoring Model

The risk engine accumulates weighted points based on identified indicators up to a maximum cap of 100:

| Category | Triggers & Match Type | Score Contribution |
| :--- | :--- | :---: |
| **High Urgency** | `urgent`, `immediately`, `tonight`, `right now`, `act now`, `last chance`, `within 24`, `expire`, `final notice`, `asap`, `hurry` (Substring) | **+20** |
| **Mild Urgency** | `today` (Isolated whole word) | **+5** |
| **Threats & Coercion** | `blocked`, `suspend`, `disconnect`, `deactivat`, `terminated`, `legal action`, `penalty`, `freeze`, `frozen`, `cut off`, `locked`, `will be closed` (Substring) | **+20** |
| **Sensitive Data Solicitation** | `kyc`, `verif`, `password`, `aadhaar`, `aadhar`, `cvv`, `bank details`, `card number`, `account number` (Root matches), `otp`, `pin`, `pan` (Whole words) | **+25** |
| **Reward Bait** | `congratulation`, `winner`, `prize`, `cashback`, `lottery`, `reward`, `gift card`, `you have won`, `you won` (Substring) | **+35** |
| **Call to Action** | `click`, `claim`, `submit`, `update`, `download`, `install`, `login`, `log in`, `tap` (Whole words) | **+15** |
| **Shortened Link** | `bit.ly`, `tinyurl`, `cutt.ly`, `goo.gl`, `rb.gy`, `is.gd`, `shorturl` (Substring) | **+35** |
| **Standard Hyperlink** | `http`, `www.`, `.com/`, `.xyz`, `.top` (Substring) | **+25** |
| **Brand Impersonation Combo** | Any brand/entity (`sbi`, `hdfc`, `icici`, `axis`, `pnb`, `paytm`, `phonepe`, `gpay`, `upi`, `bank`, `electricity`, `kyc`) co-occurring with a link | **+10** |

### Risk Tiers

- **0 – 24 (SAFE):** Minimal risk detected. Expected everyday communication.
- **25 – 49 (CAUTION):** Ambiguous or suspicious patterns present. Verification via official external channels advised.
- **50 – 100 (HIGH RISK):** High probability of malicious intent, credential phishing, or social engineering extortion.

---

## 📁 Directory Layout

```text
scamshield/
├── scamshield.c          # Core detection engine and cross-platform CLI implementation
├── README.md             # Project documentation and specifications
└── test_cases.txt        # Benchmark inputs for evaluation and regression checks
```

---

## 🚀 Getting Started

### Prerequisites

- A C99-compliant compiler (`gcc`, `clang`, or MSVC / MinGW).

### Compilation

```bash
# Clone the repository
git clone [https://github.com/your-username/scamshield.git](https://github.com/your-username/scamshield.git)
cd scamshield

# Compile for Linux / macOS
gcc -O2 scamshield.c -o scamshield

# Compile for Windows (MinGW / CMD)
gcc -O2 scamshield.c -o scamshield.exe
```

### Running ScamShield

```bash
./scamshield
```

Paste your message into the prompt and press **Enter**. The application evaluates the input buffer and outputs the detailed risk analysis report immediately.

---

## 🧪 Benchmark Examples

### Case 1: Utility Disconnection Extortion

**Input:**
```text
Your electricity connection will be disconnected tonight at 9:30 PM.
Immediately update your bill verification:
[http://bit.ly/power-pay](http://bit.ly/power-pay)
```

**Console Output:**
```text
=========================================
     ScamShield - Message Scanner        
=========================================

Paste the message you received and press Enter:
> 
--- Analysis Report ---
[!] Urgency/pressure detected ("tonight")      +20
[!] Threat/service disruption ("disconnect")  +20
[!] Asks for sensitive info/KYC ("verif")     +25
[!] Pushes you to act ("update")              +15
[!] SHORTENED link detected ("bit.ly")        +35
[!] Company/bank name + link ("electricity")  +10

Total Risk Score: 100 / 100
VERDICT: HIGH RISK! This looks like a SCAM. Do NOT click or reply.
```

---

### Case 2: Benign Peer-to-Peer Financial Transfer

**Input:**
```text
Bro, I paid ₹1,200 for the movie tickets. Your half is ₹600.
You can send your share to my UPI ID when you get time. We decided to split it fifty-fifty.
```

**Console Output:**
```text
=========================================
     ScamShield - Message Scanner        
=========================================

Paste the message you received and press Enter:
> 
--- Analysis Report ---

Total Risk Score: 0 / 100
VERDICT: SAFE. No common scam indicators found.
```

---

### Case 3: Academic Notice with Benign Deadline

**Input:**
```text
Please submit your engineering drawing sheet to the 2nd period lab today.
Verify that you have your notebook.
```

**Console Output:**
```text
=========================================
     ScamShield - Message Scanner        
=========================================

Paste the message you received and press Enter:
> 
--- Analysis Report ---
[i] Mild time word ("today")               +5
[!] Asks for sensitive info/KYC ("verif")  +25
[!] Pushes you to act ("submit")           +15

Total Risk Score: 45 / 100
VERDICT: CAUTION! Suspicious elements found. Verify with the official source.
```

---

## 🔒 Privacy & Security

- **Strict Local Execution:** Operates entirely within the user's host environment without requiring socket connections or network interfaces.
- **Volatile Storage Lifecycle:** Analyzed text resides only in volatile memory during scanning and is discarded upon program exit.
- **Air-Gapped Compatibility:** Functions identically in isolated or air-gapped test environments.

---

## 🗺️ Roadmap

- [x] **v1.0 (Current):** C-based deterministic heuristic engine, explainable multi-signal risk reporting, and non-blocking multi-line input handling.
- [ ] **v2.0 (Planned):**
  - Edge NLP model integration for contextual understanding beyond hardcoded keyword stems.
  - Multilingual support for Hindi and Hinglish phonetics and transliterations.
  - Native mobile OS integration via platform "Share Sheet" handlers for single-tap inbox triage.
  - Offline domain entropy and lexical analysis to flag typosquatted lookalike URLs.

---

## 👥 Team Logic X

Developed as a prototype for the **Cyber Security × AI Ideathon**:

- **Jayesh Pandey** — Core Development, CLI Input Handling & Scoring Engine Architecture
- **MD Jishan** — Threat & Pattern Research, Benchmark Datasets & NLP Strategy
- **Jitesh** — Vulnerability Analysis, Privacy Architecture & Evaluation Protocols

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
