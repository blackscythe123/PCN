import os
import shutil

def main():
    # Change working directory to script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if script_dir:
        os.chdir(script_dir)

    # 1. Copy instructions.pdf or syllabus.pdf from parent directories as sample.pdf
    pdf_candidates = [
        "../../instructions.pdf",
        "../../cheat_sheet.pdf",
        "../../syllabus.pdf"
    ]
    copied = False
    for candidate in pdf_candidates:
        if os.path.exists(candidate):
            shutil.copy(candidate, "sample.pdf")
            print(f"Copied {candidate} to sample.pdf")
            copied = True
            break
            
    if not copied:
        # Generate a minimal valid PDF if no candidates exist
        minimal_pdf = (
            b"%PDF-1.4\n"
            b"1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n"
            b"2 0 obj<</Type/Pages/Kids[3 0 R]/Count 1>>endobj\n"
            b"3 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 612 792]/Contents 4 0 R>>endobj\n"
            b"4 0 obj<</Length 44>>stream\n"
            b"BT /F1 24 Tf 100 700 Td (Hello World PDF) Tj ET\n"
            b"endstream\n"
            b"endobj\n"
            b"xref\n"
            b"0 5\n"
            b"0000000000 65535 f\n"
            b"0000000009 00000 n\n"
            b"0000000056 00000 n\n"
            b"0000000111 00000 n\n"
            b"0000000202 00000 n\n"
            b"trailer<</Size 5/Root 1 0 R>>\n"
            b"startxref\n"
            b"297\n"
            b"%%EOF\n"
        )
        with open("sample.pdf", "wb") as f:
            f.write(minimal_pdf)
        print("Generated minimal valid PDF as sample.pdf")

    # 2. Generate a 1x1 black JPEG image
    jpeg_bytes = (
        b'\xff\xd8\xff\xe0\x00\x10JFIF\x00\x01\x01\x01\x00H\x00H\x00\x00\xff\xdb\x00C\x00\x08\x06'
        b'\x06\x07\x06\x05\x08\x07\x07\x07\t\t\x08\n\x0c\x14\r\x0c\x0b\x0b\x0c\x19\x12\x13\x0f\x14'
        b'\x1d\x1a\x1f\x1e\x1d\x1a\x1c\x1c $.\' ",#\x1c\x1c(7),01444\x1f\'9=82<.342\xff\xc0\x00\x0b'
        b'\x08\x00\x01\x00\x01\x01\x01\x11\x00\xff\xc4\x00\x1f\x00\x00\x01\x05\x01\x01\x01\x01\x01'
        b'\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\xff\xc4\x00'
        b'\xb5\x10\x00\x02\x01\x03\x03\x02\x04\x03\x05\x05\x04\x04\x00\x00\x01}\x01\x02\x03\x00\x04'
        b'\x11\x05\x12!1A\x06\x13Qa\x07"q\x142\x81\x91\xa1\x08#B\xb1\xc1\x15R\xd1\xf0$3br\x82\x16'
        b'\x17\x18\x19\x1a%&\'()*56789:CDEFGHIJSTUVWXYZcdefghijstuvwxyz\x83\x84\x85\x86\x87\x88'
        b'\x89\x8a\x92\x93\x94\x95\x96\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3'
        b'\xb4\xb5\xb6\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4\xd5\xd6\xd7'
        b'\xd8\xd9\xda\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa'
        b'\xff\xda\x00\x0c\x03\x01\x00\x02\x11\x03\x11\x00?\x00\xed\xfc\xff\xd9'
    )
    with open("image.jpg", "wb") as f:
        f.write(jpeg_bytes)
    print("Generated 1x1 black JPEG as image.jpg")

if __name__ == "__main__":
    main()
