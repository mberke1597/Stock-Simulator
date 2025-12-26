# PDF Conversion Guide

## Converting PerformanceReport.md to PDF

You have several options to convert the Markdown report to PDF format.

---

## Option 1: Using Pandoc (Recommended - Best Quality)

### Install Pandoc

**Windows:**
```powershell
# Using Chocolatey
choco install pandoc

# Or download from: https://pandoc.org/installing.html
```

**Linux:**
```bash
sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended
```

**macOS:**
```bash
brew install pandoc
brew install --cask basictex
```

### Convert to PDF

```bash
# Navigate to project directory
cd c:\Users\berke\Desktop\OSProject

# Basic conversion
pandoc PerformanceReport.md -o PerformanceReport.pdf

# With better formatting
pandoc PerformanceReport.md -o PerformanceReport.pdf --pdf-engine=xelatex -V geometry:margin=1in

# Professional formatting with table of contents
pandoc PerformanceReport.md -o PerformanceReport.pdf \
    --pdf-engine=xelatex \
    -V geometry:margin=1in \
    -V fontsize=12pt \
    -V documentclass=article \
    --toc \
    --number-sections
```

**Best Command for This Report:**
```bash
pandoc PerformanceReport.md -o PerformanceReport.pdf --pdf-engine=xelatex -V geometry:margin=1in -V fontsize=11pt --toc --number-sections
```

---

## Option 2: Using VS Code Extensions

### Install Extension

1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "Markdown PDF" by yzane
4. Install the extension

### Convert

1. Open `PerformanceReport.md` in VS Code
2. Right-click in the editor
3. Select "Markdown PDF: Export (pdf)"
4. PDF will be saved in the same directory

---

## Option 3: Using Online Converters

### Recommended Services

1. **Markdown to PDF** - https://www.markdowntopdf.com/
   - Upload `PerformanceReport.md`
   - Click Convert
   - Download PDF

2. **Dillinger** - https://dillinger.io/
   - Paste markdown content
   - Export as → PDF

3. **HackMD** - https://hackmd.io/
   - Import markdown file
   - Export → PDF

---

## Option 4: Using Microsoft Word

1. Open `PerformanceReport.md` in VS Code
2. Copy all content (Ctrl+A, Ctrl+C)
3. Open Microsoft Word
4. Paste content
5. Format as needed
6. Save As → PDF

---

## Option 5: Using Chrome/Edge Browser

### Method 1: Markdown Preview
1. Install "Markdown Viewer" Chrome extension
2. Open `PerformanceReport.md` in browser
3. Press Ctrl+P
4. Select "Save as PDF"

### Method 2: GitHub Preview
1. Push to GitHub
2. View the file on GitHub (it renders markdown)
3. Use browser print function → Save as PDF

---

## Formatting Tips for Better PDF

If you want to enhance the PDF output, add to the top of `PerformanceReport.md`:

```markdown
---
title: "Real-Time Stock Price Simulator - Performance Analysis Report"
author: "Your Name"
date: "December 26, 2025"
geometry: margin=1in
fontsize: 11pt
documentclass: article
header-includes:
  - \usepackage{fancyhdr}
  - \pagestyle{fancy}
  - \fancyhead[L]{OS Project Report}
  - \fancyhead[R]{December 2025}
---
```

---

## Recommended Workflow

### For Academic Submission:

```bash
# 1. Ensure Pandoc is installed
pandoc --version

# 2. Convert with professional formatting
pandoc PerformanceReport.md -o PerformanceReport.pdf \
    --pdf-engine=xelatex \
    -V geometry:margin=1in \
    -V fontsize=11pt \
    -V documentclass=article \
    --toc \
    --number-sections \
    --highlight-style=tango

# 3. Verify PDF was created
ls PerformanceReport.pdf

# 4. Open and review
start PerformanceReport.pdf  # Windows
# open PerformanceReport.pdf  # macOS
# xdg-open PerformanceReport.pdf  # Linux
```

---

## Quality Checks

Before submitting the PDF:

✅ **Check page count**: Should be 8-10 pages (3-5 pages of main content)  
✅ **Verify formatting**: Headings, tables, code blocks render correctly  
✅ **Test hyperlinks**: References and section links work  
✅ **Check margins**: 1 inch on all sides  
✅ **Verify fonts**: Readable size (11-12pt)  
✅ **Table of contents**: If included, verify page numbers  
✅ **Code blocks**: Syntax highlighting preserved  

---

## Troubleshooting

### "pandoc: command not found"
- Ensure Pandoc is installed and in PATH
- Restart terminal after installation

### LaTeX errors
- Install full LaTeX distribution (MiKTeX or TeXLive)
- Or use `--pdf-engine=wkhtmltopdf` instead of xelatex

### Tables not rendering
- Ensure tables have proper markdown syntax
- Use `--wrap=preserve` option

### Large file size
- Reduce image quality if any
- Use compressed PDF tools after generation

---

## Final PDF Location

After conversion, you'll have:
```
c:\Users\berke\Desktop\OSProject\PerformanceReport.pdf
```

This file is ready for submission! 📄✅

---

## Alternative: Pre-formatted Report

If you prefer, I can also create a LaTeX version that compiles directly to PDF with perfect formatting. Let me know if you need this option.
