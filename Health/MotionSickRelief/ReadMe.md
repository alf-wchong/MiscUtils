# MotionSickRelief: 100Hz Acoustic Motion Sickness Relief

**MotionSickRelief** is a lightweight Windows utility designed to alleviate motion sickness using localized sound frequency therapy. By generating a consistent **100Hz pure sine wave**, the application helps stabilize the vestibular system during travel or disorienting activities.

---

## 💡 How to Use
1.  **Connect Headphones:** For the best results, use high-quality earbuds or headphones capable of clear low-frequency reproduction.
2.  **Select Duration:** Click one of the timer buttons (**1min** to **30min**) based on your needs.
    * *Research Suggestion:* 1 minute of exposure before travel can provide relief for up to 2 hours.
3.  **Adjust Volume:** Use the slider to set a comfortable level. It should be audible and clear but not uncomfortably loud.
4.  **Monitor Progress:** A blue circular wedge will visualize the time remaining. 
5.  **Auto-Shutdown:** Once the timer expires, the application will automatically close to save battery and system resources. Use the **STOP** button if you wish to end the tone early without closing the app.

---

## 🔬 The Science
This application is based on research into the **"Sound Spice"** effect. Clinical studies have indicated that a 100Hz tone stimulates the **otolith organs** in the inner ear, which helps the brain reconcile the "sensory mismatch" that causes motion sickness.

* **Reference Paper:** *“Effects of 100 Hz Pure Tone on Motion Sickness and Autonomic Nervous Activity”* (Nagoya University).[^1]
* **Mechanism:** The tone activates the sympathetic nervous system to reduce nausea and dizziness caused by linear acceleration or visual-vestibular conflict.

---

## 🛠 Build Instructions

This program is written in C++ using the Win32 API and is designed to be cross-compiled from **Fedora/Linux** using **MinGW** for a portable Windows executable.

### **Prerequisites**
You must have the MinGW cross-compiler installed on your Fedora system:
```bash
sudo dnf install mingw64-gcc-c++
```

### **Compilation**
Run the following command to build the executable. This command uses **static linking** to ensure the `.exe` runs on any Windows machine without requiring extra DLLs.

```bash
x86_64-w64-mingw32-g++ MotionSickRelief.cpp -o MotionSickRelief.exe \
    -mwindows \
    -municode \
    -static \
    -lwinmm \
    -lcomctl32
```

### **Flag Breakdown**
* `-mwindows`: Runs as a GUI application (no console window).
* `-municode`: Enables Unicode support for `wWinMain`.
* `-static`: Bakes all necessary libraries into the executable for portability.
* `-lwinmm`: Links the Windows Multimedia library (required for `waveOut`).
* `-lcomctl32`: Links the Common Controls library (required for the volume slider).

---

## ⚖️ Disclaimer
*This tool is for informational/experimental purposes and is not a substitute for professional medical advice. Use at a volume level that is safe for your hearing.*

[^1]:https://pmc.ncbi.nlm.nih.gov/articles/PMC11955832/
