"""
Create a simple ICO file without external dependencies
Creates a basic ASM icon with text
"""

import struct

def create_simple_ico():
    # Create a simple 32x32 icon with blue background and white text
    size = 32
    pixels = []

    # Blue background color
    bg_color = (37, 99, 235, 255)  # BGRA format
    text_color = (255, 255, 255, 255)

    # Simple pixel pattern for "ASM" text (very basic)
    text_pattern = [
        "     AAA   SSSS  M   M     ",
        "    A   A S     MM MM     ",
        "   AAAAA   SSS  M M M     ",
        "  A     A     S M   M     ",
        " A       A SSSS M   M     ",
    ]

    # Create 32x32 pixel array
    for y in range(size):
        for x in range(size):
            # Check if we're in text area (centered)
            text_y = (y - 8) // 3
            text_x = (x - 2)

            if 0 <= text_y < len(text_pattern) and 0 <= text_x < len(text_pattern[text_y]):
                if text_pattern[text_y][text_x] != ' ':
                    pixels.append(text_color)
                else:
                    pixels.append(bg_color)
            else:
                pixels.append(bg_color)

    # Build BMP data (32-bit RGBA)
    bmp_data = bytearray()

    # BMP header
    bmp_header_size = 40
    bmp_data_size = size * size * 4

    # BITMAPINFOHEADER
    bmp_data.extend(struct.pack('<I', bmp_header_size))  # Header size
    bmp_data.extend(struct.pack('<i', size))  # Width
    bmp_data.extend(struct.pack('<i', size * 2))  # Height (double for XOR and AND masks)
    bmp_data.extend(struct.pack('<H', 1))  # Planes
    bmp_data.extend(struct.pack('<H', 32))  # Bits per pixel
    bmp_data.extend(struct.pack('<I', 0))  # Compression
    bmp_data.extend(struct.pack('<I', bmp_data_size))  # Image size
    bmp_data.extend(struct.pack('<i', 0))  # X pixels per meter
    bmp_data.extend(struct.pack('<i', 0))  # Y pixels per meter
    bmp_data.extend(struct.pack('<I', 0))  # Colors used
    bmp_data.extend(struct.pack('<I', 0))  # Important colors

    # XOR mask (actual image data) - bottom to top
    for y in range(size - 1, -1, -1):
        for x in range(size):
            pixel = pixels[y * size + x]
            bmp_data.extend(bytes([pixel[0], pixel[1], pixel[2], pixel[3]]))

    # AND mask (transparency) - all opaque
    and_mask_size = ((size + 31) // 32) * 4
    for y in range(size):
        bmp_data.extend(bytes([0] * and_mask_size))

    # ICO file structure
    ico_data = bytearray()

    # ICONDIR header
    ico_data.extend(struct.pack('<H', 0))  # Reserved (must be 0)
    ico_data.extend(struct.pack('<H', 1))  # Type (1 = ICO)
    ico_data.extend(struct.pack('<H', 1))  # Number of images

    # ICONDIRENTRY
    ico_data.extend(struct.pack('<B', size))  # Width
    ico_data.extend(struct.pack('<B', size))  # Height
    ico_data.extend(struct.pack('<B', 0))  # Color palette
    ico_data.extend(struct.pack('<B', 0))  # Reserved
    ico_data.extend(struct.pack('<H', 1))  # Color planes
    ico_data.extend(struct.pack('<H', 32))  # Bits per pixel
    ico_data.extend(struct.pack('<I', len(bmp_data)))  # Image data size
    ico_data.extend(struct.pack('<I', 22))  # Image data offset (6 + 16)

    # Append BMP data
    ico_data.extend(bmp_data)

    return bytes(ico_data)

if __name__ == "__main__":
    import os

    script_dir = os.path.dirname(os.path.abspath(__file__))
    ico_path = os.path.join(script_dir, "icon.ico")

    print("Creating basic ASM icon...")
    ico_data = create_simple_ico()

    with open(ico_path, 'wb') as f:
        f.write(ico_data)

    file_size = len(ico_data)
    print(f"[OK] Created {ico_path}")
    print(f"  Size: {file_size:,} bytes ({file_size/1024:.1f} KB)")
    print("\nNote: This is a basic icon. For better quality:")
    print("1. Go to https://convertio.co/svg-ico/")
    print("2. Upload icon.svg")
    print("3. Replace icon.ico with the downloaded file")
