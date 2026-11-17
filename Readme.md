# 🚀 ZENTIX PC (XMB Game Optimizer & Telemetry Suite)

> An open-source, ultra-lightweight Windows PC gaming utility and hardware telemetry suite built from scratch in pure C and GTK 4, featuring an immersive PlayStation 3 XMB-inspired interface, real-time hardware monitoring, and multi-language support.

---

## 🌟 What Makes Zentix PC Unique?
Unlike traditional, resource-heavy monitoring software, **Zentix PC** bridges the gap between raw hardware telemetry and a cinematic console-like user experience:
* **PlayStation 3 XMB Interface:** A fully custom horizontal carousel layout (`HomeScreen`) that mimics the iconic PS3 aesthetic, complete with original system sounds (`ps3_click.mp3`, `ps3_startup.mp3`) and game banner management.
* **All-in-One Powerhouse:** Combines real-time FPS/hardware tracking, a dedicated game performance optimizer (`GameMod`), an in-depth system info dashboard (`SystemInfoScreen`), and an onboarding wizard (`SetupWizard`) into a single lightweight binary.
* **Multi-Language Architecture:** Native, strict-indexed support for **9 different languages**, built with proper UTF-8 with BOM encoding for flawless international text rendering.

---

## 📸 Complete Showcase Gallery 

<p align="center">
  <img src="x64/screen/Screenshot (3).png" width="48%" alt="Zentix PC Setup - Step 1">
  <img src="x64/screen/Screenshot (4).png" width="48%" alt="Zentix PC Setup - Step 2">
</p>
<p align="center">
  <img src="x64/screen/Screenshot (5).png" width="48%" alt="Zentix PC Setup - Step 3">
  <img src="x64/screen/Screenshot (6).png" width="48%" alt="Zentix PC Setup - Step 4">
</p>
<p align="center">
  <img src="x64/screen/Screenshot (7).png" width="48%" alt="Zentix PC Telemetry Panel - 1">
  <img src="x64/screen/Screenshot (8).png" width="48%" alt="Zentix PC Telemetry Panel - 2">
</p>
<p align="center">
  <img src="x64/screen/Screenshot (9).png" width="48%" alt="Zentix PC Code Architecture">
  <img src="x64/screen/Screenshot (10).png" width="48%" alt="Zentix XMB Carousel - Mafia II">
</p>
<p align="center">
  <img src="x64/screen/Screenshot (11).png" width="48%" alt="Zentix XMB Carousel - Blur">
  <img src="x64/screen/Screenshot (12).png" width="48%" alt="Zentix PC Final Build View">
</p>

---

## 🛠️ Tech Stack & Low-Level Architecture
* **Core Language:** Pure C (modularly structured into UI, Logic, Telemetry, and Storage layers).
* **UI Framework:** GTK 4 with custom CSS styling and GStreamer media pipeline integration.
* **Hardware Telemetry APIs:**
  * **WinRing0:** Low-level kernel driver interface for direct hardware sensor access.
  * **NVIDIA NVAPI:** GPU core clock, voltage, and usage telemetry.
  * **AMD ADL SDK:** Radeon performance metrics integration.
  * **Intel IGCL:** Integrated graphics monitoring.
  * **RTSS & MSI Afterburner:** Real-time OSD synchronization.
* **Deployment:** Custom NSIS (Modern UI 2) installer script with integrated custom `nsDialogs` for dependency checks and registry configuration.

---

## 🔍 Problems Solved During Development (المشاكل التي تم حلها)

Throughout the development of Zentix PC, several complex engineering challenges were conquered:
1. **GStreamer DLL Initialization Conflicts:** Resolved runtime path dependencies and missing DLL issues on standard Windows environments by bundling local asset pipelines and fixing relative linking.
2. **Memory Leaks in Continuous Telemetry Threads:** Fixed polling overhead and resource leaks in multi-threaded hardware loops by implementing strict thread synchronization and efficient memory allocation.
3. **NSIS UTF-8 Text Encoding & RTL Alignment:** Overcame standard installer character corruption issues (mojibake) by enforcing strict UTF-8 with BOM encoding and True Unicode directives for multi-language string tables.
4. **RTSS Integration Fallbacks:** Handled missing RivaTuner Statistics Server processes gracefully to prevent application crashes during startup telemetry handshakes.
5. **Repository History Optimization:** Cleaned and structured commit history via strategic interactive rebasing and force-pushing to present a clean, professional timeline of v1.0.0.

---

## 📦 Download & Installation

You can download the official lightweight installer package:
* **[Download Zentix_PC_Installer.zip (3 MB)](./Zentix_PC_Installer.zip)** *(Compressed binary package)*

Run the installer on Windows 10/11, follow the setup wizard steps, and enjoy an optimized gaming experience!

---

## 📬 Contact & Support (تواصل معي)

If you have any questions, feedback, or collaboration proposals, feel free to reach out:
* **📧 Email:** [yahyaemad887@gmail.com](mailto:yahyaemad887@gmail.com)
* **💬 WhatsApp:** [+201553427179](https://wa.me/201553427179)