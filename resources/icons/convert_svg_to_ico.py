"""
SVG to ICO Converter for AudioSwitchManager
Converts icon.svg to icon.ico with multiple resolutions
Requires: Pillow and cairosvg
Install: pip install Pillow cairosvg
"""

try:
    from PIL import Image
    import cairosvg
    import io
    import os
except ImportError:
    print("ERROR: Required libraries not installed.")
    print("Please run: pip install Pillow cairosvg")
    exit(1)

def svg_to_ico(svg_path, ico_path, sizes=[16, 32, 48, 64, 128, 256]):
    """Convert SVG to ICO with multiple sizes"""
    print(f"Converting {svg_path} to {ico_path}...")

    images = []

    for size in sizes:
        print(f"  Generating {size}x{size}...")

        # Convert SVG to PNG bytes
        png_data = cairosvg.svg2png(
            url=svg_path,
            output_width=size,
            output_height=size
        )

        # Load PNG into PIL Image
        img = Image.open(io.BytesIO(png_data))
        images.append(img)

    # Save as ICO
    images[0].save(
        ico_path,
        format='ICO',
        sizes=[(img.width, img.height) for img in images],
        append_images=images[1:]
    )

    print(f"✓ Successfully created {ico_path}")
    print(f"  Sizes: {', '.join([f'{s}x{s}' for s in sizes])}")

    # Get file size
    file_size = os.path.getsize(ico_path)
    print(f"  File size: {file_size:,} bytes ({file_size/1024:.1f} KB)")

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    svg_file = os.path.join(script_dir, "icon.svg")
    ico_file = os.path.join(script_dir, "icon.ico")

    if not os.path.exists(svg_file):
        print(f"ERROR: {svg_file} not found!")
        exit(1)

    try:
        svg_to_ico(svg_file, ico_file)
        print("\n✓ Icon conversion completed successfully!")
    except Exception as e:
        print(f"\n✗ ERROR: {e}")
        print("\nAlternative method: Use online converter")
        print("1. Go to https://convertio.co/svg-ico/")
        print(f"2. Upload {svg_file}")
        print("3. Download as icon.ico")
        exit(1)
