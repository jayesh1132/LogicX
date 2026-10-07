#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

#define MAX_LEN 5000

/* ---------- Reading the WHOLE pasted message ----------
   fgets() reads only ONE line. A pasted message can have many lines,
   so after every line we check: "is more text still waiting?"
   If yes, we keep reading. If no, the message is finished.
   This way you just paste + press Enter. No blank line needed. */
int more_input_waiting() {
#ifdef _WIN32
    Sleep(150);
    return _kbhit();
#else
    fd_set set;
    struct timeval wait;
    FD_ZERO(&set);
    FD_SET(0, &set);
    wait.tv_sec = 1;       /* wait 1 second for the next line */
    wait.tv_usec = 0;
    return select(1, &set, NULL, NULL, &wait) > 0;
#endif
}

/* ---------- Helper functions ---------- */

int is_letter_or_digit(char c) {
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

/* Finds a word only as a WHOLE word.
   So "otp" will NOT match inside "hotpot" and "pin" will NOT match "spin". */
int has_word(const char *text, const char *word) {
    int len = strlen(word);
    const char *p = text;
    char before, after;

    while ((p = strstr(p, word)) != NULL) {
        before = (p == text) ? ' ' : *(p - 1);
        after = *(p + len);
        if (!is_letter_or_digit(before) && !is_letter_or_digit(after)) {
            return 1;
        }
        p++;
    }
    return 0;
}

/* Checks a list of words. Returns the first one found, or NULL.
   whole = 1 -> whole word match, whole = 0 -> match anywhere (good for
   word roots like "verif" which catches verify / verification). */
const char *find_in_list(const char *text, const char *list[], int count, int whole) {
    int i;
    for (i = 0; i < count; i++) {
        if (whole == 1) {
            if (has_word(text, list[i])) return list[i];
        } else {
            if (strstr(text, list[i]) != NULL) return list[i];
        }
    }
    return NULL;
}

int main() {
    static char message[MAX_LEN];
    static char line[MAX_LEN];
    int risk_score = 0;
    int i, length;
    const char *found;
    int link_found = 0;

    /* ---------- Keyword lists ---------- */
    const char *urgency_words[] = {"urgent", "immediately", "tonight", "right now",
        "act now", "last chance", "within 24", "expire", "final notice", "asap", "hurry"};
    const char *threat_words[] = {"blocked", "suspend", "disconnect", "deactivat",
        "terminated", "legal action", "penalty", "freeze", "frozen", "cut off",
        "locked", "will be closed"};
    const char *kyc_roots[] = {"kyc", "verif", "password", "aadhaar", "aadhar",
        "cvv", "bank details", "card number", "account number"};
    const char *kyc_short[] = {"otp", "pin", "pan"};
    const char *reward_words[] = {"congratulation", "winner", "prize", "cashback",
        "lottery", "reward", "gift card", "you have won", "you won"};
    const char *action_words[] = {"click", "claim", "submit", "update", "download",
        "install", "login", "log in", "tap"};
    const char *link_words[] = {"http", "www.", ".com/", ".xyz", ".top"};
    const char *short_links[] = {"bit.ly", "tinyurl", "cutt.ly", "goo.gl",
        "rb.gy", "is.gd", "shorturl"};
    const char *brand_words[] = {"sbi", "hdfc", "icici", "axis", "pnb", "paytm",
        "phonepe", "gpay", "upi", "bank", "electricity", "kyc"};

    printf("=========================================\n");
    printf("     ScamShield - Message Scanner        \n");
    printf("=========================================\n\n");

    printf("Paste the message you received and press Enter:\n> ");

    /* ---------- Read the whole message ---------- */
    message[0] = '\0';
    while (fgets(line, MAX_LEN, stdin) != NULL) {
        if (strlen(message) + strlen(line) < MAX_LEN - 1) {
            strcat(message, line);
        }
        if (!more_input_waiting()) break;
    }

    length = strlen(message);
    if (length == 0) {
        printf("\nNo message entered.\n");
        return 0;
    }

    /* Convert to lowercase (manual ASCII method) */
    for (i = 0; i < length; i++) {
        if (message[i] >= 'A' && message[i] <= 'Z') {
            message[i] = message[i] + 32;
        }
    }

    printf("\n--- Analysis Report ---\n");

    /* 1. Urgency (strong = 20 points, "today" alone = only 5 points) */
    found = find_in_list(message, urgency_words, 11, 0);
    if (found != NULL) {
        printf("[!] Urgency/pressure detected (\"%s\")      +20\n", found);
        risk_score = risk_score + 20;
    } else if (has_word(message, "today")) {
        printf("[i] Mild time word (\"today\")               +5\n");
        risk_score = risk_score + 5;
    }

    /* 2. Threat */
    found = find_in_list(message, threat_words, 12, 0);
    if (found != NULL) {
        printf("[!] Threat/service disruption (\"%s\")      +20\n", found);
        risk_score = risk_score + 20;
    }

    /* 3. Sensitive info / KYC / OTP */
    found = find_in_list(message, kyc_roots, 9, 0);
    if (found == NULL) found = find_in_list(message, kyc_short, 3, 1);
    if (found != NULL) {
        printf("[!] Asks for sensitive info/KYC (\"%s\")    +25\n", found);
        risk_score = risk_score + 25;
    }

    /* 4. Reward bait */
    found = find_in_list(message, reward_words, 9, 0);
    if (found != NULL) {
        printf("[!] Reward/prize bait (\"%s\")              +35\n", found);
        risk_score = risk_score + 35;
    }

    /* 5. Call to action (click, claim, update...) */
    found = find_in_list(message, action_words, 9, 1);
    if (found != NULL) {
        printf("[!] Pushes you to act (\"%s\")              +15\n", found);
        risk_score = risk_score + 15;
    }

    /* 6. Links */
    found = find_in_list(message, short_links, 7, 0);
    if (found != NULL) {
        printf("[!] SHORTENED link detected (\"%s\")        +35\n", found);
        risk_score = risk_score + 35;
        link_found = 1;
    } else {
        found = find_in_list(message, link_words, 5, 0);
        if (found != NULL) {
            printf("[!] Web link detected                    +25\n");
            risk_score = risk_score + 25;
            link_found = 1;
        }
    }

    /* 7. Bank/company name + link = possible fake impersonation */
    if (link_found == 1) {
        found = find_in_list(message, brand_words, 12, 1);
        if (found != NULL) {
            printf("[!] Company/bank name + link (\"%s\")      +10\n", found);
            risk_score = risk_score + 10;
        }
    }

    /* Keep score between 0 and 100 */
    if (risk_score > 100) risk_score = 100;

    printf("\nTotal Risk Score: %d / 100\n", risk_score);

    if (risk_score >= 50) {
        printf("VERDICT: HIGH RISK! This looks like a SCAM. Do NOT click or reply.\n");
    } else if (risk_score >= 25) {
        printf("VERDICT: CAUTION! Suspicious elements found. Verify with the official source.\n");
    } else {
        printf("VERDICT: SAFE. No common scam indicators found.\n");
    }

    return 0;
}
