# AudioSwitchManager Icons

## Опис іконки

Іконка ASM поєднує в собі:
- **Синій круглий фон** - сучасний мінімалістичний дизайн
- **Динамік/Speaker** - символ аудіо
- **Звукові хвилі** - ліворуч та праворуч від динаміка
- **Стрілки перемикання** - внизу, символізують швидке перемикання між пристроями
- **Літери "ASM"** - назва проєкту вгорі

## Конвертація SVG в ICO

### Варіант 1: Онлайн конвертер
1. Відкрийте https://convertio.co/svg-ico/
2. Завантажте `icon.svg`
3. Виберіть розміри: 16x16, 32x32, 48x48, 64x64, 128x128, 256x256
4. Завантажте готовий `icon.ico`

### Варіант 2: ImageMagick (command line)
```bash
magick convert -background none icon.svg -define icon:auto-resize=256,128,64,48,32,16 icon.ico
```

### Варіант 3: Inkscape
```bash
inkscape icon.svg --export-type=png --export-filename=icon-256.png -w 256 -h 256
inkscape icon.svg --export-type=png --export-filename=icon-128.png -w 128 -h 128
inkscape icon.svg --export-type=png --export-filename=icon-64.png -w 64 -h 64
inkscape icon.svg --export-type=png --export-filename=icon-48.png -w 48 -h 48
inkscape icon.svg --export-type=png --export-filename=icon-32.png -w 32 -h 32
inkscape icon.svg --export-type=png --export-filename=icon-16.png -w 16 -h 16

# Потім об'єднайте в .ico
magick convert icon-*.png icon.ico
```

## Використання в проєкті

### У CMakeLists.txt
Додайте ресурсний файл для Windows:

```cmake
# Додати до sources
set(RESOURCES
    resources/app.rc
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
```

### Створіть app.rc файл
```rc
IDI_ICON1 ICON "icons/icon.ico"
```

### У TrayManager.cpp
Замість `LoadIcon(nullptr, IDI_APPLICATION)` використовуйте:
```cpp
m_nid.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
```

## Кольорова схема

- **Основний синій**: `#2563eb` (RGB: 37, 99, 235)
- **Світло-синій**: `#60a5fa` (RGB: 96, 165, 250)
- **Темно-синій**: `#1e40af` (RGB: 30, 64, 175)
- **Білий**: `#ffffff` (RGB: 255, 255, 255)

## Альтернативні концепції

Якщо потрібен інший стиль, можна створити:
1. **Мінімалістичний** - тільки літери ASM на градієнтному фоні
2. **Технічний** - еквалайзер з перемикачем
3. **Flat Design** - плоскі геометричні форми
4. **Skeuomorphic** - реалістичний динамік 3D

Дайте знати, якщо потрібен інший варіант!
