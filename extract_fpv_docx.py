import zipfile, re
path = r"C:\Users\Jay Patel\OneDrive\Desktop\Projects ( Web )\be1JD Web\FPV Car Build Guide.docx"
with zipfile.ZipFile(path) as z:
    xml = z.read('word/document.xml').decode('utf-8')
texts = re.findall(r'<w:t[^>]*>(.*?)</w:t>', xml)
print('\n'.join(texts))
