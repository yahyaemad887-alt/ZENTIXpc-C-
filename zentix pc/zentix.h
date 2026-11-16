#ifndef ZENTIX_H
#define ZENTIX_H

#include <windows.h>
#include <gtk/gtk.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZENTIX_CONFIG_FILE "zentix_config.txt"

    // 1. مؤشرات اللغات مرتبة تماماً حسب القائمة المنسدلة (9 لغات)
    typedef enum {
        Z_LANG_EN = 0, // English (EN)
        Z_LANG_AR,     // العربية (AR)
        Z_LANG_DE,     // Deutsch (DE)
        Z_LANG_FR,     // Français (FR)
        Z_LANG_JA,     // 日本語 (JA)
        Z_LANG_KO,     // 한국어 (KO)
        Z_LANG_RU,     // Русский (RU)
        Z_LANG_TR,     // Türkçe (TR)
        Z_LANG_ZH,     // 中文 (ZH)
        Z_LANG_COUNT
    } ZentixAppLanguage;

    // 2. هيكل مفاتيح الترجمة الموحد لكل شاشات التطبيق
    typedef struct {
        // شاشة القياسات والإطارات (FPS)
        const char* back_btn;
        const char* title;
        const char* subtitle;
        const char* master_title;
        const char* master_sub;
        const char* fps_title;
        const char* fps_sub;
        const char* avg_title;
        const char* avg_sub;
        const char* min_title;
        const char* min_sub;
        const char* cpu_temp_title;
        const char* cpu_temp_sub;
        const char* gpu_temp_title;
        const char* gpu_temp_sub;
        const char* gpu_usage_title;
        const char* gpu_usage_sub;
        const char* ram_title;
        const char* ram_sub;
        const char* status_connected;
        const char* status_warning;

        // شاشة وضع الألعاب (Game Mode)
        const char* gm_brand;
        const char* gm_search_placeholder;
        const char* gm_exit;
        const char* gm_no_games;
        const char* gm_no_matching;
        const char* gm_play;
        const char* gm_settings;
        const char* gm_change_poster;
        const char* gm_open_folder;
        const char* gm_remove_game;
        const char* gm_dlg_select_exe;
        const char* gm_filter_exe;
        const char* gm_dlg_select_poster;
        const char* gm_filter_images;

        // الشاشة الرئيسية (Home Screen)
        const char* home_app_name;
        const char* home_subtitle;
        const char* home_card_fps_title;
        const char* home_card_fps_desc;
        const char* home_card_sys_title;
        const char* home_card_sys_desc;
        const char* home_game_mode_btn;
        const char* home_toast_tip;

        // شاشة الإعدادات (Settings Screen)
        const char* settings_title;
        const char* settings_subtitle;
        const char* settings_general_title;
        const char* settings_lang_label;
        const char* settings_version_label;

        // شاشة معلومات النظام (System Info Screen)
        const char* sys_header_title;
        const char* sys_header_subtitle;
        const char* sys_card_ram_title;
        const char* sys_card_ram_sub;
        const char* sys_card_cpu_title;
        const char* sys_card_cpu_sub;
        const char* sys_card_gpu_title;
        const char* sys_card_gpu_sub;
        const char* sys_card_disk_title;
        const char* sys_card_disk_sub;
        const char* sys_specs_title;
        const char* sys_row_device_name;
        const char* sys_row_processor;
        const char* sys_row_ram;
        const char* sys_row_gpu;
        const char* sys_row_storage;
        const char* sys_row_sys_type;
        const char* sys_row_os;
        const char* sys_val_sys_type;
        const char* sys_val_os;
    } ZentixTranslationStrings;

    // جدول النصوص المترجمة للغات التسعة
    static const ZentixTranslationStrings g_zentix_translations[Z_LANG_COUNT] = {
        // 0. English (EN)
        {
            "← Back to Dashboard",
            "FPS & Telemetry Suite (RTSS Control)",
            "Configure on-screen display metrics, telemetry overlays, and hardware monitoring parameters.",
            "Master OSD Telemetry Toggle",
            "Enable or disable all performance monitoring feeds instantly",
            "Current Framerate (FPS)",
            "Display live real-time rendering frames per second",
            "Average Framerate (AVG)",
            "Show historical average performance metric",
            "1% Low / Minimum FPS",
            "Track frame dips and stuttering indicators",
            "CPU Temperature Monitoring",
            "Extract core thermal status via Windows telemetry bridge",
            "GPU Temperature Monitoring",
            "Track graphics card thermal performance",
            "GPU Usage Percentage",
            "Display real-time graphics processor load",
            "System RAM Allocation",
            "Monitor available and consumed physical memory",
            "RTSS & Afterburner Telemetry Connected Successfully.",
            "Warning: RTSS not fully detected. Start RivaTuner Statistics Server.",
            // Game Mode English
            "ZENTIX XMB GAME OPTIMIZER",
            "Search Games...",
            "Exit Game Mode",
            "No Games Added Yet",
            "No Matching Games Found",
            "PLAY NOW",
            "Game Options",
            "Change Custom Poster",
            "Open Game Folder",
            "Remove From List",
            "Select Game Executable (.exe)",
            "Executable Files (*.exe)",
            "Select Game Poster Image",
            "Image Files (*.jpg, *.png)",
            // Home Screen English
            "ZENTIX SUITE",
            "Next-Gen Performance Optimizer & Telemetry",
            "FPS & Telemetry Suite",
            "Monitor live framerates, 1% lows, temperatures and hardware loads via RTSS bridge.",
            "System Information & Benchmarks",
            "Inspect full hardware specifications, system architecture, and run performance benchmarks.",
            "GAME MODE",
            "Tip: Press Alt + Enter anytime to toggle fullscreen mode instantly!",
            // Settings Screen English
            "Settings & Preferences",
            "Configure application preferences, language, and system parameters.",
            "General Preferences",
            "Application Language",
            "Software Version",
            // System Info Screen English
            "System Information & Benchmarks",
            "Inspect full hardware specifications and system architecture.",
            "System RAM",
            "Physical Memory",
            "Processor (CPU)",
            "Architecture",
            "Graphics (GPU)",
            "Display Adapter",
            "Storage Drive",
            "Local Fixed Drives",
            "Hardware Specifications Details",
            "Device Name",
            "Processor",
            "Installed RAM",
            "Graphics Card",
            "Total Storage",
            "System Type",
            "Operating System",
            "64-bit Operating System, x64-based processor",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 1. العربية (AR)
        {
            "← رجوع إلى لوحة التحكم",
            "حزمة الإطارات والقياسات (تحكم RTSS)",
            "تكوين مقاييس العرض على الشاشة، طبقات القياس، ومعلمات مراقبة الأجهزة.",
            "مفتاح القياس الرئيسي للـ OSD",
            "تمكين أو تعطيل جميع تدفقات مراقبة الأداء فوراً",
            "معدل الإطارات الحالي (FPS)",
            "عرض إطارات العرض في الوقت الفعلي في الثانية",
            "متوسط معدل الإطارات (AVG)",
            "عرض مقياس الأداء التاريخي المتوسط",
            "أقل إطارات / 1% منخفض",
            "تتبع انخفاض الإطارات ومؤشرات التقطع",
            "مراقبة حرارة المعالج (CPU)",
            "استخراج الحالة الحرارية عبر جسر قياس ويندوز",
            "مراقبة حرارة كارت الشاشة (GPU)",
            "تتبع الأداء الحراري لبطاقة الرسوميات",
            "نسبة استهلاك كارت الشاشة",
            "عرض حمل معالج الرسوميات في الوقت الفعلي",
            "تخصيص ذاكرة الوصول العشوائي (RAM)",
            "مراقبة الذاكرة الفيزيائية المتاحة والمستهلكة",
            "تم اتصال قياسات RTSS و Afterburner بنجاح.",
            "تحذير: لم يتم اكتشاف RTSS بالكامل. قم تشغيل RivaTuner.",
            // Game Mode Arabic
            "محسن ألعاب ZENTIX XMB",
            "البحث عن الألعاب...",
            "خروج من وضع اللعب",
            "لم يتم إضافة ألعاب بعد",
            "لم يتم العثور على ألعاب مطابقة",
            "العب الآن",
            "خيارات اللعبة",
            "تغيير الغلاف المخصص",
            "فتح مجلد اللعبة",
            "إزالة من القائمة",
            "حدد ملف تشغيل اللعبة (.exe)",
            "ملفات التنفيذ (*.exe)",
            "حدد صورة غلاف اللعبة",
            "ملفات الصور (*.jpg, *.png)",
            // Home Screen Arabic
            "منظومة ZENTIX",
            "محسن الأداء والقياسات من الجيل التالي",
            "حزمة الإطارات والقياسات",
            "مراقبة الإطارات الحية، أدنى 1%、 الحرارة وحمل الأجهزة عبر جسر RTSS.",
            "معلومات النظام والقياسات",
            "فحص مواصفات الأجهزة بالكامل، هندسة النظام، وإجراء قياسات الأداء.",
            "وضع الألعاب",
            "تنبيه: اضغط Alt + Enter في أي وقت لتبديل وضع ملء الشاشة فوراً!",
            // Settings Screen Arabic
            "الإعدادات والتفضيلات",
            "تكوين تفضيلات التطبيق، اللغة، ومعلمات النظام.",
            "التفضيلات العامة",
            "لغة التطبيق",
            "إصدار البرنامج",
            // System Info Screen Arabic
            "معلومات النظام والقياسات",
            "فحص مواصفات الأجهزة بالكامل وهندسة النظام.",
            "ذاكرة الوصول العشوائي",
            "الذاكرة الفيزيائية",
            "المعالج (CPU)",
            "هندسة المعمارية",
            "معالج الرسوميات (GPU)",
            "بطاقة العرض",
            "محرك التخزين",
            "الأقراص الثابتة المحلية",
            "تفاصيل مواصفات الأجهزة",
            "اسم الجهاز",
            "المعالج",
            "الرام المثبت",
            "كارت الشاشة",
            "إجمالي التخزين",
            "نوع النظام",
            "نظام التشغيل",
            "نظام تشغيل 64-bit، معالج بمعمارية x64",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 2. Deutsch (DE)
        {
            "← Zurück zum Dashboard",
            "FPS & Telemetrie-Suite (RTSS-Steuerung)",
            "Konfigurieren Sie OSD-Metriken, Telemetrie-Overlays und Hardware-Überwachungsparameter.",
            "Master-OSD-Telemetrie-Umschalter",
            "Alle Leistungsüberwachungsfeeds sofort aktivieren oder deaktivieren",
            "Aktuelle Bildrate (FPS)",
            "Echtzeit-Rendering-Bilder pro Sekunde anzeigen",
            "Durchschnittliche Bildrate (AVG)",
            "Historische Durchschnittsleistungsmetrik anzeigen",
            "1% Low / Minimale FPS",
            "Bildrade-Einbrüche und Ruckelindikatoren verfolgen",
            "CPU-Temperaturüberwachung",
            "Kerntemperaturstatus über Windows-Telemetriebrücke abrufen",
            "GPU-Temperaturüberwachung",
            "Thermische Leistung der Grafikkarte verfolgen",
            "GPU-Auslastung in Prozent",
            "Echtzeit-Grafikprozessorauslastung anzeigen",
            "System-RAM-Zuweisung",
            "Verfügbaren und verbrauchten physischen Speicher überwachen",
            "RTSS- und Afterburner-Telemetrie erfolgreich verbunden.",
            "Warnung: RTSS nicht vollständig erkannt. Starten Sie RivaTuner Statistics Server.",
            // Game Mode German
            "ZENTIX XMB SPIEL-OPTIMIERER",
            "Spiele suchen...",
            "Spielmodus beenden",
            "Noch keine Spiele hinzugefügt",
            "Keine übereinstimmenden Spiele gefunden",
            "JETZT SPIELEN",
            "Spieloptionen",
            "Benutzerdefiniertes Poster ändern",
            "Spielordner öffnen",
            "Aus Liste entfernen",
            "Spiel-Executable auswählen (.exe)",
            "Ausführbare Dateien (*.exe)",
            "Spielposter-Bild auswählen",
            "Bilddateien (*.jpg, *.png)",
            // Home Screen German
            "ZENTIX SUITE",
            "Leistungsoptimierer & Telemetrie der nächsten Generation",
            "FPS & Telemetrie-Suite",
            "Live-Bildraten, 1% Lows, Temperaturen und Hardware-Auslastung über RTSS überwachen.",
            "Systeminformationen & Benchmarks",
            "Hardware-Spezifikationen und Systemarchitektur inspizieren und Benchmarks ausführen.",
            "SPIELMODUS",
            "Tipp: Drücken Sie jederzeit Alt + Enter, um den Vollbildmodus umzuschalten!",
            // Settings Screen German
            "Einstellungen & Präferenzen",
            "Konfigurieren Sie Anwendungseinstellungen, Sprache und Systemparameter.",
            "Allgemeine Einstellungen",
            "Anwendungssprache",
            "Softwareversion",
            // System Info Screen German
            "Systeminformationen & Benchmarks",
            "Hardware-Spezifikationen und Systemarchitektur inspizieren.",
            "System-RAM",
            "Physischer Speicher",
            "Prozessor (CPU)",
            "Architektur",
            "Grafik (GPU)",
            "Anzeigeadapter",
            "Speicherlaufwerk",
            "Lokale feste Laufwerke",
            "Hardware-Spezifikationsdetails",
            "Gerätename",
            "Prozessor",
            "Installierter RAM",
            "Grafikkarte",
            "Gesamtspeicher",
            "Systemtyp",
            "Betriebssystem",
            "64-Bit-Betriebssystem, x64-basierter Prozessor",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 3. Français (FR)
        {
            "← Retour au tableau de bord",
            "Suite FPS & Télémétrie (Contrôle RTSS)",
            "Configurer les métriques OSD, les superpositions et les paramètres de surveillance.",
            "Interrupteur maître de télémétrie OSD",
            "Activer ou désactiver instantanément tous les flux de surveillance",
            "Fréquence d'images actuelle (FPS)",
            "Afficher les images par seconde en temps réel",
            "Fréquence d'images moyenne (AVG)",
            "Afficher la métrique de performance historique moyenne",
            "1% Bas / FPS minimal",
            "Suivre les baisses d'images et les indicateurs de saccade",
            "Surveillance de la température du processeur",
            "Extraire l'état thermique du cœur via le pont de télémétrie Windows",
            "Surveillance de la température du GPU",
            "Suivre les performances thermiques de la carte graphique",
            "Pourcentage d'utilisation du GPU",
            "Afficher la charge du processeur graphique en temps réel",
            "Allocation de la RAM système",
            "Surveiller la mémoire physique disponible et consommée",
            "Télémétrie RTSS et Afterburner connectée avec succès.",
            "Avertissement : RTSS non détecté. Démarrez RivaTuner Statistics Server.",
            // Game Mode French
            "OPTIMISEUR DE JEU ZENTIX XMB",
            "Rechercher des jeux...",
            "Quitter le mode jeu",
            "Aucun jeu ajouté pour le moment",
            "Aucun jeu correspondant trouvé",
            "JOUER MAINTENANT",
            "Options du jeu",
            "Changer l'affiche personnalisée",
            "Ouvrir le dossier du jeu",
            "Retirer de la liste",
            "Sélectionner l'exécutable du jeu (.exe)",
            "Fichiers exécutables (*.exe)",
            "Sélectionner l'image de l'affiche",
            "Fichiers image (*.jpg, *.png)",
            // Home Screen French
            "SUITE ZENTIX",
            "Optimiseur de performance et télémétrie de nouvelle génération",
            "Suite FPS & Télémétrie",
            "Surveillez les images par seconde, les baisses, les températures et la charge via RTSS.",
            "Informations système et benchmarks",
            "Inspectez les spécifications matérielles et exécutez des benchmarks de performance.",
            "MODE JEU",
            "Astuce : Appuyez sur Alt + Entrée pour basculer en plein écran !",
            // Settings Screen French
            "Paramètres et préférences",
            "Configurez les préférences de l'application, la langue et les paramètres système.",
            "Préférences générales",
            "Langue de l'application",
            "Version du logiciel",
            // System Info Screen French
            "Informations système & Benchmarks",
            "Inspectez les spécifications matérielles et l'architecture système.",
            "RAM système",
            "Mémoire physique",
            "Processeur (CPU)",
            "Architecture",
            "Graphiques (GPU)",
            "Carte graphique",
            "Lecteur de stockage",
            "Disques fixes locaux",
            "Détails des spécifications matérielles",
            "Nom de l'appareil",
            "Processeur",
            "RAM installée",
            "Carte graphique",
            "Stockage total",
            "Type de système",
            "Système d'exploitation",
            "Système d'exploitation 64 bits, processeur x64",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 4. 日本語 (JA)
        {
            "← ダッシュボードに戻る",
            "FPS & テレメトリースイート (RTSSコントロール)",
            "オンスクリーンディスプレイメトリクス、オーバーレイ、ハードウェア監視パラメータを設定します。",
            "マスターOSDテレメトリトグル",
            "すべてのパフォーマンス監視フィードを即座に有効または無効にします",
            "現在のフレームレート (FPS)",
            "リアルタイムのレンダリングフレーム数/秒を表示",
            "平均フレームレート (AVG)",
            "履歴平均パフォーマンスメトリクスを表示",
            "1%ロー / 最小FPS",
            "フレームの低下やカクつきの指標を追跡",
            "CPU温度監視",
            "Windowsテレメトリーブリッジ経由でコア熱状態を取得",
            "GPU温度監視",
            "グラフィックスカードの熱性能を追跡",
            "GPU使用率",
            "リアルタイムのグラフィックスプロセッサ負荷を表示",
            "システムRAM割り当て",
            "利用可能および消費された物理メモリを監視",
            "RTSS & Afterburner テレメトリーが正常に接続されました。",
            "警告: RTSSが完全に検出されません。RivaTunerを起動してください。",
            // Game Mode Japanese
            "ZENTIX XMB ゲームオプティマイザー",
            "ゲームを検索...",
            "ゲームモードを終了",
            "ゲームがまだ追加されていません",
            "一致するゲームが見つかりません",
            "今すぐプレイ",
            "ゲームオプション",
            "カスタムポスターを変更",
            "ゲームフォルダを開く",
            "リストから削除",
            "ゲーム実行ファイルを選択 (.exe)",
            "実行ファイル (*.exe)",
            "ゲームポスター画像を選択",
            "画像ファイル (*.jpg, *.png)",
            // Home Screen Japanese
            "ZENTIX スイート",
            "次世代パフォーマンスオプティマイザー ＆ テレメトリー",
            "FPS & テレメトリースイート",
            "RTSSブリッジを介してライブフレームレート、温度、ハードウェア負荷を監視します。",
            "システム情報 & ベンチマーク",
            "ハードウェア仕様を確認し、パフォーマンスベンチマークを実行します。",
            "ゲームモード",
            "ヒント: Alt + Enter を押すといつでも全画面表示に切り替えられます！",
            // Settings Screen Japanese
            "設定と環境設定",
            "アプリケーションの環境設定、言語、システムパラメータを設定します。",
            "一般設定",
            "アプリケーション言語",
            "ソフトウェアバージョン",
            // System Info Screen Japanese
            "システム情報 & ベンチマーク",
            "ハードウェア仕様とシステムアーキテクチャを詳細に検査します。",
            "システムRAM",
            "物理メモリ",
            "プロセッサ (CPU)",
            "アーキテクチャ",
            "グラフィックス (GPU)",
            "ディスプレイアダプター",
            "ストレージドライブ",
            "ローカル固定ドライブ",
            "ハードウェア仕様の詳細",
            "デバイス名",
            "プロセッサ",
            "実装されたRAM",
            "グラフィックスカード",
            "合計ストレージ",
            "システムの種類",
            "オペレーティングシステム",
            "64 ビット オペレーティング システム、x64 ベース プロセッサ",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 5. 한국어 (KO)
        {
            "← 대시보드로 돌아가기",
            "FPS 및 원격 측정 스위트 (RTSS 제어)",
            "온스크린 디스플레이 메트릭, 오버레이 및 하드웨어 모니터링 매개변수를 구성합니다.",
            "마스터 OSD 원격 측정 토글",
            "모든 성능 모니터링 피드를 즉시 활성화 또는 비활성화",
            "현재 프레임 레이트 (FPS)",
            "실시간 렌더링 초당 프레임 표시",
            "평균 프레임 레이트 (AVG)",
            "기록 평균 성능 지표 표시",
            "1% Low / 최소 FPS",
            "프레임 저하 및 버벅임 지표 추적",
            "CPU 온도 모니터링",
            "Windows 원격 측정 브리지를 통해 코어 열 상태 추출",
            "GPU 온도 모니터링",
            "그래픽 카드 열 성능 추적",
            "GPU 사용률",
            "실시간 그래픽 프로세서 부하 표시",
            "시스템 RAM 할당",
            "사용 가능한 물리 메모리 및 소비된 메모리 모니터링",
            "RTSS 및 Afterburner 원격 측정이 성공적으로 연결되었습니다.",
            "경고: RTSS가 감지되지 않았습니다. RivaTuner를 실행하세요.",
            // Game Mode Korean
            "ZENTIX XMB 게임 최적화 도구",
            "게임 검색...",
            "게임 모드 종료",
            "추가된 게임이 없습니다",
            "일치하는 게임을 찾을 수 없습니다",
            "지금 플레이",
            "게임 옵션",
            "사용자 정의 포스터 변경",
            "게임 폴더 열기",
            "목록에서 제거",
            "게임 실행 파일 선택 (.exe)",
            "실행 파일 (*.exe)",
            "게임 포스터 이미지 선택",
            "이미지 파일 (*.jpg, *.png)",
            // Home Screen Korean
            "ZENTIX 스위트",
            "차세대 성능 최적화 도구 및 원격 측정",
            "FPS 및 원격 측정 스위트",
            "RTSS 브리지를 통해 실시간 프레임 레이트, 온도 및 하드웨어 부하를 모니터링합니다.",
            "시스템 정보 및 벤치마크",
            "하드웨어 사양을 검사하고 성능 벤치마크를 실행합니다.",
            "게임 모드",
            "팁: Alt + Enter를 누르면 언제든지 전체 화면 모드로 전환됩니다!",
            // Settings Screen Korean
            "설정 및 기본 설정",
            "애플리케이션 기본 설정, 언어 및 시스템 매개변수를 구성합니다.",
            "일반 설정",
            "응용 프로그램 언어",
            "소프트웨어 버전",
            // System Info Screen Korean
            "시스템 정보 및 벤치마크",
            "하드웨어 사양 및 시스템 아키텍처를 검사합니다.",
            "시스템 RAM",
            "물리 메모리",
            "프로세서 (CPU)",
            "아키텍처",
            "그래픽 (GPU)",
            "디스플레이 어댑터",
            "저장 드라이브",
            "로컬 고정 드라이브",
            "하드웨어 사양 세부 정보",
            "장치 이름",
            "프로세서",
            "설치된 RAM",
            "그래픽 카드",
            "총 저장용량",
            "시스템 종류",
            "운영 체제",
            "64비트 운영 체제, x64 기반 프로세서",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 6. Русский (RU)
        {
            "← Назад к панели",
            "Пакет FPS и телеметрии (Управление RTSS)",
            "Настройка параметров экранного отображения, оверлеев и мониторинга оборудования.",
            "Главный переключатель телеметрии OSD",
            "Мгновенное включение или отключение всех потоков мониторинга",
            "Текущая частота кадров (FPS)",
            "Отображение кадров рендеринга в реальном времени в секунду",
            "Средняя частота кадров (AVG)",
            "Показать историческую среднюю метрику производительности",
            "1% редких / Мин. FPS",
            "Отслеживание просадок кадров и статтеринга",
            "Мониторинг температуры ЦП",
            "Извлечение теплового состояния ядер через мост телеметрии Windows",
            "Мониторинг температуры ГП",
            "Отслеживание тепловой производительности видеокарты",
            "Загрузка графического процессора",
            "Отображение нагрузки на графический процессор в реальном времени",
            "Распределение системной ОЗУ",
            "Мониторинг доступной и используемой физической памяти",
            "Телеметрия RTSS и Afterburner успешно подключена.",
            "Предупреждение: RTSS не обнаружен. Запустите RivaTuner.",
            // Game Mode Russian
            "ОПТИМИЗАТОР ИГР ZENTIX XMB",
            "Поиск игр...",
            "Выйти из игрового режима",
            "Игры еще не добавлены",
            "Подходящие игры не найдены",
            "ИГРАТЬ СЕЙЧАС",
            "Параметры игры",
            "Изменить пользовательский постер",
            "Открыть папку с игрой",
            "Удалить из списка",
            "Выбрать исполняемый файл игры (.exe)",
            "Исполняемые файлы (*.exe)",
            "Выбрать изображение постера",
            "Изображения (*.jpg, *.png)",
            // Home Screen Russian
            "КОМПЛЕКТ ZENTIX",
            "Оптимизатор производительности и телеметрия нового поколения",
            "Пакет FPS и телеметрии",
            "Мониторинг FPS, температур и нагрузки оборудования через мост RTSS.",
            "Системная информация и тесты",
            "Просмотр характеристик оборудования и запуск тестов производительности.",
            "ИГРОВОЙ РЕЖИМ",
            "Совет: Нажмите Alt + Enter, чтобы мгновенно переключить полноэкранный режим!",
            // Settings Screen Russian
            "Настройки и параметры",
            "Настройка параметров приложения, языка и системных параметров.",
            "Общие настройки",
            "Язык приложения",
            "Версия ПО",
            // System Info Screen Russian
            "Системная информация и тесты",
            "Просмотр полных характеристик оборудования и архитектуры системы.",
            "Системная ОЗУ",
            "Физическая память",
            "Процессор (ЦП)",
            "Архитектура",
            "Графика (ГП)",
            "Видеоадаптер",
            "Накопитель",
            "Локальные жесткие диски",
            "Характеристики оборудования",
            "Имя устройства",
            "Процессор",
            "Установленная ОЗУ",
            "Видеокарта",
            "Общий объем памяти",
            "Тип системы",
            "Операционная система",
            "64-разрядная операционная система, процессор x64",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 7. Türkçe (TR)
        {
            "← Panele Geri Dön",
            "FPS ve Telemetri Paketi (RTSS Kontrolü)",
            "Ekran üstü gösterge metriklerini, telemetri katmanlarını و izleme parametrelerini yapılandırın.",
            "Ana OSD Telemetri Anahtarı",
            "Tüm performans izleme akışlarını anında etkinleştirin أو devre dışı bırakın",
            "Mevcut Kare Hızı (FPS)",
            "Saniyedeki canlı gerçek zamanlı işleme karelerini görüntüle",
            "Ortalama Kare Hızı (AVG)",
            "Geçmiş ortalama performans metriğini göster",
            "%1 Düşük / Minimum FPS",
            "Kare düşüşlerini و takılma göstergelerini takip et",
            "CPU Sıcaklık İzleme",
            "Windows telemetri köprüsü üzerinden çekirdek ısıl durumunu çıkar",
            "GPU Sıcaklık İzleme",
            "Ekran kartı ısıl performansını takip et",
            "GPU Kullanım Yüzdesi",
            "Gerçek zamanlı grafik işlemci yükünü görüntüle",
            "Sistem RAM Tahsisi",
            "Kullanılabilir و tüketilen fiziksel belleği izle",
            "RTSS و Afterburner Telemetrisi Başarıyla Bağlandı.",
            "Uyarı: RTSS tamamen algılanmadı. RivaTuner Statistics Server'ı başlatın.",
            // Game Mode Turkish
            "ZENTIX XMB OYUN OPTİMİZATÖRÜ",
            "Oyun Ara...",
            "Oyun Modundan Çık",
            "Henüz Oyun Eklenmedi",
            "Eşleşen Oyun Bulunamadı",
            "HEMEN OYNA",
            "Oyun Seçenekleri",
            "Özel Afişi Değiştir",
            "Oyun Klasörünü Aç",
            "Listeden Kaldır",
            "Oyun Çalıştırılabilir Dosyasını Seç (.exe)",
            "Çalıştırılabilir Dosyalar (*.exe)",
            "Oyun Afiş Resmi Seç",
            "Resim Dosyaları (*.jpg, *.png)",
            // Home Screen Turkish
            "ZENTIX PAKETİ",
            "Yeni Nesil Performans Optimizatörü ve Telemetri",
            "FPS ve Telemetri Paketi",
            "RTSS köprüsü üzerinden canlı kare hızlarını, sıcaklıkları ve donanım yüklerini izleyin.",
            "Sistem Bilgileri ve Testler",
            "Donanım özelliklerini inceleyin ve performans testleri çalıştırın.",
            "OYUN MODU",
            "İpucu: Tam ekran moduna geçmek için istediğiniz zaman Alt + Enter tuşlarına basın!",
            // Settings Screen Turkish
            "Ayarlar ve Tercihler",
            "Uygulama tercihlerini, dili ve sistem parametrelerini yapılandırın.",
            "Genel Tercihler",
            "Uygulama Dili",
            "Yazılım Sürümü",
            // System Info Screen Turkish
            "Sistem Bilgileri ve Testler",
            "Donanım özelliklerini ve sistem mimarisini inceleyin.",
            "Sistem RAM'i",
            "Fiziksel Bellek",
            "İşlemci (CPU)",
            "Mimari",
            "Grafik (GPU)",
            "Görüntü Bağdaştırıcısı",
            "Depolama Sürücüsü",
            "Yerel Sabit Sürücüler",
            "Donanım Özellikleri Detayları",
            "Cihaz Adı",
            "İşlemci",
            "Yüklü RAM",
            "Ekran Kartı",
            "Toplam Depolama",
            "Sistem Türü",
            "İşletim Sistemi",
            "64 bit İşletim Sistemi, x64 tabanlı işlemci",
            "Microsoft Windows 10/11 Pro (x64)"
        },
        // 8. 中文 (ZH)
        {
            "← 返回仪表盘",
            "FPS 与遥测套件 (RTSS 控制)",
            "配置屏幕显示指标、遥测覆盖层和硬件监控参数。",
            "主 OSD 遥测开关",
            "立即启用或禁用所有性能监控源",
            "当前帧率 (FPS)",
            "显示实时每秒渲染帧数",
            "平均帧率 (AVG)",
            "显示历史平均性能指标",
            "1% 低帧率 / 最小 FPS",
            "追踪掉帧和卡顿指标",
            "CPU 温度监控",
            "通过 Windows 遥测桥接提取核心热状态",
            "GPU 温度监控",
            "追踪显卡散热性能",
            "GPU 使用率",
            "显示实时图形处理器负载",
            "系统内存分配",
            "监控可用和已消耗的物理内存",
            "RTSS 和 Afterburner 遥测连接成功。",
            "警告：未完全检测到 RTSS。请启动 RivaTuner Statistics Server。",
            // Game Mode Chinese
            "ZENTIX XMB 游戏优化器",
            "搜索游戏...",
            "退出游戏模式",
            "尚未添加游戏",
            "未找到匹配的游戏",
            "立即开始",
            "游戏选项",
            "更换自定义海报",
            "打开游戏文件夹",
            "从列表中移除",
            "选择游戏主程序 (.exe)",
            "可执行文件 (*.exe)",
            "选择游戏海报图片",
            "图片文件 (*.jpg, *.png)",
            // Home Screen Chinese
            "ZENTIX 套件",
            "下一代性能优化器与遥测",
            "FPS 与遥测套件",
            "通过 RTSS 桥接监控实时帧率、温度和硬件负载。",
            "系统信息与基准测试",
            "检查硬件规格并运行性能基准测试。",
            "游戏模式",
            "提示：随时按 Alt + Enter 键可切换全屏模式！",
            // Settings Screen Chinese
            "设置与偏好",
            "配置应用程序偏好、语言和系统参数。",
            "常规偏好",
            "应用程序语言",
            "软件版本",
            // System Info Screen Chinese
            "系统信息与基准测试",
            "检查完整的硬件规格和系统架构。",
            "系统内存",
            "物理内存",
            "处理器 (CPU)",
            "架构",
            "图形处理器 (GPU)",
            "显示适配器",
            "存储驱动器",
            "本地固定驱动器",
            "硬件规格详情",
            "设备名称",
            "处理器",
            "已安装内存",
            "显卡",
            "总存储空间",
            "系统类型",
            "操作系统",
            "64 位操作系统，基于 x64 的处理器",
            "Microsoft Windows 10/11 Pro (x64)"
        }
    };

    // 3. قراءة وكتابة اللغة بشكل مشترك ومباشر من ملف التكوين لتتزامن كل الشاشات لحظياً
    static inline int zentix_read_lang_from_file(void) {
        FILE* f = fopen(ZENTIX_CONFIG_FILE, "r");
        if (f) {
            int lang = 0;
            if (fscanf(f, "%d", &lang) == 1) {
                fclose(f);
                if (lang >= 0 && lang < Z_LANG_COUNT) return lang;
            }
            fclose(f);
        }
        return 0; // افتراضي إنجليزي
    }

    static inline void ZentixSetLanguage(ZentixAppLanguage lang) {
        FILE* f = fopen(ZENTIX_CONFIG_FILE, "w");
        if (f) {
            fprintf(f, "%d", (int)lang);
            fclose(f);
        }
    }

    static inline ZentixAppLanguage ZentixGetLanguage(void) {
        return (ZentixAppLanguage)zentix_read_lang_from_file();
    }

    static inline const ZentixTranslationStrings* ZentixGetTranslations(void) {
        int current_lang = zentix_read_lang_from_file();
        return &g_zentix_translations[current_lang];
    }

    // 4. هياكل الـ Callbacks لكل الشاشات
    typedef struct {
        void (*onNavigateToSettings)(void);
        void (*onNavigateToMSISuite)(void);
        void (*onNavigateToBenchmark)(void);
        void (*onNavigateToProfiles)(void);
        void (*onNavigateToOverclock)(void);
        void (*onNavigateToGameMode)(void);
    } HomeScreenCallbacks;

    typedef struct {
        void (*onBackToHome)(void);
    } SystemInfoCallbacks;

    typedef struct {
        void (*onNavigateBack)(void);
    } FpsAndOtherScreenCallbacks;

    // 5. إعلانات الدوال العامة لكافة الشاشات والتطبيقات
    __declspec(dllexport) void ZentixSetSelectedProcess(DWORD pid, const char* name);
    __declspec(dllexport) DWORD ZentixGetSelectedProcess(void);
    __declspec(dllexport) void ZentixPollAllTelemetry(void* data);
    __declspec(dllexport) void ZentixUpdateRTSSOSD(void);
    __declspec(dllexport) int ZentixRTSSGetActiveApplications(void* out_apps, int max_slots, int* out_count);
    __declspec(dllexport) int ZentixCheckRequiredSoftware(void);

    // شاشات الواجهة (GTK Widgets)
    __declspec(dllexport) GtkWidget* HomeScreen(HomeScreenCallbacks callbacks);
    __declspec(dllexport) GtkWidget* SystemInfoScreen(SystemInfoCallbacks callbacks);
    __declspec(dllexport) GtkWidget* KernelEngineScreen(void (*onNavigateBack)(void));
    __declspec(dllexport) GtkWidget* SettingsScreen(void (*onNavigateBack)(void));
    __declspec(dllexport) GtkWidget* FpsAndOtherScreen(void (*on_back)(void));
    __declspec(dllexport) GtkWidget* GameModeScreen(void (*onNavigateBack)(void));

    // متغيرات التوافق المؤقتة لمنع أخطاء LNK2001 للشاشات القديمة
    __declspec(selectany) int g_home_current_lang = 0;
    __declspec(selectany) int g_sys_current_lang = 0;
    __declspec(selectany) int g_setting_current_lang = 0;

#ifdef __cplusplus
}
#endif

#endif // ZENTIX_H